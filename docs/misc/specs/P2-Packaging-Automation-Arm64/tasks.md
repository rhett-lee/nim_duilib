# 任务清单：P2-1 打包自动化 与 P2-4 arm64 接入

## P2-4 arm64 接入

- [x] T1 新增 `msvc/manifest/duilib.arm64.manifest`（ARM64 + Common Controls + supportedOS）。
- [x] T2 `build/pack.py`：`WINDOWS_MANIFEST_ARCHES` 常量、接入文件收集三份
      manifest、arm64 的 CEF/JPEG 文案、MANIFEST 支持范围说明。
- [x] T3 `packaging/msvc/nim_duilib.props`：`NimDuilibManifestArch` 映射与 arm64 manifest 接入。
- [x] T4 `packaging/msvc/nim_duilib.targets`：如有架构白名单则补 arm64（无白名单，无需改动）。
- [x] T5 `packaging/cmake/nim_duilibConfig.cmake`：Windows 无 generator platform 时的 ARM64 识别。
- [x] T6 `msvc/PropertySheets/SkiaSettings.props`：ARM64 的 release/debug 与 `.md`/`.mdd` 目录。
- [x] T7 `cmake/duilib_common.cmake`：`DUILIB_SKIA_CPU_ARCH` 的 ARM64 识别。

## P2-1 测试与自动化

- [x] T8 `build/test_pack.py`：包名/元数据、MANIFEST、接入文件、arm64、文档链接、防回归用例。
- [x] T9 新增 `build/autotest.py`：预设表、环境检测、可选构建、打包、`--verify`、结构断言、
      可选冒烟、JSON/日志报告、`--dry-run`。
- [x] T10 新增 `build/test_autotest.py`：预设与命令拼装等纯单元测试。

## 文档与验证

- [x] T11 运行 `test_pack.py`、`test_autotest.py`（工作区 Python 与 MSYS2 Python）。
- [x] T12 运行 `autotest.py --preset unit` 与 `--preset arm64-synthetic`。
- [x] T13 对现有可用产物运行至少一个 `pack-*` 预设并记录报告（`pack-msvc-x64-release`）。
- [x] T14 更新 `Packaging-Follow-Up-Items.md`（P2-1、P2-4 状态 + 更新记录）。
- [x] T15 更新 `build/README.md` / `README.en.md`，并更新 `docs/Release.md` / `Release.en.md`。
- [x] T16 同步 E: 仓库副本，检查 `git status`，最终汇报变更、测试结果与未验证项。
