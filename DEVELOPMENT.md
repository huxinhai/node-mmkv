# mmkv Development

本文档用于约束 `mmkv` 的本地开发方式，尤其是 macOS 下 Apple Silicon(M1/M2/M3/M4) 与 Intel(x86_64) 两类机器的兼容开发流程。

当前仓库还处在项目骨架阶段，根目录里暂时只有：

- `MMKV/`：Tencent MMKV 源码子模块
- `src/`：预留给 Node.js / Electron 原生绑定代码
- `DEVELOPMENT.md`：当前开发文档

这意味着本文档优先解决两件事：

- 如何把开发环境稳定搭起来
- 后续开始写 Node 原生模块时，怎样避免只在某一类 Mac 上能编译

## 1. 开发目标

`mmkv` 的目标是基于 Tencent MMKV 提供一个 Node.js / Electron 可用的原生模块。

建议的技术路线：

- 绑定层：Node-API(N-API)
- 核心依赖：仓库内 `MMKV` 子模块
- 首要支持平台：macOS
- macOS 目标架构：
  - `arm64`，对应 Apple Silicon
  - `x86_64`，对应 Intel Mac

如果后面扩展到 Windows，可以在不改动 JS API 的前提下补充平台实现。

## 2. 基本要求

建议统一以下工具链：

- Node.js 20 或 22 LTS
- pnpm 10+
- Python 3
- CMake 3.10+
- Xcode Command Line Tools
- git，且支持 submodule

先检查本机环境：

```bash
uname -m
node -p "process.platform + ' ' + process.arch"
pnpm -v
python3 --version
cmake --version
xcode-select -p
```

重点关注两件事：

- `uname -m` 表示当前终端运行架构
- `process.arch` 表示当前 Node 进程架构

这两个值在本地编译原生模块时必须和你要产出的目标一致。

## 3. 初始化仓库

首次拉取建议直接带上子模块：

```bash
git clone --recursive <repo-url>
cd mmkv
```

如果仓库已经拉下来了，再补子模块：

```bash
git submodule update --init --recursive
```

验证 `MMKV` 子模块是否正常：

```bash
git submodule status
```

## 4. macOS 双架构原则

这是本项目最重要的约束。

### 4.1 不要混用架构

如果你在 Apple Silicon Mac 上开发，系统可能同时存在两套执行环境：

- 原生 `arm64`
- 通过 Rosetta 运行的 `x86_64`

最容易出问题的情况是：

- 终端是 `arm64`
- 但 `node` 是 `x86_64`
- 或者反过来

这样在编译 `.node` 原生模块时，很容易出现架构不匹配、链接失败、运行时报 `wrong architecture`。

### 4.2 开发时的推荐策略

建议把本地开发分成两种模式：

1. 原生开发模式
   - Apple Silicon 机器用 `arm64`
   - Intel 机器用 `x86_64`
2. 兼容验证模式
   - 在 Apple Silicon 机器上额外用 Rosetta 跑一次 `x86_64` 构建验证

也就是说：

- M 系列 Mac 不是只验证 `arm64`
- Intel Mac 也不能依赖 Apple Silicon 专属配置

### 4.3 常用判断命令

查看终端当前架构：

```bash
uname -m
arch
```

查看 Node 当前架构：

```bash
node -p "process.arch"
```

在 Apple Silicon 机器上强制使用 Rosetta x86_64 启动命令：

```bash
arch -x86_64 zsh
```

在 Apple Silicon 机器上强制使用原生 arm64 启动命令：

```bash
arch -arm64 zsh
```

## 5. Apple Silicon 开发建议

### 5.1 原生 arm64 开发

在 M 系列 Mac 上，优先使用原生 arm64 开发：

```bash
arch -arm64 zsh
node -p "process.arch"
```

输出应为：

```bash
arm64
```

如果后续项目加入 `package.json`，则依赖安装和构建也要在同一架构终端里完成：

```bash
pnpm install
pnpm build
```

### 5.2 额外验证 x86_64 兼容

Apple Silicon 机器还应该定期做一轮 Rosetta 验证：

```bash
arch -x86_64 zsh
node -p "process.arch"
```

预期输出：

```bash
x64
```

如果此时 `node` 没法运行，通常说明你还没有可用的 x64 Node 环境。

## 6. Intel Mac 开发建议

Intel Mac 的目标更直接，终端和 Node 正常情况下都会是 `x86_64` / `x64`：

