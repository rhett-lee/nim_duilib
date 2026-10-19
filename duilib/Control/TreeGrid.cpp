#include "TreeGrid.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/ListBox.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/CheckBox.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/GlobalManager.h"
#include <functional>

namespace ui
{

//展开 / 收起开关的宽度（原始像素，做 DPI 缩放）
static constexpr int32_t kTreeGridExpandWidth = 16;
//行首复选框的宽度（原始像素，做 DPI 缩放）
static constexpr int32_t kTreeGridCheckWidth = 16;
//找不到滚动条时，表头右侧预留宽度的默认值（原始像素，做 DPI 缩放）
static constexpr int32_t kTreeGridReserveWidth = 16;
//展开/收起开关与行首复选框之间的间隙（原始像素，做 DPI 缩放）
static constexpr int32_t kTreeGridExpandCheckGap = 4;
//行首复选框与文本之间的间隙（原始像素，做 DPI 缩放）
static constexpr int32_t kTreeGridCheckTextGap = 4;

/** 列表体的实现类：用于在键盘事件到达行控件时，先截获 Left / Right 键（收起 / 展开节点）
*/
class TreeGridBodyBox : public VListBox
{
public:
    explicit TreeGridBodyBox(Window* pWindow) :
        VListBox(pWindow)
    {
    }

    /** 键盘事件处理回调（返回 true 表示已消费该事件） */
    std::function<bool(const EventArgs&)> m_onKeyDown;

    /** 判断某一项是否处于选择态（开放 ListBox 的 protected 接口） */
    bool IsRowSelected(size_t iIndex) const { return IsItemSelected(iIndex); }

protected:
    virtual bool OnListBoxItemKeyEvent(const EventArgs& msg) override
    {
        if ((msg.eventType == kEventKeyDown) && m_onKeyDown) {
            if (m_onKeyDown(msg)) {
                return true;
            }
        }
        return false;
    }
};

const size_t TreeGridNode::npos;

//////////////////////////////////////////////////////////////////////////
// TreeGridRow

TreeGridRow::TreeGridRow(Window* pWindow) :
    ListBoxItemH(pWindow)
{
}

TreeGridRow::~TreeGridRow()
{
}

//////////////////////////////////////////////////////////////////////////
// TreeGridNode

TreeGridNode::TreeGridNode(TreeGrid* pTreeGrid) :
    m_pTreeGrid(pTreeGrid)
{
}

TreeGridNode::~TreeGridNode()
{
    //递归释放所有子节点
    for (TreeGridNode* pChild : m_childNodes) {
        delete pChild;
    }
    m_childNodes.clear();
}

TreeGridNode* TreeGridNode::AddChildNode(TreeGridNode* pNode)
{
    if ((m_pTreeGrid == nullptr) || (pNode == nullptr)) {
        return nullptr;
    }
    if (m_pTreeGrid->InsertNode(this, pNode)) {
        return pNode;
    }
    return nullptr;
}

bool TreeGridNode::RemoveChildNode(TreeGridNode* pNode)
{
    if ((m_pTreeGrid == nullptr) || (pNode == nullptr)) {
        return false;
    }
    return m_pTreeGrid->RemoveNode(pNode);
}

void TreeGridNode::RemoveAllChildNodes()
{
    if (m_pTreeGrid == nullptr) {
        return;
    }
    m_pTreeGrid->RemoveAllNodesOf(this);
}

TreeGridNode* TreeGridNode::GetChildNode(size_t iIndex) const
{
    if (iIndex < m_childNodes.size()) {
        return m_childNodes[iIndex];
    }
    return nullptr;
}

size_t TreeGridNode::GetChildNodeIndex(TreeGridNode* pNode) const
{
    for (size_t i = 0; i < m_childNodes.size(); ++i) {
        if (m_childNodes[i] == pNode) {
            return i;
        }
    }
    return TreeGridNode::npos;
}

size_t TreeGridNode::GetDescendantNodeCount() const
{
    size_t nCount = 0;
    for (TreeGridNode* pChild : m_childNodes) {
        if (pChild != nullptr) {
            nCount += 1 + pChild->GetDescendantNodeCount();
        }
    }
    return nCount;
}

bool TreeGridNode::IsChildOf(TreeGridNode* pNode) const
{
    TreeGridNode* pParent = m_pParentNode;
    while (pParent != nullptr) {
        if (pParent == pNode) {
            return true;
        }
        pParent = pParent->m_pParentNode;
    }
    return false;
}

bool TreeGridNode::SetExpand(bool bExpand)
{
    if (m_pTreeGrid == nullptr) {
        return false;
    }
    return m_pTreeGrid->SetNodeExpand(this, bExpand, true);
}

bool TreeGridNode::SetChecked(bool bChecked)
{
    if (m_pTreeGrid == nullptr) {
        return false;
    }
    return m_pTreeGrid->SetNodeChecked(this, bChecked, true);
}

TreeGridCheck TreeGridNode::GetCheckState() const
{
    if (m_pTreeGrid == nullptr) {
        return TreeGridCheck::UnCheck;
    }
    return m_pTreeGrid->GetNodeCheckState(const_cast<TreeGridNode*>(this));
}

bool TreeGridNode::SetSelected(bool bSelected)
{
    if (m_pTreeGrid == nullptr) {
        return false;
    }
    return m_pTreeGrid->SelectNode(this, bSelected, true);
}

bool TreeGridNode::SetCellText(size_t nColumnId, const DString& text)
{
    if (m_pTreeGrid == nullptr) {
        m_cellTexts[nColumnId] = text;
        return true;
    }
    return m_pTreeGrid->SetCellText(this, nColumnId, text);
}

DString TreeGridNode::GetCellText(size_t nColumnId) const
{
    auto iter = m_cellTexts.find(nColumnId);
    if (iter != m_cellTexts.end()) {
        return iter->second;
    }
    return DString();
}

//////////////////////////////////////////////////////////////////////////
// TreeGrid

TreeGrid::TreeGrid(Window* pWindow) :
    BaseClass(pWindow),
    m_pHeader(nullptr),
    m_pListBox(nullptr),
    m_pRootNode(nullptr),
    m_columns(),
    m_rowNodes()
{
    m_pRootNode = new TreeGridNode(this);
}

TreeGrid::~TreeGrid()
{
    if (!IsInited()) {
        //未初始化时，内部子控件尚未创建，此处无需处理
        m_pHeader = nullptr;
        m_pListBox = nullptr;
    }
    if (m_pRootNode != nullptr) {
        delete m_pRootNode;
        m_pRootNode = nullptr;
    }
}

DString TreeGrid::GetType() const
{
    return DUI_CTR_TREE_GRID;
}

void TreeGrid::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //表头（固定不滚动）
    m_pHeader = new HBox(GetWindow());
    m_pHeader->SetFixedHeight(UiFixedInt(m_nHeaderHeight), true, true);
    m_pHeader->SetVisible(m_bShowHeader);
    AddItem(m_pHeader);

