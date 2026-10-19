#ifndef DUI_TREE_GRID_H_
#define DUI_TREE_GRID_H_

#include <vector>
#include <map>
#include "duilib/Core/Box.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/ListBoxItem.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/CheckBox.h"

namespace ui {

class TreeGrid;
class TreeGridNode;
class VListBox;

/** 树表格的列定义
*/
struct DUILIB_API TreeGridColumn
{
    /** 列ID（默认取列索引，从 0 开始）
    */
    size_t nColumnId = 0;

    /** 列标题（可用于多语言：以 id 形式给出的文本）
    */
    DString strTitle;

    /** 列宽（原始像素值，不做 DPI 缩放，内部按当前 DPI 缩放后使用）
    */
    int32_t nWidth = 120;

    /** 列对齐方式：left（默认）/ center / right
    */
    DString strAlign = _T("left");
};

/** 树表格节点的勾选状态（三态）
*/
enum class TreeGridCheck
{
    /** 未勾选 */
    UnCheck,
    /** 全部勾选 */
    CheckedAll,
    /** 部分勾选（存在勾选与未勾选的子孙） */
    CheckedPartly
};

/** 复选框状态变化的事件数据（kEventTreeGridCheckChanged 的 pEventData）
*/
struct DUILIB_API TreeGridCheckValue
{
    /** 发生状态变化的节点 */
    TreeGridNode* pNode = nullptr;
    /** 该节点当前是否处于"全部勾选"状态 */
    bool bChecked = false;
};

/** 树表格的数据行控件（内部使用）：一个 HBox，包含 N 个等宽单元格，树列单元格位于 m_pTreeCell
*/
class DUILIB_API TreeGridRow : public ListBoxItemH
{
public:
    explicit TreeGridRow(Window* pWindow);
    virtual ~TreeGridRow() override;

    TreeGridNode* GetNode() const { return m_pNode; }
    void SetNode(TreeGridNode* pNode) { m_pNode = pNode; }

public:
    /** 关联的数据节点 */
    TreeGridNode* m_pNode = nullptr;
    /** 树列单元格容器（HBox 控件，仅当存在树列时非空） */
    Control* m_pTreeCell = nullptr;
    /** 缩进占位控件（仅树列单元格内） */
    Control* m_pIndent = nullptr;
    /** 展开/收起开关（仅树列单元格内） */
    CheckBox* m_pExpand = nullptr;
    /** 行复选框（仅树列单元格内） */
    CheckBox* m_pCheck = nullptr;
    /** 每列的单元格控件（数量与列数一致；树列指向 m_pTreeCell） */
    std::vector<Control*> m_cellControls;
    /** 每列的文本控件（数量与列数一致；便于快速更新文本） */
    std::vector<Label*> m_cellLabels;
};

/** 树表格的数据节点
*  说明：TreeGridNode 是"数据节点"（不继承 Control），行的 UI 由 TreeGrid 内部按节点维护。
*/
class DUILIB_API TreeGridNode
{
public:
    explicit TreeGridNode(TreeGrid* pTreeGrid);
    ~TreeGridNode();

    TreeGrid* GetTreeGrid() const { return m_pTreeGrid; }
    TreeGridNode* GetParentNode() const { return m_pParentNode; }

    /** 节点层级（虚拟根的一级子节点深度为 1）
    */
    size_t GetDepth() const { return m_nDepth; }

public:
    /** 追加一个子节点（转发到 TreeGrid::AddNode）
    */
    TreeGridNode* AddChildNode(TreeGridNode* pNode);
    /** 移除一个子节点（转发到 TreeGrid::RemoveNode）
    */
    bool RemoveChildNode(TreeGridNode* pNode);
    /** 移除所有子节点
    */
    void RemoveAllChildNodes();
    /** 子节点个数
    */
    size_t GetChildNodeCount() const { return m_childNodes.size(); }
    /** 获取指定索引的子节点
    */
    TreeGridNode* GetChildNode(size_t iIndex) const;
    /** 获取指定子节点的索引，不存在则返回静态常量 npos
    */
    size_t GetChildNodeIndex(TreeGridNode* pNode) const;
    /** 递归子孙节点总数（不含自身）
    */
    size_t GetDescendantNodeCount() const;
    /** 是否为指定节点的子孙
    */
    bool IsChildOf(TreeGridNode* pNode) const;

public:
    bool IsExpand() const { return m_bExpand; }
    bool SetExpand(bool bExpand);