```bash
uname -m
node -p "process.arch"
```

预期：

```bash
x86_64
x64
```

Intel Mac 上开发时，重点不是切架构，而是避免把只在 Apple Silicon 上成立的参数、路径或 Homebrew 位置写死到脚本里。

## 7. Homebrew 路径兼容

macOS 下最常见的问题之一，是 Homebrew 安装路径因芯片不同而不同：

- Apple Silicon 默认是 `/opt/homebrew`
- Intel 默认是 `/usr/local`

所以后续如果要在脚本里查找依赖，不要写死单一路径。推荐优先使用：

```bash
brew --prefix
```

或者：

```bash
command -v brew
```

不推荐：

```bash
/opt/homebrew/bin/brew
/usr/local/bin/brew
```

除非脚本里已经做了分支判断。

## 8. MMKV 在 macOS 下的实现选择

根据上游 `MMKV/Core/CMakeLists.txt`，Apple 平台默认会启用：

- `FORCE_POSIX`
- `MMKV_OSX.cpp`
- `MemoryFile_OSX.cpp`
- `MiniPBCoder_OSX.cpp`
- `CodedInputData_OSX.cpp`

这说明 macOS 开发不应把 MMKV 当成 Linux POSIX 的纯替代实现来处理，而应沿用上游针对 Darwin 的文件组合。

对本项目的直接约束是：

- 不要手动删掉上游 macOS 专用源码
- 不要假定 macOS 只需要 `MemoryFile_Linux.cpp`
- 如果后续写 `binding.gyp` 或 CMake 包装层，源码列表必须和平台条件保持一致

## 9. 后续原生模块开发约定

等根目录开始补齐 `package.json`、`binding.gyp`、`src/*.cpp` 后，建议遵守以下规则。

### 9.1 绑定层使用 Node-API

优先使用 Node-API，而不是 V8 私有 API。原因：

- Node 版本兼容性更稳
- Electron 适配成本更低
- 更适合后续发预编译产物

### 9.2 编译时始终让 Node 架构与编译目标一致

例如：

- 你要产出 arm64 版本 `.node`，就用 arm64 的 Node 执行安装和编译
- 你要产出 x64 版本 `.node`，就用 x64 的 Node 执行安装和编译

不要指望一次混编自动成功。

### 9.3 本地验证至少覆盖两组组合

在发布前，macOS 至少验证：

- `darwin-arm64`
- `darwin-x64`

如果只有一台 Apple Silicon 机器，也至少要做：

- 原生 `arm64` 验证
- Rosetta 下 `x64` 验证

## 10. 推荐测试矩阵

后续补 CI 时，建议最少覆盖：

| 系统 | 架构 | 用途 |
|------|------|------|
| macOS | arm64 | Apple Silicon 原生验证 |
| macOS | x64 | Intel Mac 兼容验证 |

如果发包方式是预编译二进制，也建议产物名显式区分：

- `darwin-arm64`
- `darwin-x64`

不要把两种架构混成一个不透明产物名。

## 11. 常见问题

### 11.1 `wrong architecture`

通常表示以下某一项架构不一致：

- 终端架构
- Node 架构
- 原生依赖编译产物架构
- 最终 `.node` 文件架构

先检查：

```bash
uname -m
node -p "process.arch"
file <native-module.node>
```

### 11.2 Apple Silicon 上找不到依赖

先检查 Homebrew 前缀：

```bash
brew --prefix
```

再确认你当前终端是不是切到了 Rosetta：

```bash
arch
```

很多“依赖找不到”本质上不是依赖缺失，而是你切到了另一套架构环境。

### 11.3 子模块代码不完整

执行：

```bash
git submodule update --init --recursive
```

## 12. 当前阶段建议

在这个仓库现状下，下一步最合理的落地顺序是：

1. 补 `package.json`
2. 补 `binding.gyp`
3. 在 `src/` 中加入最小可编译的 N-API addon
4. 先打通 macOS `arm64`
5. 再在 Apple Silicon 的 Rosetta 或 Intel Mac 上验证 `x64`
6. 最后再考虑发布和 CI

## 13. 一句话原则

在 `mmkv` 里，macOS 兼容的关键不是“代码能编过一次”，而是：

- 同一套源码
- 不写死芯片相关路径
- 同时能在 `darwin-arm64` 和 `darwin-x64` 下完成构建与加载

只要一直按这个原则推进，Apple Silicon 和 Intel Mac 都能稳定支持。
