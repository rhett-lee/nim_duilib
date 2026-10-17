#include "ToolBar.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/Flyout.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Layout/Layout.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>

namespace ui
{

namespace
{
/** 将文本中的 XML 特殊字符转义，用于动态生成浮层的 XML 文本
*/
DString EscapeXmlText(const DString& strText)
{
    DString strResult;
    strResult.reserve(strText.size() + 16);
    for (DString::value_type ch : strText) {
        switch (ch) {
        case _T('&'):  strResult += _T("&amp;");  break;
        case _T('<'):  strResult += _T("&lt;");   break;
        case _T('>'):  strResult += _T("&gt;");   break;
        case _T('\"'): strResult += _T("&quot;"); break;
        case _T('\''): strResult += _T("&apos;"); break;
        default:       strResult += ch;           break;
        }
    }
    return strResult;
}

//浮层列表项的固定尺寸（DPI 逻辑值，XML 中会自动按 DPI 缩放）
constexpr int32_t kOverflowFlyoutWidthDip = 180;
constexpr int32_t kOverflowFlyoutItemHeightDip = 30;
constexpr int32_t kOverflowFlyoutPaddingDip = 4;
} // namespace

////////////////////////////////////////////////////////////////////////////////

ToolBar::ToolBar(Window* pWindow) :
    HBox(pWindow),
    m_bOverflowEnabled(true),
    m_nOverflowButtonWidth(0),
    m_overflowButtonText(_T(">>")),
    m_bInLayout(false)
{
}

ToolBar::~ToolBar() = default;

DString ToolBar::GetType() const { return DUI_CTR_TOOL_BAR; }

void ToolBar::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("overflow")) || (strName == _T("overflow_enabled")) || (strName == _T("overflowenabled"))) {
        SetOverflowEnabled(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("overflow_button_class")) || (strName == _T("overflowbuttonclass"))) {
        SetOverflowButtonClass(strValue);
    }
    else if ((strName == _T("overflow_button_width")) || (strName == _T("overflowbuttonwidth"))) {
        SetOverflowButtonWidth(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("overflow_button_text")) || (strName == _T("overflowbuttontext"))) {
        SetOverflowButtonText(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void ToolBar::AttachOverflowItemActivated(OverflowItemActivatedEvent callback)
{
    if (callback != nullptr) {
        m_callbackList.push_back(callback);
    }
}

void ToolBar::ClearOverflowItemActivated()
{
    m_callbackList.clear();
}

void ToolBar::SetOverflowEnabled(bool bEnabled)
{
    if (m_bOverflowEnabled != bEnabled) {
        m_bOverflowEnabled = bEnabled;
        if (!bEnabled) {
            //关闭溢出收纳时，恢复所有被自动隐藏的项
            for (Control* pItem : m_hiddenItems) {
                if (IsChildControl(pItem)) {
                    pItem->SetVisible(true);
                }
            }
            m_hiddenItems.clear();
            Control* pButton = m_pOverflowButton.get();
            if ((pButton != nullptr) && pButton->IsVisible()) {
                pButton->SetVisible(false);
            }
        }
        Invalidate();
    }
}

bool ToolBar::IsOverflowEnabled() const
{
    return m_bOverflowEnabled;
}

void ToolBar::SetOverflowButtonClass(const DString& strClass)
{
    if (m_overflowButtonClass != strClass) {
        m_overflowButtonClass = strClass;
        Control* pButton = m_pOverflowButton.get();
        if (pButton != nullptr) {
            pButton->SetClass(m_overflowButtonClass.c_str());
        }
        Invalidate();
    }
}

DString ToolBar::GetOverflowButtonClass() const
{
    return m_overflowButtonClass.c_str();
}

void ToolBar::SetOverflowButtonWidth(int32_t nWidth)
{
    if (m_nOverflowButtonWidth != nWidth) {
        m_nOverflowButtonWidth = nWidth;
        Control* pButton = m_pOverflowButton.get();
        if (pButton != nullptr) {
            if (m_nOverflowButtonWidth > 0) {
                pButton->SetFixedWidth(UiFixedInt(m_nOverflowButtonWidth), true, true);
            }
            else {
                pButton->SetFixedWidth(UiFixedInt::MakeAuto(), true, false);
            }
        }
        Invalidate();
    }
}

int32_t ToolBar::GetOverflowButtonWidth() const
{
    return m_nOverflowButtonWidth;
}

void ToolBar::SetOverflowButtonText(const DString& strText)
{
    if (m_overflowButtonText != strText) {
        m_overflowButtonText = strText;
        Label* pButton = dynamic_cast<Label*>(m_pOverflowButton.get());
        if (pButton != nullptr) {
            pButton->SetText(m_overflowButtonText.c_str());
        }
        Invalidate();
    }
}

DString ToolBar::GetOverflowButtonText() const
{
    return m_overflowButtonText.c_str();
}

bool ToolBar::IsOverflowing() const
{
    return !m_hiddenItems.empty();
}

bool ToolBar::IsChildControl(Control* pControl) const
{
    if (pControl == nullptr) {
        return false;
    }
    const size_t nCount = GetItemCount();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (GetItemAt(nIndex) == pControl) {
            return true;
        }
    }
    return false;
}

bool ToolBar::ContainsItem(const std::vector<Control*>& items, Control* pControl)
{
    for (Control* pItem : items) {
        if (pItem == pControl) {
            return true;
        }
    }
    return false;
}

void ToolBar::CreateOverflowButton()
{
    if (m_pOverflowButton != nullptr) {
        return;
    }
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    Button* pButton = new Button(pWindow);
    pButton->SetText(m_overflowButtonText.c_str());
    if (!m_overflowButtonClass.empty()) {
        pButton->SetClass(m_overflowButtonClass.c_str());
    }
    if (m_nOverflowButtonWidth > 0) {
        pButton->SetFixedWidth(UiFixedInt(m_nOverflowButtonWidth), false, true);
    }
    else {
        pButton->SetFixedWidth(UiFixedInt::MakeAuto(), false, false);
    }
    pButton->SetVisible(false);
    pButton->AttachClick([this](const EventArgs& /*args*/) {
        ShowOverflowFlyout();
        return true;
        });
    if (AddItem(pButton)) {
        m_pOverflowButton = pButton;
    }
    else {
        delete pButton;
    }
}

void ToolBar::SetPos(UiRect rc)
{
    if (!m_bOverflowEnabled || m_bInLayout) {
        BaseClass::SetPos(rc);
        return;
    }
    if (m_pOverflowButton == nullptr) {
        CreateOverflowButton();
    }
    m_bInLayout = true;
    UpdateOverflow(rc);
    m_bInLayout = false;
}

void ToolBar::UpdateOverflow(const UiRect& rc)
{
    Control* pButton = m_pOverflowButton.get();
    if (pButton == nullptr) {
        //未创建"更多"按钮时，按普通容器布局
        BaseClass::SetPos(rc);
        return;
    }

    const UiPadding padding = GetPadding();
    const int32_t nContentLeft = rc.left + padding.left;
    const int32_t nContentRight = rc.right - padding.right;
    const int32_t nAvailable = nContentRight - nContentLeft;

    //收集参与布局的子控件（排除"更多"按钮、浮动控件以及由应用层隐藏的控件）
    std::vector<Control*> items;
    bool bHasStretch = false;
    if (nAvailable > 0) {
        const size_t nCount = GetItemCount();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            Control* pItem = GetItemAt(nIndex);
            if ((pItem == nullptr) || (pItem == pButton) || pItem->IsFloat()) {
                continue;
            }
            const bool bAutoHidden = ContainsItem(m_hiddenItems, pItem);
            if (!pItem->IsVisible() && !bAutoHidden) {
                //应用层隐藏的控件，不参与溢出收纳
                continue;
            }
            if (pItem->EstimateSize(UiSize(nAvailable, rc.Height())).cx.IsStretch()) {
                //存在拉伸子控件时不做溢出收纳
                bHasStretch = true;
                break;
            }
            items.push_back(pItem);
        }
    }

    //计算期望被收纳的项（从前往后累计宽度，超出可用宽度后开始收纳）
    std::vector<Control*> desiredHidden;
    if ((nAvailable > 0) && !bHasStretch && !items.empty()) {
        //"更多"按钮占用的宽度
        int32_t nButtonWidth = 0;
        if (m_nOverflowButtonWidth > 0) {
            nButtonWidth = Dpi().GetScaleInt(m_nOverflowButtonWidth);
        }
        else {
            UiEstSize estButton = pButton->EstimateSize(UiSize(nAvailable, rc.Height()));
            nButtonWidth = std::max(estButton.cx.GetInt32(), 0);
        }

        const int32_t nLimit = nContentRight - nButtonWidth;
        const int32_t nMarginX = (GetLayout() != nullptr) ? GetLayout()->GetChildMarginX() : 0;
        int32_t nPosX = nContentLeft;
        bool bFirstItem = true;
        for (size_t nIndex = 0; nIndex < items.size(); ++nIndex) {
            Control* pItem = items[nIndex];
            if (!bFirstItem) {
                nPosX += nMarginX;
            }
            bFirstItem = false;
            const UiMargin margin = pItem->GetMargin();
            nPosX += margin.left;
            UiEstSize estItem = pItem->EstimateSize(UiSize(nAvailable, rc.Height()));
            int32_t nItemWidth = std::clamp(estItem.cx.GetInt32(), pItem->GetMinWidth(), pItem->GetMaxWidth());
            nItemWidth = std::max(nItemWidth, 0);
            if ((nPosX + nItemWidth) > nLimit) {
                desiredHidden.assign(items.begin() + nIndex, items.end());
                break;
            }
            nPosX += nItemWidth + margin.right;
        }
    }
    const bool bNeedOverflow = !desiredHidden.empty();

    //仅在收纳状态发生变化时修改子控件可见性，避免反复触发布局
    bool bChanged = (desiredHidden.size() != m_hiddenItems.size());
    if (!bChanged) {
        for (size_t nIndex = 0; nIndex < desiredHidden.size(); ++nIndex) {
            if (desiredHidden[nIndex] != m_hiddenItems[nIndex]) {
                bChanged = true;
                break;
            }
        }
    }
    if (bChanged) {
        for (Control* pItem : m_hiddenItems) {
            if (!ContainsItem(desiredHidden, pItem) && IsChildControl(pItem)) {
                pItem->SetVisible(true);
            }
        }
        for (Control* pItem : desiredHidden) {
            if (!ContainsItem(m_hiddenItems, pItem)) {
                pItem->SetVisible(false);
            }
        }
        m_hiddenItems = desiredHidden;
    }

    if (pButton->IsVisible() != bNeedOverflow) {
        pButton->SetVisible(bNeedOverflow);
    }

    //应用布局
    BaseClass::SetPos(rc);

    if (bNeedOverflow) {
        //将"更多"按钮对齐到内容区右端
        UiRect rcButton = pButton->GetRect();
        int32_t nWidth = rcButton.Width();
        if (nWidth <= 0) {
            nWidth = Dpi().GetScaleInt(24);
        }
        rcButton.right = nContentRight;
        rcButton.left = std::max(nContentRight - nWidth, nContentLeft);
        pButton->SetPos(rcButton);
    }
}