    //列表体（自带纵向滚动条）
    TreeGridBodyBox* pBody = new TreeGridBodyBox(GetWindow());
    pBody->m_onKeyDown = [this](const EventArgs& msg) { return OnBodyKeyDown(msg); };
    m_pListBox = pBody;
    m_pListBox->SetFixedHeight(UiFixedInt::MakeStretch(), true, false);
    m_pListBox->EnableScrollBar(true, false);
    AddItem(m_pListBox);

    //列表体的选择 / 键盘事件，转发到 TreeGrid 自身
    const EventCallbackID callbackID = (EventCallbackID)(Control*)this;
    m_pListBox->AttachSelect([this](const EventArgs& msg) { OnBodyItemSelected(msg); return true; }, callbackID);
    m_pListBox->AttachUnSelect([this](const EventArgs& msg) { OnBodyItemUnSelected(msg); return true; }, callbackID);
    m_pListBox->AttachSelChanged([this](const EventArgs& msg) { OnBodySelChanged(msg); return true; }, callbackID);

    RebuildHeader();
    RebuildRows();
}

void TreeGrid::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("column_title")) {
        //列标题列表（`|` 分隔）
        std::vector<DString> items;
        for (const DString& item : StringUtil::Split(strValue, _T("|"))) {
            items.push_back(item);
        }
        EnsureColumnCount(items.size());
        for (size_t i = 0; i < items.size(); ++i) {
            m_columns[i].strTitle = items[i];
        }
        RefreshHeaderAndRows();
    }
    else if (strName == _T("column_width")) {
        //列宽列表（`|` 分隔，单位像素）
        std::vector<DString> items;
        for (const DString& item : StringUtil::Split(strValue, _T("|"))) {
            items.push_back(item);
        }
        EnsureColumnCount(items.size());
        for (size_t i = 0; i < items.size(); ++i) {
            int32_t nWidth = StringUtil::StringToInt32(items[i]);
            if ((nWidth > 0) || items[i].empty()) {
                m_columns[i].nWidth = nWidth > 0 ? nWidth : m_nDefaultColumnWidth;
            }
        }
        RefreshHeaderAndRows();
    }
    else if (strName == _T("column_id")) {
        //列ID列表（`|` 分隔，缺省按列索引）
        std::vector<DString> items;
        for (const DString& item : StringUtil::Split(strValue, _T("|"))) {
            items.push_back(item);
        }
        EnsureColumnCount(items.size());
        for (size_t i = 0; i < items.size(); ++i) {
            if (!items[i].empty()) {
                m_columns[i].nColumnId = (size_t)StringUtil::StringToInt32(items[i]);
            }
        }
        RefreshHeaderAndRows();
    }
    else if (strName == _T("column_align")) {
        //列对齐列表（`|` 分隔：left / center / right）
        std::vector<DString> items;
        for (const DString& item : StringUtil::Split(strValue, _T("|"))) {
            items.push_back(item);
        }
        EnsureColumnCount(items.size());
        for (size_t i = 0; i < items.size(); ++i) {
            if (!items[i].empty()) {
                m_columns[i].strAlign = items[i];
            }
        }
        RefreshHeaderAndRows();
    }
    else if (strName == _T("default_column_width")) {
        m_nDefaultColumnWidth = StringUtil::StringToInt32(strValue);
    }
    else if (strName == _T("tree_column")) {
        m_nTreeColumn = (size_t)StringUtil::StringToInt32(strValue);
    }
    else if (strName == _T("indent")) {
        int32_t nIndent = StringUtil::StringToInt32(strValue);
        if (nIndent >= 0) {
            m_nIndent = nIndent;
        }
        UpdateAllRowIndent();
    }
    else if (strName == _T("header_height")) {
        int32_t nHeight = StringUtil::StringToInt32(strValue);
        if (nHeight >= 0) {
            m_nHeaderHeight = nHeight;
        }
        if (m_pHeader != nullptr) {
            m_pHeader->SetFixedHeight(UiFixedInt(m_nHeaderHeight), true, true);
        }
    }
    else if (strName == _T("row_height")) {
        int32_t nHeight = StringUtil::StringToInt32(strValue);
        if (nHeight >= 0) {
            m_nRowHeight = nHeight;
        }
        UpdateAllRowHeight();
    }
    else if (strName == _T("show_header")) {
        m_bShowHeader = StringUtil::IsValueTrue(strValue);
        if (m_pHeader != nullptr) {
            m_pHeader->SetVisible(m_bShowHeader);
        }
    }
    else if (strName == _T("multi_select")) {
        SetMultiSelect(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("check_mode")) {
        SetCheckMode(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("check_cascade")) {
        m_bCheckCascade = StringUtil::IsValueTrue(strValue);
    }
    else if (strName == _T("expand_image_class")) {
        m_expandClass = strValue;
        RefreshAllRows();
    }
    else if (strName == _T("check_image_class")) {
        m_checkClass = strValue;
        RefreshAllRows();
    }
    else if (strName == _T("header_cell_class")) {
        m_headerCellClass = strValue;
        RebuildHeader();
    }
    else if (strName == _T("cell_class")) {
        m_cellClass = strValue;
        RefreshAllRows();
    }
    else if (strName == _T("row_class")) {
        m_rowClass = strValue;
        RefreshAllRows();
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void TreeGrid::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    //重新计算表头右侧的滚动条预留宽度
    UpdateHeaderPadding();
}

void TreeGrid::EnsureColumnCount(size_t nCount)
{
    while (m_columns.size() < nCount) {
        TreeGridColumn column;
        column.nColumnId = m_columns.size();
        column.nWidth = m_nDefaultColumnWidth;
        column.strAlign = _T("left");
        m_columns.push_back(column);
    }
}

void TreeGrid::RefreshHeaderAndRows()
{
    if (!IsInited()) {
        return;
    }
    RebuildHeader();
    //列数可能变化，重建所有行以保证单元格数量一致
    RebuildRows();
}

void TreeGrid::RefreshAllRows()
{
    if (!IsInited()) {
        return;
    }
    for (TreeGridNode* pNode : m_rowNodes) {
        UpdateRow(pNode);
    }
}

//////////////////////////////////////////////////////////////////////////
// 列管理

void TreeGrid::AddColumn(const TreeGridColumn& column)
{
    m_columns.push_back(column);
    RefreshHeaderAndRows();
}

bool TreeGrid::GetColumn(size_t nColumnIndex, TreeGridColumn& column) const
{
    if (nColumnIndex < m_columns.size()) {
        column = m_columns[nColumnIndex];
        return true;
    }
    return false;
}

bool TreeGrid::SetColumnWidthById(size_t nColumnId, int32_t nWidth)
{
    if (nWidth <= 0) {
        return false;
    }
    size_t nIndex = GetColumnIndexById(nColumnId);
    if (nIndex == TreeGridNode::npos) {
        return false;
    }
    m_columns[nIndex].nWidth = nWidth;
    UpdateCellWidths();
    return true;
}

int32_t TreeGrid::GetColumnWidthById(size_t nColumnId) const
{
    size_t nIndex = GetColumnIndexById(nColumnId);
    if (nIndex == TreeGridNode::npos) {
        return 0;
    }
    return m_columns[nIndex].nWidth;
}

bool TreeGrid::SetColumnTitleById(size_t nColumnId, const DString& title)
{
    size_t nIndex = GetColumnIndexById(nColumnId);
    if (nIndex == TreeGridNode::npos) {
        return false;
    }
    m_columns[nIndex].strTitle = title;
    RebuildHeader();
    return true;
}

void TreeGrid::ClearColumns()
{
    m_columns.clear();
    RefreshHeaderAndRows();
}

size_t TreeGrid::GetColumnIndexById(size_t nColumnId) const
{
    for (size_t i = 0; i < m_columns.size(); ++i) {
        if (m_columns[i].nColumnId == nColumnId) {
            return i;
        }
    }
    return TreeGridNode::npos;
}

//////////////////////////////////////////////////////////////////////////
// 表头

void TreeGrid::RebuildHeader()
{
    if (m_pHeader == nullptr) {
        return;
    }
    m_pHeader->RemoveAllItems();
    for (size_t i = 0; i < m_columns.size(); ++i) {
        const TreeGridColumn& column = m_columns[i];
        Label* pLabel = new Label(GetWindow());
        pLabel->SetClass(m_headerCellClass);
        pLabel->SetFixedWidth(UiFixedInt(column.nWidth), true, true);
        pLabel->SetSingleLine(true);
        //列标题：优先按语言 ID 解析，找不到时按字面文本使用
        DString strTitle = column.strTitle;
        if (!strTitle.empty()) {
            LangManager& lang = GlobalManager::Instance().Lang();
            if (lang.HasStringByID(strTitle)) {
                strTitle = lang.GetStringByID(strTitle);
            }
        }
        pLabel->SetText(strTitle);
        pLabel->SetAttribute(_T("text_align"), _T("vcenter,") + AlignText(column.strAlign));
        m_pHeader->AddItem(pLabel);
    }
    UpdateHeaderPadding();
}

void TreeGrid::UpdateHeaderPadding()
{
    if (m_pHeader == nullptr) {
        return;
    }
    //表头右侧预留滚动条宽度，避免末列与滚动条重叠
    m_pHeader->SetPadding(UiPadding(0, 0, GetScrollBarReserveWidth(), 0), false);
}

int32_t TreeGrid::GetScrollBarReserveWidth() const
{
    int32_t nWidth = 0;
    if (m_pListBox != nullptr) {
        ScrollBar* pVScrollBar = m_pListBox->GetVScrollBar();
        if (pVScrollBar != nullptr) {
            UiRect rc = pVScrollBar->GetPos();
            nWidth = rc.right - rc.left;
        }
    }
    if (nWidth <= 0) {
        nWidth = Dpi().GetScaleInt(kTreeGridReserveWidth);
    }
    return nWidth;
}

//////////////////////////////////////////////////////////////////////////
// 节点管理

bool TreeGrid::InsertNode(TreeGridNode* pParent, TreeGridNode* pNode)
{
    if ((pParent == nullptr) || (pNode == nullptr)) {
        return false;
    }
    //如果该节点已挂在其它父节点下，先从原父节点上摘除
    if (pNode->m_pParentNode != nullptr) {
        std::vector<TreeGridNode*>& siblings = pNode->m_pParentNode->m_childNodes;
        for (auto iter = siblings.begin(); iter != siblings.end(); ++iter) {
            if (*iter == pNode) {
                siblings.erase(iter);
                break;
            }
        }
    }
    pNode->m_pParentNode = pParent;
    pParent->m_childNodes.push_back(pNode);
    //递归更新深度
    std::function<void(TreeGridNode*, size_t)> updateDepth = [&updateDepth](TreeGridNode* node, size_t depth) {
        node->m_nDepth = depth;
        for (TreeGridNode* pChild : node->m_childNodes) {
            updateDepth(pChild, depth + 1);
        }
    };
    updateDepth(pNode, pParent->m_nDepth + 1);
    OnNodeStructureChanged();
    return true;
}

TreeGridNode* TreeGrid::AddNode(TreeGridNode* pParent)
{
    TreeGridNode* pNode = new TreeGridNode(this);
    TreeGridNode* pInsertParent = (pParent != nullptr) ? pParent : m_pRootNode;
    if (!InsertNode(pInsertParent, pNode)) {
        delete pNode;
        return nullptr;
    }
    return pNode;
}

bool TreeGrid::RemoveNode(TreeGridNode* pNode)
{
    if ((pNode == nullptr) || (pNode == m_pRootNode) || (pNode->m_pParentNode == nullptr)) {
        return false;
    }
    std::vector<TreeGridNode*>& siblings = pNode->m_pParentNode->m_childNodes;
    for (auto iter = siblings.begin(); iter != siblings.end(); ++iter) {
        if (*iter == pNode) {
            siblings.erase(iter);
            break;
        }
    }
    pNode->m_pParentNode = nullptr;
    delete pNode;
    OnNodeStructureChanged();
    return true;
}

void TreeGrid::RemoveAllNodes()
{
    RemoveAllNodesOf(m_pRootNode);
}

void TreeGrid::RemoveAllNodesOf(TreeGridNode* pParent)
{
    if (pParent == nullptr) {
        return;
    }
    for (TreeGridNode* pChild : pParent->m_childNodes) {
        pChild->m_pParentNode = nullptr;
        delete pChild;
    }
    pParent->m_childNodes.clear();
    OnNodeStructureChanged();
}

void TreeGrid::OnNodeStructureChanged()
{
    if (!IsInited()) {
        return;
    }
    RebuildRows();
}

//////////////////////////////////////////////////////////////////////////
// 数据设置

bool TreeGrid::SetCellText(TreeGridNode* pNode, size_t nColumnId, const DString& text)
{
    if ((pNode == nullptr) || (pNode->m_pTreeGrid != this)) {
        return false;
    }
    pNode->m_cellTexts[nColumnId] = text;
    //仅刷新目标单元格
    UpdateCellText(pNode, nColumnId, text);
    return true;
}

DString TreeGrid::GetCellText(TreeGridNode* pNode, size_t nColumnId) const
{
    if (pNode == nullptr) {
        return DString();
    }
    return pNode->GetCellText(nColumnId);
}

void TreeGrid::UpdateCellText(TreeGridNode* pNode, size_t nColumnId, const DString& text)
{
    size_t nColumnIndex = GetColumnIndexById(nColumnId);
    if (nColumnIndex == TreeGridNode::npos) {
        return;
    }
    TreeGridRow* pRow = GetRow(pNode);
    if ((pRow == nullptr) || (nColumnIndex >= pRow->m_cellLabels.size())) {
        return;
    }
    Label* pLabel = pRow->m_cellLabels[nColumnIndex];
    if (pLabel != nullptr) {
        pLabel->SetText(text);
    }
}

bool TreeGrid::SetNodeRowHeight(TreeGridNode* pNode, int32_t nHeight)
{
    if ((pNode == nullptr) || (pNode->m_pTreeGrid != this)) {
        return false;
    }
    pNode->m_nRowHeight = nHeight;
    UpdateRow(pNode);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// 行构建与刷新

void TreeGrid::RebuildRows()
{
    if (m_pListBox == nullptr) {
        return;
    }
    for (TreeGridNode* pNode : m_rowNodes) {
        if (pNode != nullptr) {
            pNode->m_pRow = nullptr;
            pNode->m_nRowIndex = 0;
        }
    }
    m_rowNodes.clear();
    m_pListBox->RemoveAllItems();

    if (m_pRootNode != nullptr) {
        std::function<void(TreeGridNode*)> walk = [&walk, this](TreeGridNode* pParent) {
            for (TreeGridNode* pChild : pParent->m_childNodes) {
                if (pChild == nullptr) {
                    continue;
                }
                TreeGridRow* pRow = CreateRow(pChild);
                if (pRow != nullptr) {
                    pChild->m_pRow = pRow;
                    pChild->m_nRowIndex = m_rowNodes.size();
                    m_rowNodes.push_back(pChild);
                    m_pListBox->AddItem(pRow);
                    UpdateRow(pChild);
                }
                walk(pChild);
            }
        };
        walk(m_pRootNode);
    }

    //恢复选中状态
    for (size_t i = 0; i < m_rowNodes.size(); ++i) {
        TreeGridNode* pNode = m_rowNodes[i];
        if ((pNode != nullptr) && pNode->m_bSelected) {
            m_pListBox->SelectItem(i, false, false);
        }
    }
}

TreeGridRow* TreeGrid::CreateRow(TreeGridNode* pNode)
{
    TreeGridRow* pRow = new TreeGridRow(GetWindow());
    pRow->SetNode(pNode);
    pRow->SetClass(m_rowClass);
    pRow->SetFixedHeight(UiFixedInt(GetNodeRowHeight(pNode)), true, true);
    pRow->m_cellControls.resize(m_columns.size(), nullptr);
    pRow->m_cellLabels.resize(m_columns.size(), nullptr);

    for (size_t i = 0; i < m_columns.size(); ++i) {
        const TreeGridColumn& column = m_columns[i];
        if (i == m_nTreeColumn) {
            //树列：单元格内依次为 [缩进占位][展开开关][复选框][文本]
            HBox* pCell = new HBox(GetWindow());
            pCell->SetFixedWidth(UiFixedInt(column.nWidth), true, true);
            //整行内容在行高内纵向居中
            pCell->SetVerAlignType(VerAlignType::kAlignCenter);
            //单元格容器不拦截鼠标，点击穿透到行控件，以便触发行选择；
            //展开开关/行首复选框仍保持可响应鼠标，因此它们仍可正常点击
            pCell->SetMouseEnabled(false);

            Control* pIndent = new Control(GetWindow());
            pIndent->SetFixedWidth(UiFixedInt(GetTreeIndentWidth(pNode)), true, true);
            //缩进占位控件不拦截鼠标，点击穿透到行控件，以便触发行选择
            pIndent->SetMouseEnabled(false);
            pCell->AddItem(pIndent);

            CheckBox* pExpand = new CheckBox(GetWindow());
            pExpand->SetClass(m_expandClass);
            pExpand->SetSupportCheckMode(true);
            pExpand->SetVerAlignType(VerAlignType::kAlignCenter);
            pExpand->SetFixedWidth(UiFixedInt(kTreeGridExpandWidth), true, true);
            pExpand->SetVisible(false);
            pExpand->AttachCheck([this, pNode](const EventArgs&) { OnExpandToggle(pNode, true); return true; });
            pExpand->AttachUnCheck([this, pNode](const EventArgs&) { OnExpandToggle(pNode, false); return true; });
            pCell->AddItem(pExpand);

            CheckBox* pCheck = new CheckBox(GetWindow());
            pCheck->SetClass(m_checkClass);
            pCheck->SetSupportCheckMode(true);
            pCheck->SetVerAlignType(VerAlignType::kAlignCenter);
            //左侧留出与展开开关的间隙，右侧留出与文本的间隙
            pCheck->SetMargin(UiMargin(kTreeGridExpandCheckGap, 0, kTreeGridCheckTextGap, 0), true);
            pCheck->SetFixedWidth(UiFixedInt(kTreeGridCheckWidth), true, true);
            pCheck->SetVisible(false);
            pCheck->AttachCheck([this, pNode](const EventArgs&) { OnCheckToggle(pNode, true); return true; });
            pCheck->AttachUnCheck([this, pNode](const EventArgs&) { OnCheckToggle(pNode, false); return true; });
            pCell->AddItem(pCheck);

            Label* pLabel = new Label(GetWindow());
            pLabel->SetClass(m_cellClass);
            pLabel->SetSingleLine(true);
            pLabel->SetAutoToolTip(true);
            pLabel->SetVerAlignType(VerAlignType::kAlignCenter);
            //文本不拦截鼠标，点击穿透到行控件，以便触发行选择
            pLabel->SetMouseEnabled(false);
            pCell->AddItem(pLabel);

            pRow->m_pTreeCell = pCell;
            pRow->m_pIndent = pIndent;
            pRow->m_pExpand = pExpand;
            pRow->m_pCheck = pCheck;
            pRow->m_cellControls[i] = pCell;
            pRow->m_cellLabels[i] = pLabel;
            pRow->AddItem(pCell);
        }
        else {
            //普通列：一个固定宽度 + 指定对齐的文本标签
            Label* pLabel = new Label(GetWindow());
            pLabel->SetClass(m_cellClass);
            pLabel->SetFixedWidth(UiFixedInt(column.nWidth), true, true);
            pLabel->SetSingleLine(true);
            pLabel->SetAutoToolTip(true);
            pLabel->SetVerAlignType(VerAlignType::kAlignCenter);
            pLabel->SetAttribute(_T("text_align"), _T("vcenter,") + AlignText(column.strAlign));
            //文本不拦截鼠标，点击穿透到行控件，以便触发行选择
            pLabel->SetMouseEnabled(false);
            pRow->m_cellControls[i] = pLabel;
            pRow->m_cellLabels[i] = pLabel;
            pRow->AddItem(pLabel);
        }
    }

    //点击行（非子控件区域）→ 派发 TreeGrid 自身的 kEventClick
    pRow->AttachClick([this, pNode](const EventArgs&) { OnRowClicked(pNode); return false; });
    return pRow;
}

void TreeGrid::UpdateRow(TreeGridNode* pNode)
{
    TreeGridRow* pRow = GetRow(pNode);
    if (pRow == nullptr) {
        return;
    }
    //行高
    pRow->SetFixedHeight(UiFixedInt(GetNodeRowHeight(pNode)), true, true);

    //各列文本
    for (size_t i = 0; i < m_columns.size(); ++i) {
        if (i < pRow->m_cellLabels.size()) {
            Label* pLabel = pRow->m_cellLabels[i];
            if (pLabel != nullptr) {
                pLabel->SetText(pNode->GetCellText(m_columns[i].nColumnId));
            }
        }
    }

    //展开开关（仅有子节点时显示）
    if (pRow->m_pExpand != nullptr) {
        const bool bHasChildren = pNode->GetChildNodeCount() > 0;
        pRow->m_pExpand->SetVisible(bHasChildren);
        if (bHasChildren) {
            pRow->m_pExpand->SetChecked(pNode->m_bExpand, false);
        }
    }
    //行首复选框（仅勾选模式显示）
    if (pRow->m_pCheck != nullptr) {
        pRow->m_pCheck->SetVisible(m_bCheckMode);
        if (m_bCheckMode) {
            UpdateCheckBoxDisplay(pNode);
        }
    }

    UpdateRowIndent(pNode);
    //可见性（VLayout 会自动跳过不可见子项）
    pRow->SetVisible(IsNodeRowVisible(pNode));
}

int32_t TreeGrid::GetTreeIndentWidth(TreeGridNode* pNode) const
{
    if (pNode == nullptr) {
        return 0;
    }
    //基础缩进：按节点层级递增
    int32_t nWidth = m_nIndent * (int32_t)pNode->m_nDepth;
    if (pNode->GetChildNodeCount() == 0) {
        //叶节点没有展开开关（隐藏后不占宽度），补足展开开关的占位宽度，
        //保证同一层级的有子节点行与叶行的复选框/文本左对齐。
        //注意：此处必须使用"原始像素"值（由 SetFixedWidth 内部统一做一次 DPI 缩放）；
        //不能使用 GetFixedWidth()，因为它返回的是已缩放过的值，会造成二次缩放。
        nWidth += kTreeGridExpandWidth;
    }
    return nWidth;
}

void TreeGrid::UpdateRowIndent(TreeGridNode* pNode)
{
    TreeGridRow* pRow = GetRow(pNode);
    if ((pRow == nullptr) || (pRow->m_pIndent == nullptr)) {
        return;
    }
    pRow->m_pIndent->SetFixedWidth(UiFixedInt(GetTreeIndentWidth(pNode)), true, true);
}

void TreeGrid::UpdateAllRowIndent()
{
    for (TreeGridNode* pNode : m_rowNodes) {
        UpdateRowIndent(pNode);
    }
}

void TreeGrid::UpdateAllRowHeight()
{
    for (TreeGridNode* pNode : m_rowNodes) {
        TreeGridRow* pRow = GetRow(pNode);
        if (pRow != nullptr) {
            pRow->SetFixedHeight(UiFixedInt(GetNodeRowHeight(pNode)), true, true);
        }
    }
}

void TreeGrid::UpdateCellWidths()
{
    //表头
    if (m_pHeader != nullptr) {
        for (size_t i = 0; i < m_columns.size(); ++i) {
            Control* pControl = m_pHeader->GetItemAt(i);
            if (pControl != nullptr) {
                pControl->SetFixedWidth(UiFixedInt(m_columns[i].nWidth), true, true);
            }
        }
    }
    //行
    for (TreeGridNode* pNode : m_rowNodes) {
        TreeGridRow* pRow = GetRow(pNode);
        if (pRow == nullptr) {
            continue;
        }
        for (size_t i = 0; i < m_columns.size(); ++i) {
            if (i < pRow->m_cellControls.size()) {
                Control* pCell = pRow->m_cellControls[i];
                if (pCell != nullptr) {
                    pCell->SetFixedWidth(UiFixedInt(m_columns[i].nWidth), true, true);
                }
            }
        }
    }
}

void TreeGrid::UpdateDescendantVisible(TreeGridNode* pNode)
{
    if (pNode == nullptr) {
        return;
    }
    for (TreeGridNode* pChild : pNode->m_childNodes) {
        if (pChild == nullptr) {
            continue;
        }
        TreeGridRow* pRow = GetRow(pChild);
        if (pRow != nullptr) {
            pRow->SetVisible(IsNodeRowVisible(pChild));
        }
        UpdateDescendantVisible(pChild);
    }
}

bool TreeGrid::IsNodeRowVisible(TreeGridNode* pNode) const
{
    if (pNode == nullptr) {
        return false;
    }
    TreeGridNode* pParent = pNode->m_pParentNode;
    while ((pParent != nullptr) && (pParent != m_pRootNode)) {
        if (!pParent->m_bExpand) {
            return false;
        }
        pParent = pParent->m_pParentNode;
    }
    return true;
}

int32_t TreeGrid::GetNodeRowHeight(TreeGridNode* pNode) const
{
    if ((pNode != nullptr) && (pNode->m_nRowHeight > 0)) {
        return pNode->m_nRowHeight;
    }
    return m_nRowHeight;
}

TreeGridRow* TreeGrid::GetRow(TreeGridNode* pNode) const
{
    if ((pNode == nullptr) || (pNode->m_pRow == nullptr)) {
        return nullptr;
    }
    return static_cast<TreeGridRow*>(pNode->m_pRow);
}

size_t TreeGrid::GetRowIndex(TreeGridNode* pNode) const
{
    if (pNode == nullptr) {
        return TreeGridNode::npos;
    }
    if ((pNode->m_pRow != nullptr) && (pNode->m_nRowIndex < m_rowNodes.size()) &&
        (m_rowNodes[pNode->m_nRowIndex] == pNode)) {
        return pNode->m_nRowIndex;
    }
    for (size_t i = 0; i < m_rowNodes.size(); ++i) {
        if (m_rowNodes[i] == pNode) {
            return i;
        }
    }
    return TreeGridNode::npos;
}

DString TreeGrid::AlignText(const DString& strAlign) const
{
    if ((strAlign == _T("center")) || (strAlign == _T("hcenter"))) {
        return _T("hcenter");
    }
    if (strAlign == _T("right")) {
        return _T("right");
    }
    return _T("left");
}

//////////////////////////////////////////////////////////////////////////
// 展开 / 收起

bool TreeGrid::SetNodeExpand(TreeGridNode* pNode, bool bExpand, bool bTriggerEvent)
{
    if ((pNode == nullptr) || (pNode->m_pTreeGrid != this)) {
        return false;
    }
    if (pNode->GetChildNodeCount() == 0) {
        //无子节点，不做展开 / 收起
        return false;
    }
    if (pNode->m_bExpand == bExpand) {
        return false;
    }
    pNode->m_bExpand = bExpand;
    //刷新开关图片
    TreeGridRow* pRow = GetRow(pNode);
    if ((pRow != nullptr) && (pRow->m_pExpand != nullptr)) {
        pRow->m_pExpand->SetChecked(pNode->m_bExpand, false);
    }
    //按需刷新子孙行可见性
    UpdateDescendantVisible(pNode);

    if (bTriggerEvent) {
        if (bExpand) {
            SendEvent(kEventExpand, (WPARAM)pNode, 0, pNode);
        }
        else {
            SendEvent(kEventCollapse, (WPARAM)pNode, 0, pNode);
        }
    }
    return true;
}

bool TreeGrid::IsNodeExpand(TreeGridNode* pNode) const
{
    return (pNode != nullptr) && pNode->m_bExpand;
}

bool TreeGrid::ExpandNode(TreeGridNode* pNode)
{
    if ((pNode == nullptr) || (pNode->m_pTreeGrid != this) || (pNode == m_pRootNode)) {
        return false;
    }
    //级联展开祖先
    TreeGridNode* pParent = pNode->m_pParentNode;
    while ((pParent != nullptr) && (pParent != m_pRootNode)) {
        SetNodeExpand(pParent, true, true);
        pParent = pParent->m_pParentNode;
    }
    SetNodeExpand(pNode, true, true);
    return true;
}

bool TreeGrid::EnsureNodeVisible(TreeGridNode* pNode)
{
    if ((pNode == nullptr) || (m_pListBox == nullptr)) {
        return false;
    }
    size_t nIndex = GetRowIndex(pNode);
    if (nIndex == TreeGridNode::npos) {
        return false;
    }
    //先展开祖先，保证该行处于可见状态
    TreeGridNode* pParent = pNode->m_pParentNode;
    while ((pParent != nullptr) && (pParent != m_pRootNode)) {
        if (!pParent->m_bExpand) {
            SetNodeExpand(pParent, true, false);
        }
        pParent = pParent->m_pParentNode;
    }
    m_pListBox->EnsureVisible(nIndex);
    return true;
}

void TreeGrid::OnExpandToggle(TreeGridNode* pNode, bool bExpand)
{
    SetNodeExpand(pNode, bExpand, true);
}

//////////////////////////////////////////////////////////////////////////
// 选择

void TreeGrid::SetMultiSelect(bool bMultiSelect)
{
    m_bMultiSelect = bMultiSelect;
    if (m_pListBox != nullptr) {
        m_pListBox->SetMultiSelect(bMultiSelect);
    }
}

bool TreeGrid::IsMultiSelect() const
{
    return m_bMultiSelect;
}

size_t TreeGrid::GetSelectNodeCount() const
{
    return GetSelectNodes().size();
}

TreeGridNode* TreeGrid::GetSelectNodeAt(size_t nIndex) const
{
    std::vector<TreeGridNode*> nodes = GetSelectNodes();
    if (nIndex < nodes.size()) {
        return nodes[nIndex];
    }
    return nullptr;
}

std::vector<TreeGridNode*> TreeGrid::GetSelectNodes() const
{
    std::vector<TreeGridNode*> nodes;
    for (TreeGridNode* pNode : m_rowNodes) {
        if ((pNode != nullptr) && pNode->m_bSelected) {
            nodes.push_back(pNode);
        }
    }
    return nodes;
}

bool TreeGrid::IsNodeSelected(TreeGridNode* pNode) const
{
    return (pNode != nullptr) && pNode->m_bSelected;
}

bool TreeGrid::SelectNode(TreeGridNode* pNode, bool bSelect, bool bTriggerEvent)
{
    if ((pNode == nullptr) || (m_pListBox == nullptr)) {
        return false;
    }
    size_t nIndex = GetRowIndex(pNode);
    if (nIndex == TreeGridNode::npos) {
        return false;
    }
    if (bSelect) {
        m_pListBox->SelectItem(nIndex, false, bTriggerEvent);
        if (!m_bMultiSelect) {
            m_pListBox->EnsureSingleSelection();
        }
    }
    else {
        m_pListBox->UnSelectItem(nIndex, bTriggerEvent);
    }
    SyncSelectedFromBody();
    return true;
}

void TreeGrid::SelectAllNodes(bool bSelect)
{
    if (m_pListBox == nullptr) {
        return;
    }
    if (bSelect) {
        m_pListBox->SetSelectAll();
    }
    else {
        m_pListBox->SetSelectNone();
    }
    SyncSelectedFromBody();
}

void TreeGrid::ClearSelectNodes()
{
    SelectAllNodes(false);
}

void TreeGrid::SyncSelectedFromBody()
{
    if (m_pListBox == nullptr) {
        return;
    }
    TreeGridBodyBox* pBody = static_cast<TreeGridBodyBox*>(m_pListBox);
    for (size_t i = 0; i < m_rowNodes.size(); ++i) {
        TreeGridNode* pNode = m_rowNodes[i];
        if (pNode != nullptr) {
            pNode->m_bSelected = pBody->IsRowSelected(i);
        }
    }
}

void TreeGrid::OnBodyItemSelected(const EventArgs& msg)
{
    size_t nIndex = (size_t)msg.wParam;
    if (nIndex < m_rowNodes.size()) {
        TreeGridNode* pNode = m_rowNodes[nIndex];
        if (pNode != nullptr) {
            pNode->m_bSelected = true;
            SendEvent(kEventSelect, (WPARAM)pNode, (LPARAM)nIndex, pNode);
        }
    }
}

void TreeGrid::OnBodyItemUnSelected(const EventArgs& msg)
{
    size_t nIndex = (size_t)msg.wParam;
    if (nIndex < m_rowNodes.size()) {
        TreeGridNode* pNode = m_rowNodes[nIndex];
        if (pNode != nullptr) {
            pNode->m_bSelected = false;
            SendEvent(kEventUnSelect, (WPARAM)pNode, (LPARAM)nIndex, pNode);
        }
    }
}

void TreeGrid::OnBodySelChanged(const EventArgs&)
{
    SyncSelectedFromBody();
    SendEvent(kEventSelChanged);
}

bool TreeGrid::OnBodyKeyDown(const EventArgs& msg)
{
    if (!IsEnabled()) {
        return false;
    }
    if ((msg.vkCode != kVK_LEFT) && (msg.vkCode != kVK_RIGHT)) {
        return false;
    }
    //取当前选中节点（优先），否则取当前焦点行
    TreeGridNode* pNode = GetSelectNodeAt(0);
    if (pNode == nullptr) {
        size_t nCurSel = m_pListBox->GetCurSel();
        if (nCurSel < m_rowNodes.size()) {
            pNode = m_rowNodes[nCurSel];
        }
    }
    if (pNode == nullptr) {
        return false;
    }
    if (msg.vkCode == kVK_RIGHT) {
        //Right：展开当前节点（有子节点时）
        return SetNodeExpand(pNode, true, true);
    }
    //Left：收起当前节点；若已收起，则收起其父节点
    if (pNode->m_bExpand) {
        return SetNodeExpand(pNode, false, true);
    }
    TreeGridNode* pParent = pNode->m_pParentNode;
    if ((pParent != nullptr) && (pParent != m_pRootNode) && pParent->m_bExpand) {
        return SetNodeExpand(pParent, false, true);
    }
    return false;
}

void TreeGrid::OnRowClicked(TreeGridNode* pNode)
{
    if (pNode != nullptr) {
        SendEvent(kEventClick, (WPARAM)pNode, 0, pNode);
    }
}

//////////////////////////////////////////////////////////////////////////
// 复选框（级联）

void TreeGrid::SetCheckMode(bool bCheckMode)
{
    if (m_bCheckMode == bCheckMode) {
        return;
    }
    m_bCheckMode = bCheckMode;
    //刷新所有行的复选框显隐
    for (TreeGridNode* pNode : m_rowNodes) {
        TreeGridRow* pRow = GetRow(pNode);
        if ((pRow != nullptr) && (pRow->m_pCheck != nullptr)) {
            pRow->m_pCheck->SetVisible(m_bCheckMode);
            if (m_bCheckMode) {
                UpdateCheckBoxDisplay(pNode);
            }
        }
    }
}

bool TreeGrid::IsNodeChecked(TreeGridNode* pNode) const
{
    return (pNode != nullptr) && pNode->m_bChecked;
}

TreeGridCheck TreeGrid::GetNodeCheckState(TreeGridNode* pNode) const
{
    if (pNode == nullptr) {
        return TreeGridCheck::UnCheck;
    }
    if (pNode->m_bChecked) {
        return TreeGridCheck::CheckedAll;
    }
    if (pNode->m_bPartChecked) {
        return TreeGridCheck::CheckedPartly;
    }
    return TreeGridCheck::UnCheck;
}

bool TreeGrid::SetNodeChecked(TreeGridNode* pNode, bool bChecked, bool bTriggerEvent)
{
    if ((pNode == nullptr) || (pNode->m_pTreeGrid != this) || (pNode == m_pRootNode)) {
        return false;
    }
    if (!m_bCheckMode) {
        return false;
    }
    const bool bChanged = (pNode->m_bChecked != bChecked) || pNode->m_bPartChecked;
    pNode->m_bChecked = bChecked;
    pNode->m_bPartChecked = false;
    UpdateCheckBoxDisplay(pNode);
    if (bTriggerEvent && bChanged) {
        FireCheckEvent(pNode);
    }

    if (m_bCheckCascade) {
        SetChildrenCheckState(pNode, bChecked, bTriggerEvent);
        UpdateParentCheckState(pNode->m_pParentNode, bTriggerEvent);
    }
    return true;
}

void TreeGrid::SetChildrenCheckState(TreeGridNode* pNode, bool bChecked, bool bTriggerEvent)
{
    if (pNode == nullptr) {
        return;
    }
    for (TreeGridNode* pChild : pNode->m_childNodes) {
        if (pChild == nullptr) {
            continue;
        }
        const bool bChanged = (pChild->m_bChecked != bChecked) || pChild->m_bPartChecked;
        pChild->m_bChecked = bChecked;
        pChild->m_bPartChecked = false;
        UpdateCheckBoxDisplay(pChild);
        if (bTriggerEvent && bChanged) {
            FireCheckEvent(pChild);
        }
        SetChildrenCheckState(pChild, bChecked, bTriggerEvent);
    }
}

void TreeGrid::UpdateParentCheckState(TreeGridNode* pNode, bool bTriggerEvent)
{
    while ((pNode != nullptr) && (pNode != m_pRootNode)) {
        size_t nChildCount = pNode->m_childNodes.size();
        size_t nAll = 0;
        size_t nPart = 0;
        for (TreeGridNode* pChild : pNode->m_childNodes) {
            if (pChild == nullptr) {
                continue;
            }
            if (pChild->m_bPartChecked) {
                ++nPart;
            }
            else if (pChild->m_bChecked) {
                ++nAll;
            }
        }
        const bool bAllChecked = (nChildCount > 0) && (nAll == nChildCount);
        const bool bPartChecked = !bAllChecked && ((nAll > 0) || (nPart > 0));
        const bool bChanged = (pNode->m_bChecked != bAllChecked) || (pNode->m_bPartChecked != bPartChecked);
        pNode->m_bChecked = bAllChecked;
        pNode->m_bPartChecked = bPartChecked;
        UpdateCheckBoxDisplay(pNode);
        if (bTriggerEvent && bChanged) {
            FireCheckEvent(pNode);
        }
        pNode = pNode->m_pParentNode;
    }
}

void TreeGrid::UpdateCheckBoxDisplay(TreeGridNode* pNode)
{
    TreeGridRow* pRow = GetRow(pNode);
    if ((pRow == nullptr) || (pRow->m_pCheck == nullptr) || !m_bCheckMode) {
        return;
    }
    CheckBox* pCheckBox = pRow->m_pCheck;
    switch (GetNodeCheckState(pNode)) {
    case TreeGridCheck::CheckedAll:
        pCheckBox->SetChecked(true, false);
        pCheckBox->SetPartChecked(false);
        break;
    case TreeGridCheck::CheckedPartly:
        pCheckBox->SetChecked(false, false);
        pCheckBox->SetPartChecked(true);
        break;
    default:
        pCheckBox->SetChecked(false, false);
        pCheckBox->SetPartChecked(false);
        break;
    }
}

void TreeGrid::OnCheckToggle(TreeGridNode* pNode, bool bChecked)
{
    SetNodeChecked(pNode, bChecked, true);
}

void TreeGrid::FireCheckEvent(TreeGridNode* pNode)
{
    TreeGridCheckValue checkValue;
    checkValue.pNode = pNode;
    checkValue.bChecked = (pNode != nullptr) && pNode->m_bChecked;
    SendEvent(kEventTreeGridCheckChanged, (WPARAM)pNode, 0, &checkValue);
}

void TreeGrid::SetAllNodesChecked(bool bChecked)
{
    if (!m_bCheckMode || (m_pRootNode == nullptr)) {
        return;
    }
    for (TreeGridNode* pChild : m_pRootNode->m_childNodes) {
        if (pChild == nullptr) {
            continue;
        }
        pChild->m_bChecked = bChecked;
        pChild->m_bPartChecked = false;
        UpdateCheckBoxDisplay(pChild);
        SetChildrenCheckState(pChild, bChecked, false);
    }
}

std::vector<TreeGridNode*> TreeGrid::GetCheckedNodes() const
{
    std::vector<TreeGridNode*> nodes;
    std::function<void(TreeGridNode*)> walk = [&nodes, &walk](TreeGridNode* pParent) {
        for (TreeGridNode* pChild : pParent->m_childNodes) {
            if (pChild == nullptr) {
                continue;
            }
            if (pChild->m_bChecked && !pChild->m_bPartChecked) {
                nodes.push_back(pChild);
            }
            walk(pChild);
        }
    };
    if (m_pRootNode != nullptr) {
        walk(m_pRootNode);
    }
    return nodes;
}

} // namespace ui
