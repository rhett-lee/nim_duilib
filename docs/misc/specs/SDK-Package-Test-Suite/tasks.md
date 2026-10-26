# 任务清单：发布 SDK 包测试用例与测试方法

依赖：[spec.md](spec.md)。用户批准后按顺序执行，不提交代码。

## T1 消费者用例工程

- [x] 新建 `build/sdk_test/CMakeLists.txt`：
      `NIM_DUILIB_SDK_DIR` + `SDK_EXPECT_*` 输入、`find_package(nim_duilib CONFIG REQUIRED)`、
      `sdk_expect()` 元数据断言、`nim_duilib_sdk_consumer` 目标、按能力追加头文件用例、
      `nim_duilib_copy_runtime(... RESOURCES)`。
- [x] 新建 `main.cpp`：打印 `SDK_TEST_FEATURES`，启动/关闭 `GlobalManager`，失败返回非 0。
- [x] 新建 `header_duilib.cpp`、`header_cef.cpp`、`header_webview2.cpp`。

## T2 编排器

- [x] 新建 `build/sdk_package_test.py`：
      - 纯函数 `parse_package_name`、`parse_cmake_info`、`parse_manifest_fields`、
        `infer_capabilities`、`select_archive_root`、`scenario_applicability`；
      - 解包与校验 P01–P08；
      - CMake 消费用例 C01/C02（配置、编译、运行、输出断言）；
      - 负向用例 C03–C09；
      - 可选 C10 示例冒烟；
      - CLI（`--package/--scenarios/--no-build/--run-examples/--out-dir/--keep/--jobs/
        --dry-run/--list-scenarios`）、表格汇总、`report.json`、退出码 0/1/2。

## T3 编排器单元测试

- [x] 新建 `build/test_sdk_package.py`：覆盖上述纯函数与场景适用性判断，
      不依赖真实 SDK 包，可直接被 `build.py test` 发现。

## T4 文档

- [x] 新建 `build/sdk-package-test.md`（目的、前置条件、快速开始、场景表、
      消费者工程用法、报告与排障、扩展方法、覆盖矩阵）。
- [x] `build/README.md` 文件概览表登记 `sdk_package_test.py`、`sdk_test/`、
      `test_sdk_package.py` 与 `sdk-package-test.md`。

## T5 验证

- [x] `python -m py_compile build/sdk_package_test.py build/test_sdk_package.py`
- [x] `python build/build.py test`
- [x] `python build/sdk_package_test.py --list-scenarios`
- [x] 对 `dist/nim_duilib-1.0.2026.1023-win-x64-examples-debug.zip` 执行全部场景
- [x] 对 `dist/nim_duilib-1.0.2026.1023-win-x64-dll-debug.zip` 执行全部场景
      （C02 通过；P02/P05/P07/P08/C09 因该包为旧版 `pack.py` 产物而失败，
      已在文档排障表中记录）
- [x] 确认临时产物只落在 `C:\develop\temp\sdk_test\`，仓库无多余文件
