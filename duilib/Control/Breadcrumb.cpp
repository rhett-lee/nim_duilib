#include "Breadcrumb.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>
#include <list>
#include <memory>
#include <vector>

namespace ui
{

Breadcrumb::Breadcrumb(Window* pWindow) :
    BaseClass(pWindow),
    m_nCurrentIndex(-1),
    m_nSpacing(4),
    m_nItemPadding(6),
    m_nItemRound(4),
    m_bClickToJump(true),
    m_bMuteEvent(false),
    m_nHoveredIndex(-1),
    m_nPressedIndex(-1)
{
    //各状态默认色（语义色名，不写死色值）
    m_strSeparator = _T("\u203A"); // ›
    m_strItemColor = _T("color_blue");
    m_strCurrentColor = _T("text_default");
    m_strHoveredColor = _T("color_blue_light");
    m_strPressedColor = _T("color_blue");
    m_strDisabledColor = _T("text_disabled");
    m_strSeparatorColor = _T("color_gray");
    m_strHoveredBgColor = _T("bg_btn_hovered");
    m_strPressedBgColor = _T("bg_btn_pressed");
    m_strFont = _T("system_regular_14");
    m_strCurrentFont = _T("system_bold_14");
}

Breadcrumb::~Breadcrumb()
{
}

DString Breadcrumb::GetType() const
{
    return DUI_CTR_BREADCRUMB;
}

void Breadcrumb::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("item_text")) || (strName == _T("itemtext"))) {
        m_strItemTextIds.clear();
        ParseItemsField(strValue, false);
    }
    else if ((strName == _T("item_text_id")) || (strName == _T("itemtextid"))) {
        m_strItemTextIds = strValue;
        ParseItemsField(strValue, true);
    }
    else if ((strName == _T("current")) || (strName == _T("current_index")) || (strName == _T("currentindex"))) {
        const int32_t nIndex = StringUtil::StringToInt32(strValue);
        m_nCurrentIndex = (nIndex < 0) ? -1 : nIndex;
        //属性顺序可能与条目数据无关：条目已存在时才立即钳位
        if (!m_items.empty()) {
            ClampCurrentIndex();
        }
        Invalidate();
    }
    else if ((strName == _T("separator")) || (strName == _T("separator_text")) || (strName == _T("separatortext"))) {
        SetSeparator(strValue);
    }
    else if (strName == _T("spacing")) {
        SetSpacing(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("item_padding")) || (strName == _T("itempadding"))) {
        SetItemPadding(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("item_round")) || (strName == _T("itemround"))) {
        SetItemRound(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("click_to_jump")) || (strName == _T("clicktojump"))) {
        SetClickToJump(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("disabled_items")) || (strName == _T("disableditems"))) {
        ParseDisabledItems(strValue);
    }
    else if ((strName == _T("item_color")) || (strName == _T("itemcolor"))) {
        SetItemColor(strValue);
    }
    else if ((strName == _T("current_color")) || (strName == _T("currentcolor"))) {
        SetCurrentColor(strValue);
    }
    else if ((strName == _T("hovered_color")) || (strName == _T("hoveredcolor"))) {
        SetHoveredColor(strValue);
    }
    else if ((strName == _T("pressed_color")) || (strName == _T("pressedcolor"))) {
        SetPressedColor(strValue);
    }
    else if ((strName == _T("disabled_color")) || (strName == _T("disabledcolor"))) {
        SetDisabledColor(strValue);
    }
    else if ((strName == _T("separator_color")) || (strName == _T("separatorcolor"))) {
        SetSeparatorColor(strValue);
    }
    else if ((strName == _T("hover_bg_color")) || (strName == _T("hoverbgcolor"))) {
        SetHoveredBgColor(strValue);
    }
    else if ((strName == _T("pressed_bg_color")) || (strName == _T("pressedbgcolor"))) {
        SetPressedBgColor(strValue);
    }
    else if (strName == _T("font")) {
        SetFont(strValue);
    }
    else if ((strName == _T("current_font")) || (strName == _T("currentfont"))) {
        SetCurrentFont(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Breadcrumb::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    //语言切换时，重新解析多语言数据绑定（item_text_id）
    if (!m_strItemTextIds.empty()) {
        ParseItemsField(m_strItemTextIds, true);
    }
}

void Breadcrumb::ParseItemsField(const DString& strValue, bool bTranslate)
{
    //只有文本字段：整体替换，避免残留旧条目
    m_items.clear();
    std::list<DString> parts = StringUtil::Split(strValue, _T("|"));
    for (const DString& part : parts) {
        DString s = part;
        StringUtil::Trim(s);
        if (bTranslate && !s.empty()) {
            //多语言ID模式：按 STRID 从语言文件取实际文本
            s = GlobalManager::GetTextById(s);
        }
        Item item;
        item.text = s;
        m_items.push_back(item);
    }
    ClampCurrentIndex();
    Invalidate();
}

void Breadcrumb::ParseDisabledItems(const DString& strValue)
{
    //统一分隔符：把逗号替换为竖线，再按 `|` 拆分
    DString str = strValue;
    std::replace(str.begin(), str.end(), _T(','), _T('|'));
    std::list<DString> parts = StringUtil::Split(str, _T("|"));
    m_disabledItems.clear();
    for (const DString& part : parts) {
        DString s = part;
        StringUtil::Trim(s);
        if (s.empty()) {
            continue;
        }
        const int32_t nIndex = StringUtil::StringToInt32(s);
        if (nIndex >= 0) {
            m_disabledItems.push_back(static_cast<size_t>(nIndex));
        }
    }
    Invalidate();
}

void Breadcrumb::ClampCurrentIndex()
{
    if (m_items.empty()) {
        m_nCurrentIndex = -1;
        return;
    }
    //-1 表示自动跟随最后一项，保持不变
    if (m_nCurrentIndex < 0) {
        return;
    }
    if (m_nCurrentIndex >= static_cast<int32_t>(m_items.size())) {
        m_nCurrentIndex = static_cast<int32_t>(m_items.size()) - 1;
    }
}

size_t Breadcrumb::GetCurrentIndex() const
{
    if (m_items.empty()) {
        return 0;
    }
    if ((m_nCurrentIndex < 0) || (m_nCurrentIndex >= static_cast<int32_t>(m_items.size()))) {
        return m_items.size() - 1;
    }
    return static_cast<size_t>(m_nCurrentIndex);
}

void Breadcrumb::SetCurrentIndex(size_t nIndex)
{
    if (m_items.empty()) {
        return;
    }
    size_t nTarget = nIndex;
    if (nTarget >= m_items.size()) {
        nTarget = m_items.size() - 1;
    }
    const size_t nOld = GetCurrentIndex();
    const bool bChanged = (nTarget != nOld);
    //显式设置（即使值相同，也从"自动跟随末项"转为固定当前项）
    m_nCurrentIndex = static_cast<int32_t>(nTarget);
    if (!bChanged) {
        return;
    }
    Invalidate();
    EmitCurrentChanged(nOld);
}

void Breadcrumb::SetCurrentIndexNoEvent(size_t nIndex)
{
    const bool bOldMute = m_bMuteEvent;
    m_bMuteEvent = true;
    SetCurrentIndex(nIndex);
    m_bMuteEvent = bOldMute;
}

bool Breadcrumb::GoTo(size_t nIndex)
{
    if (m_items.empty() || (nIndex >= m_items.size())) {
        return false;
    }
    if (IsItemDisabled(nIndex)) {
        return false;
    }
    if (nIndex == GetCurrentIndex()) {
        return false;
    }
    SetCurrentIndex(nIndex);
    return true;
}

bool Breadcrumb::GoToNoEvent(size_t nIndex)
{
    const bool bOldMute = m_bMuteEvent;
    m_bMuteEvent = true;
    const bool bRet = GoTo(nIndex);
    m_bMuteEvent = bOldMute;
    return bRet;
}

void Breadcrumb::EmitCurrentChanged(size_t nOldIndex)
{
    if (m_bMuteEvent) {
        return;
    }
    //当前项通过 pEventData 携带（栈上临时对象，仅在本次同步派发期间有效）
    BreadcrumbValue value;
    value.currentIndex = GetCurrentIndex();
    value.oldIndex = nOldIndex;
    SendEvent(kEventValueChanged, (WPARAM)3, 0, static_cast<void*>(&value));
}

void Breadcrumb::AddItem(const DString& text)
{
    Item item;
    item.text = text;
    m_items.push_back(item);
    ClampCurrentIndex();
    Invalidate();
}

bool Breadcrumb::InsertItem(size_t index, const DString& text)
{
    if (index > m_items.size()) {
        return false;
    }
    Item item;
    item.text = text;
    m_items.insert(m_items.begin() + index, item);
    //显式当前索引指向的条目被插入元素挤到后面时，索引跟随其后移
    if ((m_nCurrentIndex >= 0) && (static_cast<size_t>(m_nCurrentIndex) >= index)) {
        ++m_nCurrentIndex;
    }
    ClampCurrentIndex();
    Invalidate();
    return true;
}

bool Breadcrumb::RemoveItemAt(size_t index)
{
    if (index >= m_items.size()) {
        return false;
    }
    m_items.erase(m_items.begin() + index);
    if (m_nCurrentIndex >= 0) {
        if (static_cast<size_t>(m_nCurrentIndex) > index) {
            --m_nCurrentIndex;
        }
    }
    ClampCurrentIndex();
    if (m_nHoveredIndex >= static_cast<int32_t>(m_items.size())) {
        m_nHoveredIndex = -1;
        UpdateCursor(m_nHoveredIndex);
    }
    if (m_nPressedIndex >= static_cast<int32_t>(m_items.size())) {
        m_nPressedIndex = -1;
    }
    Invalidate();
    return true;
}

void Breadcrumb::ClearItems()
{
    m_items.clear();
    m_disabledItems.clear();
    m_nCurrentIndex = -1;
    m_nHoveredIndex = -1;
    m_nPressedIndex = -1;
    UpdateCursor(m_nHoveredIndex);
    Invalidate();
}

void Breadcrumb::SetItems(const std::vector<Item>& items)
{
    m_items = items;
    ClampCurrentIndex();
    if (m_nHoveredIndex >= static_cast<int32_t>(m_items.size())) {
        m_nHoveredIndex = -1;
        UpdateCursor(m_nHoveredIndex);
    }
    if (m_nPressedIndex >= static_cast<int32_t>(m_items.size())) {
        m_nPressedIndex = -1;
    }
    Invalidate();
}

const Breadcrumb::Item* Breadcrumb::GetItemAt(size_t index) const
{
    if (index >= m_items.size()) {
        return nullptr;
    }
    return &m_items[index];
}

void Breadcrumb::SetItemEnabled(size_t index, bool bEnabled)
{
    if (index >= m_items.size()) {
        return;
    }
    auto it = std::find(m_disabledItems.begin(), m_disabledItems.end(), index);
    const bool bExists = (it != m_disabledItems.end());
    if (bEnabled && bExists) {
        m_disabledItems.erase(it);
        Invalidate();
    }
    else if (!bEnabled && !bExists) {
        m_disabledItems.push_back(index);
        if (m_nHoveredIndex == static_cast<int32_t>(index)) {
            m_nHoveredIndex = -1;
            UpdateCursor(m_nHoveredIndex);
        }
        if (m_nPressedIndex == static_cast<int32_t>(index)) {
            m_nPressedIndex = -1;
        }
        Invalidate();
    }
}

bool Breadcrumb::IsItemEnabled(size_t index) const
{
    return !IsItemDisabled(index);
}

bool Breadcrumb::IsItemDisabled(size_t index) const
{
    return std::find(m_disabledItems.begin(), m_disabledItems.end(), index) != m_disabledItems.end();
}

void Breadcrumb::SetDisabledItems(const std::vector<size_t>& disabledItems)
{
    m_disabledItems = disabledItems;
    Invalidate();
}

void Breadcrumb::SetSeparator(const DString& strSeparator)
{
    m_strSeparator = strSeparator;
    Invalidate();
}

void Breadcrumb::SetSpacing(int32_t nSpacing)
{
    m_nSpacing = (nSpacing < 0) ? 0 : nSpacing;
    Invalidate();
}

void Breadcrumb::SetItemPadding(int32_t nPadding)
{
    m_nItemPadding = (nPadding < 0) ? 0 : nPadding;
    Invalidate();
}

void Breadcrumb::SetClickToJump(bool bEnable)
{
    if (m_bClickToJump == bEnable) {
        return;
    }
    m_bClickToJump = bEnable;
    if (!m_bClickToJump) {
        m_nHoveredIndex = -1;
        m_nPressedIndex = -1;
        UpdateCursor(m_nHoveredIndex);
    }
    Invalidate();
}

void Breadcrumb::SetItemColor(const DString& strColor)
{
    m_strItemColor = strColor;
    Invalidate();
}

void Breadcrumb::SetCurrentColor(const DString& strColor)
{
    m_strCurrentColor = strColor;
    Invalidate();
}

void Breadcrumb::SetHoveredColor(const DString& strColor)
{
    m_strHoveredColor = strColor;
    Invalidate();
}

void Breadcrumb::SetPressedColor(const DString& strColor)
{
    m_strPressedColor = strColor;
    Invalidate();
}

void Breadcrumb::SetDisabledColor(const DString& strColor)
{
    m_strDisabledColor = strColor;
    Invalidate();
}

void Breadcrumb::SetSeparatorColor(const DString& strColor)
{
    m_strSeparatorColor = strColor;
    Invalidate();
}

void Breadcrumb::SetFont(const DString& strFontId)
{
    m_strFont = strFontId;
    Invalidate();
}

void Breadcrumb::SetCurrentFont(const DString& strFontId)
{
    m_strCurrentFont = strFontId;
    Invalidate();
}

void Breadcrumb::SetHoveredBgColor(const DString& strColor)
{
    m_strHoveredBgColor = strColor;
    Invalidate();
}

void Breadcrumb::SetPressedBgColor(const DString& strColor)
{
    m_strPressedBgColor = strColor;
    Invalidate();
}

void Breadcrumb::SetItemRound(int32_t nRound)
{
    m_nItemRound = (nRound < 0) ? 0 : nRound;
    Invalidate();
}

bool Breadcrumb::IsItemInteractive(size_t index) const
{
    if ((index >= m_items.size()) || !IsEnabled() || !IsMouseEnabled() || !m_bClickToJump) {
        return false;
    }
    if (index == GetCurrentIndex()) {
        return false;
    }
    return !IsItemDisabled(index);
}

int32_t Breadcrumb::MeasureTextWidth(IRender* pRender, const DString& strText, const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty()) {
        return 0;
    }
    MeasureStringParam param;
    param.pFont = GetIFontById(strFontId);
    param.uFormat = TEXT_SINGLELINE;
    return pRender->MeasureString(strText, param).Width();
}

