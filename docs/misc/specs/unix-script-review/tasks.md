# Tasks：Unix 三平台脚本复查修复

- [x] T1 修复 pack.py wrapper 名归一（修复 1）：`_lib_base_name` 兼容 `cef_dll_wrapper` / `cef_dll_wrapper_109`
- [x] T2 test_pack.py 新增 Linux wrapper 归一用例
- [x] T3 修复 deps_run SDL stamp 配置分量（修复 2）：Unix 省略 `${DUILIB_CONFIG}`
- [x] T4 修复 common.cmake Skia 绝对路径解析扩展到 Unix（修复 3）+ 更正注释
- [x] T5 修复 cef.cmake `_duilib_cef_tracked_dir` 按平台返回（修复 4）
- [x] T6 更正 duilib_compiler_flags.cmake 头注释（修复 5）
- [x] T7 test-matrix.md 登记真机验证缺口（修复 2/3/4）
- [x] T8 同步 CHANGELOG.md/.en.md（修复区 3 条新增 + 1 条修订）；build-reference 双语无需改动
- [x] T9 运行 `python3 build/build.py test`，160 项全部通过
- [x] T10 汇总发现清单报告用户（提醒手动提交）