    bool IsChecked() const { return m_bChecked; }
    bool SetChecked(bool bChecked);

    /** 节点勾选状态（三态）
    */
    TreeGridCheck GetCheckState() const;

    bool IsSelected() const { return m_bSelected; }
    bool SetSelected(bool bSelected);

public:
    bool SetCellText(size_t nColumnId, const DString& text);
    DString GetCellText(size_t nColumnId) const;

    void SetUserData(void* pData) { m_pUserData = pData; }
    void* GetUserData() const { return m_pUserData; }

public:
    /** 无效索引 */
    static const size_t npos = size_t(-1);

private:
    /** 内部：由 TreeGrid 调用，更新层级
    */
    void SetDepth(size_t nDepth) { m_nDepth = nDepth; }
    void SetParentNode(TreeGridNode* pParent) { m_pParentNode = pParent; }

    /** 内部：子节点列表
    */
    std::vector<TreeGridNode*>& GetChildNodes() { return m_childNodes; }

    friend class TreeGrid;

private:
    TreeGrid* m_pTreeGrid = nullptr;
    TreeGridNode* m_pParentNode = nullptr;
    std::vector<TreeGridNode*> m_childNodes;
    size_t m_nDepth = 0;
    bool m_bExpand = false;      // 是否展开
    bool m_bChecked = false;     // 是否"全部勾选"
    bool m_bPartChecked = false; // 是否"部分勾选"
    bool m_bSelected = false;    // 是否选中
    int32_t m_nRowHeight = 0;    // 逐行行高（原始像素，0 表示使用全局 row_height）
    std::map<size_t, DString> m_cellTexts;
    void* m_pUserData = nullptr;

    /** 内部：对应的行控件（由 TreeGrid 维护，可能为空） */
    Control* m_pRow = nullptr;
    /** 内部：行控件在列表体中的索引 */
    size_t m_nRowIndex = 0;
};

/** 树表格控件（TreeGrid）
*  在同一控件内呈现"可展开/收起的层级行 + 多列表格"。
*/
class DUILIB_API TreeGrid : public VBox
{
public:
    typedef VBox BaseClass;

    explicit TreeGrid(Window* pWindow);
    virtual ~TreeGrid() override;

    /** 获取控件类型
    */
    virtual DString GetType() const override;

    /** 设置控件属性
    */
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** DPI 缩放变化
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

protected:
    /** 初始化（内部创建表头与列表体）
    */
    virtual void OnInit() override;

public:
    /* ---- 列管理 ---- */
    /** 追加一列（会重建表头并刷新所有已建行的单元格）
    */
    void AddColumn(const TreeGridColumn& column);
    size_t GetColumnCount() const { return m_columns.size(); }
    bool GetColumn(size_t nColumnIndex, TreeGridColumn& column) const;
    bool SetColumnWidthById(size_t nColumnId, int32_t nWidth);
    int32_t GetColumnWidthById(size_t nColumnId) const;
    bool SetColumnTitleById(size_t nColumnId, const DString& title);
    void ClearColumns();

public:
    /* ---- 节点管理 ---- */
    TreeGridNode* GetRootNode() const { return m_pRootNode; }
    TreeGridNode* AddNode(TreeGridNode* pParent);
    bool RemoveNode(TreeGridNode* pNode);
    void RemoveAllNodes();

public:
    /* ---- 数据设置 ---- */
    bool SetCellText(TreeGridNode* pNode, size_t nColumnId, const DString& text);
    DString GetCellText(TreeGridNode* pNode, size_t nColumnId) const;
    bool SetNodeRowHeight(TreeGridNode* pNode, int32_t nHeight);

public:
    /* ---- 展开 / 收起 ---- */
    bool SetNodeExpand(TreeGridNode* pNode, bool bExpand, bool bTriggerEvent = false);
    bool IsNodeExpand(TreeGridNode* pNode) const;
    bool ExpandNode(TreeGridNode* pNode);
    bool EnsureNodeVisible(TreeGridNode* pNode);

public:
    /* ---- 选择 ---- */
    void SetMultiSelect(bool bMultiSelect);
    bool IsMultiSelect() const;
    size_t GetSelectNodeCount() const;
    TreeGridNode* GetSelectNodeAt(size_t nIndex) const;
    std::vector<TreeGridNode*> GetSelectNodes() const;
    bool IsNodeSelected(TreeGridNode* pNode) const;
    bool SelectNode(TreeGridNode* pNode, bool bSelect = true, bool bTriggerEvent = true);
    void SelectAllNodes(bool bSelect = true);
    void ClearSelectNodes();

public:
    /* ---- 复选框（级联） ---- */
    bool IsCheckMode() const { return m_bCheckMode; }
    void SetCheckMode(bool bCheckMode);
    bool IsCheckCascade() const { return m_bCheckCascade; }
    void SetCheckCascade(bool bCascade) { m_bCheckCascade = bCascade; }
    bool IsNodeChecked(TreeGridNode* pNode) const;
    bool SetNodeChecked(TreeGridNode* pNode, bool bChecked, bool bTriggerEvent = true);
    TreeGridCheck GetNodeCheckState(TreeGridNode* pNode) const;
    void SetAllNodesChecked(bool bChecked);
    std::vector<TreeGridNode*> GetCheckedNodes() const;

public:
    /* ---- 事件 ---- */
    void AttachNodeExpanded(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventExpand, callback, callbackID); }
    void AttachNodeCollapsed(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventCollapse, callback, callbackID); }
    void AttachNodeClick(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventClick, callback, callbackID); }
    void AttachNodeSelect(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventSelect, callback, callbackID); }
    void AttachNodeUnSelect(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventUnSelect, callback, callbackID); }
    void AttachSelectChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventSelChanged, callback, callbackID); }
    void AttachCheckChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventTreeGridCheckChanged, callback, callbackID); }

