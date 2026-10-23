# 更新日志

本项目的所有重要变更记录在此文件中，格式参考
[Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)。

版本号由 `duilib/duilib_version.h` 统一维护，格式为
`MAJOR.MINOR.PATCH.BUILD`，其中 `BUILD` 按 `MMDD` 书写且不补零。

## [Unreleased]

## [1.0.2026.1023] - 2026-10-23

### 新增

- 发布打包脚本 `build/pack.py`：按 SDK 契约收集已构建产物，生成 zip/tgz、
  `MANIFEST.txt`、`SHA256SUMS` 与包内 `VERSION`。
- 发布流程编排脚本 `build/release.py`：提供 `check`、`package`、`finalize`、`clean`
  四个子命令，不执行 git 写操作。
- 包内接入层：VS 的 `msvc/nim_duilib.props`、`.targets` 与 x64/x86 manifest；
  CMake 的 `lib/cmake/nim_duilib/*.cmake`，提供 `nim_duilib::duilib`。
- 顶层使用文档随 SDK 打包，并按包内布局重写 Markdown 链接。
- `--with-cef` 同时打包 CEF 运行时：保持 `bin/libcef_win[_109]/<架构>/` 结构，
  含 `libcef.dll`、`*.pak`、`icudtl.dat`、`locales/` 等（Windows）。
- 发布相关文档：`docs/Release.md`、`docs/misc/Release-Checklist.md` 与打包工作记录。

### 变更

- 版本号统一到 `duilib/duilib_version.h`，`msvc/dllRes/version.h` 改为 RC 适配层。
- 资源目录调整为 `bin/resources/`，与示例程序同级。
- 包名追加特性后缀：`-sdl`、`-cef` / `-cef109`、`-examples`、`-debug`，
  同一版本的不同特性包可共存于 `dist/`。
- `docs/` 只打包顶层使用文档，`misc/`、`specs/` 等子目录不随包。
- 归档语义：tar 按源文件权限写入（保留可执行与 setuid 位），zip 用 `S_IFLNK`
  条目保存符号链接；Unix 包解包后 `.so` 链接链与示例程序的可执行权限得以保留，
  符号链接不再计入 `SHA256SUMS`。

### 修复

- 修复 macOS 上根目录 `VERSION` 与标准头 `<version>` 冲突导致的编译失败。
- SDK 接入层（VS 属性表与 CMake Config）在 Debug 下补充
  `/NODEFAULTLIB:"libcmt.lib"`，消除下游链接的 LNK4098 告警。

### 移除

- 移除根目录 `VERSION.txt`，避免版本数据出现第二份来源。
