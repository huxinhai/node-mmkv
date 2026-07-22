# mmkv

**语言：** [English](https://github.com/huxinhai/node-mmkv/blob/main/README.md) | 简体中文

`@huxinhai/mmkv` 是一个面向 [Tencent MMKV](https://github.com/Tencent/MMKV) 的 Node-API 原生绑定。它把仓库内 `MMKV/` 目录中的上游 MMKV 核心封装成 Node.js / Electron 可用的高性能持久化 KV 存储模块。

这个项目优先覆盖桌面运行时最常用的本地存储能力：类型化值、二进制 Buffer、加密密钥、存储维护以及备份恢复。

## 当前状态

- 包名：`@huxinhai/mmkv`
- 运行时：Node.js 20+
- 包管理器：pnpm 10.12.4
- 原生层：基于 `node-addon-api` 的 Node-API
- 原生产物：`build/Release/mmkv.node`
- 当前 CI 目标：`darwin-arm64`、`darwin-x64`、`linux-x64`、`windows-x64`
- TypeScript 声明：[`index.d.ts`](https://github.com/huxinhai/node-mmkv/blob/main/index.d.ts)
- 包可见性：public，不设置 `private` 标记
- npm 发布：通过 GitHub Release 和 npm Trusted Publishing 自动发布

## 功能特性

- 布尔值
- `int32`、`uint32`、`int64`、`uint64`
- `float` 和 `double`
- `string`
- Node.js `Buffer`
- 按 id 创建多个 MMKV store
- `defaultMMKV`
- 单 store 配置：`rootPath`、`multiProcess`、`readOnly`、`expectedCapacity`、`cryptKey`、`aes256`
- 通过 `reKey` 轮换或移除加密密钥
- key 查询、列举、删除、数量统计和尺寸查询
- `clearAll`、`trim`、`sync`、`lock`、`unlock`、`tryLock`、`close`
- 单 store 和全量 store 的备份恢复

当前绑定还没有暴露上游 MMKV 的全部能力，优先覆盖 Node.js / Electron 已经实现并测试过的常用接口。

## 环境要求

- Node.js 20 或更新版本
- pnpm 10 或更新版本
- Python 3
- 支持 C++20 的原生构建工具链
- 支持 submodule 的 `git`

macOS 需要安装 Xcode Command Line Tools。Windows 需要可被 `node-gyp` 使用的 Visual Studio/MSVC 构建环境。

## 构建

如果需要，先初始化 MMKV 子模块，然后安装、构建和测试：

```bash
git submodule update --init --recursive
pnpm install
pnpm build
pnpm test
```

`pnpm test` 会用 `--expose-gc` 运行 Node 测试，因为 smoke 测试里包含内存回归检查。

Apple Silicon 上要保证终端架构和 Node.js 架构与目标二进制一致：

```bash
arch -arm64 zsh
node -p "process.arch"
pnpm install
pnpm build
pnpm test
```

在 Apple Silicon 上验证 Intel/Rosetta 构建：

```bash
arch -x86_64 zsh
node -p "process.arch"
pnpm install
pnpm build
pnpm test
```

原生 Apple Silicon 运行时使用 `darwin-arm64` 构建。Intel Mac 或 Rosetta 下的 x64 Electron 使用 `darwin-x64` 构建。

## 快速开始

```js
const { MMKV } = require("@huxinhai/mmkv");

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

如果 store 依赖全局 MMKV 根目录，需要先调用 `MMKV.initialize(rootDir)`。也可以在单个 store 的 options 里传入 `rootPath`。

## 打开 Store

```js
const { MMKV } = require("@huxinhai/mmkv");

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

`MMKV.initialize` 的第二个参数是上游 MMKV 的数字日志等级。传入 `4` 表示关闭 MMKV 日志。

## 值类型

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

超过 JavaScript 安全整数范围的 64 位整数请使用 `bigint`。`getInt64` 和 `getUInt64` 始终返回 `bigint`。

`getString` 和 `getBuffer` 在找不到值时返回 `null`。

## Key 和维护接口

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

调用 `close()` 后，实例的原生 MMKV 句柄已经关闭，继续调用实例方法会抛错。

## 加密

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

`cryptKey()` 返回的是这个 JavaScript wrapper 实例已知的密钥。它不会从 MMKV 存储里读取或泄露密钥。

## 备份和恢复

```js
const { MMKV } = require("@huxinhai/mmkv");

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

`backupOneToDirectory` 和 `restoreOneFromDirectory` 返回布尔值。`backupAllToDirectory` 和 `restoreAllFromDirectory` 返回处理的 store 数量。

## TypeScript

类型声明已经包含在 [`index.d.ts`](https://github.com/huxinhai/node-mmkv/blob/main/index.d.ts)。主要运行时导出如下：

```ts
import { MMKV, version } from "@huxinhai/mmkv";
```

公开 API 包括：

- 静态方法：`initialize`、`onExit`、`version`、`defaultMMKV`、`backupOneToDirectory`、`restoreOneFromDirectory`、`backupAllToDirectory`、`restoreAllFromDirectory`
- 值读写：`setBool/getBool`、`setInt32/getInt32`、`setUInt32/getUInt32`、`setInt64/getInt64`、`setUInt64/getUInt64`、`setFloat/getFloat`、`setDouble/getDouble`、`setString/getString`、`setBuffer/getBuffer`
- Key 操作：`containsKey`、`removeValueForKey`、`removeValuesForKeys`、`allKeys`、`count`
- 维护接口：`totalSize`、`actualSize`、`clearAll`、`trim`、`sync`、`lock`、`unlock`、`tryLock`、`reKey`、`cryptKey`、`close`

## Electron 接入建议

- 优先在主进程加载原生模块。
- `.node` 二进制架构必须和 Electron 运行时架构一致。
- 原生 Apple Silicon Electron 使用 `darwin-arm64`。
- Intel Mac 或 Rosetta 下的 x64 Electron 使用 `darwin-x64`。
- 64 位 Windows Electron 使用 `windows-x64`。
- 不要把 arm64 的 `.node` 加载到 x64 Electron 进程，反过来也不行。
- 每个应用建议使用独立的 `rootPath`。

MMKV 数据由 `rootPath` 和 store id 共同隔离。不同应用使用不同 root 时不会互相覆盖。同一个 root 下，不同 id 会对应不同 store。真正冲突的情况是两个运行时用同一个 `rootPath` 和同一个 id 存放不兼容的数据。

## CI 二进制产物

仓库包含 [`build-binaries.yml`](https://github.com/huxinhai/node-mmkv/blob/main/.github/workflows/build-binaries.yml)，触发场景包括：

- 推送到 `main`
- Pull request
- 手动 `workflow_dispatch`
- GitHub Release 发布

工作流会对每个目标执行构建和测试，在可用时裁剪原生二进制符号，然后打包：

- `darwin-arm64`
- `darwin-x64`
- `linux-x64`
- `windows-x64`

Release 构建会上传每个平台的 `.tar.gz` release asset，并发布包含全部预编译二进制的 npm 包。

## 项目结构

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
  上游 Tencent MMKV 源码树
binding.gyp
index.d.ts
test/smoke.test.ts
```

更多本地开发说明见 [`DEVELOPMENT.md`](https://github.com/huxinhai/node-mmkv/blob/main/DEVELOPMENT.md)，尤其是 macOS 多架构处理方式。

## 测试

[`test/smoke.test.ts`](https://github.com/huxinhai/node-mmkv/blob/main/test/smoke.test.ts) 中的 smoke 测试覆盖：

- 基础类型读写
- `defaultMMKV`
- 加密存储和 `reKey`
- 单 store 备份恢复
- 全量 store 备份恢复
- 锁和维护方法
- 反复 open/write/clear/close，并采样内存

运行：

```bash
pnpm test
```

## 当前限制

- 当前预编译二进制覆盖 macOS arm64、macOS x64、Linux x64 和 Windows x64。
- 其他平台会在安装时回退到本机 `node-gyp` 编译。
- 当前绑定覆盖 Node.js / Electron 常用存储场景，但还不是完整的上游 MMKV API。
