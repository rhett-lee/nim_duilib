#include "CascadingSelect.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>
#include <array>
#include <list>
#include <memory>

namespace ui
{

namespace
{
/** 禁用路径前缀匹配：prefix 是 path 的某一级前缀（以分隔符结尾）
 */
bool IsPathPrefix(const DString& prefix, const DString& path, const DString& strSeparator)
{
    if (prefix.empty() || path.empty() || strSeparator.empty()) {
        return false;
    }
    if (prefix.size() >= path.size()) {
        return false;
    }
    if (path.compare(0, prefix.size(), prefix) != 0) {
        return false;
    }
    return path.compare(prefix.size(), strSeparator.size(), strSeparator) == 0;
}

} // namespace

CascadingSelect::CascadingSelect(Window* pWindow) :
    BaseClass(pWindow)
{
    m_strPathSeparator = _T("/");
    m_strValueSeparator = _T(" / ");
    m_strFont = _T("system_regular_14");
    m_strItemColor = _T("text_default");
    m_strSelectedColor = _T("color_accent");
    m_strDisabledColor = _T("text_disabled");
    m_strHoverBgColor = _T("bg_list_item_hovered");
    m_strSelectedBgColor = _T("bg_list_item_selected");
    m_strDividerColor = _T("border_split_level1");
    m_strChevronColor = _T("color_gray");
    m_strScrollbarColor = _T("color_gray");
}

CascadingSelect::~CascadingSelect()
{
}

DString CascadingSelect::GetType() const
{
    return DUI_CTR_CASCADING_SELECT;
}

void CascadingSelect::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("items")) || (strName == _T("item_paths"))) {
        m_strIdPaths.clear();
        std::list<DString> parts = StringUtil::Split(strValue, _T("|"));
        std::vector<DString> paths;
        for (DString part : parts) {
            StringUtil::Trim(part);
            if (!part.empty()) {
                paths.push_back(part);
            }
        }
        SetItems(paths);
    }
    else if ((strName == _T("items_id")) || (strName == _T("item_paths_id"))) {
        std::list<DString> parts = StringUtil::Split(strValue, _T("|"));
        std::vector<DString> paths;
        for (DString part : parts) {
            StringUtil::Trim(part);
            if (!part.empty()) {
                paths.push_back(part);
            }
        }
        SetItemIds(paths);
    }
    else if ((strName == _T("path_separator")) || (strName == _T("pathseparator"))) {
        SetPathSeparator(strValue);
    }
    else if ((strName == _T("value_separator")) || (strName == _T("valueseparator"))) {
        SetValueSeparator(strValue);
    }
    else if ((strName == _T("column_width")) || (strName == _T("columnwidth"))) {
        SetColumnWidth(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("row_height")) || (strName == _T("rowheight"))) {
        SetRowHeight(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("text_padding")) || (strName == _T("textpadding"))) {
        SetTextPadding(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("disabled_paths")) || (strName == _T("disabledpaths"))) {
        std::list<DString> parts = StringUtil::Split(strValue, _T("|"));
        std::vector<DString> paths;
        for (DString part : parts) {
            StringUtil::Trim(part);
            if (!part.empty()) {
                paths.push_back(part);
            }
        }
        SetDisabledPaths(paths);
    }
    else if ((strName == _T("item_color")) || (strName == _T("itemcolor"))) {
        SetItemColor(strValue);
    }
    else if ((strName == _T("selected_color")) || (strName == _T("selectedcolor"))) {
        SetSelectedColor(strValue);
    }
    else if ((strName == _T("disabled_color")) || (strName == _T("disabledcolor"))) {
        SetDisabledColor(strValue);
    }
    else if ((strName == _T("hover_bg_color")) || (strName == _T("hoverbgcolor"))) {
        SetHoverBgColor(strValue);
    }
    else if ((strName == _T("selected_bg_color")) || (strName == _T("selectedbgcolor"))) {
        SetSelectedBgColor(strValue);
    }
    else if ((strName == _T("divider_color")) || (strName == _T("dividercolor"))) {
        SetDividerColor(strValue);
    }
    else if ((strName == _T("chevron_color")) || (strName == _T("chevroncolor"))) {
        SetChevronColor(strValue);
    }
    else if ((strName == _T("scrollbar_color")) || (strName == _T("scrollbarcolor"))) {
        SetScrollbarColor(strValue);
    }
    else if (strName == _T("font")) {
        SetFont(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void CascadingSelect::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    //语言切换：按保存的 ID 串重建文本，树形与选择索引不变
    RebuildFromIds();
}

void CascadingSelect::PaintStateImages(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    BaseClass::PaintStateImages(pRender);

    const UiRect rcContent = GetContentRect();
    if (rcContent.IsEmpty()) {
        return;
    }
    BuildLayout(rcContent);

    const int32_t nTextPadding = Dpi().GetScaleInt(std::max(0, m_nTextPadding));
    const int32_t nRadius = Dpi().GetScaleInt(3);
    const int32_t nVisibleRows = GetVisibleRowCount(rcContent);

    AutoClip clip(pRender, rcContent);
    for (size_t nLevel = 0; nLevel < m_columns.size(); ++nLevel) {
        const ColumnLayout& column = m_columns[nLevel];
        //列分隔线（第 2 列起，绘制在列左边缘）
        if (nLevel > 0) {
            UiRect rcDivider = column.rcColumn;
            rcDivider.right = rcDivider.left + Dpi().GetScaleInt(1);
            pRender->FillRect(UiRectF::MakeFromRect(rcDivider), GetUiColor(m_strDividerColor));
        }
        if (!column.bActive) {
            continue;
        }
        const std::vector<Node>* pSiblings = GetSiblings(nLevel);
        if (pSiblings == nullptr) {
            continue;
        }
        const int32_t nOffset = (nLevel < m_columnScroll.size()) ? m_columnScroll[nLevel] : 0;
        for (size_t nRow = 0; nRow < column.itemRects.size(); ++nRow) {
            const int32_t nNodeIndex = column.nodeIndexes[nRow];
            if ((nNodeIndex < 0) || (static_cast<size_t>(nNodeIndex) >= pSiblings->size())) {
                continue;
            }
            const Node& node = (*pSiblings)[nNodeIndex];
            const UiRect& rcRow = column.itemRects[nRow];

            const bool bSelected = (nLevel < m_selection.size()) && (m_selection[nLevel] == nNodeIndex);
            const bool bHovered = (m_nHoverLevel == static_cast<int32_t>(nLevel)) && (m_nHoverIndex == nNodeIndex);
            if (bSelected || bHovered) {
                DString strBgColor = bSelected ? m_strSelectedBgColor : m_strHoverBgColor;
                if (!strBgColor.empty()) {
                    UiRect rcBg = rcRow;
                    rcBg.left += nRadius;
                    rcBg.top += nRadius;
                    rcBg.right -= nRadius;
                    rcBg.bottom -= nRadius;
                    pRender->FillRoundRect(UiRectF::MakeFromRect(rcBg),
                        static_cast<float>(nRadius), static_cast<float>(nRadius), GetUiColor(strBgColor));
                }
            }

            const bool bDisabled = IsNodeDisabled(m_selection, nLevel, static_cast<size_t>(nNodeIndex));
            DString strTextColor = bDisabled ? m_strDisabledColor : (bSelected ? m_strSelectedColor : m_strItemColor);
            UiRect rcText = rcRow;
            rcText.left += nTextPadding;
            rcText.right -= nTextPadding;
            if (!node.children.empty()) {
                rcText.right -= Dpi().GetScaleInt(12);
            }
            DrawItemText(pRender, node.text, rcText, GetUiColor(strTextColor));

            if (!node.children.empty()) {
                DrawChevron(pRender, rcRow, GetUiColor(m_strChevronColor));
            }
        }
        DrawScrollbar(pRender, column.rcColumn, static_cast<int32_t>(pSiblings->size()), nVisibleRows, nOffset);
    }
}

bool CascadingSelect::ButtonDown(const EventArgs& msg)
{
    if (!IsEnabled() || !IsMouseEnabled()) {
        return BaseClass::ButtonDown(msg);
    }
    BuildLayout(GetContentRect());
    size_t nLevel = 0;
    size_t nIndex = 0;
    if (!HitTest(msg.ptMouse, nLevel, nIndex)) {
        return BaseClass::ButtonDown(msg);
    }
    if (IsNodeDisabled(m_selection, nLevel, nIndex)) {
        return true;
    }
    //父项变更：只保留到当前层，深层选择由此自动重置
    std::vector<size_t> indexes;
    for (size_t i = 0; i < nLevel; ++i) {
        if ((i >= m_selection.size()) || (m_selection[i] < 0)) {
            return BaseClass::ButtonDown(msg);
        }
        indexes.push_back(static_cast<size_t>(m_selection[i]));
    }
    indexes.push_back(nIndex);
    m_nHoverLevel = static_cast<int32_t>(nLevel);
    m_nHoverIndex = static_cast<int32_t>(nIndex);
    SetSelection(indexes);
    return true;
}

bool CascadingSelect::MouseMove(const EventArgs& msg)
{
    if (!IsEnabled() || !IsMouseEnabled()) {
        if (m_nHoverLevel >= 0) {
            m_nHoverLevel = -1;
            m_nHoverIndex = -1;
            UpdateCursor();
            Invalidate();
        }
        return BaseClass::MouseMove(msg);
    }
    BuildLayout(GetContentRect());
    int32_t nHoverLevel = -1;
    int32_t nHoverIndex = -1;
    size_t nLevel = 0;
    size_t nIndex = 0;
    if (HitTest(msg.ptMouse, nLevel, nIndex) && !IsNodeDisabled(m_selection, nLevel, nIndex)) {
        nHoverLevel = static_cast<int32_t>(nLevel);
        nHoverIndex = static_cast<int32_t>(nIndex);
    }
    if ((nHoverLevel != m_nHoverLevel) || (nHoverIndex != m_nHoverIndex)) {
        m_nHoverLevel = nHoverLevel;
        m_nHoverIndex = nHoverIndex;
        UpdateCursor();
        Invalidate();
    }
    return BaseClass::MouseMove(msg);
}

bool CascadingSelect::MouseLeave(const EventArgs& msg)
{
    if (m_nHoverLevel >= 0) {
        m_nHoverLevel = -1;
        m_nHoverIndex = -1;
        UpdateCursor();
        Invalidate();
    }
    return BaseClass::MouseLeave(msg);
}

bool CascadingSelect::MouseWheel(const EventArgs& msg)
{
    if (!IsEnabled() || !IsMouseEnabled()) {
        return BaseClass::MouseWheel(msg);
    }
    const UiRect rcContent = GetContentRect();
    BuildLayout(rcContent);
    const int32_t nVisibleRows = GetVisibleRowCount(rcContent);
    for (size_t nLevel = 0; nLevel < m_columns.size(); ++nLevel) {
        const ColumnLayout& column = m_columns[nLevel];
        if (!column.bActive || !column.rcColumn.ContainsPt(msg.ptMouse)) {
            continue;
        }
        const std::vector<Node>* pSiblings = GetSiblings(nLevel);
        if (pSiblings == nullptr) {
            return BaseClass::MouseWheel(msg);
        }
        const int32_t nCount = static_cast<int32_t>(pSiblings->size());
        if (nCount <= nVisibleRows) {
            return BaseClass::MouseWheel(msg);
        }
        const int32_t nStep = (msg.eventData > 0) ? -3 : 3;
        m_columnScroll[nLevel] += nStep;
        ClampColumnScroll(nLevel);
        Invalidate();
        return true;
    }
    return BaseClass::MouseWheel(msg);
}

void CascadingSelect::SetItems(const std::vector<DString>& paths)
{
    m_strIdPaths.clear();
    BuildTree(paths, false);
    m_selection.assign(GetDepth(), -1);
    ResetAllScroll();
    Invalidate();
}

void CascadingSelect::SetItemIds(const std::vector<DString>& pathIds)
{
    m_strIdPaths.clear();
    for (size_t i = 0; i < pathIds.size(); ++i) {
        if (i > 0) {
            m_strIdPaths += _T("|");
        }
        m_strIdPaths += pathIds[i];
    }
    BuildTree(pathIds, true);
    m_selection.assign(GetDepth(), -1);
    ResetAllScroll();
    Invalidate();
}

void CascadingSelect::AddPath(const DString& path)
{
    InsertPath(path, false);
    const std::vector<int32_t> oldSelection = m_selection;
    ClampSelection();
    Invalidate();
    if (oldSelection != m_selection) {
        EmitSelectionChanged(oldSelection);
    }
}

void CascadingSelect::ClearItems()
{
    m_roots.clear();
    m_strIdPaths.clear();
    m_selection.clear();
    m_columnScroll.clear();
    m_columns.clear();
    m_nHoverLevel = -1;
    m_nHoverIndex = -1;
    Invalidate();
}

size_t CascadingSelect::GetDepth() const
{
    if (m_roots.empty()) {
        return 0;
    }
    //按层遍历计算最大层级（列数）
    size_t nDepth = 0;
    std::vector<const std::vector<Node>*> current;
    current.push_back(&m_roots);
    while (!current.empty()) {
        ++nDepth;
        std::vector<const std::vector<Node>*> next;
        for (const std::vector<Node>* pNodes : current) {
            for (const Node& node : *pNodes) {
                if (!node.children.empty()) {
                    next.push_back(&node.children);
                }
            }
        }
        if (next.empty()) {
            break;
        }
        current.swap(next);
    }
    return nDepth;
}

size_t CascadingSelect::GetItemCount(size_t level) const
{
    const std::vector<Node>* pSiblings = GetSiblings(level);
    return (pSiblings == nullptr) ? 0 : pSiblings->size();
}

bool CascadingSelect::GetItemText(size_t level, size_t index, DString& text) const
{
    const Node* pNode = GetNode(level, index);
    if (pNode == nullptr) {
        return false;
    }
    text = pNode->text;
    return true;
}

bool CascadingSelect::SetSelection(const std::vector<size_t>& indexes, bool bTriggerEvent)
{
    const size_t nDepth = GetDepth();
    std::vector<int32_t> newSelection(nDepth, -1);
    for (size_t nLevel = 0; (nLevel < nDepth) && (nLevel < indexes.size()); ++nLevel) {
        const std::vector<Node>* pSiblings = GetSiblings(newSelection, nLevel);
        if ((pSiblings == nullptr) || pSiblings->empty()) {
            break;
        }
        const size_t nIndex = indexes[nLevel];
        if (nIndex >= pSiblings->size()) {
            break;
        }
        if (IsNodeDisabled(newSelection, nLevel, nIndex)) {
            break;
        }
        newSelection[nLevel] = static_cast<int32_t>(nIndex);
    }
    return CommitSelection(std::move(newSelection), bTriggerEvent);
}

bool CascadingSelect::SetSelectionByText(const DString& strText, bool bTriggerEvent)
{
    const size_t nDepth = GetDepth();
    std::vector<int32_t> newSelection(nDepth, -1);
    std::list<DString> parts = StringUtil::Split(strText, m_strValueSeparator);
    const std::vector<Node>* pSiblings = &m_roots;
    size_t nLevel = 0;
    for (DString part : parts) {
        if (nLevel >= nDepth) {
            break;
        }
        StringUtil::Trim(part);
        if (part.empty() || (pSiblings == nullptr)) {
            break;
        }
        int32_t nFound = -1;
        for (size_t i = 0; i < pSiblings->size(); ++i) {
            if ((*pSiblings)[i].text == part) {
                nFound = static_cast<int32_t>(i);
                break;
            }
        }
        if (nFound < 0) {
            break;
        }
        if (IsNodeDisabled(newSelection, nLevel, static_cast<size_t>(nFound))) {
            break;
        }
        newSelection[nLevel] = nFound;
        pSiblings = &(*pSiblings)[nFound].children;
        ++nLevel;
    }
    return CommitSelection(std::move(newSelection), bTriggerEvent);
}

void CascadingSelect::ClearSelection(bool bTriggerEvent)
{
    CommitSelection(std::vector<int32_t>(GetDepth(), -1), bTriggerEvent);
}

DString CascadingSelect::GetSelectionText() const
{
    DString strText;
    DString strPath;
    MakeSelectionText(m_selection, strText, strPath);
    return strText;
}

DString CascadingSelect::GetSelectionPath() const
{
    DString strText;
    DString strPath;
    MakeSelectionText(m_selection, strText, strPath);
    return strPath;
}

void CascadingSelect::SetDisabledPaths(const std::vector<DString>& paths)
{
    m_disabledPaths = paths;
    const std::vector<int32_t> oldSelection = m_selection;
    ClampSelection();
    m_nHoverLevel = -1;
    m_nHoverIndex = -1;
    UpdateCursor();
    Invalidate();
    if (oldSelection != m_selection) {
        EmitSelectionChanged(oldSelection);
    }
}

bool CascadingSelect::IsItemDisabled(size_t level, size_t index) const
{
    return IsNodeDisabled(m_selection, level, index);
}

void CascadingSelect::SetPathSeparator(const DString& strSeparator)
{
    m_strPathSeparator = strSeparator.empty() ? _T("/") : strSeparator;
    Invalidate();
}

void CascadingSelect::SetValueSeparator(const DString& strSeparator)
{
    m_strValueSeparator = strSeparator;
    Invalidate();
}

void CascadingSelect::SetColumnWidth(int32_t nWidth)
{
    m_nColumnWidth = std::max(0, nWidth);
    Invalidate();
}

void CascadingSelect::SetRowHeight(int32_t nHeight)
{
    m_nRowHeight = std::max(1, nHeight);
    Invalidate();
}

void CascadingSelect::SetTextPadding(int32_t nPadding)
{
    m_nTextPadding = std::max(0, nPadding);
    Invalidate();
}

void CascadingSelect::SetItemColor(const DString& strColor)
{
    m_strItemColor = strColor;
    Invalidate();
}

void CascadingSelect::SetSelectedColor(const DString& strColor)
{
    m_strSelectedColor = strColor;
    Invalidate();
}

void CascadingSelect::SetDisabledColor(const DString& strColor)
{
    m_strDisabledColor = strColor;
    Invalidate();
}

void CascadingSelect::SetHoverBgColor(const DString& strColor)
{
    m_strHoverBgColor = strColor;
    Invalidate();
}

void CascadingSelect::SetSelectedBgColor(const DString& strColor)
{
    m_strSelectedBgColor = strColor;
    Invalidate();
}

void CascadingSelect::SetDividerColor(const DString& strColor)
{
    m_strDividerColor = strColor;
    Invalidate();
}

void CascadingSelect::SetChevronColor(const DString& strColor)
{
    m_strChevronColor = strColor;
    Invalidate();
}

void CascadingSelect::SetScrollbarColor(const DString& strColor)
{
    m_strScrollbarColor = strColor;
    Invalidate();
}

void CascadingSelect::SetFont(const DString& strFontId)
{
    m_strFont = strFontId;
    Invalidate();
}

void CascadingSelect::BuildTree(const std::vector<DString>& paths, bool bUseIds)
{
    m_roots.clear();
    for (const DString& path : paths) {
        InsertPath(path, bUseIds);
    }
    ClampSelection();
    m_nHoverLevel = -1;
    m_nHoverIndex = -1;
}

void CascadingSelect::InsertPath(const DString& path, bool bUseIds)
{
    DString strPath = path;
    StringUtil::Trim(strPath);
    if (strPath.empty()) {
        return;
    }
    std::list<DString> segments = StringUtil::Split(strPath, m_strPathSeparator);
    std::vector<Node>* pNodes = &m_roots;
    for (DString segment : segments) {
        StringUtil::Trim(segment);
        if (segment.empty()) {
            continue;
        }
        DString strText = bUseIds ? GlobalManager::GetTextById(segment) : segment;
        if (strText.empty()) {
            strText = segment;
        }
        const DString& strKey = bUseIds ? segment : strText;
        auto it = std::find_if(pNodes->begin(), pNodes->end(), [&strKey, bUseIds](const Node& node) {
            return bUseIds ? (node.textId == strKey) : (node.text == strKey);
        });
        if (it == pNodes->end()) {
            Node node;
            node.text = strText;
            node.textId = bUseIds ? segment : DString();
            pNodes->push_back(node);
            it = pNodes->end() - 1;
        }
        pNodes = &it->children;
    }
}

void CascadingSelect::RebuildFromIds()
{
    if (m_strIdPaths.empty()) {
        return;
    }
    std::list<DString> parts = StringUtil::Split(m_strIdPaths, _T("|"));
    std::vector<DString> paths;
    for (DString part : parts) {
        StringUtil::Trim(part);
        if (!part.empty()) {
            paths.push_back(part);
        }
    }
    BuildTree(paths, true);
    for (size_t nLevel = 0; nLevel < m_columnScroll.size(); ++nLevel) {
        ClampColumnScroll(nLevel);
    }
    Invalidate();
}

const std::vector<CascadingSelect::Node>* CascadingSelect::GetSiblings(const std::vector<int32_t>& selection, size_t level) const
{
    if (level > selection.size()) {
        return nullptr;
    }
    const std::vector<Node>* pNodes = &m_roots;
    for (size_t i = 0; i < level; ++i) {
        if ((i >= selection.size()) || (selection[i] < 0)) {
            return nullptr;
        }
        if (static_cast<size_t>(selection[i]) >= pNodes->size()) {
            return nullptr;
        }
        pNodes = &(*pNodes)[selection[i]].children;
    }
    return pNodes;
}

const std::vector<CascadingSelect::Node>* CascadingSelect::GetSiblings(size_t level) const
{
    return GetSiblings(m_selection, level);
}

const CascadingSelect::Node* CascadingSelect::GetNode(size_t level, size_t index) const
{
    const std::vector<Node>* pSiblings = GetSiblings(level);
    if ((pSiblings == nullptr) || (index >= pSiblings->size())) {
        return nullptr;
    }
    return &(*pSiblings)[index];
}

void CascadingSelect::MakeNodePath(const std::vector<int32_t>& selection, size_t level, size_t index,
                                   DString& textPath, DString& idPath) const
{
    textPath.clear();
    idPath.clear();
    const std::vector<Node>* pNodes = &m_roots;
    bool bHasId = true;
    for (size_t i = 0; i <= level; ++i) {
        int32_t nNodeIndex = (i == level) ? static_cast<int32_t>(index) : ((i < selection.size()) ? selection[i] : -1);
        if ((nNodeIndex < 0) || (static_cast<size_t>(nNodeIndex) >= pNodes->size())) {
            break;
        }
        const Node& node = (*pNodes)[nNodeIndex];
        if (!textPath.empty()) {
            textPath += m_strPathSeparator;
        }
        textPath += node.text;
        if (bHasId) {
            if (node.textId.empty()) {
                bHasId = false;
                idPath.clear();
            }
            else {
                if (!idPath.empty()) {
                    idPath += m_strPathSeparator;
                }
                idPath += node.textId;
            }
        }
        pNodes = &node.children;
    }
}

bool CascadingSelect::IsNodeDisabled(const std::vector<int32_t>& selection, size_t level, size_t index) const
{
    if (m_disabledPaths.empty()) {
        return false;
    }
    DString textPath;
    DString idPath;
    MakeNodePath(selection, level, index, textPath, idPath);
    if (textPath.empty()) {
        return false;
    }
    for (const DString& disabledPath : m_disabledPaths) {
        if (disabledPath.empty()) {
            continue;
        }
        if (disabledPath == textPath) {
            return true;
        }
        if (!idPath.empty() && (disabledPath == idPath)) {
            return true;
        }
        if (IsPathPrefix(disabledPath, textPath, m_strPathSeparator)) {
            return true;
        }
        if (!idPath.empty() && IsPathPrefix(disabledPath, idPath, m_strPathSeparator)) {
            return true;
        }
    }
    return false;
}

void CascadingSelect::ClampSelection()
{
    const size_t nDepth = GetDepth();
    m_selection.resize(nDepth, -1);
    m_columnScroll.resize(nDepth, 0);
    for (size_t nLevel = 0; nLevel < nDepth; ++nLevel) {
        const int32_t nIndex = m_selection[nLevel];
        const std::vector<Node>* pSiblings = GetSiblings(m_selection, nLevel);
        if ((pSiblings == nullptr) || pSiblings->empty() || (nIndex < 0) ||
            (static_cast<size_t>(nIndex) >= pSiblings->size())) {
            m_selection[nLevel] = -1;
            continue;
        }
        if (IsNodeDisabled(m_selection, nLevel, static_cast<size_t>(nIndex))) {
            m_selection[nLevel] = -1;
        }
    }
}

bool CascadingSelect::CommitSelection(std::vector<int32_t> newSelection, bool bTriggerEvent)
{
    const size_t nDepth = GetDepth();
    newSelection.resize(nDepth, -1);
    if (newSelection == m_selection) {
        return false;
    }
    const std::vector<int32_t> oldSelection = m_selection;
    m_selection = newSelection;
    for (size_t nLevel = 0; nLevel < nDepth; ++nLevel) {
        if ((nLevel >= oldSelection.size()) || (oldSelection[nLevel] != m_selection[nLevel])) {
            if (nLevel < m_columnScroll.size()) {
                m_columnScroll[nLevel] = 0;
            }
            if (static_cast<int32_t>(nLevel) == m_nHoverLevel) {
                m_nHoverLevel = -1;
                m_nHoverIndex = -1;
            }
            break;
        }
    }
    Invalidate();
    if (bTriggerEvent) {
        EmitSelectionChanged(oldSelection);
    }
    return true;
}

void CascadingSelect::EmitSelectionChanged(const std::vector<int32_t>& oldSelection)
{
    (void)oldSelection;
    if (m_bMuteEvent) {
        return;
    }
    //选择数据使用栈上临时对象，仅在本次同步派发期间有效
    CascadingSelectValue value;
    value.indexes = m_selection;
    MakeSelectionText(m_selection, value.text, value.path);
    SendEvent(kEventValueChanged, static_cast<WPARAM>(4), 0, static_cast<void*>(&value));
}

void CascadingSelect::MakeSelectionText(const std::vector<int32_t>& selection, DString& text, DString& path) const
{
    text.clear();
    path.clear();
    const std::vector<Node>* pNodes = &m_roots;
    for (size_t nLevel = 0; nLevel < selection.size(); ++nLevel) {
        const int32_t nIndex = selection[nLevel];
        if ((nIndex < 0) || (static_cast<size_t>(nIndex) >= pNodes->size())) {
            break;
        }
        const Node& node = (*pNodes)[nIndex];
        if (!text.empty()) {
            text += m_strValueSeparator;
        }
        text += node.text;
        if (!path.empty()) {
            path += m_strPathSeparator;
        }
        path += node.text;
        pNodes = &node.children;
    }
}

void CascadingSelect::BuildLayout(const UiRect& rc)
{
    m_columns.clear();
    const size_t nDepth = GetDepth();
    if ((nDepth == 0) || rc.IsEmpty()) {
        return;
    }
    const int32_t nRowHeight = Dpi().GetScaleInt(std::max(1, m_nRowHeight));
    const int32_t nVisibleRows = std::max(1, rc.Height() / nRowHeight);
    const int32_t nFixedWidth = (m_nColumnWidth > 0) ? Dpi().GetScaleInt(m_nColumnWidth) : 0;
    const int32_t nEqualWidth = std::max(1, rc.Width() / static_cast<int32_t>(nDepth));

    m_columns.resize(nDepth);
    int32_t nLeft = rc.left;
    for (size_t nLevel = 0; nLevel < nDepth; ++nLevel) {
        int32_t nWidth = (nFixedWidth > 0) ? nFixedWidth : nEqualWidth;
        if ((nFixedWidth <= 0) && (nLevel + 1 == nDepth)) {
            nWidth = std::max(1, rc.right - nLeft);
        }
        if (nLeft + nWidth > rc.right) {
            nWidth = std::max(1, rc.right - nLeft);
        }

        ColumnLayout& column = m_columns[nLevel];
        column.rcColumn = UiRect(nLeft, rc.top, nLeft + nWidth, rc.bottom);
        nLeft += nWidth;

        const std::vector<Node>* pSiblings = GetSiblings(nLevel);
        column.bActive = (pSiblings != nullptr) && !pSiblings->empty();
        if (!column.bActive) {
            continue;
        }
        ClampColumnScroll(nLevel);
        const int32_t nOffset = (nLevel < m_columnScroll.size()) ? m_columnScroll[nLevel] : 0;
        const int32_t nCount = static_cast<int32_t>(pSiblings->size());
        for (int32_t nRow = nOffset; (nRow < nCount) && (nRow < nOffset + nVisibleRows); ++nRow) {
            const int32_t nTop = rc.top + (nRow - nOffset) * nRowHeight;
            UiRect rcItem(column.rcColumn.left, nTop, column.rcColumn.right, std::min(nTop + nRowHeight, rc.bottom));
            column.itemRects.push_back(rcItem);
            column.nodeIndexes.push_back(nRow);
        }
    }
}

bool CascadingSelect::HitTest(const UiPoint& ptMouse, size_t& level, size_t& nodeIndex) const
{
    for (size_t nLevel = 0; nLevel < m_columns.size(); ++nLevel) {
        const ColumnLayout& column = m_columns[nLevel];
        if (!column.bActive || !column.rcColumn.ContainsPt(ptMouse)) {
            continue;
        }
        for (size_t i = 0; i < column.itemRects.size(); ++i) {
            if (column.itemRects[i].ContainsPt(ptMouse)) {
                level = nLevel;
                nodeIndex = static_cast<size_t>(column.nodeIndexes[i]);
                return true;
            }
        }
    }
    return false;
}

UiRect CascadingSelect::GetContentRect() const
{
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    return rc;
}

int32_t CascadingSelect::GetVisibleRowCount(const UiRect& rcContent) const
{
    const int32_t nRowHeight = Dpi().GetScaleInt(std::max(1, m_nRowHeight));
    if (nRowHeight <= 0) {
        return 1;
    }
    return std::max(1, rcContent.Height() / nRowHeight);
}

void CascadingSelect::ClampColumnScroll(size_t level)
{
    if (level >= m_columnScroll.size()) {
        return;
    }
    const std::vector<Node>* pSiblings = GetSiblings(level);
    const int32_t nCount = (pSiblings == nullptr) ? 0 : static_cast<int32_t>(pSiblings->size());
    const int32_t nVisibleRows = GetVisibleRowCount(GetContentRect());
    const int32_t nMaxOffset = std::max(0, nCount - nVisibleRows);
    m_columnScroll[level] = std::clamp(m_columnScroll[level], 0, nMaxOffset);
}

void CascadingSelect::ResetAllScroll()
{
    m_columnScroll.assign(GetDepth(), 0);
}

void CascadingSelect::DrawItemText(IRender* pRender, const DString& strText, const UiRect& rc, UiColor textColor)
{
    if ((pRender == nullptr) || strText.empty()) {
        return;
    }
    DrawStringParam param;
    param.textRect = rc;
    param.dwTextColor = textColor;
    param.pFont = GetIFontById(m_strFont);
    param.uFormat = TEXT_LEFT | TEXT_SINGLELINE | TEXT_VCENTER | TEXT_END_ELLIPSIS;
    pRender->DrawString(strText, param);
}

void CascadingSelect::DrawChevron(IRender* pRender, const UiRect& rc, UiColor color)
{
    if ((pRender == nullptr) || color.IsEmpty()) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory == nullptr) {
        return;
    }
    const int32_t nHeight = Dpi().GetScaleInt(8);
    const int32_t nWidth = Dpi().GetScaleInt(4);
    const int32_t nMargin = Dpi().GetScaleInt(8);
    const int32_t nCx = rc.right - nMargin - nWidth / 2;
    const int32_t nCy = (rc.top + rc.bottom) / 2;
    std::array<UiPointF, 3> pts = { {
        UiPointF(static_cast<float>(nCx - nWidth / 2), static_cast<float>(nCy - nHeight / 2)),
        UiPointF(static_cast<float>(nCx - nWidth / 2), static_cast<float>(nCy + nHeight / 2)),
        UiPointF(static_cast<float>(nCx + nWidth / 2), static_cast<float>(nCy)),
    } };
    std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
    if (path == nullptr) {
        return;
    }
    path->AddPolygon(pts.data(), static_cast<int32_t>(pts.size()));
    path->Close();
    std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(color));
    if (brush != nullptr) {
        pRender->FillPath(path.get(), brush.get());
    }
}