void Breadcrumb::BuildLayout(IRender* pRender, const UiRect& rc)
{
    m_layout.clear();
    if ((pRender == nullptr) || m_items.empty() || (rc.Width() <= 0) || (rc.Height() <= 0)) {
        return;
    }

    const int32_t nPadding = Dpi().GetScaleInt(m_nItemPadding);
    const int32_t nSpacing = Dpi().GetScaleInt(m_nSpacing);

    //测量每个条目与分隔符的宽度（当前项使用加粗字体，需按对应字体测量）
    const size_t nCurrent = GetCurrentIndex();
    std::vector<int32_t> itemWidths;
    itemWidths.reserve(m_items.size());
    int32_t nTotalWidth = 0;
    for (size_t i = 0; i < m_items.size(); ++i) {
        const DString& strFontId = (i == nCurrent) ? m_strCurrentFont : m_strFont;
        int32_t nWidth = MeasureTextWidth(pRender, m_items[i].text, strFontId) + nPadding * 2;
        if (nWidth < 1) {
            nWidth = 1;
        }
        itemWidths.push_back(nWidth);
        nTotalWidth += nWidth;
    }
    const int32_t nSeparatorWidth = MeasureTextWidth(pRender, m_strSeparator, m_strFont);
    const int32_t nSeparatorCount = static_cast<int32_t>(m_items.size()) - 1;
    if (nSeparatorCount > 0) {
        nTotalWidth += nSeparatorCount * (nSeparatorWidth + nSpacing * 2);
    }

    //宽度不足时右对齐（左端由 AutoClip 裁剪），保证末项/当前项可见
    int32_t x = rc.left;
    if (nTotalWidth > rc.Width()) {
        x = rc.right - nTotalWidth;
    }

    for (size_t i = 0; i < m_items.size(); ++i) {
        LayoutEntry entry;
        entry.nItemIndex = static_cast<int32_t>(i);
        entry.rc = UiRect(x, rc.top, x + itemWidths[i], rc.bottom);
        m_layout.push_back(entry);
        x += itemWidths[i];

        if ((i + 1 < m_items.size()) && !m_strSeparator.empty()) {
            x += nSpacing;
            if (nSeparatorWidth > 0) {
                LayoutEntry separator;
                separator.nItemIndex = -1;
                separator.rc = UiRect(x, rc.top, x + nSeparatorWidth, rc.bottom);
                m_layout.push_back(separator);
            }
            x += nSeparatorWidth + nSpacing;
        }
    }
}

