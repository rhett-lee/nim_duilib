/* 2025-07 新增：NavigationView 侧边栏导航控件（NavigationView + NavigationViewItem） */

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
    m_pOwner(nullptr)
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
    if (IsSelectable() && !m_bHovered) {
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

    //选中/悬停背景（圆角热区，左右各缩 6 DIP，上下各缩 3 DIP）
    UiColor bgColor;
    if (m_bSelected) {
        bgColor = GetUiColor(_T("bg_list_item_selected"));
    }
    else if (m_bHovered) {
        bgColor = GetUiColor(_T("bg_list_item_hovered"));
    }
    if (!bgColor.IsEmpty()) {
        UiRect rcPill = rc;
        rcPill.Deflate(Dpi().GetScaleInt(6), Dpi().GetScaleInt(3));
        UiRectF rcPillF;
        rcPillF.left = static_cast<float>(rcPill.left);
        rcPillF.top = static_cast<float>(rcPill.top);
        rcPillF.right = static_cast<float>(rcPill.right);
        rcPillF.bottom = static_cast<float>(rcPill.bottom);
        float fRadius = Dpi().GetScaleFloat(6.0f);
        pRender->FillRoundRect(rcPillF, fRadius, fRadius, bgColor);
    }

    const int32_t nIconSize = Dpi().GetScaleInt(16);
    UiRect rcIcon;
    rcIcon.top = rc.CenterY() - nIconSize / 2;
    rcIcon.bottom = rcIcon.top + nIconSize;
    if (m_bCompact) {
        //紧凑态：图标在窗格内水平居中
        rcIcon.left = rc.CenterX() - nIconSize / 2;
        rcIcon.right = rcIcon.left + nIconSize;
    }
    else {
        //展开态：图标左边距 16 DIP
        rcIcon.left = rc.left + Dpi().GetScaleInt(16);
        rcIcon.right = rcIcon.left + nIconSize;
    }

    //绘制图标
    Image* pImage = GetStateImageData(kStateImageFore, kControlStateNormal);
    if (pImage != nullptr) {
        PaintImage(pRender, pImage, _T(""), -1, nullptr, &rcIcon, nullptr);
    }

    //展开态绘制文字；紧凑态仅显示图标
    if (!m_bCompact) {
        DString strText = GetDisplayText();
        if (!strText.empty()) {
            UiRect rcText = rc;
            rcText.left = rc.left + Dpi().GetScaleInt(44);
            rcText.right = rc.right - Dpi().GetScaleInt(8);
            UiColor textColor = m_bSelected ? GetUiColor(_T("color_accent"))
                                            : GetUiColor(_T("text_default"));
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
    UiColor textColor = GetUiColor(_T("text_muted"));
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
    UiColor lineColor = GetUiColor(_T("border_window"));
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
    //子控件由父容器持有，无需手动释放
    m_pSelected = nullptr;
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

    //底部设置项（在导航项列表之后加入窗格，借助列表区的 stretch 固定在底部）
    if (m_bSettingsItem && (m_pSettingsNavItem == nullptr)) {
        m_pSettingsNavItem = new NavigationViewItem(GetWindow());
        m_pSettingsNavItem->SetPageName(m_settingsPage.c_str());
        m_pSettingsNavItem->SetItemTextId(m_settingsTextId.c_str());
        if (!m_settingsIcon.empty()) {
            m_pSettingsNavItem->SetIconAttr(m_settingsIcon.c_str());
        }
        RegisterNavItem(m_pSettingsNavItem);
        m_pPane->AddItem(m_pSettingsNavItem);
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
    m_pPane->AddItem(m_pItemHost);

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
        //默认选中第一个非设置项的可选项，避免设置页成为初始页
        for (NavigationViewItem* pItem : m_items) {
            if ((pItem != nullptr) && pItem->IsSelectable() && (pItem != m_pSettingsNavItem)) {
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
    SelectItem(pItem, true);
}

void NavigationView::SetCollapsed(bool bCollapsed)
{
    if (m_bCollapsed == bCollapsed) {
        return;
    }
    m_bCollapsed = bCollapsed;
    ApplyCollapsed();
}

bool NavigationView::SelectItem(NavigationViewItem* pItem, bool bFireEvent)
{
    if ((pItem == nullptr) || !pItem->IsSelectable()) {
        return false;
    }
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
        SendEvent(kEventTabSelect, static_cast<WPARAM>(GetSelectedIndex()), 0);
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
        if ((pItem != nullptr) && pItem->IsSelectable()) {
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
        if ((pItem != nullptr) && pItem->IsSelectable()) {
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
                                               const DString& strPage)
{
    EnsureInternals();
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
        delete pItem;
        return nullptr;
    }
    return pItem;
}

} // namespace ui
