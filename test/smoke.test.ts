import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import { createRequire } from "node:module";

const require = createRequire(import.meta.url);
const { MMKV, version } = require("..") as typeof import("../index");

const rootDir = fs.mkdtempSync(path.join(os.tmpdir(), "mmkv-"));
MMKV.initialize(rootDir, 4);

function toMB(bytes: number) {
  return bytes / 1024 / 1024;
}

function collectMemorySnapshot() {
  global.gc?.();
  const usage = process.memoryUsage();
  return {
    rssMB: toMB(usage.rss),
    heapUsedMB: toMB(usage.heapUsed),
    externalMB: toMB(usage.external),
    arrayBuffersMB: toMB(usage.arrayBuffers),
  };
}

test.after(() => {
  MMKV.onExit();
  fs.rmSync(rootDir, { recursive: true, force: true });
});

test("MMKV addon can initialize and read/write basic values", () => {
  assert.equal(typeof version, "string");
  assert.equal(typeof MMKV.version(), "string");

  const kv = new MMKV("smoke");
  assert.equal(kv.setBool("feature", true), true);
  assert.equal(kv.getBool("feature"), true);

  assert.equal(kv.setInt32("count", 42), true);
  assert.equal(kv.getInt32("count"), 42);

  assert.equal(kv.setUInt32("u32", 4294967295), true);
  assert.equal(kv.getUInt32("u32"), 4294967295);

  assert.equal(kv.setUInt64("u64", 18446744073709551615n), true);
  assert.equal(kv.getUInt64("u64"), 18446744073709551615n);

  assert.equal(kv.setInt64("big", 9007199254740991n), true);
  assert.equal(kv.getInt64("big"), 9007199254740991n);

  assert.equal(kv.setFloat("ratio", 1.25), true);
  assert.equal(kv.getFloat("ratio"), 1.25);

  assert.equal(kv.setDouble("pi", 3.1415926), true);
  assert.equal(kv.getDouble("pi"), 3.1415926);

  assert.equal(kv.setString("greeting", "hello"), true);
  assert.equal(kv.getString("greeting"), "hello");

  const binary = Buffer.from([0, 1, 2, 3, 255]);
  assert.equal(kv.setBuffer("blob", binary), true);
  assert.deepEqual(kv.getBuffer("blob"), binary);

  assert.equal(kv.containsKey("greeting"), true);
  assert.deepEqual(
    new Set(kv.allKeys()),
    new Set(["feature", "count", "u32", "u64", "big", "ratio", "pi", "greeting", "blob"])
  );
  assert.equal(kv.count(), 9);
  assert.ok(kv.totalSize() >= kv.actualSize());

  assert.equal(kv.removeValueForKey("count"), true);
  assert.equal(kv.containsKey("count"), false);
  assert.equal(kv.removeValuesForKeys(["u32", "u64", "blob"]), true);
  assert.equal(kv.containsKey("u32"), false);
  assert.equal(kv.containsKey("u64"), false);
  assert.equal(kv.containsKey("blob"), false);

  assert.equal(typeof kv.tryLock(), "boolean");
  kv.lock();
  kv.unlock();
  kv.sync();
  kv.sync(true);

  kv.trim();
  kv.clearAll();
  assert.equal(kv.count(), 0);

  kv.close();
});

test("MMKV addon can open encrypted storage with cryptKey and reopen it", () => {
  const secret = "short";
  let encrypted = new MMKV("secure", { rootPath: rootDir, cryptKey: secret });
  assert.equal(encrypted.setString("token", "secret-value"), true);
  assert.equal(encrypted.cryptKey(), secret);
  encrypted.close();

  encrypted = new MMKV("secure", { rootPath: rootDir, cryptKey: secret });
  assert.equal(encrypted.getString("token"), "secret-value");
  encrypted.close();
});

test("MMKV addon can reKey the current instance", () => {
  const secret = "short";
  const kv = new MMKV("rekey-live");
  assert.equal(kv.setString("token", "secret-value"), true);
  assert.equal(kv.reKey(secret), true);
  assert.equal(kv.cryptKey(), secret);
  assert.equal(kv.getString("token"), "secret-value");
  assert.equal(kv.reKey(null), true);
  assert.equal(kv.cryptKey(), null);
  assert.equal(kv.getString("token"), "secret-value");
  kv.close();
});

test("MMKV addon can open defaultMMKV and persist values", () => {
  const kv = MMKV.defaultMMKV({ rootPath: rootDir });
  assert.equal(kv.setString("default-key", "default-value"), true);
  assert.equal(kv.getString("default-key"), "default-value");
  kv.close();

  const reopened = MMKV.defaultMMKV({ rootPath: rootDir });
  assert.equal(reopened.getString("default-key"), "default-value");
  reopened.close();
});

