/* 2026-10 新增：NavigationView 侧边栏导航控件（NavigationView + NavigationViewItem） */

#include "duilib/Control/NavigationView.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/ScrollBox.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/Line.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>

namespace ui
{

// ============================================================================
// NavigationViewItem
// ============================================================================

NavigationViewItem::NavigationViewItem(Window* pWindow) :
    Control(pWindow),
    m_itemType(ItemType::kItem),
    m_bSelected(false),
    m_bHovered(false),
    m_bCompact(false),
    m_pOwner(nullptr),
    m_nIconSize(16),
    m_nIconLeft(16),
    m_nTextLeft(44),
    m_nTextRight(8),
    m_nPillRadius(6),
    m_nPillInsetX(6),
    m_nPillInsetY(3),
    m_selectedBgColor(_T("bg_list_item_selected")),
    m_hoveredBgColor(_T("bg_list_item_hovered")),
    m_selectedTextColor(_T("color_accent")),
    m_normalTextColor(_T("text_default")),
    m_disabledTextColor(_T("text_disabled")),
    m_headerTextColor(_T("text_muted")),
    m_separatorColor(_T("border_window"))
{
}

DString NavigationViewItem::GetType() const
{
    return DUI_CTR_NAVIGATION_VIEW_ITEM;
}

void NavigationViewItem::OnInit()
{
    BaseClass::OnInit();
    //不同形态使用不同的固定高度（DIP，DPI 自适应）
    switch (m_itemType) {
    case ItemType::kItem:
        SetFixedHeight(UiFixedInt(36), true, true);
        break;
    case ItemType::kHeader:
        SetFixedHeight(UiFixedInt(32), true, true);
        break;
    case ItemType::kSeparator:
        SetFixedHeight(UiFixedInt(9), true, true);
        break;
    default:
        break;
    }
}

void NavigationViewItem::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("text")) {
        SetItemText(strValue);
    }
    else if (strName == _T("text_id")) {
        SetItemTextId(strValue);
    }
    else if (strName == _T("icon")) {
        SetIconAttr(strValue);
    }
    else if (strName == _T("page")) {
        SetPageName(strValue);
    }
    else if (strName == _T("item_type")) {
        if ((strValue == _T("header")) || (strValue == _T("group_header"))) {
            SetItemType(ItemType::kHeader);
        }
        else if ((strValue == _T("separator")) || (strValue == _T("sep"))) {
            SetItemType(ItemType::kSeparator);
        }
        else {
            SetItemType(ItemType::kItem);
        }
    }
    else if (strName == _T("icon_size")) {
        int32_t nValue = StringUtil::StringToInt32(strValue);
        if (nValue > 0) {
            m_nIconSize = nValue;
        }
    }
    else if (strName == _T("icon_left")) {
        m_nIconLeft = StringUtil::StringToInt32(strValue);
    }
    else if (strName == _T("text_left")) {
        m_nTextLeft = StringUtil::StringToInt32(strValue);
    }
    else if (strName == _T("text_right")) {
        m_nTextRight = StringUtil::StringToInt32(strValue);
    }
    else if (strName == _T("pill_radius")) {
        m_nPillRadius = StringUtil::StringToInt32(strValue);
    }
    else if (strName == _T("pill_inset_x")) {
        m_nPillInsetX = StringUtil::StringToInt32(strValue);
    }
    else if (strName == _T("pill_inset_y")) {
        m_nPillInsetY = StringUtil::StringToInt32(strValue);
    }
    else if (strName == _T("selected_bg_color")) {
        m_selectedBgColor = strValue;
    }
    else if (strName == _T("hovered_bg_color")) {
        m_hoveredBgColor = strValue;
    }
    else if (strName == _T("selected_text_color")) {
        m_selectedTextColor = strValue;
    }
    else if (strName == _T("normal_text_color")) {
        m_normalTextColor = strValue;
    }
    else if (strName == _T("disabled_text_color")) {
        m_disabledTextColor = strValue;
    }
    else if (strName == _T("header_text_color")) {
        m_headerTextColor = strValue;
    }
    else if (strName == _T("separator_color")) {
        m_separatorColor = strValue;
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void NavigationViewItem::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }
    switch (m_itemType) {
    case ItemType::kItem:
        PaintItem(pRender);
        break;
    case ItemType::kHeader:
        PaintHeader(pRender);
        break;
    case ItemType::kSeparator:
        PaintSeparator(pRender);
        break;
    default:
        break;
    }
}

void NavigationViewItem::PaintStateImages(IRender* /*pRender*/)
{
    //前景图标由 PaintItem 按展开/紧凑布局自行定位绘制，屏蔽基类在整行区域的默认绘制，避免图标重复/错位
}

