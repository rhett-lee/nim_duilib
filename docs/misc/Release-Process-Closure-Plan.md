# 发布流程闭环方案（P0-5）

> 状态：**已实施**（2026-10-23）：`build/release.py`、中英 CHANGELOG、
> 发布 checklist 与包名特性后缀均已完成。
> 对应待办：[Packaging-Follow-Up-Items.md](Packaging-Follow-Up-Items.md) 的 P0-5。
> 本文件位于 `docs/misc/`，不随发布包分发。

## 一、目标与现状

目标：按固定 checklist 从干净工作区产出唯一版本包，版本、变更记录、归档与清理都有明确规则，步骤可复现。

现状问题：

1. 版本数据曾存在三处：`duilib/duilib_version.h`、`VERSION.txt`、`msvc/dllRes/version.h`；已于 2026-10-23 统一到 `duilib/duilib_version.h`（详见第十三节）；
2. 同一版本的不同特性包（`--with-cef`、`--with-sdl`、`--with-examples`）包名完全相同，在 `dist/` 中会互相覆盖；
3. 没有 CHANGELOG / Release Notes，`dist/` 没有归档结构与清理规则；
4. 发布步骤只存在于 `docs/Release.md`，没有可执行的预检。

## 二、已确认的决策

| 项目 | 决策 |
| :--- | :--- |
| 自动化程度 | 脚本编排；commit / tag / push 只打印命令，由维护者手动执行 |
| CHANGELOG | 中英双语：`CHANGELOG.md` + `CHANGELOG.en.md` |
| dist 保留 | 全部保留，仅支持手动清理，脚本默认不删除 |
| 多特性包命名 | 包名追加特性后缀，`dist/` 平铺存放 |
| 版本号单一源 | `duilib/duilib_version.h`，删除 `VERSION.txt`；`BUILD` 段不补零 |

## 三、发布入口

新增 `build/release.py`，只做编排，不替代 `build/pack.py`，不执行 git 写操作。

```
python build/release.py check
python build/release.py package --config Release --arch x64 --with-cef --dry-run
python build/release.py package --config Release --arch x64 --with-cef
python build/release.py finalize
python build/release.py clean --list
python build/release.py clean --version 1.0.2026.1015 --yes
```

- `package` 带默认参数调用 `build/pack.py`，输出目录固定为 `dist/<版本>/`；
- `--dry-run` 只打印将执行的命令与最终包名，便于核对；
- 不自动 commit、tag、push；`check` 结束时打印建议命令。

## 四、包名规则

在 `build/pack.py` 现有包名后追加特性后缀，顺序固定：

```
nim_duilib-<版本>-<系统>-<架构>[-sdl][-cef|-cef109][-examples][-debug]
```

- `-sdl`：仅 Windows 显式 `--with-sdl` 时追加；Unix 默认带 SDL，不加后缀，避免改变现有 Unix 包名；
- `-cef`：`--with-cef --cef-version latest`；`-cef109`：109 版本；
- `-examples`：`--with-examples`；
- `-debug` 保持在最后，兼容现有 Debug 包命名。

示例：

```
nim_duilib-1.0.2026.1020-win-x64.zip
nim_duilib-1.0.2026.1020-win-x64-cef.zip
nim_duilib-1.0.2026.1020-win-x64-cef109-examples-debug.zip
nim_duilib-1.0.2026.1020-linux-x64.tar.gz
```

## 五、CHANGELOG

根目录新增 `CHANGELOG.md` 与 `CHANGELOG.en.md`，Keep a Changelog 风格，两者版本标题必须一致：

```
## [Unreleased]

## [1.0.2026.1020] - 2026-10-23

### Added
### Changed
### Fixed
### Removed
```

`check` 校验两个文件都包含当前 `duilib_version.h` 对应的版本段落，缺任意一份即失败。
根目录文件不会进入 SDK 包（打包只收集 `docs/`）。

## 六、check 校验项

1. 工作区无已跟踪文件的未提交修改（`dist/` 忽略）；
2. 能用统一解析函数从 `duilib/duilib_version.h` 解析出四段版本，且各段唯一、范围 `0..65535`；
3. 不存在 `v<版本>` tag；
4. 相对最近的 `v*` tag 严格递增（历史非 `v*` tag 不参与）；
5. 中英 CHANGELOG 均存在当前版本条目。

