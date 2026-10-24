# 任务清单：构建/打包脚本控制台输出英文化

- [ ] T1 `build/pack.py`：版本解析、收集函数告警、`PackageError`、`verify_package`
      错误、`argparse` help、运行摘要与关键产物阻断提示全部改英文；
      `write_manifest` 与注释保持中文。
- [ ] T2 `build/release.py`：`check` / `package` / `finalize` / `clean` 控制台消息、
      校验失败原因、`argparse` help 改英文。
- [ ] T3 `build/autotest.py`：预设描述、前置检查/构建/步骤/断言/冒烟原因、
      汇总与 `argparse` help、日志与报告路径提示改英文。
- [ ] T4 `build/build_duilib_all_in_one.sh`（11 行 `echo`）与
      `build/msys2_build.sh`（5 行 `echo`）改英文。
- [ ] T5 同步测试：`test_pack.py` 中依赖中文告警的断言改英文子串；
      检查 `test_autotest.py` 是否依赖中文文本并同步。
- [ ] T6 验证：
      `python build/test_pack.py`、`python build/test_autotest.py`；
      `PYTHONIOENCODING=ascii` 下运行 `pack.py --help/--list`、
      `release.py --help/check`、`autotest.py --help/--list-presets/--preset unit`；
      CJK 扫描确认输出上下文无中文、shell `echo` 无中文。
- [ ] T7 汇报改动文件与验证结果（不提交、不编译 Release）。