int32_t Breadcrumb::HitTestItem(const UiPoint& ptMouse) const
{
    for (const LayoutEntry& entry : m_layout) {
        if ((entry.nItemIndex >= 0) && entry.rc.ContainsPt(ptMouse)) {
            return entry.nItemIndex;
        }
    }
    return -1;
}

void Breadcrumb::PaintStateImages(IRender* pRender)
{
    BaseClass::PaintStateImages(pRender);
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if ((rc.Width() <= 0) || (rc.Height() <= 0) || m_items.empty()) {
        m_layout.clear();
        return;
    }

    BuildLayout(pRender, rc);
    if (m_layout.empty()) {
        return;
    }

    std::unique_ptr<AutoClip> spClip = CreateRectClip(pRender, rc, true);
    const int32_t nPadding = Dpi().GetScaleInt(m_nItemPadding);
    const int32_t nRound = Dpi().GetScaleInt(m_nItemRound);
    const int32_t nPillPadding = Dpi().GetScaleInt(2);
    const size_t nCurrent = GetCurrentIndex();

    for (const LayoutEntry& entry : m_layout) {
        if (entry.nItemIndex < 0) {
            //分隔符不参与交互，固定使用分隔符颜色
            DrawBreadcrumbText(pRender, m_strSeparator, entry.rc,
                               TEXT_HCENTER | TEXT_SINGLELINE | TEXT_VCENTER,
                               GetUiColor(m_strSeparatorColor), m_strFont);
            continue;
        }

        const size_t nIndex = static_cast<size_t>(entry.nItemIndex);
        const bool bDisabled = IsItemDisabled(nIndex);
        const bool bPressed = (m_nPressedIndex == entry.nItemIndex);
        const bool bHovered = (m_nHoveredIndex == entry.nItemIndex);

        //胶囊背景：仅在悬停/按下时绘制，悬停优先于普通态
        DString strBgColor;
        if (bPressed) {
            strBgColor = m_strPressedBgColor;
        }
        else if (bHovered) {
            strBgColor = m_strHoveredBgColor;
        }
        if (!strBgColor.empty()) {
            UiRect rcBg = entry.rc;
            rcBg.top += nPillPadding;
            rcBg.bottom -= nPillPadding;
            const UiColor bgColor = GetUiColor(strBgColor);
            if ((rcBg.Width() > 0) && (rcBg.Height() > 0) && !bgColor.IsEmpty()) {
                pRender->FillRoundRect(UiRectF((float)rcBg.left, (float)rcBg.top,
                                               (float)rcBg.right, (float)rcBg.bottom),
                                       (float)nRound, (float)nRound, bgColor);
            }
        }

        //文本颜色优先级：禁用 > 按下 > 悬停 > 当前 > 普通
        UiColor textColor;
        if (bDisabled) {
            textColor = GetUiColor(m_strDisabledColor);
        }
        else if (bPressed) {
            textColor = GetUiColor(m_strPressedColor);
        }
        else if (bHovered) {
            textColor = GetUiColor(m_strHoveredColor);
        }
        else if (nIndex == nCurrent) {
            textColor = GetUiColor(m_strCurrentColor);
        }
        else {
            textColor = GetUiColor(m_strItemColor);
        }

        //当前项使用加粗字体，突出"当前层级"
        const DString& strFontId = (nIndex == nCurrent) ? m_strCurrentFont : m_strFont;

        UiRect rcText = entry.rc;
        rcText.left += nPadding;
        rcText.right -= nPadding;
        if (rcText.Width() <= 0) {
            continue;
        }
        DrawBreadcrumbText(pRender, m_items[nIndex].text, rcText,
                           TEXT_LEFT | TEXT_SINGLELINE | TEXT_VCENTER, textColor, strFontId);
    }
    spClip.reset();
}