任一项失败返回非 0，并输出具体原因与建议命令。

版本号的唯一编辑点是 `duilib/duilib_version.h`，具体方案见第十三节；不再比对 `msvc/dllRes/version.h`。

## 七、finalize 处理

对 `dist/<版本>/` 下每个压缩包：

1. 解压到临时目录；
2. 全量校验 `SHA256SUMS`；
3. 检查 `MANIFEST.txt`、`VERSION`、`include/duilib/duilib.h`；
4. 检查包内文档相对链接无断链；
5. 全部通过后生成：
   - `dist/<版本>/SHA256SUMS.txt`（压缩包级哈希）；
   - `dist/<版本>/RELEASE-NOTES.md` 与 `RELEASE-NOTES.en.md`（从 CHANGELOG 提取）；
6. 任一包校验失败则返回非 0，不生成发布说明。

## 八、dist 清理

按“全部保留、仅手动清理”：

- `clean --list` 只列出版本目录、包数量与占用；
- 默认不带参数时不删除任何内容；
- 删除必须显式给 `--version <版本>` 且带 `--yes`；
- 拒绝删除当前解析版本对应的目录。

## 九、人工 checklist

新增 `docs/misc/Release-Checklist.md`（不随包），覆盖：

1. 确认依赖产物（duilib、Skia、SDL、CEF）已按目标配置编译；
2. 更新 `duilib/duilib_version.h`、中英 CHANGELOG；
3. `release.py check`；
4. 按矩阵调用 `release.py package`，必要时先 `--dry-run`；
5. `release.py finalize`；
6. 手动 `git commit`、`git tag -a v<版本>`、`push`；
7. 归档/上传产物；
8. `release.py clean` 仅按需手动执行。

## 十、文件改动清单

| 文件 | 动作 |
| :--- | :--- |
| `build/release.py` | 新增：check / package / finalize / clean 四个子命令 |
| `build/pack.py` | 解析 `duilib_version.h`；包名追加特性后缀；更新文件头用法说明 |
| `duilib/duilib_version.h` | 唯一版本源（已提交，当前 `1.0.2026.1023`） |
| `msvc/dllRes/version.h` | 改为 Windows RC 适配层，引用新头文件，不再保存版本数字 |
| `VERSION.txt` | 删除 |
| `CHANGELOG.md`、`CHANGELOG.en.md` | 新增：Unreleased 与当前版本条目 |
| `docs/misc/Release-Checklist.md` | 新增：人工发布 checklist |
| `docs/Release.md`、`docs/Release.en.md` | 更新版本号、包名后缀、发布流程命令 |
| `docs/misc/Packaging-Follow-Up-Items.md` | P0-5 完成后更新状态与记录 |

## 十一、验收标准

- `py_compile` 通过；
- `release.py check` 能准确报出版本、CHANGELOG、工作区问题；
- `package --dry-run` 输出正确包名；
- 用现有 Windows x64 Debug 产物实际打包后，`finalize` 通过 SHA 与断链校验；
- `clean --list` 不删除任何文件；
- `clean --version <非当前版本> --yes` 才执行删除，且拒绝删除当前版本。

## 十二、范围边界

- 不实现全平台矩阵自动构建；矩阵仍按 checklist 逐项执行，后续并入 P2-1 的 CI；
- 不做签名与公证（P0-4 已关闭）；
- 不做关键产物缺失时的 fail-fast（P0-2 已关闭），打包脚本保持“告警后继续”的行为；
- 不自动提交、打 tag 或推送。

## 十三、版本号单一源方案（已实施，2026-10-23）

> 本节取代 `Release-Packaging-Notes` 中“让 version.h 从 VERSION.txt 派生”的旧结论。
> 实施结果：`pack.py` 已改为解析 `duilib/duilib_version.h`，`VERSION.txt` 已删除，
> `version.h` 已改为适配层；`--list` 输出 `1.0.2026.1023`。

### 13.1 职责划分