bool NavigationViewItem::MouseEnter(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseEnter(msg);
    //禁用态不显示悬停热区
    if (IsSelectable() && IsEnabled() && !m_bHovered) {
        m_bHovered = true;
        Invalidate();
    }
    return bRet;
}

bool NavigationViewItem::MouseLeave(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseLeave(msg);
    if (m_bHovered) {
        m_bHovered = false;
        Invalidate();
    }
    return bRet;
}

bool NavigationViewItem::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    //禁用态不触发选中，避免误操作
    if (IsSelectable() && IsEnabled() && (msg.GetSender() == this)) {
        if (m_pOwner != nullptr) {
            m_pOwner->OnItemClicked(this);
        }
    }
    return bRet;
}

void NavigationViewItem::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    if (bRedraw) {
        Invalidate();
    }
}

uint32_t NavigationViewItem::GetControlFlags() const
{
    //可选中且可用时允许 Tab 聚焦（用于键盘导航）
    return (IsSelectable() && IsEnabled() && IsKeyboardEnabled()) ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

void NavigationViewItem::HandleEvent(const EventArgs& msg)
{
    if ((msg.eventType == kEventKeyDown) && IsSelectable() && IsEnabled()) {
        if ((msg.vkCode == kVK_RETURN) || (msg.vkCode == kVK_SPACE)) {
            //回车/空格：激活当前项（等价于鼠标点击）
            if (m_pOwner != nullptr) {
                m_pOwner->OnItemClicked(this);
            }
            return;
        }
        if ((msg.vkCode == kVK_UP) || (msg.vkCode == kVK_DOWN)) {
            //上/下方向键：在可选项之间移动焦点
            if (m_pOwner != nullptr) {
                m_pOwner->MoveFocusByKey(this, msg.vkCode == kVK_DOWN);
            }
            return;
        }
    }
    BaseClass::HandleEvent(msg);
}

void NavigationViewItem::SetItemText(const DString& strText)
{
    m_text = strText;
    //紧凑态下用 tooltip 显示完整文字
    if (m_bCompact && IsSelectable()) {
        SetToolTipText(GetDisplayText());
    }
    Invalidate();
}

DString NavigationViewItem::GetItemText() const
{
    return m_text.c_str();
}

void NavigationViewItem::SetItemTextId(const DString& strTextId)
{
    m_textId = strTextId;
    if (m_bCompact && IsSelectable()) {
        SetToolTipText(GetDisplayText());
    }
    Invalidate();
}

DString NavigationViewItem::GetItemTextId() const
{
    return m_textId.c_str();
}

DString NavigationViewItem::GetDisplayText() const
{
    if (!m_textId.empty()) {
        DString strText = GlobalManager::GetTextById(m_textId.c_str());
        if (!strText.empty()) {
            return strText;
        }
    }
    return m_text.c_str();
}

void NavigationViewItem::SetIconAttr(const DString& strIcon)
{
    m_iconAttr = strIcon;
    if (!strIcon.empty()) {
        SetForeStateImage(kControlStateNormal, strIcon);
    }
}

DString NavigationViewItem::GetIconAttr() const
{
    return m_iconAttr.c_str();
}

void NavigationViewItem::SetPageName(const DString& strPage)
{
    m_pageName = strPage;
}

DString NavigationViewItem::GetPageName() const
{
    return m_pageName.c_str();
}

void NavigationViewItem::SetItemType(ItemType itemType)
{
    m_itemType = itemType;
}

void NavigationViewItem::SetSelected(bool bSelected)
{
    if (m_bSelected == bSelected) {
        return;
    }
    m_bSelected = bSelected;
    Invalidate();
}

void NavigationViewItem::SetCompact(bool bCompact)
{
    if (m_bCompact == bCompact) {
        return;
    }
    m_bCompact = bCompact;
    if (IsSelectable()) {
        SetToolTipText(bCompact ? GetDisplayText() : DString());
    }
    else if (m_itemType != ItemType::kItem) {
        //紧凑态下分组标题/分隔线不绘制，同时收起占位高度
        SetFixedHeight(UiFixedInt(bCompact ? 0 : (m_itemType == ItemType::kHeader ? 32 : 9)), true, true);
    }
    Invalidate();
}

void NavigationViewItem::PaintItem(IRender* pRender)
{
    UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }

    //选中/悬停背景（圆角热区，内缩量可定制）
    UiColor bgColor;
    if (m_bSelected) {
        bgColor = GetUiColor(m_selectedBgColor.c_str());
    }
    else if (m_bHovered && IsEnabled()) {
        bgColor = GetUiColor(m_hoveredBgColor.c_str());
    }
    const int32_t nInsetX = Dpi().GetScaleInt(m_nPillInsetX);
    const int32_t nInsetY = Dpi().GetScaleInt(m_nPillInsetY);
    const float fRadius = Dpi().GetScaleFloat(static_cast<float>(m_nPillRadius));
    if (!bgColor.IsEmpty()) {
        UiRect rcPill = rc;
        rcPill.Deflate(nInsetX, nInsetY);
        UiRectF rcPillF;
        rcPillF.left = static_cast<float>(rcPill.left);
        rcPillF.top = static_cast<float>(rcPill.top);
        rcPillF.right = static_cast<float>(rcPill.right);
        rcPillF.bottom = static_cast<float>(rcPill.bottom);
        pRender->FillRoundRect(rcPillF, fRadius, fRadius, bgColor);
    }

    //键盘焦点指示：在热区外沿绘制强调色描边（仅焦点态）
    if (IsFocused() && IsEnabled()) {
        UiColor focusColor = GetUiColor(_T("color_accent"));
        if (!focusColor.IsEmpty()) {
            UiRect rcRing = rc;
            rcRing.Deflate(nInsetX, nInsetY);
            UiRectF rcRingF;
            rcRingF.left = static_cast<float>(rcRing.left);
            rcRingF.top = static_cast<float>(rcRing.top);
            rcRingF.right = static_cast<float>(rcRing.right);
            rcRingF.bottom = static_cast<float>(rcRing.bottom);
            const float fStroke = Dpi().GetScaleFloat(1.0f);
            pRender->DrawRoundRect(rcRingF, fRadius, fRadius, focusColor, fStroke);
        }
    }

    const int32_t nIconSize = Dpi().GetScaleInt(m_nIconSize);
    UiRect rcIcon;
    rcIcon.top = rc.CenterY() - nIconSize / 2;
    rcIcon.bottom = rcIcon.top + nIconSize;
    if (m_bCompact) {
        //紧凑态：图标在窗格内水平居中
        rcIcon.left = rc.CenterX() - nIconSize / 2;
        rcIcon.right = rcIcon.left + nIconSize;
    }
    else {
        //展开态：图标左边距可定制
        rcIcon.left = rc.left + Dpi().GetScaleInt(m_nIconLeft);
        rcIcon.right = rcIcon.left + nIconSize;
    }

    //绘制图标（禁用态做半透明淡出处理）
    Image* pImage = GetStateImageData(kStateImageFore, kControlStateNormal);
    if (pImage != nullptr) {
        const int32_t nFade = IsEnabled() ? -1 : 110;   //-1 表示使用默认不透明度
        PaintImage(pRender, pImage, _T(""), nFade, nullptr, &rcIcon, nullptr);
    }

    //展开态绘制文字；紧凑态仅显示图标
    if (!m_bCompact) {
        DString strText = GetDisplayText();
        if (!strText.empty()) {
            UiRect rcText = rc;
            rcText.left = rc.left + Dpi().GetScaleInt(m_nTextLeft);
            rcText.right = rc.right - Dpi().GetScaleInt(m_nTextRight);
            UiColor textColor;
            if (!IsEnabled()) {
                //禁用态：弱化色
                textColor = GetUiColor(m_disabledTextColor.c_str());
            }
            if (textColor.IsEmpty()) {
                textColor = m_bSelected ? GetUiColor(m_selectedTextColor.c_str())
                                        : GetUiColor(m_normalTextColor.c_str());
            }
            if (textColor.IsEmpty()) {
                textColor = UiColor(0xFF000000);
            }
            IFont* pFont = GetIFontById(_T("system_regular_14"));
            DrawStringParam param;
            param.textRect = rcText;
            param.dwTextColor = textColor;
            param.pFont = pFont;
            param.uFormat = TEXT_LEFT | TEXT_VCENTER | TEXT_SINGLELINE | TEXT_END_ELLIPSIS;
            pRender->DrawString(strText, param);
        }
    }
}

