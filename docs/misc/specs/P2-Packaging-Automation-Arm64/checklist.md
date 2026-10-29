# 验收清单

## 代码

- [x] 三份 Windows manifest 均随包，arm64 manifest 的 `processorArchitecture` 为 `ARM64`。
- [x] VS props 对 `ARM64` 平台能选中 `duilib.arm64.manifest`。
- [x] CMake 在 `CMAKE_GENERATOR_PLATFORM` 为空且 `CMAKE_SYSTEM_PROCESSOR` 为
      `aarch64`/`arm64` 时判为 arm64。
- [x] `SkiaSettings.props` 有 `Release|ARM64`、`Debug|ARM64` 与 DLL 变体路径。
- [x] `pack.py` 对 arm64 的 CEF 报错、JPEG 告警文案明确。
- [x] `autotest.py` 默认不构建、不写 `bin/`、`lib/`、`dist/`。

## 测试

- [x] `python3 build/test_pack.py` 全通过。
- [x] MSYS2 Python 运行 `build/test_pack.py` 全通过。
- [x] `python3 build/test_autotest.py` 全通过。
- [x] `build/autotest.py --preset unit` 退出码 0。
- [x] `build/autotest.py --preset arm64-synthetic` 退出码 0 且结构断言通过。
- [x] 至少一个 `pack-*` 预设的 `--verify` 通过。

## 文档

- [x] P2-1 标注「自动化已完成、CI 不做」。
- [x] P2-4 标注「接入文件与产物结构就绪；未做真机编译/打包/运行验证；CEF/JPEG 不适用」。
- [x] 2026-10-29 回填：arm64 编译、出包（静态/DLL、CEF142/CEF109）与包测试已完成，
      仅运行验证未测；CEF/JPEG 的 arm64 预编译包已入库，原「不适用」结论撤销
      （见 `spec.md` §2 更新说明与 `docs/misc/Packaging-Follow-Up-Items.md` P2-4）。
- [x] `build/README(.en).md` 有 `autotest.py` 用法与安全说明。
- [x] 更新记录有新条目。
