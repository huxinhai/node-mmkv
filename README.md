# mmkv

**Language:** English | [简体中文](README.zh-CN.md)

`mmkv` is a Node-API native binding for [Tencent MMKV](https://github.com/Tencent/MMKV). It provides high-performance, persistent key-value storage for Node.js and Electron applications by wrapping the upstream MMKV core that is vendored in this repository under `MMKV/`.

This project is focused on practical local storage APIs for desktop runtime use: typed values, binary buffers, encryption keys, store maintenance, and backup/restore.

## Status

- Package name: `mmkv`
- Runtime: Node.js 20+
- Package manager: pnpm 10.12.4
- Native layer: Node-API through `node-addon-api`
- Native output: `build/Release/mmkv.node`
- Current CI targets: `darwin-arm64`, `darwin-x64`, `windows-x64`
- TypeScript declarations: [`index.d.ts`](index.d.ts)
- Package visibility: public, with no `private` flag
- npm publishing: automated publishing is not configured yet

## Features

- Boolean values
- `int32`, `uint32`, `int64`, and `uint64`
- `float` and `double`
- `string`
- Node.js `Buffer`
- Multiple MMKV stores by id
- `defaultMMKV`
- Per-store options: `rootPath`, `multiProcess`, `readOnly`, `expectedCapacity`, `cryptKey`, and `aes256`
- Encryption key rotation and removal with `reKey`
- Key lookup, listing, deletion, counts, and size inspection
- `clearAll`, `trim`, `sync`, `lock`, `unlock`, `tryLock`, and `close`
- Single-store and all-store backup/restore

The binding does not expose every upstream MMKV capability yet. It prioritizes the API surface currently used and tested for Node.js and Electron.

## Requirements

- Node.js 20 or newer
- pnpm 10 or newer
- Python 3
- A C++20-capable native build toolchain
- `git` with submodule support

On macOS, install Xcode Command Line Tools. On Windows, use a Visual Studio/MSVC build environment that works with `node-gyp`.

## Build

Initialize the MMKV submodule if needed, then install, build, and test:

```bash
git submodule update --init --recursive
pnpm install
pnpm build
pnpm test
```

`pnpm test` runs Node's test runner with `--expose-gc`, because the smoke suite includes memory regression checks.

On Apple Silicon, keep the terminal architecture and Node.js architecture aligned with the binary you want to build:

```bash
arch -arm64 zsh
node -p "process.arch"
pnpm install
pnpm build
pnpm test
```

To verify the Intel/Rosetta build on Apple Silicon:

```bash
arch -x86_64 zsh
node -p "process.arch"
pnpm install
pnpm build
pnpm test
```

Use the `darwin-arm64` build for native Apple Silicon runtimes and the `darwin-x64` build for Intel Mac or x64 Electron under Rosetta.

## Quick Start

```js
const { MMKV } = require("mmkv");

MMKV.initialize("./.mmkv");

const kv = new MMKV("app");

kv.setBool("ready", true);
kv.setInt32("count", 42);
kv.setUInt64("max", 18446744073709551615n);
kv.setFloat("ratio", 1.25);
kv.setDouble("price", 19.99);
kv.setString("name", "demo");
kv.setBuffer("payload", Buffer.from([1, 2, 3]));

console.log(kv.getBool("ready"));
console.log(kv.getInt32("count"));
console.log(kv.getUInt64("max"));
console.log(kv.getFloat("ratio"));
console.log(kv.getDouble("price"));
console.log(kv.getString("name"));
console.log(kv.getBuffer("payload"));

kv.close();
MMKV.onExit();
```

Call `MMKV.initialize(rootDir)` before opening stores that rely on the global MMKV root directory. You can also pass `rootPath` per store.

## Opening Stores

```js
const { MMKV } = require("mmkv");

MMKV.initialize("/tmp/mmkv-root", 4);

const kv = new MMKV("user-cache", {
  rootPath: "/tmp/mmkv-root",
  multiProcess: false,
  readOnly: false,
  expectedCapacity: 1024 * 1024,
});

const defaultStore = MMKV.defaultMMKV({
  rootPath: "/tmp/mmkv-root",
});
```

The second argument to `MMKV.initialize` is the numeric upstream MMKV log level. Passing `4` disables MMKV logs.

## Values

```js
kv.setBool("flag", true);
kv.getBool("flag", false);

kv.setInt32("signed", -1);
kv.getInt32("signed", 0);

kv.setUInt32("unsigned", 4294967295);
kv.getUInt32("unsigned", 0);

kv.setInt64("largeSigned", -9007199254740991n);
kv.getInt64("largeSigned", 0n);

kv.setUInt64("largeUnsigned", 18446744073709551615n);
kv.getUInt64("largeUnsigned", 0n);

kv.setFloat("floatValue", 1.25);
kv.getFloat("floatValue", 0);

kv.setDouble("doubleValue", Math.PI);
kv.getDouble("doubleValue", 0);

kv.setString("message", "hello");
kv.getString("message");

kv.setBuffer("bytes", Buffer.from([0xde, 0xad, 0xbe, 0xef]));
kv.getBuffer("bytes");
```

Use `bigint` for 64-bit integer values outside JavaScript's safe integer range. `getInt64` and `getUInt64` always return `bigint`.

`getString` and `getBuffer` return `null` when the value is not found.

## Keys and Maintenance

```js
kv.containsKey("message");
kv.allKeys();
kv.count();

kv.removeValueForKey("message");
kv.removeValuesForKeys(["flag", "bytes"]);

kv.totalSize();
kv.actualSize();

kv.sync();
kv.sync(true);
kv.trim();
kv.clearAll();
kv.clearAll(true);

if (kv.tryLock()) {
  try {
    // protected work
  } finally {
    kv.unlock();
  }
}

kv.close();
```

Once `close()` has been called, instance methods throw because the native MMKV handle is closed.

## Encryption

```js
const secure = new MMKV("secure", {
  rootPath: "/tmp/mmkv-root",
  cryptKey: "initial-secret",
  aes256: true,
});

secure.setString("token", "abc123");
console.log(secure.cryptKey());

secure.reKey("rotated-secret", true);
secure.reKey(null);
secure.close();
```

`cryptKey()` reports the key known to this JavaScript wrapper instance. It does not read or reveal a key from MMKV storage.

## Backup and Restore

```js
const { MMKV } = require("mmkv");

MMKV.initialize("/tmp/mmkv-root");

const kv = new MMKV("session");
kv.setString("token", "hello");
kv.sync();
kv.close();

MMKV.backupOneToDirectory("session", "/tmp/mmkv-backup", "/tmp/mmkv-root");
MMKV.restoreOneFromDirectory("session", "/tmp/mmkv-backup", "/tmp/mmkv-restore");

MMKV.backupAllToDirectory("/tmp/mmkv-backup-all", "/tmp/mmkv-root");
MMKV.restoreAllFromDirectory("/tmp/mmkv-backup-all", "/tmp/mmkv-restore-all");
```

`backupOneToDirectory` and `restoreOneFromDirectory` return booleans. `backupAllToDirectory` and `restoreAllFromDirectory` return the number of stores processed.

## TypeScript

Type declarations are included in [`index.d.ts`](index.d.ts). The main runtime exports are:

```ts
import { MMKV, version } from "mmkv";
```

The public API includes:

- Static: `initialize`, `onExit`, `version`, `defaultMMKV`, `backupOneToDirectory`, `restoreOneFromDirectory`, `backupAllToDirectory`, `restoreAllFromDirectory`
- Values: `setBool/getBool`, `setInt32/getInt32`, `setUInt32/getUInt32`, `setInt64/getInt64`, `setUInt64/getUInt64`, `setFloat/getFloat`, `setDouble/getDouble`, `setString/getString`, `setBuffer/getBuffer`
- Keys: `containsKey`, `removeValueForKey`, `removeValuesForKeys`, `allKeys`, `count`
- Maintenance: `totalSize`, `actualSize`, `clearAll`, `trim`, `sync`, `lock`, `unlock`, `tryLock`, `reKey`, `cryptKey`, `close`

## Electron Notes

- Prefer loading the native module in the main process.
- Match the `.node` binary architecture to the Electron runtime architecture.
- Use `darwin-arm64` for native Apple Silicon Electron.
- Use `darwin-x64` for Intel Mac or x64 Electron under Rosetta.
- Use `windows-x64` for 64-bit Windows Electron.
- Do not load an arm64 `.node` binary into an x64 Electron process, or the reverse.
- Use a dedicated `rootPath` per application.

MMKV data is isolated by `rootPath` and store id. Two applications will not overwrite each other if they use different roots. Under the same root, different ids map to different stores. A conflict happens when two runtimes use the same `rootPath` and the same id for incompatible data.

## CI Binary Bundles

The repository includes [`build-binaries.yml`](.github/workflows/build-binaries.yml). It runs on:

- Pushes to `main`
- Pull requests
- Manual `workflow_dispatch`
- Published GitHub Releases

The workflow builds and tests each target, strips the native binary when possible, then packages:

- `darwin-arm64`
- `darwin-x64`
- `windows-x64`

Release builds upload `dist/mmkv-v<version>-<target>.tar.gz` archives as release assets.

## Project Layout

```text
src/
  core/
    node_mmkv_binding.h
    node_mmkv_core.cpp
  helpers/
    config_parser.*
    value_utils.*
  bindings/
    module_init.cpp
    static/
      admin.cpp
      backup.cpp
    instance/
      numbers.cpp
      strings.cpp
      buffers.cpp
      keys.cpp
      maintenance.cpp
MMKV/
  Upstream Tencent MMKV source tree
binding.gyp
index.d.ts
test/smoke.test.ts
```

See [`DEVELOPMENT.md`](DEVELOPMENT.md) for local development notes, especially macOS architecture handling.

## Tests

The smoke test suite in [`test/smoke.test.ts`](test/smoke.test.ts) covers:

- Basic typed reads and writes
- `defaultMMKV`
- Encrypted storage and `reKey`
- Single-store backup/restore
- All-store backup/restore
- Locking and maintenance methods
- Repeated open/write/clear/close cycles with memory snapshots

Run:

```bash
pnpm test
```

## Current Limits

- No automated npm publish flow yet.
- No install-time prebuilt binary downloader yet.
- Linux is not part of the package `os` list or CI target matrix.
- The binding covers common Node.js and Electron storage needs, but not the full upstream MMKV API.
- Native binaries are stripped for size, but the project avoids high-risk compression or obfuscation steps such as UPX to preserve Electron and macOS loading compatibility.