void NavigationViewItem::PaintHeader(IRender* pRender)
{
    //分组标题不响应点击、不绘制热区
    UiRect rc = GetRect();
    if (rc.IsEmpty() || m_bCompact) {
        //紧凑态下隐藏分组标题文字（保留占位高度会浪费空间，这里直接不绘制）
        return;
    }
    DString strText = GetDisplayText();
    if (strText.empty()) {
        return;
    }
    UiRect rcText = rc;
    rcText.left = rc.left + Dpi().GetScaleInt(16);
    rcText.right = rc.right - Dpi().GetScaleInt(8);
    UiColor textColor = GetUiColor(m_headerTextColor.c_str());
    if (textColor.IsEmpty()) {
        textColor = UiColor(0xFF808080);
    }
    IFont* pFont = GetIFontById(_T("system_regular_12"));
    DrawStringParam param;
    param.textRect = rcText;
    param.dwTextColor = textColor;
    param.pFont = pFont;
    param.uFormat = TEXT_LEFT | TEXT_VCENTER | TEXT_SINGLELINE | TEXT_END_ELLIPSIS;
    pRender->DrawString(strText, param);
}

void NavigationViewItem::PaintSeparator(IRender* pRender)
{
    UiRect rc = GetRect();
    if (rc.IsEmpty() || m_bCompact) {
        return;
    }
    UiColor lineColor = GetUiColor(m_separatorColor.c_str());
    if (lineColor.IsEmpty()) {
        lineColor = UiColor(0xFFD5D5D5);
    }
    const int32_t nMargin = Dpi().GetScaleInt(12);
    UiRectF rcLine;
    rcLine.left = static_cast<float>(rc.left + nMargin);
    rcLine.right = static_cast<float>(rc.right - nMargin);
    rcLine.top = static_cast<float>(rc.CenterY());
    rcLine.bottom = rcLine.top + 1.0f;
    pRender->FillRect(rcLine, lineColor);
}