void Breadcrumb::DrawBreadcrumbText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat,
                                    UiColor textColor, const DString& strFontId)
{
    if ((pRender == nullptr) || strText.empty()) {
        return;
    }
    if (textColor.IsEmpty()) {
        textColor = UiColor(0xFF000000);
    }
    DrawStringParam param;
    param.textRect = rc;
    param.dwTextColor = textColor;
    param.pFont = GetIFontById(strFontId);
    param.uFormat = uFormat;
    pRender->DrawString(strText, param);
}

void Breadcrumb::UpdateCursor(int32_t nHoveredIndex)
{
    if (nHoveredIndex >= 0) {
        SetCursorType(CursorType::kCursorHand);
    }
    else {
        SetCursorType(CursorType::kCursorArrow);
    }
}

bool Breadcrumb::ButtonDown(const EventArgs& msg)
{
    if (IsEnabled() && IsMouseEnabled() && m_bClickToJump && !m_items.empty()) {
        const int32_t nHit = HitTestItem(msg.ptMouse);
        if ((nHit >= 0) && IsItemInteractive(static_cast<size_t>(nHit))) {
            //记录按下态，并获取鼠标焦点，使按住期间持续收到鼠标消息以结束按下态
            m_nPressedIndex = nHit;
            SetMouseFocused(true);
            Invalidate();
            GoTo(static_cast<size_t>(nHit));
            return true;
        }
    }
    return BaseClass::ButtonDown(msg);
}