private:
    /** 内部：把已存在的节点插入到指定父节点下（供 TreeGridNode::AddChildNode 使用）
    */
    bool InsertNode(TreeGridNode* pParent, TreeGridNode* pNode);
    /** 内部：保证列数不少于 nCount（不足时补默认列）
    */
    void EnsureColumnCount(size_t nCount);
    /** 内部：列定义变化后，重建表头并刷新所有行
    */
    void RefreshHeaderAndRows();
    /** 内部：按当前列定义刷新全部行的显示
    */
    void RefreshAllRows();
    /** 内部：移除某父节点的全部子节点
    */
    void RemoveAllNodesOf(TreeGridNode* pParent);
    /** 内部：重建表头
    */
    void RebuildHeader();
    /** 内部：重建所有数据行（结构变化时调用）
    */
    void RebuildRows();
    /** 内部：节点结构发生变化（增删节点）后刷新行
    */
    void OnNodeStructureChanged();
    /** 内部：为一个节点创建行控件（并追加到列表体）
    */
    TreeGridRow* CreateRow(TreeGridNode* pNode);
    /** 内部：刷新一个节点的行（文本 / 缩进 / 展开开关 / 复选框 / 可见性）
    */
    void UpdateRow(TreeGridNode* pNode);
    /** 内部：根据祖先展开状态递归刷新子孙行的可见性
    */
    void UpdateDescendantVisible(TreeGridNode* pNode);
    /** 内部：刷新单元格宽度（列宽变化 / DPI 变化后调用）
    */
    void UpdateCellWidths();
    /** 内部：刷新一个单元格的文本
    */
    void UpdateCellText(TreeGridNode* pNode, size_t nColumnId, const DString& text);
    /** 内部：计算一个节点的缩进占位宽度
     *  叶节点没有展开开关（隐藏后不占宽度），此处补足展开开关的占位宽度（原始像素），
     *  以保证同一层级的有子节点行与叶行的复选框/文本左对齐。
     */
    int32_t GetTreeIndentWidth(TreeGridNode* pNode) const;
    /** 内部：刷新缩进占位宽度
    */
    void UpdateRowIndent(TreeGridNode* pNode);
    /** 内部：刷新全部行的缩进占位宽度
    */
    void UpdateAllRowIndent();
    /** 内部：刷新全部行的行高
    */
    void UpdateAllRowHeight();
    /** 内部：刷新表头右侧的滚动条预留宽度
    */
    void UpdateHeaderPadding();
    /** 内部：把列对齐字符串归一化为 text_align 可用的取值
    */
    DString AlignText(const DString& strAlign) const;
    /** 内部：刷新行复选框显示状态
    */
    void UpdateCheckBoxDisplay(TreeGridNode* pNode);
    /** 内部：向上回溯修正祖先的勾选状态
    */
    void UpdateParentCheckState(TreeGridNode* pNode, bool bTriggerEvent);
    /** 内部：递归设置子孙勾选状态
    */
    void SetChildrenCheckState(TreeGridNode* pNode, bool bChecked, bool bTriggerEvent);
    /** 内部：从列表体的选择状态同步到节点
    */
    void SyncSelectedFromBody();
    /** 内部：获取节点在列表体中的索引
    */
    size_t GetRowIndex(TreeGridNode* pNode) const;
    /** 内部：按列ID查找列索引
    */
    size_t GetColumnIndexById(size_t nColumnId) const;
    /** 内部：取节点对应的行控件
    */
    TreeGridRow* GetRow(TreeGridNode* pNode) const;
    /** 内部：表头右侧预留滚动条宽度
    */
    int32_t GetScrollBarReserveWidth() const;
    /** 内部：列表体选择事件（转发为 TreeGrid 自身的 kEventSelect，pEventData 为节点） */
    void OnBodyItemSelected(const EventArgs& msg);
    /** 内部：列表体取消选择事件 */
    void OnBodyItemUnSelected(const EventArgs& msg);
    /** 内部：列表体选择状态变化事件 */
    void OnBodySelChanged(const EventArgs& msg);
    /** 内部：列表体键盘事件（Left / Right 收起 / 展开当前节点） */
    bool OnBodyKeyDown(const EventArgs& msg);
    /** 内部：行控件被点击（转发为 TreeGrid 自身的 kEventClick，pEventData 为节点） */
    void OnRowClicked(TreeGridNode* pNode);
    /** 内部：展开开关被切换 */
    void OnExpandToggle(TreeGridNode* pNode, bool bExpand);
    /** 内部：行复选框被切换 */
    void OnCheckToggle(TreeGridNode* pNode, bool bChecked);

    /** 内部：判断节点的行是否应当可见（其所有祖先都处于展开状态）
    */
    bool IsNodeRowVisible(TreeGridNode* pNode) const;
    /** 内部：取节点的行高（逐行行高优先，否则用全局值）
    */
    int32_t GetNodeRowHeight(TreeGridNode* pNode) const;
    /** 内部：派发复选框状态变化事件
    */
    void FireCheckEvent(TreeGridNode* pNode);