void ToolBar::ShowOverflowFlyout()
{
    if (m_hiddenItems.empty()) {
        return;
    }
    Window* pWindow = GetWindow();
    Control* pButton = m_pOverflowButton.get();
    if ((pWindow == nullptr) || (pButton == nullptr)) {
        return;
    }

    //同一锚点再次点击：关闭已经打开的浮层
    Flyout* pActive = Flyout::GetActiveFlyout();
    if ((pActive != nullptr) && (pActive->GetAnchor() == pButton)) {
        pActive->Dismiss();
        return;
    }

    //收集被收纳项的名称与显示文本
    std::vector<DString> itemNames;
    std::vector<DString> itemTexts;
    for (Control* pItem : m_hiddenItems) {
        if (!IsChildControl(pItem)) {
            continue;
        }
        DString strText;
        Label* pLabel = dynamic_cast<Label*>(pItem);
        if (pLabel != nullptr) {
            strText = pLabel->GetText();
        }
        if (strText.empty()) {
            strText = pItem->GetToolTipText();
        }
        if (strText.empty()) {
            strText = pItem->GetName();
        }
        itemNames.push_back(pItem->GetName().c_str());
        itemTexts.push_back(strText.c_str());
    }
    if (itemNames.empty()) {
        return;
    }

    //动态生成浮层内容（XML 文本）
    const int32_t nWidthDip = kOverflowFlyoutWidthDip;
    const int32_t nItemHeightDip = kOverflowFlyoutItemHeightDip;
    const int32_t nPaddingDip = kOverflowFlyoutPaddingDip;
    const int32_t nHeightDip = (int32_t)itemTexts.size() * nItemHeightDip + nPaddingDip * 2;

    DString xml;
    xml += _T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    xml += _T("<Window size=\"");
    xml += StringUtil::Int32ToString(nWidthDip);
    xml += _T(",");
    xml += StringUtil::Int32ToString(nHeightDip);
    xml += _T("\" caption=\"0,0,0,0\" use_system_caption=\"false\"");
    xml += _T(" shadow_type=\"default\" shadow_attached=\"true\"");
    xml += _T(" layered_window=\"true\" size_box=\"0,0,0,0\"");
    xml += _T(" icon=\"public/caption/logo.ico\">");
    xml += _T("<VBox bkcolor=\"bg_menu\" width=\"");
    xml += StringUtil::Int32ToString(nWidthDip);
    xml += _T("\" height=\"auto\" padding=\"");
    xml += StringUtil::Int32ToString(nPaddingDip);
    xml += _T(",");
    xml += StringUtil::Int32ToString(nPaddingDip);
    xml += _T(",");
    xml += StringUtil::Int32ToString(nPaddingDip);
    xml += _T(",");
    xml += StringUtil::Int32ToString(nPaddingDip);
    xml += _T("\">");
    for (size_t nIndex = 0; nIndex < itemTexts.size(); ++nIndex) {
        xml += _T("<Button name=\"ovf_");
        xml += StringUtil::Int32ToString((int32_t)nIndex);
        xml += _T("\" text=\"");
        xml += EscapeXmlText(itemTexts[nIndex]);
        xml += _T("\" width=\"stretch\" height=\"");
        xml += StringUtil::Int32ToString(nItemHeightDip);
        xml += _T("\" text_align=\"left,vcenter\" text_padding=\"12,0,0,0\"");
        xml += _T(" normal_text_color=\"text_default\" hot_text_color=\"color_accent\"");
        xml += _T(" cursor_type=\"hand\"/>");
    }
    xml += _T("</VBox></Window>");

    Flyout* pFlyout = new Flyout(pWindow);
    pFlyout->SetSkinFolder(pWindow->GetResourcePath().ToString());
    if (!pFlyout->ShowAt(pButton, xml, Flyout::Placement::BottomEnd)) {
        //创建失败时，ShowAt 内部已销毁对象
        return;
    }

    //绑定各个列表项的点击事件
    std::weak_ptr<WeakFlag> toolbarFlag = GetWeakFlag();
    for (size_t nIndex = 0; nIndex < itemNames.size(); ++nIndex) {
        DString strItemControlName = _T("ovf_") + StringUtil::Int32ToString((int32_t)nIndex);
        Button* pItemButton = dynamic_cast<Button*>(pFlyout->FindControl(strItemControlName));
        if (pItemButton == nullptr) {
            continue;
        }
        DString strItemName = itemNames[nIndex];
        pItemButton->AttachClick([this, toolbarFlag, strItemName, pFlyout](const EventArgs& /*args*/) {
            if (!toolbarFlag.expired()) {
                FireOverflowItemActivated(strItemName);
            }
            pFlyout->Dismiss();
            return true;
            });
    }
}

void ToolBar::FireOverflowItemActivated(const DString& itemName)
{
    std::vector<OverflowItemActivatedEvent> callbackList(m_callbackList);
    for (const OverflowItemActivatedEvent& callback : callbackList) {
        if (callback != nullptr) {
            callback(itemName);
        }
    }
}

} // namespace ui
