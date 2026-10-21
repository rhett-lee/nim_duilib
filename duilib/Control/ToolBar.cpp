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
    else if ((strName == _T("tool_class")) || (strName == _T("toolclass"))) {
        SetToolClass(strValue);
    }
    else if ((strName == _T("tool_checkbox_class")) || (strName == _T("toolcheck_box_class")) ||
             (strName == _T("toolcheckboxclass"))) {
        SetToolCheckBoxClass(strValue);
    }
    else if ((strName == _T("tool_separator_class")) || (strName == _T("toolseparatorclass"))) {
        SetToolSeparatorClass(strValue);
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

void ToolBar::AttachToolItemClick(ToolItemClickEvent callback)
{
    if (callback != nullptr) {
        m_toolClickCallbacks.push_back(callback);
    }
}

void ToolBar::ClearToolItemClick()
{
    m_toolClickCallbacks.clear();
}

void ToolBar::AttachToolItemCheckedChanged(ToolItemCheckedChangedEvent callback)
{
    if (callback != nullptr) {
        m_toolCheckedCallbacks.push_back(callback);
    }
}

void ToolBar::ClearToolItemCheckedChanged()
{
    m_toolCheckedCallbacks.clear();
}

void ToolBar::SetToolClass(const DString& strClass)
{
    m_toolClass = strClass;
}

DString ToolBar::GetToolClass() const
{
    return m_toolClass.c_str();
}

void ToolBar::SetToolCheckBoxClass(const DString& strClass)
{
    m_toolCheckBoxClass = strClass;
}

DString ToolBar::GetToolCheckBoxClass() const
{
    return m_toolCheckBoxClass.c_str();
}

void ToolBar::SetToolSeparatorClass(const DString& strClass)
{
    m_toolSeparatorClass = strClass;
}

DString ToolBar::GetToolSeparatorClass() const
{
    return m_toolSeparatorClass.c_str();
}

bool ToolBar::AddItem(Control* pControl)
{
    if (!BaseClass::AddItem(pControl)) {
        return false;
    }
    BindToolEvents(pControl);
    return true;
}

bool ToolBar::AddItemAt(Control* pControl, size_t iIndex)
{
    if (!BaseClass::AddItemAt(pControl, iIndex)) {
        return false;
    }
    BindToolEvents(pControl);
    return true;
}

Button* ToolBar::AddToolButton(const DString& strName, const DString& strText,
                               const DString& strClass, bool bTextId)
{
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return nullptr;
    }
    Button* pButton = new Button(pWindow);
    const DString strFinalClass = strClass.empty() ? DString(m_toolClass.c_str()) : strClass;
    if (!strFinalClass.empty()) {
        pButton->SetClass(strFinalClass);
    }
    if (bTextId) {
        pButton->SetTextId(strText);
    }
    else {
        pButton->SetText(strText);
    }
    if (!AddTool(pButton, strName)) {
        delete pButton;
        return nullptr;
    }
    return pButton;
}

CheckBox* ToolBar::AddToolCheckBox(const DString& strName, const DString& strText,
                                   bool bChecked, const DString& strClass, bool bTextId)
{
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return nullptr;
    }
    CheckBox* pCheckBox = new CheckBox(pWindow);
    DString strFinalClass = strClass;
    if (strFinalClass.empty()) {
        strFinalClass = m_toolCheckBoxClass.c_str();
    }
    if (strFinalClass.empty()) {
        strFinalClass = m_toolClass.c_str();
    }
    if (!strFinalClass.empty()) {
        pCheckBox->SetClass(strFinalClass);
    }
    if (bTextId) {
        pCheckBox->SetTextId(strText);
    }
    else {
        pCheckBox->SetText(strText);
    }
    if (!AddTool(pCheckBox, strName)) {
        delete pCheckBox;
        return nullptr;
    }
    pCheckBox->Selected(bChecked, false);
    return pCheckBox;
}

Line* ToolBar::AddToolSeparator(const DString& strName, const DString& strClass)
{
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return nullptr;
    }
    Line* pLine = new Line(pWindow);
    pLine->SetLineVertical(true);
    const DString strFinalClass = strClass.empty() ? DString(m_toolSeparatorClass.c_str()) : strClass;
    if (!strFinalClass.empty()) {
        pLine->SetClass(strFinalClass);
    }
    if (!AddTool(pLine, strName)) {
        delete pLine;
        return nullptr;
    }
    return pLine;
}

bool ToolBar::AddTool(Control* pControl, const DString& strName)
{
    return InsertTool(GetToolCount(), pControl, strName);
}