// ============================================================================
// NavigationView
// ============================================================================

NavigationView::NavigationView(Window* pWindow) :
    HBox(pWindow),
    m_bInternalsReady(false),
    m_nPaneWidth(220),
    m_nCompactPaneWidth(48),
    m_bCollapsed(false),
    m_bShowHeader(true),
    m_bSettingsItem(false),
    m_pPane(nullptr),
    m_pTopBar(nullptr),
    m_pToggleBtn(nullptr),
    m_pPaneTitleLabel(nullptr),
    m_pItemHost(nullptr),
    m_pBottomHost(nullptr),
    m_pSettingsNavItem(nullptr),
    m_pSeparator(nullptr),
    m_pRight(nullptr),
    m_pHeader(nullptr),
    m_pContent(nullptr),
    m_pSelected(nullptr)
{
    m_settingsPage = _T("settings");
    //设置项文字/图标与汉堡按钮图标均由使用方通过属性提供，控件本身不内置具体资源
}

NavigationView::~NavigationView()
{
    //子控件由父容器持有，内部容器会在析构时统一释放；此处仅解除弱引用
    m_pSelected = nullptr;
    m_pSettingsNavItem = nullptr;
    m_items.clear();
}

DString NavigationView::GetType() const
{
    return DUI_CTR_NAVIGATIONVIEW;
}

void NavigationView::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("pane_width")) {
        int32_t nValue = StringUtil::StringToInt32(strValue);
        if (nValue > 0) {
            m_nPaneWidth = nValue;
        }
    }
    else if (strName == _T("compact_pane_width")) {
        int32_t nValue = StringUtil::StringToInt32(strValue);
        if (nValue > 0) {
            m_nCompactPaneWidth = nValue;
        }
    }
    else if (strName == _T("pane_title")) {
        m_paneTitle = strValue;
        if (m_pPaneTitleLabel != nullptr) {
            m_pPaneTitleLabel->SetText(strValue);
        }
    }
    else if (strName == _T("pane_title_id")) {
        m_paneTitleId = strValue;
        if (m_pPaneTitleLabel != nullptr) {
            m_pPaneTitleLabel->SetTextId(strValue);
        }
    }
    else if (strName == _T("toggle_icon")) {
        m_toggleIcon = strValue;
        if ((m_pToggleBtn != nullptr) && !strValue.empty()) {
            m_pToggleBtn->SetStateImage(kControlStateNormal, strValue);
        }
    }
    else if (strName == _T("collapsed")) {
        m_bCollapsed = StringUtil::IsValueTrue(strValue);
    }
    else if (strName == _T("show_header")) {
        m_bShowHeader = StringUtil::IsValueTrue(strValue);
    }
    else if (strName == _T("settings_item")) {
        m_bSettingsItem = StringUtil::IsValueTrue(strValue);
    }
    else if (strName == _T("settings_page")) {
        m_settingsPage = strValue;
    }
    else if (strName == _T("settings_text_id")) {
        m_settingsTextId = strValue;
    }
    else if (strName == _T("settings_icon")) {
        m_settingsIcon = strValue;
    }
    else if (strName == _T("selected_id")) {
        m_initSelectedId = strValue;
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

bool NavigationView::AddItem(Control* pControl)
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return false;
    }
    //首个用户子节点加入前完成内部结构创建
    EnsureInternals();

    NavigationViewItem* pItem = dynamic_cast<NavigationViewItem*>(pControl);
    if (pItem != nullptr) {
        //导航项（含分组标题/分隔线）进入窗格列表
        RegisterNavItem(pItem);
        return m_pItemHost->AddItem(pItem);
    }

    TabBox* pTabBox = dynamic_cast<TabBox*>(pControl);
    if (pTabBox != nullptr) {
        //使用方显式提供内容区 TabBox：接管为内容容器（仅允许一个）
        if (m_pContent != nullptr) {
            ASSERT(m_pContent == nullptr);
            return false;
        }
        m_pContent = pTabBox;
        m_pContent->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
        m_pContent->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
        return m_pRight->AddItem(pControl);
    }

    //其他控件作为内容页进入内容区；未显式提供 TabBox 时自动创建
    EnsureContentHost();
    return m_pContent->AddItem(pControl);
}