void CascadingSelect::DrawScrollbar(IRender* pRender, const UiRect& rcColumn, int32_t nItemCount,
                                    int32_t nVisibleRows, int32_t nOffset)
{
    if ((pRender == nullptr) || (nItemCount <= nVisibleRows) || (nVisibleRows <= 0)) {
        return;
    }
    UiColor color = GetUiColor(m_strScrollbarColor);
    if (color.IsEmpty()) {
        return;
    }
    const int32_t nTrackWidth = Dpi().GetScaleInt(3);
    const int32_t nTrackX = rcColumn.right - Dpi().GetScaleInt(1) - nTrackWidth;
    const int32_t nTrackTop = rcColumn.top + Dpi().GetScaleInt(2);
    const int32_t nTrackBottom = rcColumn.bottom - Dpi().GetScaleInt(2);
    if (nTrackBottom <= nTrackTop) {
        return;
    }
    const int32_t nTrackHeight = nTrackBottom - nTrackTop;
    const int32_t nThumbHeight = std::max(Dpi().GetScaleInt(12),
        nTrackHeight * nVisibleRows / std::max(1, nItemCount));
    const int32_t nMaxOffset = std::max(1, nItemCount - nVisibleRows);
    const int32_t nThumbTop = nTrackTop + (nTrackHeight - nThumbHeight) * nOffset / nMaxOffset;
    UiRectF rcThumb(static_cast<float>(nTrackX), static_cast<float>(nThumbTop),
                    static_cast<float>(nTrackX + nTrackWidth), static_cast<float>(nThumbTop + nThumbHeight));
    pRender->FillRoundRect(rcThumb, static_cast<float>(nTrackWidth) / 2.0f,
        static_cast<float>(nTrackWidth) / 2.0f, color);
}

void CascadingSelect::UpdateCursor()
{
    if (m_nHoverLevel >= 0) {
        SetCursorType(CursorType::kCursorHand);
    }
    else {
        SetCursorType(CursorType::kCursorArrow);
    }
}

} // namespace ui
