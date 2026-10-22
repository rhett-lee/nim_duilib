#include "CascadingSelect.h"
#include "duilib/Control/Flyout.h"
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

    //下拉列表框形态的默认样式
    m_strPopupClass = _T("cascading_select_popup");
    m_strTextColor = _T("text_default");
    m_strPlaceholderColor = _T("text_muted");
    m_strDisabledTextColor = _T("text_disabled");
    m_strBgColor = _T("bg_window_card");
    m_strBorderColor = _T("border_control_normal");
    m_strHoverBorderColor = _T("border_btn_hovered");
    m_strClearColor = _T("color_gray");
    m_strClearHoverColor = _T("text_default");
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
    else if (strName == _T("dropdown")) {
        SetDropdown(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("expand_on_hover")) || (strName == _T("expandonhover"))) {
        SetExpandOnHover(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("allow_clear")) || (strName == _T("allowclear"))) {
        SetAllowClear(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("placeholder")) {
        SetPlaceholder(strValue);
    }
    else if (strName == _T("placeholder_id")) {
        SetPlaceholderId(strValue);
    }
    else if ((strName == _T("popup_width")) || (strName == _T("popupwidth"))) {
        SetPopupWidth(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("popup_max_rows")) || (strName == _T("popupmaxrows"))) {
        SetPopupMaxRows(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("popup_class")) || (strName == _T("popupclass"))) {
        SetPopupClass(strValue);
    }
    else if ((strName == _T("text_color")) || (strName == _T("textcolor"))) {
        SetTextColor(strValue);
    }
    else if ((strName == _T("placeholder_color")) || (strName == _T("placeholdercolor"))) {
        SetPlaceholderColor(strValue);
    }
    else if ((strName == _T("disabled_text_color")) || (strName == _T("disabledtextcolor"))) {
        SetDisabledTextColor(strValue);
    }
    else if ((strName == _T("bg_color")) || (strName == _T("bgcolor"))) {
        SetBgColor(strValue);
    }
    else if ((strName == _T("border_color")) || (strName == _T("bordercolor"))) {
        SetBorderColor(strValue);
    }
    else if ((strName == _T("hover_border_color")) || (strName == _T("hoverbordercolor"))) {
        SetHoverBorderColor(strValue);
    }
    else if ((strName == _T("clear_color")) || (strName == _T("clearcolor"))) {
        SetClearColor(strValue);
    }
    else if ((strName == _T("clear_hover_color")) || (strName == _T("clearhovercolor"))) {
        SetClearHoverColor(strValue);
    }
    else if ((strName == _T("border_round")) || (strName == _T("borderround"))) {
        SetBorderRound(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("border_size")) || (strName == _T("bordersize"))) {
        SetBorderSize(StringUtil::StringToInt32(strValue));
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
    if (m_bDropdown) {
        //下拉列表框形态：只绘制选择框，多列内容在浮层中绘制
        PaintDropdown(pRender);
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
    if (m_bDropdown) {
        if (!IsEnabled() || !IsMouseEnabled()) {
            return BaseClass::ButtonDown(msg);
        }
        const UiRect rcClear = GetClearRect();
        if (!rcClear.IsEmpty() && rcClear.ContainsPt(msg.ptMouse)) {
            //清空按钮：先关闭浮层，再清空选择
            ClosePopup();
            ClearSelection();
            return true;
        }
        if (IsPopupOpen()) {
            ClosePopup();
        }
        else {
            OpenPopup();
        }
        return true;
    }
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
    if (m_bDropdown) {
        const bool bHovered = IsEnabled() && IsMouseEnabled() && GetRect().ContainsPt(msg.ptMouse);
        const UiRect rcClear = bHovered ? GetClearRect() : UiRect();
        const bool bClearHovered = !rcClear.IsEmpty() && rcClear.ContainsPt(msg.ptMouse);
        if ((bHovered != m_bHovered) || (bClearHovered != m_bClearHovered)) {
            m_bHovered = bHovered;
            m_bClearHovered = bClearHovered;
            UpdateCursor();
            Invalidate();
        }
        return BaseClass::MouseMove(msg);
    }
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
    //移入展开：悬停在父项上时静默展开子列（不派发选择事件，点击才提交）
    if (m_bExpandOnHover && (nHoverLevel >= 0)) {
        const Node* pNode = GetNode(static_cast<size_t>(nHoverLevel), static_cast<size_t>(nHoverIndex));
        if ((pNode != nullptr) && !pNode->children.empty()) {
            std::vector<size_t> indexes;
            bool bValid = true;
            for (int32_t i = 0; i < nHoverLevel; ++i) {
                if ((i >= static_cast<int32_t>(m_selection.size())) || (m_selection[i] < 0)) {
                    bValid = false;
                    break;
                }
                indexes.push_back(static_cast<size_t>(m_selection[i]));
            }
            if (bValid) {
                indexes.push_back(static_cast<size_t>(nHoverIndex));
                SetSelection(indexes, false);
            }
        }
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
    if (m_bDropdown) {
        if (m_bHovered) {
            m_bHovered = false;
            m_bClearHovered = false;
            UpdateCursor();
            Invalidate();
        }
        return BaseClass::MouseLeave(msg);
    }
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
    if (m_bDropdown) {
        //下拉形态：滚轮由浮层内的级联列表处理
        return BaseClass::MouseWheel(msg);
    }
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
    m_strItemPaths.clear();
    m_bItemIdMode = false;
    for (size_t i = 0; i < paths.size(); ++i) {
        if (i > 0) {
            m_strItemPaths += _T("|");
        }
        m_strItemPaths += paths[i];
    }
    BuildTree(paths, false);
    m_selection.assign(GetDepth(), -1);
    ResetAllScroll();
    Invalidate();
}

void CascadingSelect::SetItemIds(const std::vector<DString>& pathIds)
{
    m_strIdPaths.clear();
    m_strItemPaths.clear();
    m_bItemIdMode = true;
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
    if (m_bItemIdMode) {
        if (!m_strIdPaths.empty()) {
            m_strIdPaths += _T("|");
        }
        m_strIdPaths += path;
        InsertPath(path, true);
    }
    else {
        if (!m_strItemPaths.empty()) {
            m_strItemPaths += _T("|");
        }
        m_strItemPaths += path;
        InsertPath(path, false);
    }
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
    m_strItemPaths.clear();
    m_bItemIdMode = false;
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

void CascadingSelect::SetDropdown(bool bDropdown)
{
    if (m_bDropdown == bDropdown) {
        return;
    }
    m_bDropdown = bDropdown;
    if (!m_bDropdown) {
        ClosePopup();
        m_bHovered = false;
    }
    UpdateCursor();
    Invalidate();
}

void CascadingSelect::SetExpandOnHover(bool bExpand)
{
    m_bExpandOnHover = bExpand;
    Invalidate();
}

void CascadingSelect::SetAllowClear(bool bAllow)
{
    m_bAllowClear = bAllow;
    if (!m_bAllowClear) {
        m_bClearHovered = false;
    }
    Invalidate();
}

void CascadingSelect::SetPlaceholder(const DString& strText)
{
    m_strPlaceholder = strText;
    Invalidate();
}

DString CascadingSelect::GetPlaceholder() const
{
    if (!m_strPlaceholderId.empty()) {
        DString strText = GlobalManager::GetTextById(m_strPlaceholderId);
        if (!strText.empty()) {
            return strText;
        }
    }
    return m_strPlaceholder;
}

void CascadingSelect::SetPlaceholderId(const DString& strTextId)
{
    m_strPlaceholderId = strTextId;
    Invalidate();
}

void CascadingSelect::SetPopupWidth(int32_t nWidth)
{
    m_nPopupWidth = std::max(0, nWidth);
    Invalidate();
}

void CascadingSelect::SetPopupMaxRows(int32_t nRows)
{
    m_nPopupMaxRows = std::max(1, nRows);
    Invalidate();
}

void CascadingSelect::SetPopupClass(const DString& strClass)
{
    m_strPopupClass = strClass.empty() ? _T("cascading_select_popup") : strClass;
    Invalidate();
}

DString CascadingSelect::GetDisplayText() const
{
    CascadingSelectValue value;
    BuildValue(value);
    if (m_displayTextCallback) {
        DString strText = m_displayTextCallback(value);
        if (!strText.empty()) {
            return strText;
        }
    }
    return value.text;
}

void CascadingSelect::AttachDisplayText(const DisplayTextEvent& callback)
{
    m_displayTextCallback = callback;
    Invalidate();
}

void CascadingSelect::SetTextColor(const DString& strColor)
{
    m_strTextColor = strColor;
    Invalidate();
}

void CascadingSelect::SetPlaceholderColor(const DString& strColor)
{
    m_strPlaceholderColor = strColor;
    Invalidate();
}

void CascadingSelect::SetDisabledTextColor(const DString& strColor)
{
    m_strDisabledTextColor = strColor;
    Invalidate();
}

void CascadingSelect::SetBgColor(const DString& strColor)
{
    m_strBgColor = strColor;
    Invalidate();
}

void CascadingSelect::SetBorderColor(const DString& strColor)
{
    m_strBorderColor = strColor;
    Invalidate();
}

void CascadingSelect::SetHoverBorderColor(const DString& strColor)
{
    m_strHoverBorderColor = strColor;
    Invalidate();
}

void CascadingSelect::SetClearColor(const DString& strColor)
{
    m_strClearColor = strColor;
    Invalidate();
}

void CascadingSelect::SetClearHoverColor(const DString& strColor)
{
    m_strClearHoverColor = strColor;
    Invalidate();
}

void CascadingSelect::SetBorderRound(int32_t nRound)
{
    m_nBorderRound = std::max(0, nRound);
    Invalidate();
}

void CascadingSelect::SetBorderSize(int32_t nSize)
{
    m_nBorderSize = std::max(0, nSize);
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

void CascadingSelect::PaintDropdown(IRender* pRender)
{
    const UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    const int32_t nRadius = Dpi().GetScaleInt(std::max(0, m_nBorderRound));
    const UiColor bgColor = GetUiColor(m_strBgColor);
    if (!bgColor.IsEmpty()) {
        pRender->FillRoundRect(UiRectF::MakeFromRect(rc),
            static_cast<float>(nRadius), static_cast<float>(nRadius), bgColor);
    }

    DString strBorderColor = (m_bHovered || IsPopupOpen()) ? m_strHoverBorderColor : m_strBorderColor;
    if (!IsEnabled()) {
        strBorderColor = m_strBorderColor;
    }
    const int32_t nBorderSize = Dpi().GetScaleInt(std::max(0, m_nBorderSize));
    if ((nBorderSize > 0) && !strBorderColor.empty()) {
        pRender->DrawRoundRect(UiRectF::MakeFromRect(rc),
            static_cast<float>(nRadius), static_cast<float>(nRadius),
            GetUiColor(strBorderColor), static_cast<float>(nBorderSize));
    }

    const UiRect rcChevron = GetChevronRect();
    const UiRect rcClear = m_bHovered ? GetClearRect() : UiRect();
    const int32_t nPadding = Dpi().GetScaleInt(std::max(0, m_nTextPadding));
    UiRect rcText = GetContentRect();
    rcText.left += nPadding;
    const int32_t nTextRight = rcClear.IsEmpty() ? rcChevron.left : rcClear.left;
    rcText.right = nTextRight - nPadding / 2;
    if (rcText.right < rcText.left) {
        rcText.right = rcText.left;
    }

    DString strText;
    DString strTextColor;
    DString strSelectionText = GetSelectionText();
    if (!IsEnabled()) {
        strText = strSelectionText.empty() ? GetPlaceholder() : GetDisplayText();
        strTextColor = m_strDisabledTextColor;
    }
    else if (strSelectionText.empty()) {
        strText = GetPlaceholder();
        strTextColor = m_strPlaceholderColor;
    }
    else {
        strText = GetDisplayText();
        strTextColor = m_strTextColor;
    }

    AutoClip clip(pRender, GetContentRect());
    DrawItemText(pRender, strText, rcText, GetUiColor(strTextColor));
    DrawDownChevron(pRender, rcChevron, GetUiColor(m_strChevronColor));
    if (!rcClear.IsEmpty()) {
        DrawClearIcon(pRender, rcClear, GetUiColor(m_bClearHovered ? m_strClearHoverColor : m_strClearColor));
    }
}

void CascadingSelect::OpenPopup()
{
    if (!m_bDropdown || !IsEnabled() || !IsMouseEnabled()) {
        return;
    }
    const size_t nDepth = GetDepth();
    if (nDepth == 0) {
        return;
    }
    const int32_t nColumnWidth = GetPopupColumnWidth();
    const int32_t nPadding = 8;
    int32_t nWidth = (m_nPopupWidth > 0) ? m_nPopupWidth
                                        : static_cast<int32_t>(nDepth) * nColumnWidth + nPadding;
    //显式设置的宽度不足以容纳全部列时，按列数自动放宽，避免右侧列被裁切
    nWidth = std::max(nWidth, static_cast<int32_t>(nDepth) * nColumnWidth + nPadding);
    const int32_t nRows = std::clamp(EstimateMaxRows(), 1, std::max(1, m_nPopupMaxRows));
    const int32_t nHeight = nRows * std::max(1, m_nRowHeight) + nPadding;

    Flyout* pFlyout = new Flyout(GetWindow());
    pFlyout->SetGap(4);
    if (!pFlyout->ShowAt(this, BuildPopupXml(nWidth, nHeight), Flyout::Placement::Bottom)) {
        //创建失败时 ShowAt 内部已销毁对象
        return;
    }
    CascadingSelect* pList = dynamic_cast<CascadingSelect*>(pFlyout->FindControl(_T("cascading_popup_list")));
    if (pList == nullptr) {
        pFlyout->Dismiss();
        return;
    }

    //把当前数据与样式复制到浮层内的级联列表
    pList->SetPathSeparator(m_strPathSeparator);
    pList->SetValueSeparator(m_strValueSeparator);
    pList->SetColumnWidth(nColumnWidth);
    pList->SetRowHeight(m_nRowHeight);
    pList->SetTextPadding(m_nTextPadding);
    pList->SetFont(m_strFont);
    pList->SetItemColor(m_strItemColor);
    pList->SetSelectedColor(m_strSelectedColor);
    pList->SetDisabledColor(m_strDisabledColor);
    pList->SetHoverBgColor(m_strHoverBgColor);
    pList->SetSelectedBgColor(m_strSelectedBgColor);
    pList->SetDividerColor(m_strDividerColor);
    pList->SetChevronColor(m_strChevronColor);
    pList->SetScrollbarColor(m_strScrollbarColor);
    pList->SetExpandOnHover(m_bExpandOnHover);
    if (m_bItemIdMode) {
        pList->SetItemIds(SplitPathList(m_strIdPaths));
    }
    else {
        pList->SetItems(SplitPathList(m_strItemPaths));
    }
    pList->SetDisabledPaths(m_disabledPaths);

    std::vector<size_t> indexes;
    for (int32_t nIndex : m_selection) {
        if (nIndex < 0) {
            break;
        }
        indexes.push_back(static_cast<size_t>(nIndex));
    }
    if (!indexes.empty()) {
        pList->SetSelection(indexes, false);
    }

    //浮层内选满整条路径时提交到本控件并关闭浮层
    pList->AttachSelectionChanged([pFlyout](const EventArgs& args) {
        CascadingSelect* pOwner = dynamic_cast<CascadingSelect*>(pFlyout->GetAnchor());
        if (pOwner == nullptr) {
            return true;
        }
        const CascadingSelectValue* pValue = static_cast<const CascadingSelectValue*>(args.pEventData);
        if (pValue != nullptr) {
            pOwner->OnPopupSelection(*pValue);
        }
        return true;
    });
    Invalidate();
}

bool CascadingSelect::IsPopupOpen() const
{
    Flyout* pFlyout = Flyout::GetActiveFlyout();
    return (pFlyout != nullptr) && (pFlyout->GetAnchor() == this);
}

void CascadingSelect::ClosePopup()
{
    Flyout* pFlyout = Flyout::GetActiveFlyout();
    if ((pFlyout != nullptr) && (pFlyout->GetAnchor() == this)) {
        pFlyout->Dismiss();
    }
}

DString CascadingSelect::BuildPopupXml(int32_t nWidth, int32_t nHeight) const
{
    const DString strClass = m_strPopupClass.empty() ? _T("cascading_select_popup") : m_strPopupClass;
    return StringUtil::Printf(
        _T("<Window size=\"%d,%d\" caption=\"0,0,0,0\" use_system_caption=\"false\" size_box=\"0,0,0,0\">")
        _T("<VBox bkcolor=\"bg_window_card\" width=\"%d\" height=\"%d\" padding=\"4,4,4,4\" border_round=\"6,6\">")
        _T("<CascadingSelect class=\"%s\" name=\"cascading_popup_list\" width=\"stretch\" height=\"stretch\"/>")
        _T("</VBox></Window>"),
        nWidth, nHeight, nWidth, nHeight, strClass.c_str());
}

void CascadingSelect::OnPopupSelection(const CascadingSelectValue& value)
{
    if (value.indexes.empty()) {
        return;
    }
    if (std::find(value.indexes.begin(), value.indexes.end(), -1) != value.indexes.end()) {
        //未选满整条路径：只展开子列，不提交
        return;
    }
    std::vector<size_t> indexes;
    indexes.reserve(value.indexes.size());
    for (int32_t nIndex : value.indexes) {
        indexes.push_back(static_cast<size_t>(nIndex));
    }
    SetSelection(indexes);
    ClosePopup();
}

int32_t CascadingSelect::EstimateMaxRows() const
{
    int32_t nMaxRows = static_cast<int32_t>(m_roots.size());
    std::vector<const std::vector<Node>*> current;
    current.push_back(&m_roots);
    while (!current.empty()) {
        std::vector<const std::vector<Node>*> next;
        for (const std::vector<Node>* pNodes : current) {
            for (const Node& node : *pNodes) {
                if (!node.children.empty()) {
                    nMaxRows = std::max(nMaxRows, static_cast<int32_t>(node.children.size()));
                    next.push_back(&node.children);
                }
            }
        }
        current.swap(next);
    }
    return std::max(1, nMaxRows);
}

int32_t CascadingSelect::GetPopupColumnWidth() const
{
    return (m_nColumnWidth > 0) ? m_nColumnWidth : 140;
}

void CascadingSelect::BuildValue(CascadingSelectValue& value) const
{
    value.indexes = m_selection;
    MakeSelectionText(m_selection, value.text, value.path);
}

std::vector<DString> CascadingSelect::SplitPathList(const DString& strList)
{
    std::vector<DString> paths;
    std::list<DString> parts = StringUtil::Split(strList, _T("|"));
    for (DString part : parts) {
        StringUtil::Trim(part);
        if (!part.empty()) {
            paths.push_back(part);
        }
    }
    return paths;
}

UiRect CascadingSelect::GetChevronRect() const
{
    const UiRect rc = GetRect();
    const int32_t nSize = Dpi().GetScaleInt(12);
    const int32_t nMargin = Dpi().GetScaleInt(10);
    UiRect rcChevron;
    rcChevron.right = rc.right - nMargin;
    rcChevron.left = rcChevron.right - nSize;
    const int32_t nTop = rc.top + (rc.Height() - nSize) / 2;
    rcChevron.top = nTop;
    rcChevron.bottom = nTop + nSize;
    return rcChevron;
}

UiRect CascadingSelect::GetClearRect() const
{
    if (!m_bDropdown || !m_bAllowClear || !IsEnabled() || m_selection.empty() || GetSelectionText().empty()) {
        return UiRect();
    }
    const UiRect rc = GetRect();
    const UiRect rcChevron = GetChevronRect();
    const int32_t nSize = Dpi().GetScaleInt(14);
    const int32_t nGap = Dpi().GetScaleInt(4);
    UiRect rcClear;
    rcClear.right = rcChevron.left - nGap;
    rcClear.left = rcClear.right - nSize;
    const int32_t nTop = rc.top + (rc.Height() - nSize) / 2;
    rcClear.top = nTop;
    rcClear.bottom = nTop + nSize;
    return rcClear;
}

void CascadingSelect::DrawDownChevron(IRender* pRender, const UiRect& rc, UiColor color)
{
    if ((pRender == nullptr) || color.IsEmpty()) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory == nullptr) {
        return;
    }
    const float cx = (rc.left + rc.right) / 2.0f;
    const float cy = (rc.top + rc.bottom) / 2.0f;
    const int32_t nWidth = Dpi().GetScaleInt(10);
    const int32_t nHeight = Dpi().GetScaleInt(6);
    std::array<UiPointF, 3> pts = { {
        UiPointF(cx - nWidth / 2.0f, cy - nHeight / 2.0f),
        UiPointF(cx + nWidth / 2.0f, cy - nHeight / 2.0f),
        UiPointF(cx, cy + nHeight / 2.0f),
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

void CascadingSelect::DrawClearIcon(IRender* pRender, const UiRect& rc, UiColor color)
{
    if ((pRender == nullptr) || color.IsEmpty() || rc.IsEmpty()) {
        return;
    }
    const float cx = (rc.left + rc.right) / 2.0f;
    const float cy = (rc.top + rc.bottom) / 2.0f;
    const float fRadius = std::min(rc.Width(), rc.Height()) / 2.0f;
    const float fPenWidth = static_cast<float>(Dpi().GetScaleInt(1));
    pRender->DrawRoundRect(UiRectF::MakeFromRect(rc), fRadius, fRadius, color, fPenWidth);
    const float fCross = static_cast<float>(std::max(2, Dpi().GetScaleInt(3)));
    pRender->DrawLine(UiPointF(cx - fCross, cy - fCross), UiPointF(cx + fCross, cy + fCross), color, fPenWidth);
    pRender->DrawLine(UiPointF(cx + fCross, cy - fCross), UiPointF(cx - fCross, cy + fCross), color, fPenWidth);
}

void CascadingSelect::UpdateCursor()
{
    if (m_bDropdown) {
        SetCursorType(m_bHovered ? CursorType::kCursorHand : CursorType::kCursorArrow);
        return;
    }
    if (m_nHoverLevel >= 0) {
        SetCursorType(CursorType::kCursorHand);
    }
    else {
        SetCursorType(CursorType::kCursorArrow);
    }
}

} // namespace ui