void NavigationView::OnInit()
{
    BaseClass::OnInit();
    //属性可能在内部结构创建前设置，此处统一同步一次
    if (!m_paneTitle.empty() && (m_pPaneTitleLabel != nullptr)) {
        m_pPaneTitleLabel->SetText(m_paneTitle.c_str());
    }
    if (!m_paneTitleId.empty() && (m_pPaneTitleLabel != nullptr)) {
        m_pPaneTitleLabel->SetTextId(m_paneTitleId.c_str());
    }

    //底部设置项（加入窗格底部固定区，导航列表滚动时仍固定在底部）
    if (m_bSettingsItem && (m_pSettingsNavItem == nullptr)) {
        m_pSettingsNavItem = new NavigationViewItem(GetWindow());
        m_pSettingsNavItem->SetPageName(m_settingsPage.c_str());
        m_pSettingsNavItem->SetItemTextId(m_settingsTextId.c_str());
        if (!m_settingsIcon.empty()) {
            m_pSettingsNavItem->SetIconAttr(m_settingsIcon.c_str());
        }
        //设置项同样登记（供点击联动/查找），但 IsIndexedItem 会将其排除在序号统计之外
        RegisterNavItem(m_pSettingsNavItem);
        if (m_pBottomHost != nullptr) {
            m_pBottomHost->AddItem(m_pSettingsNavItem);
        }
        else {
            m_pPane->AddItem(m_pSettingsNavItem);
        }
    }

    ApplyCollapsed();
    ApplyHeaderVisible();
    DoInitialSelection();
}

void NavigationView::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    //页头文字跟随当前选中项的语言文本刷新
    UpdateHeaderText(m_pSelected);
}

void NavigationView::EnsureInternals()
{
    if (m_bInternalsReady) {
        return;
    }
    m_bInternalsReady = true;
    Window* pWindow = GetWindow();

    //--- 左侧导航窗格 ---
    m_pPane = new VBox(pWindow);
    m_pPane->SetBkColor(_T("bg_window_card"));
    m_pPane->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
    BaseClass::AddItem(m_pPane);

    //窗格与内容区间的 1px 竖分隔线：使用专用区域分隔语义色 + 实线，柔和不突兀
    m_pSeparator = new Line(pWindow);
    m_pSeparator->SetAttribute(_T("vertical"), _T("true"));
    m_pSeparator->SetAttribute(_T("dash_style"), _T("solid"));
    m_pSeparator->SetAttribute(_T("line_color"), _T("border_split_level1"));
    m_pSeparator->SetFixedWidth(UiFixedInt(1), true, true);
    m_pSeparator->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
    BaseClass::AddItem(m_pSeparator);

    //--- 右侧：页头 + 内容 ---
    m_pRight = new VBox(pWindow);
    m_pRight->SetBkColor(_T("bg_window_main"));
    m_pRight->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
    m_pRight->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
    BaseClass::AddItem(m_pRight);

    //窗格顶部：汉堡按钮 + 窗格标题
    m_pTopBar = new HBox(pWindow);
    m_pTopBar->SetBkColor(_T("bg_window_card"));
    m_pTopBar->SetFixedHeight(UiFixedInt(44), true, true);
    m_pPane->AddItem(m_pTopBar);

    m_pToggleBtn = new Button(pWindow);
    m_pToggleBtn->SetFixedWidth(UiFixedInt(48), true, true);
    m_pToggleBtn->SetFixedHeight(UiFixedInt(44), true, true);
    m_pToggleBtn->SetAttribute(_T("border_size"), _T("0"));
    m_pToggleBtn->SetAttribute(_T("normal_color"), _T("bg_window_card"));
    m_pToggleBtn->SetAttribute(_T("hovered_color"), _T("bg_list_item_hovered"));
    m_pToggleBtn->SetAttribute(_T("pressed_color"), _T("bg_list_item_selected"));
    if (!m_toggleIcon.empty()) {
        m_pToggleBtn->SetStateImage(kControlStateNormal, m_toggleIcon.c_str());
    }
    m_pToggleBtn->AttachClick([this](const EventArgs& /*msg*/) {
        OnToggleClicked();
        return true;
    });
    m_pTopBar->AddItem(m_pToggleBtn);

    m_pPaneTitleLabel = new Label(pWindow);
    m_pPaneTitleLabel->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
    m_pPaneTitleLabel->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
    m_pPaneTitleLabel->SetAttribute(_T("font"), _T("system_bold_14"));
    m_pPaneTitleLabel->SetAttribute(_T("text_color"), _T("text_default"));
    m_pPaneTitleLabel->SetAttribute(_T("text_align"), _T("vcenter"));
    m_pPaneTitleLabel->SetAttribute(_T("padding"), _T("4,0,8,0"));
    if (!m_paneTitleId.empty()) {
        m_pPaneTitleLabel->SetTextId(m_paneTitleId.c_str());
    }
    else {
        m_pPaneTitleLabel->SetText(m_paneTitle.c_str());
    }
    m_pTopBar->AddItem(m_pPaneTitleLabel);

    //导航项可滚动列表（纵向拉伸占据窗格剩余空间）
    m_pItemHost = new VScrollBox(pWindow);
    m_pItemHost->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
    m_pItemHost->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
    m_pItemHost->SetAttribute(_T("vscrollbar"), _T("true")); //支持纵向滚动条
    m_pPane->AddItem(m_pItemHost);

    //窗格底部固定区（设置项所在；高度 auto，不随导航列表滚动，导航项较多时依然固定在底部）
    m_pBottomHost = new VBox(pWindow);
    m_pBottomHost->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
    m_pBottomHost->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
    m_pPane->AddItem(m_pBottomHost);

    //内容区页头
    m_pHeader = new Label(pWindow);
    m_pHeader->SetBkColor(_T("bg_window_main"));
    m_pHeader->SetFixedHeight(UiFixedInt(48), true, true);
    m_pHeader->SetAttribute(_T("font"), _T("system_bold_18"));
    m_pHeader->SetAttribute(_T("text_color"), _T("text_default"));
    m_pHeader->SetAttribute(_T("text_align"), _T("vcenter"));
    m_pHeader->SetAttribute(_T("padding"), _T("20,0,20,0"));
    m_pRight->AddItem(m_pHeader);
}

