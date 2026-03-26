# mmkv

`mmkv` 是一个基于 Node-API 的 Tencent MMKV 原生绑定，面向 Node.js / Electron 的本地高性能 KV 存储场景。

当前版本重点支持：

- macOS Apple Silicon: `arm64`
- macOS Intel: `x64`
- Windows: `x64` / `arm64`
- Node.js 20+
- 包管理器：`pnpm`

## 特性

- 基于 Node-API，接口对 Node / Electron 更稳
- 支持 `bool`、`int32`、`uint32`、`int64/BigInt`、`uint64/BigInt`
- 支持 `float`、`double`
- 支持 `string`、`Buffer`
- 支持 `reKey` 加密切换
- 支持 `defaultMMKV`
- 支持单库和全量 `backup/restore`
- 提供基础的锁、同步、清理能力

## 安装与构建

当前仓库使用 `pnpm`，不要使用 `npm`。

```bash
pnpm install
pnpm build
pnpm test
```

如果你是 Apple Silicon Mac，建议至少验证两套环境：

```bash
arch -arm64 zsh
pnpm build
pnpm test
```

```bash
arch -x86_64 zsh
pnpm build
pnpm test
```

说明：

- `arm64` 构建用于 M 系列 Mac 原生运行
- `x64` 构建用于 Intel Mac，或 Apple Silicon 上的 Rosetta / Intel Electron

## 快速开始

```js
const { MMKV } = require("mmkv");

MMKV.initialize("./.mmkv");

const kv = new MMKV("app");
kv.setBool("ready", true);
kv.setInt32("count", 42);
kv.setUInt64("big", 18446744073709551615n);
kv.setFloat("ratio", 1.25);
kv.setDouble("price", 19.99);
kv.setString("name", "demo");
kv.setBuffer("blob", Buffer.from([1, 2, 3]));

console.log(kv.getBool("ready"));
console.log(kv.getInt32("count"));
console.log(kv.getUInt64("big"));
console.log(kv.getFloat("ratio"));
console.log(kv.getDouble("price"));
console.log(kv.getString("name"));
console.log(kv.getBuffer("blob"));

kv.close();
MMKV.onExit();
```

## API 示例

### 创建实例

```js
const { MMKV } = require("mmkv");

MMKV.initialize("/tmp/mmkv-root");

const kv = new MMKV("user-cache", {
  rootPath: "/tmp/mmkv-root",
  multiProcess: false,
  readOnly: false,
  expectedCapacity: 1024 * 1024,
});
```

### 加密存储与 reKey

```js
const secure = new MMKV("secure", {
  rootPath: "/tmp/mmkv-root",
  cryptKey: "my-secret",
  aes256: true,
});

secure.setString("token", "abc123");

secure.reKey("new-secret", true);
secure.reKey(null);
secure.close();
```

### defaultMMKV

```js
const { MMKV } = require("mmkv");

MMKV.initialize("/tmp/mmkv-root");

const kv = MMKV.defaultMMKV({
  rootPath: "/tmp/mmkv-root",
});

kv.setString("theme", "dark");
console.log(kv.getString("theme"));
kv.close();
```

### Buffer

```js
const payload = Buffer.from([0xde, 0xad, 0xbe, 0xef]);

kv.setBuffer("payload", payload);
const restored = kv.getBuffer("payload");
```

### 备份与恢复

```js
const { MMKV } = require("mmkv");

MMKV.initialize("/tmp/mmkv-root");

const kv = new MMKV("session");
kv.setString("token", "hello");
kv.sync();
kv.close();

MMKV.backupOneToDirectory("session", "/tmp/mmkv-backup", "/tmp/mmkv-root");
MMKV.restoreOneFromDirectory("session", "/tmp/mmkv-backup", "/tmp/mmkv-restore");
```

全量备份：

```js
MMKV.backupAllToDirectory("/tmp/mmkv-backup-all", "/tmp/mmkv-root");
MMKV.restoreAllFromDirectory("/tmp/mmkv-backup-all", "/tmp/mmkv-restore-all");
```

## TypeScript

类型声明已包含在根目录 [index.d.ts](/Users/mac/html/node-mmkv/index.d.ts)。

支持的主要接口：

- `setBool/getBool`
- `setInt32/getInt32`
- `setUInt32/getUInt32`
- `setInt64/getInt64`
- `setUInt64/getUInt64`
- `setFloat/getFloat`
- `setDouble/getDouble`
- `setString/getString`
- `setBuffer/getBuffer`
- `containsKey/removeValueForKey/removeValuesForKeys`
- `allKeys/count/totalSize/actualSize`
- `clearAll/trim/sync`
- `lock/unlock/tryLock`
- `reKey/cryptKey`
- `defaultMMKV`
- `backupOneToDirectory/restoreOneFromDirectory`
- `backupAllToDirectory/restoreAllFromDirectory`

## Electron 接入建议

- 主进程优先使用这个原生模块
- 如果要在 Intel Electron 上运行，请使用 `darwin-x64` 构建产物
- 如果要在 Apple Silicon Electron 上运行，请使用 `darwin-arm64` 构建产物
- 不要把 `arm64` 的 `.node` 文件直接塞给 `x64` Electron
- 不同应用请使用不同的 `rootPath` 和实例 `id`

关于“其他项目也用了 MMKV，会不会覆盖我的数据”：

- 只要 `rootPath` 不同，就不会互相覆盖
- 即使 `rootPath` 相同，只要 `id` 不同，也会是不同的存储文件
- 真正会冲突的是：同一个 `rootPath` 下使用同一个 `id`

## GitHub Actions 自动打包

仓库已经提供 GitHub Actions 工作流：

- [build-binaries.yml](/Users/mac/html/node-mmkv/.github/workflows/build-binaries.yml)

行为如下：

- `push` / `pull_request` 会自动构建并测试
- `workflow_dispatch` 可以手动触发打包
- `release published` 会上传二进制压缩包到 GitHub Release
- 输出产物分为：
  - `darwin-arm64`
  - `darwin-x64`
  - `windows-arm64`
  - `windows-x64`
- 打包前会对 `.node` 做符号裁剪，并使用更高压缩级别生成归档

## 开发说明

更多本地开发约束见：

- [DEVELOPMENT.md](/Users/mac/html/node-mmkv/DEVELOPMENT.md)

## 当前限制

- 目前还没有预编译下载脚本和 npm 发布流程
- 暂未补齐全部 MMKV 上游能力，优先覆盖 Node / Electron 常用能力
- 原生二进制已做体积优化，但不会做激进混淆或 UPX 之类高风险处理，以避免 Electron / macOS 加载兼容性问题
