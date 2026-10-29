# 任务清单：构建/打包脚本控制台输出双语化

> 已完成（2026-10-29，P3-3）。原 T4（两个 `.sh` 文件英文化）作废：
> `build/build_duilib_all_in_one.sh` 与 `build/msys2_build.sh` 已不在仓库中。

- [x] T1 `build/i18n.py`：新增 `_normalize_language()`，仅 `zh`/`zh_CN`/`zh_Hans`
      显示中文，`zh_TW`/`zh_HK`/`zh_MO`/`zh_Hant` 及其他区域显示英文；
      `resolve_language()` 两级判定统一走该函数，与 `cmake/duilib_i18n.cmake` 一致。
- [x] T2 `build/pack.py`（125 个 `pkg.*` 键）：版本解析、收集函数告警、
      `PackageError`、`verify_package` 错误、argparse help、运行摘要与关键产物
      阻断提示迁到消息目录；`write_manifest` 与注释保持中文。
- [x] T3 `build/release.py`（78 个 `rel.*` 键）：`check` / `package` / `finalize` /
      `clean` 控制台消息、校验失败原因、argparse help 双语化；`--lang` 同时挂在
      子命令上；生成的 `RELEASE-NOTES` 标题保持中文。
- [x] T4（作废）~~两个 `.sh` 脚本的 `echo` 英文化~~：文件已删除，无需处理。
- [x] T5 `build/autotest.py`（45 个 `auto.*` 键）：预设描述改为 key，在
      `--list-presets` 打印处取词；前置检查/构建/步骤/断言/冒烟原因、汇总与
      argparse help 双语化。
- [x] T6 `build/sdk_package_test.py`（约 180 个 `sdk.*` 键）：场景说明改为 key，
      在 `print_scenarios()` 取词；`run_verify_cli()` 给 `pack.py --verify` 传
      `--lang`；`check_incomplete` 的预期文本改用 `pkg.headers_missing` /
      `pkg.static_libs_missing`。
- [x] T7 `build/build.py`：argparse 的 description/epilog 与全部 help 迁到 48 个
      `help.*` 键；`resolve_cef_version` 的未知版本错误、`--dll` 平台错误双语化；
      `main()` 在 `parse_args()` 之前解析 `--lang`，使 `--help` 也按语言输出。
- [x] T8 `cmake/i18n/messages.json`：合并上述条目（现 594 键），每个键同时提供
      `en` 与 `zh_CN`；原有内容只做追加，无删除。
- [x] T9 同步测试：
      `test_build.py` 固定中文并新增繁体回退、英文纯 ASCII 帮助用例；
      `test_pack.py`、`test_autotest.py`、`test_sdk_package.py` 固定中文，
      并各补英文纯 ASCII 用例。
- [x] T10 验证：`python build/build.py test`（236 用例）通过；
      `PYTHONIOENCODING=ascii` 下五个入口的 `--help` 与列表类命令（加
      `--lang en`）非 ASCII 字符数均为 0；CJK 扫描确认输出上下文无中文。
- [x] T11 文档：本目录 `spec.md` 与 `tasks.md` 改写为双语设计；
      `docs/misc/Packaging-Follow-Up-Items.md` 的 P3-3 勾选；
      `build/test-matrix.md` 第 11 条更新；中英 `CHANGELOG` 增补条目。
- [x] T12 汇报改动文件与验证结果（不提交、不编译 Release）。