void NavigationView::EnsureContentHost()
{
    if (m_pContent != nullptr) {
        return;
    }
    //使用方未显式提供 TabBox 时，自动创建内容页面容器
    m_pContent = new TabBox(GetWindow());
    m_pContent->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
    m_pContent->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
    m_pRight->AddItem(m_pContent);
}

void NavigationView::RegisterNavItem(NavigationViewItem* pItem)
{
    ASSERT(pItem != nullptr);
    if (pItem == nullptr) {
        return;
    }
    pItem->SetOwnerView(this);
    pItem->SetCompact(m_bCollapsed);
    m_items.push_back(pItem);
}

void NavigationView::UnregisterNavItem(NavigationViewItem* pItem)
{
    if (pItem == nullptr) {
        return;
    }
    auto it = std::find(m_items.begin(), m_items.end(), pItem);
    if (it != m_items.end()) {
        m_items.erase(it);
    }
    if (m_pSelected == pItem) {
        m_pSelected = nullptr;
        UpdateHeaderText(nullptr);
    }
}

bool NavigationView::IsIndexedItem(const NavigationViewItem* pItem) const
{
    //参与序号统计的项：可选中（item 形态）、非设置项
    return (pItem != nullptr) && pItem->IsSelectable() && (pItem != m_pSettingsNavItem);
}

NavigationViewItem* NavigationView::FindItemByPage(const DString& strPageName) const
{
    if (strPageName.empty()) {
        return nullptr;
    }
    for (NavigationViewItem* pItem : m_items) {
        if ((pItem != nullptr) && pItem->IsSelectable() && (pItem->GetPageName() == strPageName)) {
            return pItem;
        }
    }
    return nullptr;
}

void NavigationView::ApplyCollapsed()
{
    if (m_pPane != nullptr) {
        const int32_t nWidth = m_bCollapsed ? m_nCompactPaneWidth : m_nPaneWidth;
        m_pPane->SetFixedWidth(UiFixedInt(nWidth), true, true);
    }
    if (m_pPaneTitleLabel != nullptr) {
        m_pPaneTitleLabel->SetVisible(!m_bCollapsed);
    }
    for (NavigationViewItem* pItem : m_items) {
        if (pItem != nullptr) {
            pItem->SetCompact(m_bCollapsed);
        }
    }
}

void NavigationView::ApplyHeaderVisible()
{
    if (m_pHeader != nullptr) {
        m_pHeader->SetVisible(m_bShowHeader);
    }
}