bool ToolBar::InsertTool(size_t nIndex, Control* pControl, const DString& strName)
{
    Window* pWindow = GetWindow();
    if ((pControl == nullptr) || (pWindow == nullptr)) {
        return false;
    }
    if (IsChildControl(pControl)) {
        //已经是本容器的子控件，不重复添加
        return false;
    }

    DString strFinalName = strName;
    if (strFinalName.empty()) {
        strFinalName = pControl->GetName();
    }
    if (strFinalName.empty()) {
        strFinalName = MakeToolName();
    }
    else if (!IsToolNameAvailable(strFinalName)) {
        return false;
    }
    if (!pControl->GetName().empty() && (pControl->GetName() != strFinalName)) {
        //控件已有其它名称时，不覆盖调用方设置
        return false;
    }
    pControl->SetName(strFinalName);

    //把工具项索引换算为实际的子项索引（跳过内部的"更多"按钮）
    size_t nItemIndex = GetItemCount();
    if (nIndex < GetToolCount()) {
        size_t nToolIndex = 0;
        for (size_t i = 0; i < GetItemCount(); ++i) {
            if (GetItemAt(i) == m_pOverflowButton.get()) {
                continue;
            }
            if (nToolIndex == nIndex) {
                nItemIndex = i;
                break;
            }
            ++nToolIndex;
        }
    }

    bool bOk = false;
    if (nItemIndex >= GetItemCount()) {
        bOk = AddItem(pControl);
    }
    else {
        bOk = AddItemAt(pControl, nItemIndex);
    }
    if (bOk) {
        Invalidate();
    }
    return bOk;
}

size_t ToolBar::GetToolCount() const
{
    size_t nCount = 0;
    const size_t nItemCount = GetItemCount();
    for (size_t i = 0; i < nItemCount; ++i) {
        if (GetItemAt(i) != m_pOverflowButton.get()) {
            ++nCount;
        }
    }
    return nCount;
}

Control* ToolBar::GetToolAt(size_t nIndex) const
{
    size_t nToolIndex = 0;
    const size_t nItemCount = GetItemCount();
    for (size_t i = 0; i < nItemCount; ++i) {
        Control* pItem = GetItemAt(i);
        if (pItem == m_pOverflowButton.get()) {
            continue;
        }
        if (nToolIndex == nIndex) {
            return pItem;
        }
        ++nToolIndex;
    }
    return nullptr;
}

Control* ToolBar::FindTool(const DString& strName) const
{
    if (strName.empty()) {
        return nullptr;
    }
    const size_t nItemCount = GetItemCount();
    for (size_t i = 0; i < nItemCount; ++i) {
        Control* pItem = GetItemAt(i);
        if ((pItem != nullptr) && (pItem != m_pOverflowButton.get()) &&
            (pItem->GetName() == strName)) {
            return pItem;
        }
    }
    return nullptr;
}

size_t ToolBar::GetToolIndex(const DString& strName) const
{
    if (strName.empty()) {
        return Box::InvalidIndex;
    }
    size_t nToolIndex = 0;
    const size_t nItemCount = GetItemCount();
    for (size_t i = 0; i < nItemCount; ++i) {
        Control* pItem = GetItemAt(i);
        if (pItem == m_pOverflowButton.get()) {
            continue;
        }
        if ((pItem != nullptr) && (pItem->GetName() == strName)) {
            return nToolIndex;
        }
        ++nToolIndex;
    }
    return Box::InvalidIndex;
}

bool ToolBar::HasTool(const DString& strName) const
{
    return FindTool(strName) != nullptr;
}

bool ToolBar::RemoveTool(const DString& strName)
{
    Control* pControl = FindTool(strName);
    if (pControl == nullptr) {
        return false;
    }
    return RemoveToolAt(GetToolIndex(strName));
}

bool ToolBar::RemoveToolAt(size_t nIndex)
{
    Control* pControl = GetToolAt(nIndex);
    if (pControl == nullptr) {
        return false;
    }
    //RemoveItem 在 AutoDestroyChild 为 true 时会销毁控件，不能重复 delete
    const bool bAutoDestroyChild = IsAutoDestroyChild();
    ForgetHiddenItem(pControl);
    auto iter = std::find(m_boundToolItems.begin(), m_boundToolItems.end(), pControl);
    if (iter != m_boundToolItems.end()) {
        m_boundToolItems.erase(iter);
    }
    if (!RemoveItem(pControl)) {
        return false;
    }
    if (!bAutoDestroyChild) {
        delete pControl;
    }
    Invalidate();
    return true;
}

void ToolBar::RemoveAllTools()
{
    for (size_t nIndex = GetToolCount(); nIndex > 0; --nIndex) {
        RemoveToolAt(nIndex - 1);
    }
}

bool ToolBar::SetToolChecked(const DString& strName, bool bChecked, bool bTriggerEvent)
{
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(FindTool(strName));
    if (pCheckBox == nullptr) {
        return false;
    }
    if (pCheckBox->IsSelected() == bChecked) {
        return true;
    }
    //不触发 Select/UnSelect 事件，避免被误判为点击；需要时单独派发勾选变化回调
    pCheckBox->Selected(bChecked, false);
    if (bTriggerEvent) {
        FireToolItemCheckedChanged(strName, bChecked);
    }
    return true;
}

bool ToolBar::IsToolChecked(const DString& strName) const
{
    const CheckBox* pCheckBox = dynamic_cast<const CheckBox*>(FindTool(strName));
    if (pCheckBox == nullptr) {
        return false;
    }
    return pCheckBox->IsSelected();
}