bool Breadcrumb::ButtonUp(const EventArgs& msg)
{
    if (m_nPressedIndex >= 0) {
        m_nPressedIndex = -1;
        Invalidate();
    }
    return BaseClass::ButtonUp(msg);
}

bool Breadcrumb::MouseMove(const EventArgs& msg)
{
    if (msg.eventType == kEventMouseMove) {
        //更新悬停项（仅可交互的祖先项参与）
        int32_t nHit = -1;
        if (IsEnabled() && IsMouseEnabled() && m_bClickToJump && !m_items.empty()) {
            nHit = HitTestItem(msg.ptMouse);
            if ((nHit >= 0) && !IsItemInteractive(static_cast<size_t>(nHit))) {
                nHit = -1;
            }
        }
        if (nHit != m_nHoveredIndex) {
            m_nHoveredIndex = nHit;
            UpdateCursor(m_nHoveredIndex);
            Invalidate();
        }
    }
    return BaseClass::MouseMove(msg);
}

bool Breadcrumb::MouseLeave(const EventArgs& msg)
{
    if ((m_nHoveredIndex >= 0) || (m_nPressedIndex >= 0)) {
        m_nHoveredIndex = -1;
        m_nPressedIndex = -1;
        UpdateCursor(m_nHoveredIndex);
        Invalidate();
    }
    return BaseClass::MouseLeave(msg);
}

} // namespace ui