test("MMKV addon can backup and restore one store", () => {
  const backupDir = fs.mkdtempSync(path.join(os.tmpdir(), "mmkv-backup-one-"));
  const restoreDir = fs.mkdtempSync(path.join(os.tmpdir(), "mmkv-restore-one-"));

  try {
    const kv = new MMKV("backup-one", { rootPath: rootDir });
    assert.equal(kv.setString("message", "hello-backup"), true);
    kv.sync();
    kv.close();

    assert.equal(MMKV.backupOneToDirectory("backup-one", backupDir, rootDir), true);
    assert.equal(MMKV.restoreOneFromDirectory("backup-one", backupDir, restoreDir), true);

    const restored = new MMKV("backup-one", { rootPath: restoreDir });
    assert.equal(restored.getString("message"), "hello-backup");
    restored.close();
  } finally {
    fs.rmSync(backupDir, { recursive: true, force: true });
    fs.rmSync(restoreDir, { recursive: true, force: true });
  }
});

test("MMKV addon can backup and restore all stores", () => {
  const backupDir = fs.mkdtempSync(path.join(os.tmpdir(), "mmkv-backup-all-"));
  const restoreDir = fs.mkdtempSync(path.join(os.tmpdir(), "mmkv-restore-all-"));

  try {
    const kvA = new MMKV("backup-all-a", { rootPath: rootDir });
    const kvB = new MMKV("backup-all-b", { rootPath: rootDir });
    assert.equal(kvA.setString("name", "alpha"), true);
    assert.equal(kvB.setInt32("count", 7), true);
    kvA.sync();
    kvB.sync();
    kvA.close();
    kvB.close();

    assert.ok(MMKV.backupAllToDirectory(backupDir, rootDir) >= 2);
    assert.ok(MMKV.restoreAllFromDirectory(backupDir, restoreDir) >= 2);

    const restoredA = new MMKV("backup-all-a", { rootPath: restoreDir });
    const restoredB = new MMKV("backup-all-b", { rootPath: restoreDir });
    assert.equal(restoredA.getString("name"), "alpha");
    assert.equal(restoredB.getInt32("count"), 7);
    restoredA.close();
    restoredB.close();
  } finally {
    fs.rmSync(backupDir, { recursive: true, force: true });
    fs.rmSync(restoreDir, { recursive: true, force: true });
  }
});

test("MMKV addon does not show runaway memory growth under repeated open/write/close cycles", {
  skip: typeof global.gc !== "function",
  timeout: 30000,
}, () => {
  const bufferPayload = Buffer.alloc(512 * 1024, 7);
  const stringPayload = "x".repeat(16 * 1024);
  const snapshots = [collectMemorySnapshot()];

  for (let round = 0; round < 8; round += 1) {
    for (let i = 0; i < 12; i += 1) {
      const kv = new MMKV(`memory-${round}-${i}`, { rootPath: rootDir });
      for (let j = 0; j < 8; j += 1) {
        assert.equal(kv.setBuffer(`buffer-${j}`, bufferPayload), true);
        assert.equal(kv.setString(`text-${j}`, stringPayload), true);
        assert.equal(kv.getBuffer(`buffer-${j}`)?.length, bufferPayload.length);
        assert.equal(kv.getString(`text-${j}`), stringPayload);
      }
      kv.clearAll();
      kv.close();
    }
    snapshots.push(collectMemorySnapshot());
  }

  const finalSnapshot = collectMemorySnapshot();

  const tailSnapshots = snapshots.slice(-3);
  const tailRssValues = tailSnapshots.map((entry) => entry.rssMB);
  const tailRssDrift = Math.max(...tailRssValues) - Math.min(...tailRssValues);
  const heapGrowth = finalSnapshot.heapUsedMB - snapshots[0].heapUsedMB;
  const externalGrowth = finalSnapshot.externalMB - snapshots[0].externalMB;
  const arrayBufferGrowth = finalSnapshot.arrayBuffersMB - snapshots[0].arrayBuffersMB;

  assert.ok(
    tailRssDrift < 64,
    `RSS kept drifting instead of stabilizing: ${tailRssDrift.toFixed(2)} MB`
  );
  assert.ok(
    heapGrowth < 16,
    `Heap usage grew too much after GC: ${heapGrowth.toFixed(2)} MB`
  );
  assert.ok(
    externalGrowth < 32,
    `External memory grew too much after GC: ${externalGrowth.toFixed(2)} MB`
  );
  assert.ok(
    arrayBufferGrowth < 8,
    `ArrayBuffer memory grew too much after GC: ${arrayBufferGrowth.toFixed(2)} MB`
  );
});