void NavigationView::UpdateHeaderText(NavigationViewItem* pItem)
{
    if (m_pHeader == nullptr) {
        return;
    }
    if (pItem != nullptr) {
        m_pHeader->SetText(pItem->GetDisplayText());
    }
    else {
        m_pHeader->SetText(_T(""));
    }
}

void NavigationView::DoInitialSelection()
{
    NavigationViewItem* pInitItem = nullptr;
    if (!m_initSelectedId.empty()) {
        pInitItem = FindItemByPage(m_initSelectedId.c_str());
    }
    if (pInitItem == nullptr) {
        //默认选中第一个可选项（不含设置项，避免设置页成为初始页）
        for (NavigationViewItem* pItem : m_items) {
            if (IsIndexedItem(pItem)) {
                pInitItem = pItem;
                break;
            }
        }
    }
    if (pInitItem != nullptr) {
        SelectItem(pInitItem, false);
    }
}

void NavigationView::OnToggleClicked()
{
    SetCollapsed(!m_bCollapsed);
}

void NavigationView::OnItemClicked(NavigationViewItem* pItem)
{
    if ((pItem == nullptr) || !pItem->IsSelectable()) {
        return;
    }
    //先触发"导航项被点击"事件：重复点击已选中项也会触发，WPARAM 为可选项序号
    size_t nIndex = 0;
    for (NavigationViewItem* p : m_items) {
        if (IsIndexedItem(p)) {
            if (p == pItem) {
                SendEvent(kEventNavigationItemClick, static_cast<WPARAM>(nIndex), 0);
                break;
            }
            ++nIndex;
        }
    }
    SelectItem(pItem, true);
}

void NavigationView::MoveFocusByKey(NavigationViewItem* pFrom, bool bForward)
{
    if (pFrom == nullptr) {
        return;
    }
    //收集所有可聚焦项（含设置项；顺序按 m_items，与视觉顺序基本一致）
    std::vector<NavigationViewItem*> focusable;
    focusable.reserve(m_items.size());
    for (NavigationViewItem* p : m_items) {
        if ((p != nullptr) && p->IsSelectable() && p->IsEnabled()) {
            focusable.push_back(p);
        }
    }
    if (focusable.empty()) {
        return;
    }
    auto it = std::find(focusable.begin(), focusable.end(), pFrom);
    int32_t nIndex = (it == focusable.end()) ? (bForward ? -1 : 0)
                                             : static_cast<int32_t>(std::distance(focusable.begin(), it));
    const int32_t nCount = static_cast<int32_t>(focusable.size());
    //循环移动
    nIndex = bForward ? (nIndex + 1) : (nIndex - 1);
    if (nIndex < 0) {
        nIndex = nCount - 1;
    }
    else if (nIndex >= nCount) {
        nIndex = 0;
    }
    NavigationViewItem* pTarget = focusable[static_cast<size_t>(nIndex)];
    if ((pTarget != nullptr) && (pTarget != pFrom)) {
        pTarget->SetFocus();
    }
}

void NavigationView::SetCollapsed(bool bCollapsed)
{
    if (m_bCollapsed == bCollapsed) {
        return;
    }
    //派发"即将收起/展开"事件：所有监听者都返回 true 才放行（返回 false 取消本次切换）
    EventArgs msg;
    msg.eventType = kEventNavigationPaneToggling;
    msg.SetSender(this);
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        msg.ptMouse = pWindow->GetLastMousePos();
    }
    msg.wParam = bCollapsed ? 1 : 0;
    if (!FireAllEvents(msg)) {
        return;
    }

    m_bCollapsed = bCollapsed;
    ApplyCollapsed();

    //派发"收起/展开完成"事件，WPARAM 为当前状态（1=收起，0=展开）
    SendEvent(kEventNavigationPaneToggled, static_cast<WPARAM>(bCollapsed ? 1 : 0), 0);
}

bool NavigationView::SelectItem(NavigationViewItem* pItem, bool bFireEvent)
{
    if ((pItem == nullptr) || !pItem->IsSelectable()) {
        return false;
    }
    size_t nOldIndex = GetSelectedIndex();
    if (m_pSelected != pItem) {
        if (m_pSelected != nullptr) {
            m_pSelected->SetSelected(false);
        }
        m_pSelected = pItem;
        pItem->SetSelected(true);
    }

    //联动切换内容页
    DString strPage = pItem->GetPageName();
    if (!strPage.empty() && (m_pContent != nullptr)) {
        m_pContent->SelectItem(strPage);
    }
    UpdateHeaderText(pItem);

    if (bFireEvent) {
        //选中变化事件：WPARAM 新序号，LPARAM 旧序号
        size_t nNewIndex = GetSelectedIndex();
        if (nNewIndex != nOldIndex) {
            SendEvent(kEventNavigationSelectionChanged,
                      static_cast<WPARAM>(nNewIndex), static_cast<LPARAM>(nOldIndex));
        }
    }
    return true;
}

