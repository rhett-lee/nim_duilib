# Tasks: ListCtrl Report 单元格自动换行（单行开关）

- [x] t1 ListCtrlData 新增 `SetSubItemSingleLine` / `IsSubItemSingleLine`（已实现）
- [x] t2 ListCtrl 新增 `SetSubItemSingleLine(ById)` / `IsSubItemSingleLine(ById)` 对外接口（已实现）
- [x] t3 `list_ctrl.xml` 新增 `cell_wrap` Option 对与标签；`MainForm.cpp` 的 `InitListCtrlEvents` 新增换行处理、`OnColumnChanged` 新增选中态同步
- [x] t4 语言文件 `zh_CN-examples.txt` / `en_US-examples.txt` 新增 4 条字符串
- [x] t5 同步文档：`docs/Control.md`/`.en.md`、`docs/ListCtrl-Roadmap.md`/`.en.md`、三处技能参考副本
- [x] t6 编译 `build/examples.sln` Debug|x64 验证 0 错误，并运行 `verify_docs.py` 文档漂移校验

备注：
- 代码提交由用户手动执行，助手不提交。
- 示例换行演示的行高策略：开启换行 `SetDataItemHeight(64, true)`，恢复单行 `SetDataItemHeight(32, true)`（默认行高为 32）。
- t6 结果：编译 0 错误 0 警告；`verify_docs.py` 报告 54 处疑似漂移，全部位于本次未改动/与本次改动无关的文件（Box/Global/Menu/Theme/ThemeColor，以及 `Control.md`/`.en.md` 第 57、681/683 行），本次新增内容未引入任何漂移。