bool ToolBar::SetToolEnabled(const DString& strName, bool bEnabled)
{
    Control* pControl = FindTool(strName);
    if (pControl == nullptr) {
        return false;
    }
    pControl->SetEnabled(bEnabled);
    return true;
}

bool ToolBar::IsToolEnabled(const DString& strName) const
{
    Control* pControl = FindTool(strName);
    if (pControl == nullptr) {
        return false;
    }
    return pControl->IsEnabled();
}

bool ToolBar::SetToolVisible(const DString& strName, bool bVisible)
{
    Control* pControl = FindTool(strName);
    if (pControl == nullptr) {
        return false;
    }
    if (!bVisible) {
        //应用层主动隐藏时，从自动收纳列表移除，避免宽度恢复后被错误地重新显示
        ForgetHiddenItem(pControl);
    }
    pControl->SetVisible(bVisible);
    return true;
}

bool ToolBar::IsToolVisible(const DString& strName) const
{
    Control* pControl = FindTool(strName);
    if (pControl == nullptr) {
        return false;
    }
    return pControl->IsVisible();
}

void ToolBar::FireToolItemClick(const DString& itemName)
{
    std::vector<ToolItemClickEvent> callbackList(m_toolClickCallbacks);
    for (const ToolItemClickEvent& callback : callbackList) {
        if (callback != nullptr) {
            callback(itemName);
        }
    }
}

void ToolBar::FireToolItemCheckedChanged(const DString& itemName, bool bChecked)
{
    std::vector<ToolItemCheckedChangedEvent> callbackList(m_toolCheckedCallbacks);
    for (const ToolItemCheckedChangedEvent& callback : callbackList) {
        if (callback != nullptr) {
            callback(itemName, bChecked);
        }
    }
}

void ToolBar::ActivateTool(Control* pControl)
{
    if ((pControl == nullptr) || (pControl == m_pOverflowButton.get())) {
        return;
    }
    if (!IsChildControl(pControl) || !pControl->IsEnabled() || !pControl->IsVisible()) {
        return;
    }
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(pControl);
    if (pCheckBox != nullptr) {
        //切换选中态：会触发 kEventSelect/kEventUnSelect，由绑定的事件派发点击与勾选回调
        pCheckBox->Selected(!pCheckBox->IsSelected(), true);
    }
    else {
        FireToolItemClick(pControl->GetName().c_str());
    }
}

void ToolBar::BindToolEvents(Control* pControl)
{
    if ((pControl == nullptr) || (pControl == m_pOverflowButton.get())) {
        return;
    }
    if (ContainsItem(m_boundToolItems, pControl)) {
        return;
    }
    if (dynamic_cast<Button*>(pControl) == nullptr) {
        return;
    }
    m_boundToolItems.push_back(pControl);

    const std::weak_ptr<WeakFlag> toolbarFlag = GetWeakFlag();
    const DString strItemName = pControl->GetName().c_str();
    Button* pButton = dynamic_cast<Button*>(pControl);
    if (pButton != nullptr) {
        pButton->AttachClick([this, toolbarFlag, strItemName](const EventArgs& /*args*/) {
            if (!toolbarFlag.expired()) {
                FireToolItemClick(strItemName);
            }
            return true;
            });
    }
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(pControl);
    if (pCheckBox != nullptr) {
        pCheckBox->AttachSelect([this, toolbarFlag, strItemName](const EventArgs& /*args*/) {
            if (!toolbarFlag.expired()) {
                FireToolItemClick(strItemName);
                FireToolItemCheckedChanged(strItemName, true);
            }
            return true;
            });
        pCheckBox->AttachUnSelect([this, toolbarFlag, strItemName](const EventArgs& /*args*/) {
            if (!toolbarFlag.expired()) {
                FireToolItemClick(strItemName);
                FireToolItemCheckedChanged(strItemName, false);
            }
            return true;
            });
    }
}

DString ToolBar::MakeToolName() const
{
    for (int32_t nIndex = 1; ; ++nIndex) {
        DString strName = _T("tool_") + StringUtil::Int32ToString(nIndex);
        if (FindTool(strName) == nullptr) {
            return strName;
        }
    }
}

bool ToolBar::IsToolNameAvailable(const DString& strName) const
{
    if (strName.empty()) {
        return true;
    }
    return FindTool(strName) == nullptr;
}

void ToolBar::ForgetHiddenItem(Control* pControl)
{
    auto iter = std::find(m_hiddenItems.begin(), m_hiddenItems.end(), pControl);
    if (iter != m_hiddenItems.end()) {
        m_hiddenItems.erase(iter);
    }
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
    if (BaseClass::AddItem(pButton)) {
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

    //确保"更多"按钮始终位于末尾（后续动态添加工具项时可能把它挤到中间）
    const size_t nOverflowIndex = GetItemIndex(pButton);
    if ((nOverflowIndex != Box::InvalidIndex) && ((nOverflowIndex + 1) != GetItemCount())) {
        SetItemIndex(pButton, GetItemCount() - 1);
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
                //与直接点击工具项走同一套回调：勾选类会切换选中态并派发勾选变化
                ActivateTool(FindTool(strItemName));
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