private:
    friend class TreeGridNode;

    /** 表头控件（HBox） */
    Box* m_pHeader = nullptr;
    /** 列表体控件（VListBox） */
    VListBox* m_pListBox = nullptr;
    /** 虚拟根节点（不可见） */
    TreeGridNode* m_pRootNode = nullptr;

    /** 列定义 */
    std::vector<TreeGridColumn> m_columns;
    /** 数据行对应的节点（索引与列表体的项索引一致） */
    std::vector<TreeGridNode*> m_rowNodes;

    /** 默认列宽（原始像素） */
    int32_t m_nDefaultColumnWidth = 120;
    /** 树列所在的列索引（承载缩进与展开开关） */
    size_t m_nTreeColumn = 0;
    /** 每层缩进量（原始像素） */
    int32_t m_nIndent = 20;
    /** 表头高度（原始像素） */
    int32_t m_nHeaderHeight = 30;
    /** 数据行高（原始像素） */
    int32_t m_nRowHeight = 28;
    /** 是否显示表头 */
    bool m_bShowHeader = true;
    /** 是否多选 */
    bool m_bMultiSelect = false;
    /** 是否显示行复选框 */
    bool m_bCheckMode = false;
    /** 复选框是否父子级联 */
    bool m_bCheckCascade = true;

    /** 展开开关引用的皮肤 Class */
    DString m_expandClass = _T("tree_grid_expand");
    /** 行复选框引用的皮肤 Class */
    DString m_checkClass = _T("tree_grid_check");
    /** 表头标题单元格引用的皮肤 Class */
    DString m_headerCellClass = _T("tree_grid_header_cell");
    /** 数据单元格引用的皮肤 Class */
    DString m_cellClass = _T("tree_grid_cell");
    /** 数据行引用的皮肤 Class */
    DString m_rowClass = _T("tree_grid_row");
};

} // namespace ui

#endif // DUI_TREE_GRID_H_
