下面把这批改动完整梳理一遍，末尾附建议的测试清单。

## 一、功能总览
路线图 #17「ListCtrl 表格增强」共四项，其中 列宽拖拽调整、列头拖拽换序、单元格图标 为框架既有能力，本次 新增 三项：

能力 入口 列冻结（固定左侧前 N 列） XML`frozen_column_count` /`SetFrozenColumnCount()` /`GetFrozenColumnCount()` 单元格进度条 `SetSubItemProgress(ById)` /`GetSubItemProgress(ById)` 单元格富文本 `SetSubItemRichText(ById)` /`IsSubItemRichText(ById)`

## 二、核心代码改动
### 1. 数据层
- ListCtrlDefs.h ：`ListCtrlSubItemData` 、`ListCtrlSubItemData2` 各新增`int32_t nProgress = -1;` （`[0,100]` 有效，`-1` 不显示进度条）、`bool bRichText = false;`
- ListCtrlData.cpp / .h ：
  - `SubItemToStorage` /`StorageToSubItem` 双向同步`nProgress` 、`bRichText`
  - 新增 4 个 API：`SetSubItemProgress` （钳位`[0,100]` ，越界归`-1` ，值变化时发`EmitDataChanged` ）、`GetSubItemProgress` 、`SetSubItemRichText` 、`IsSubItemRichText`
### 2. 列表控件
- ListCtrl.h / ListCtrl.cpp ：
  - 冻结列：成员`m_nFrozenColumnCount` （构造初始化为 0）、`SetFrozenColumnCount` （变化时`Refresh()` ）/`GetFrozenColumnCount` 、`SetAttribute` 解析`frozen_column_count`
  - 进度/富文本：对外接口（`columnIndex` 与`columnId` 两种重载），经`GetColumnId` 转发到`m_pData`
### 3. 报表视图（冻结列渲染与命中）
- ListCtrlReportView.h / .cpp ：
  - 两趟绘制 ：先画非冻结区（裁剪左边界＝视口左＋冻结宽，绘制原点偏移`(scrollX, scrollY)` ），再画冻结列（横向原点不偏移）覆盖在上层；两趟裁剪区互不重叠
  - `GetFrozenColumnsWidth()` ：按「前 N 个可见列最右侧边界 − 行首内边距左边界」计算（含行首 CheckBox/内边距占位），无数据行时回退用表头计算
  - `PaintGridLines` 纵向网格线 冻结感知 ，分段裁剪，冻结列线不随横向滚动
  - `FindControl` 重写为统一处理 TOP_FIRST / 非 TOP_FIRST 两条路径，末尾对落在冻结区的命中做横向偏移补偿
  - `FillDataItem` ： 先`SetRichText` 再`SetText` （普通分支同样先`SetRichText(false)` ），并下发`SetProgress` /`SetRichText`
### 4. 单元格控件
- ListCtrlSubItem.h / .cpp ：
  - 新增成员：`m_nProgress` 、`m_sProgressColor` (默认`bg_progress` )、`m_sProgressBkColor` (默认`bg_progress_track` )、`m_sProgressTextPos` (默认`center` )
  - 新增方法：`SetProgress/GetProgress` 、`GetProgressColor/GetProgressBkColor` 、`SetProgressTextPos/GetProgressTextPos` 、`PaintProgress`
  - 新增 XML 属性：`progress_color` 、`progress_bkcolor` 、`progress_text_pos`
  - `PaintText` 分支：
    - 有进度（`>=0` ）→`PaintProgress`
    - 无图标但 显示 CheckBox → 文本矩形右移「CheckBox 宽度 + 间距」（修复富文本列文字与 CheckBox 重叠）
    - 其余 → 原有`BaseClass::PaintText`
  - `PaintProgress` 细节：
    - 左侧依次预留 CheckBox、 图标 （图标在此绘制，避免被进度条覆盖）
    - 进度条高度约 22 DIP（`Dpi().GetScaleInt` ，DPI 自适应），夹在单元格高度内， 垂直居中 、上下留白（相邻行不再连成一片）
    - 轨道与前景均用`FillRoundRect` ，圆角半径＝高度一半（ 胶囊形 ）；进度极小时退化为圆点
    - 文本 强制垂直居中 ；水平位置由`progress_text_pos` 控制：`center` （默认）/`left` /`right` /`outside` （显示在进度条右侧，进度条相应缩短）
### 5. 框架层（富文本修复）
- LabelImpl.cpp ：`SetRichText` 值变化时新增`m_pTextDrawer->SetTextChanged()` +`RelayoutOrRedraw()` ，使普通/富文本切换时解析与测量缓存失效
## 三、资源、示例与文档
资源

- global.xml ：`list_ctrl_sub_item` Class 增`progress_color="bg_progress" progress_bkcolor="bg_progress_track" progress_text_pos="center"`
- list_ctrl.xml ：新增「冻结首列」单选项
- zh_CN-examples.txt / en_US-examples.txt ：各新增 2 条多语言文字
示例

- MainForm.cpp ：默认冻结首列，第 2 列为进度条、第 3 列为富文本演示数据
文档

- Control.md / Control.en.md ：属性表加`frozen_column_count` ，新增「ListCtrl 表格增强」章节（含`progress_text_pos` 说明）
- Control-Roadmap.md / .en.md ：第 17 项标记为已完成
- README.md / README.en.md ：功能完善第 24 条
- 技能参考副本（3 处均已同步）： .trae 、 .workbuddy 、 .claude
## 四、建议测试清单
列冻结

- 冻结 0 / 1 / 多列；冻结数 ≥ 可见列数
- 横向滚动：冻结列固定、其余列滚动；纵向滚动：冻结列同步纵向滚动
- 表头冻结对齐；纵向网格线不串位
- 鼠标命中：点击/悬停冻结列与普通列的单元格、表头排序、列宽拖拽、单元格编辑是否都落在正确的单元格
- 界面切换冻结列数后立即生效（含横向滚动中的状态）
列宽拖拽 / 列头换序

- 拖拽冻结列列宽 → 冻结区右边界跟随变化
- 把列拖入/拖出冻结区后，冻结定义仍按"最左侧前 N 列"
进度条

- 文字落在进度条内、垂直居中；相邻行不连成一片；圆角胶囊形
- `progress_text_pos` =`center` /`left` /`right` /`outside` 四种效果（`outside` 时进度条缩短）
- 该列显示 CheckBox 时文字右移不重叠；显示图标时图标在左、不被覆盖
- 进度 0 / 100 / 极小值；深浅主题下`progress_color` 、`progress_bkcolor` 生效
- DPI 125% / 150% 下高度与圆角正常
富文本

- 富文本标签正确渲染，不再出现原始`<b>` 标签
- 排序、滚动、刷新、编辑后仍为富文本（缓存失效生效）
- 行高按富文本测量
- 回归 ：其他使用`rich_text` 的控件（如 RichText 示例）显示正常（`LabelImpl::SetRichText` 为公共改动）
## 五、说明
- 已通过 VS2022 MSBuild 编译`build/examples.sln` （Debug|x64）， 0 错误 、无新增警告。
- 未产生临时脚本/日志；未执行 git 提交，等测试通过后由你手动提交。
其中`LabelImpl::SetRichText` 属于框架层公共行为变更，建议测试时额外关注其他富文本场景，避免回归。