bool NavigationView::SelectItem(const DString& strPageName)
{
    NavigationViewItem* pItem = FindItemByPage(strPageName);
    return SelectItem(pItem, true);
}

bool NavigationView::SelectItemByIndex(size_t nIndex)
{
    size_t nSelectable = 0;
    for (NavigationViewItem* pItem : m_items) {
        if (IsIndexedItem(pItem)) {
            if (nSelectable == nIndex) {
                return SelectItem(pItem, true);
            }
            ++nSelectable;
        }
    }
    return false;
}

NavigationViewItem* NavigationView::GetSelectedItem() const
{
    return m_pSelected;
}

size_t NavigationView::GetSelectedIndex() const
{
    size_t nSelectable = 0;
    for (NavigationViewItem* pItem : m_items) {
        if (IsIndexedItem(pItem)) {
            if (pItem == m_pSelected) {
                return nSelectable;
            }
            ++nSelectable;
        }
    }
    return Box::InvalidIndex;
}

DString NavigationView::GetSelectedPage() const
{
    if (m_pSelected != nullptr) {
        return m_pSelected->GetPageName();
    }
    return DString();
}

NavigationViewItem* NavigationView::AddNavItem(const DString& strTextId, const DString& strIcon,
                                               const DString& strPage, Control* pPage)
{
    EnsureInternals();
    //先挂内容页（若有）：确保点击导航项时右侧能按 page name 命中，否则会出现"有节点无页面"
    if (pPage != nullptr) {
        //页面 name 与导航项 page 对齐：未命名时以 strPage 命名，保证联动可命中
        if (!strPage.empty() && pPage->GetName().empty()) {
            pPage->SetName(strPage);
        }
        if (!AddPage(pPage)) {
            return nullptr;
        }
    }

    NavigationViewItem* pItem = new NavigationViewItem(GetWindow());
    if (!strTextId.empty()) {
        pItem->SetItemTextId(strTextId);
    }
    if (!strIcon.empty()) {
        pItem->SetIconAttr(strIcon);
    }
    pItem->SetPageName(strPage);
    RegisterNavItem(pItem);
    if (!m_pItemHost->AddItem(pItem)) {
        UnregisterNavItem(pItem);
        delete pItem;
        return nullptr;
    }
    return pItem;
}

bool NavigationView::AddPage(Control* pPage)
{
    if (pPage == nullptr) {
        return false;
    }
    //其他控件作为内容页进入内容区；未显式提供 TabBox 时自动创建
    EnsureContentHost();
    if (m_pContent == nullptr) {
        return false;
    }
    return m_pContent->AddItem(pPage);
}

bool NavigationView::RemoveNavItem(NavigationViewItem* pItem, bool bRemovePage)
{
    if (pItem == nullptr) {
        return false;
    }
    auto it = std::find(m_items.begin(), m_items.end(), pItem);
    if (it == m_items.end()) {
        //不是本控件的导航项
        return false;
    }
    //从所属容器中移除（可能是列表或底部固定区）
    bool bRemoved = false;
    const DString strPage = pItem->GetPageName();
    if (m_pItemHost != nullptr) {
        bRemoved = m_pItemHost->RemoveItem(pItem);
    }
    if (!bRemoved && (m_pBottomHost != nullptr)) {
        bRemoved = m_pBottomHost->RemoveItem(pItem);
    }
    if (!bRemoved && (m_pPane != nullptr)) {
        bRemoved = m_pPane->RemoveItem(pItem);
    }

    //可选：一并移除关联的内容页（按 page name 在内容区查找并移除）
    if (bRemovePage && (m_pContent != nullptr)) {        
        if (!strPage.empty()) {
            if (Control* pPage = m_pContent->FindSubControl(strPage)) {
                m_pContent->RemoveItem(pPage);
            }
        }
    }

    UnregisterNavItem(pItem);
    //控件从容器移除后，由调用方决定是否 delete（若由本控件动态创建，见 AddNavItem 的归属约定）
    return bRemoved;
}

void NavigationView::RemoveAllNavItems()
{
    if (m_pItemHost != nullptr) {
        m_pItemHost->RemoveAllItems();
    }
    if (m_pBottomHost != nullptr) {
        m_pBottomHost->RemoveAllItems();
    }
    m_items.clear();
    m_pSelected = nullptr;
    m_pSettingsNavItem = nullptr;
    UpdateHeaderText(nullptr);
}

void NavigationView::SetItemEnabled(NavigationViewItem* pItem, bool bEnabled)
{
    if (pItem == nullptr) {
        return;
    }
    pItem->SetEnabled(bEnabled);
}

} // namespace ui