| 文件 | 角色 |
| :--- | :--- |
| `duilib/duilib_version.h` | **唯一版本源**，只放版本数字宏，随公共头一起打包 |
| `msvc/dllRes/version.h` | Windows RC 适配层：引用新头文件，补充 `STRINGIFY/TOSTRING`、公司/产品信息宏，不再保存版本数字 |
| `VERSION.txt` | 删除，避免第二份版本数据 |
| `build/pack.py` | 改为解析 `duilib_version.h` |
| `duilib/duilib.rc` | 不改，仍 `#include "version.h"`，通过适配层取得数字 |

CMake 构建不编译 RC，不受影响；Windows RC 工程也无需改动。

### 13.2 头文件约定

- 只认下面四个数字宏，后续可以自由增加版权、ABI、构建类型等其它内容；
- `DUILIB_VER_BUILD` 按 `MMDD` 语义书写，但**不得带前导零**：
  - 10 月 23 日写 `1023`，版本显示为 `1.0.2026.1023`；
  - 1 月 5 日写 `105`，版本显示为 `1.0.2026.105`；
  - 原因：C/C++/RC 中 `0105` 是八进制（等于 69），`0108`、`0109` 是非法八进制，会编译失败；
- 每段数值范围 `0..65535`；
- 全链路不补零，包名、包内 `VERSION`、DLL 属性、CMake 包版本保持一致。

适配层示例：

```c
#pragma once

#include "../../duilib/duilib_version.h"

#define STRINGIFY(x)  #x
#define TOSTRING(x)   STRINGIFY(x)

#define DUILIB_VER_FILEVERSION_STR  TOSTRING(DUILIB_VER_MAJOR) "." \
                                    TOSTRING(DUILIB_VER_MINOR) "." \
                                    TOSTRING(DUILIB_VER_PATCH) "." \
                                    TOSTRING(DUILIB_VER_BUILD)
#define DUILIB_VER_PRODVERSION_STR  DUILIB_VER_FILEVERSION_STR
#define DUILIB_VER_FILEVERSION      DUILIB_VER_MAJOR,DUILIB_VER_MINOR,DUILIB_VER_PATCH,DUILIB_VER_BUILD
#define DUILIB_VER_PRODVERSION      DUILIB_VER_FILEVERSION

#define DUILIB_VER_COMPANY          "https://github.com/rhett-lee/nim_duilib"
#define DUILIB_VER_DESC             "nim_duilib DLL"
#define DUILIB_VER_COPYRIGHT        "Copyright (C) 2026"
#define DUILIB_VER_PRODUCTNAME      "nim_duilib"
```

### 13.3 解析规则（兼容后续新增内容）

```python
r'^\s*#\s*define\s+DUILIB_VER_(MAJOR|MINOR|PATCH|BUILD)\s+(\d+)\b'
```

- 只匹配行首 `#define`，注释掉的定义、`#undef`、其它宏一律忽略；
- 四个宏各出现一次；缺失或重复定义直接报错，避免条件编译导致静默取错值；
- `\b` 边界保证不会误匹配将来新增的 `DUILIB_VER_BUILD_STR`；
- 使用 `utf-8-sig` 读取，兼容 BOM；
- 解析后校验每段范围；
- 错误信息要指出缺少/重复/越界的具体宏名。

### 13.4 pack.py 改动

- `VERSION_FILE` 改为 `duilib/duilib_version.h`，所有版本读取统一走一个函数；
- 包内 `VERSION` 内容由解析结果生成，不再读取仓库文本文件；
- 包名、`MANIFEST.txt`、`nim_duilib.sdk.props`、CMake 包信息全部复用同一结果；
- 删除 `VERSION.txt` 后，全仓库不再保留对它的引用；
- `duilib_version.h` 位于公共头目录，随包进入 `include/duilib/`，下游可直接查询版本宏。

### 13.5 验收

- `python build/pack.py --config Debug --list` 正常，版本为 `1.0.2026.1023`；
- 包名、包内 `VERSION`、`MANIFEST.txt`、CMake 包版本四处一致；
- 在 `duilib_version.h` 中加入无关宏、注释、版权声明后，解析结果不变；
- 缺失某个宏或重复定义时，脚本以非 0 退出并给出明确错误；
- VS 的 RC 通过适配层正常编译，DLL 文件属性显示 `1.0.2026.1023`。
