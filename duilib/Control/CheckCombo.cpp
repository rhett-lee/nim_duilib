#include "CheckCombo.h"
#include "duilib/Core/Window.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/ListBox.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/Macros.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui
{
class CCheckComboWnd : public ui::Window
{
public:
    void InitComboWnd(CheckCombo* pOwner);
    void UpdateComboWnd();
    void CloseComboWnd();

    virtual DString GetWindowClassName() const override;
    virtual void OnFinalMessage() override;
    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    void OnSeleteItem();

private:
    CheckCombo *m_pOwner = nullptr;
    //是否已经关闭
    bool m_bIsClosed = false;
};

void CCheckComboWnd::InitComboWnd(CheckCombo* pOwner)
{
    ASSERT(pOwner != nullptr);
    if (pOwner == nullptr) {
        return;
    }
    m_pOwner = pOwner;
    m_bIsClosed = false;
    CreateWnd(pOwner->GetWindow(), _T(""), WS_POPUP, WS_EX_TOOLWINDOW);
    UpdateComboWnd();

    HWND hWndParent = GetHWND();
    while (::GetParent(hWndParent) != NULL) {
        hWndParent = ::GetParent(hWndParent);
    }
    ::ShowWindow(GetHWND(), SW_SHOW);
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
}

void CCheckComboWnd::UpdateComboWnd()
{
    CheckCombo* pOwner = m_pOwner;
    if (pOwner == nullptr) {
        return;
    }
    // Position the popup window in absolute space
    UiSize szDrop = pOwner->GetDropBoxSize();
    UiRect rcOwner = pOwner->GetPos();
    UiPoint scrollBoxOffset = pOwner->GetScrollOffsetInScrollBox();
    rcOwner.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);

    UiRect rc = rcOwner;
    rc.top = rc.bottom + 1;            // 父窗口left、bottom位置作为弹出窗口起点
    rc.bottom = rc.top + szDrop.cy;    // 计算弹出窗口高度
    if (szDrop.cx > 0) {
        rc.right = rc.left + szDrop.cx;    // 计算弹出窗口宽度
    }
    int32_t cyFixed = 0;
    if (pOwner->GetListBox()->GetItemCount() > 0) {
        UiSize szAvailable(rc.Width(), rc.Height());
        UiFixedInt oldFixedHeight = pOwner->GetListBox()->GetFixedHeight();
        pOwner->GetListBox()->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
        UiEstSize estSize = pOwner->GetListBox()->EstimateSize(szAvailable);
        pOwner->GetListBox()->SetFixedHeight(oldFixedHeight, false, false);
        cyFixed = estSize.cy.GetInt32();
    }
    if (cyFixed == 0) {
        cyFixed = szDrop.cy;
    }
    rc.bottom = rc.top + std::min(cyFixed, szDrop.cy);

    pOwner->GetWindow()->MapWindowRect(rc);

    UiRect rcWork;
    GetMonitorWorkRect(rcWork);
    if (rc.bottom > rcWork.bottom || m_pOwner->IsPopupTop()) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        if (szDrop.cx > 0) {
            rc.right = rc.left + szDrop.cx;
        }
        rc.top = rcOwner.top - std::min(cyFixed, szDrop.cy);
        rc.bottom = rcOwner.top;
        pOwner->GetWindow()->MapWindowRect(rc);
    }
    SetWindowPos(nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

void CCheckComboWnd::CloseComboWnd()
{
    if (m_bIsClosed) {
        return;
    }
    m_bIsClosed = true;
    Box* pRootBox = GetRoot();
    if ((pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
        m_pOwner->GetListBox()->SetWindow(nullptr);
        m_pOwner->GetListBox()->SetParent(nullptr);
        pRootBox->RemoveAllItems();
    }
    //先将前端窗口切换为父窗口，避免前端窗口关闭后，切换到其他窗口
    HWND hWnd = GetHWND();
    HWND hParentWnd = ::GetParent(hWnd);
    HWND hForeWnd = ::GetForegroundWindow();
    if ((hForeWnd == hWnd) || hForeWnd == hParentWnd) {
        if (hParentWnd != nullptr) {
            ::SetForegroundWindow(hParentWnd);
        }
    }
    CloseWnd();
}

DString CCheckComboWnd::GetWindowClassName() const
{
    return _T("CCheckComboWnd");
}

void CCheckComboWnd::OnFinalMessage()
{
    if (m_pOwner != nullptr) {
        if (m_pOwner->GetListBox()->GetWindow() == this) {
            m_pOwner->GetListBox()->SetWindow(nullptr);
            m_pOwner->GetListBox()->SetParent(nullptr);
        }
        if (m_pOwner->m_pCheckComboWnd == this) {
            m_pOwner->m_pCheckComboWnd = nullptr;
            m_pOwner->SetState(kControlStateNormal);
            m_pOwner->Invalidate();
        }
    }    
    __super::OnFinalMessage();
}

void CCheckComboWnd::OnSeleteItem()
{
    PostMsg(WM_KILLFOCUS);
}

LRESULT CCheckComboWnd::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    bHandled = false;
    if (uMsg == WM_CREATE) {
        Box* pRoot = new Box(this);
        pRoot->SetAutoDestroyChild(false);
        pRoot->AddItem(m_pOwner->GetListBox());
        this->AttachBox(pRoot);
        this->SetResourcePath(m_pOwner->GetWindow()->GetResourcePath());
        this->SetShadowAttached(false);
        bHandled = true;
    }
    else if (uMsg == WM_CLOSE) {
        Box* pRootBox = GetRoot();
        if ((pRootBox != nullptr) && (pRootBox->GetItemCount() > 0)) {
            m_pOwner->GetListBox()->SetWindow(nullptr);
            m_pOwner->GetListBox()->SetParent(nullptr);
            pRootBox->RemoveAllItems();
        }
        m_pOwner->SetPos(m_pOwner->GetPos());
        m_pOwner->SetFocus();
    }
    LRESULT lResult = 0;
    if (!bHandled) {
        lResult = __super::OnWindowMessage(uMsg, wParam, lParam, bHandled);
    }
    if (uMsg == WM_KILLFOCUS) {
        //失去焦点，关闭窗口
        if (GetHWND() != (HWND)wParam) {
            CloseComboWnd();
        }
    }
    else if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        //按住ESC键，取消
        CloseComboWnd();
    }
    return lResult;
}

////////////////////////////////////////////////////////

#define CHECK_COMBO_DEFAULT_HEIGHT 20 

CheckCombo::CheckCombo(Window* pWindow) :
    Box(pWindow),
    m_pCheckComboWnd(nullptr),
    m_szDropBox(0, 0),
    m_bPopupTop(false),
    m_iOrgHeight(CHECK_COMBO_DEFAULT_HEIGHT)
{
    SetDropBoxSize({0, 150}, true);
    SetMaxHeight(m_iOrgHeight * 3, true);
    SetMinHeight(m_iOrgHeight, true);

    m_pDropList.reset(new ui::ListBox(pWindow, new ui::VLayout));
    m_pDropList->EnableScrollBar();
    
    m_pList.reset(new ui::ListBox(pWindow, new ui::VTileLayout));
    m_pList->AttachButtonDown(UiBind(&CheckCombo::OnListButtonDown, this, std::placeholders::_1));
    m_pList->SetMouseChildEnabled(false);
    m_pList->EnableScrollBar();

    Box::AddItem(m_pList.get());
}

CheckCombo::~CheckCombo()
{
    Box::RemoveItem(m_pList.get());
    m_pList.reset();
    m_pDropList.reset();
}

DString CheckCombo::GetType() const { return DUI_CTR_CHECK_COMBO; }

bool CheckCombo::AddItem(Control* pControl)
{
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(pControl);
    if (pCheckBox) {
        pCheckBox->AttachSelect(UiBind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
        pCheckBox->AttachUnSelect(UiBind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
    }
    else    {
        ui::CheckBoxBox* pCheckBoxBox = dynamic_cast<ui::CheckBoxBox*>(pControl);
        if (pCheckBoxBox) {
            pCheckBoxBox->AttachSelect(UiBind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
            pCheckBoxBox->AttachUnSelect(UiBind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
        }
        else {
            ASSERT(0);
            return false;
        }
    }
    return m_pDropList->AddItem(pControl);
}

bool CheckCombo::AddItemAt(Control* pControl, size_t iIndex)
{
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(pControl);
    if (pCheckBox) {
        pCheckBox->AttachSelect(UiBind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
        pCheckBox->AttachUnSelect(UiBind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
    }
    else {
        ui::CheckBoxBox* pCheckBoxBox = dynamic_cast<ui::CheckBoxBox*>(pControl);
        if (pCheckBoxBox) {
            pCheckBoxBox->AttachSelect(UiBind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
            pCheckBoxBox->AttachUnSelect(UiBind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
        }
        else {
            ASSERT(0);
            return false;
        }
    }
    return m_pDropList->AddItemAt(pControl, iIndex);
}

bool CheckCombo::RemoveItem(Control* pControl)
{
    return m_pDropList->RemoveItem(pControl);
}

bool CheckCombo::RemoveItemAt(size_t iIndex)
{
    return m_pDropList->RemoveItemAt(iIndex);
}

void CheckCombo::RemoveAllItems()
{
    m_pDropList->RemoveAllItems();
}

Control* CheckCombo::GetItemAt(size_t iIndex) const
{
    return m_pDropList->GetItemAt(iIndex);
}

size_t CheckCombo::GetItemIndex(Control* pControl) const
{
    return m_pDropList->GetItemIndex(pControl);
}

bool CheckCombo::SetItemIndex(Control* pControl, size_t iIndex)
{
    return m_pDropList->SetItemIndex(pControl, iIndex);
}

size_t CheckCombo::GetItemCount() const
{
    return m_pDropList->GetItemCount();
}

bool CheckCombo::AddTextItem(const DString& itemText)
{
    if (itemText.empty()) {
        return false;
    }
    //避免重复名称
    size_t itemCount = GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        CheckBox* pCheckBox = dynamic_cast<CheckBox*>(GetItemAt(index));
        if (pCheckBox != nullptr) {
            if (itemText == pCheckBox->GetText()) {
                return false;
            }
        }
    }

    CheckBox* item = new CheckBox(GetWindow());
    SetAttributeList(item, m_dropboxItemClass.c_str());
    item->SetText(itemText);
    return AddItem(item);
}

void CheckCombo::Activate()
{
    if (!IsActivatable()) {
        return;
    }
    if (m_pCheckComboWnd != nullptr) {
        return;
    }

    m_pCheckComboWnd = new CCheckComboWnd();
    m_pCheckComboWnd->InitComboWnd(this);
    m_pCheckComboWnd->AttachWindowClose(ToWeakCallback([this](const ui::EventArgs& msg) {
        FireAllEvents(msg);
        return true;
    }));
    Invalidate();
}

void CheckCombo::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("dropbox")) {
        SetDropBoxAttributeList(strValue);
    }
    else if (strName == _T("dropbox_item_class")) {        
        SetDropboxItemClass(strValue);
    }
    else if (strName == _T("selected_item_class")) {
        SetSelectedItemClass(strValue);
    }
    else if (strName == _T("vscrollbar")) {
    }
    else if ((strName == _T("dropbox_size")) || (strName == _T("dropboxsize"))) {
        UiSize szDropBoxSize;
        AttributeUtil::ParseSizeValue(strValue.c_str(), szDropBoxSize);
        SetDropBoxSize(szDropBoxSize, true);
    }
    else if ((strName == _T("popup_top")) || (strName == _T("popuptop"))) {
        SetPopupTop(strValue == _T("true"));
    }
    else if (strName == _T("height")) {
        __super::SetAttribute(strName, strValue);
        if (strValue != _T("stretch") && strValue != _T("auto")) {
            ASSERT(_wtoi(strValue.c_str()) >= 0);
            m_iOrgHeight = _wtoi(strValue.c_str());
            SetMaxHeight(m_iOrgHeight * 3, true);
            SetMinHeight(m_iOrgHeight, true);
        }
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void CheckCombo::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }

    UiSize szDropBoxSize = GetDropBoxSize();
    szDropBoxSize = Dpi().GetScaleSize(szDropBoxSize, nOldDpiScale);
    SetDropBoxSize(szDropBoxSize, false);

    if (m_pDropList != nullptr) {
        if (m_pDropList->GetWindow() == nullptr) {
            m_pDropList->SetWindow(GetWindow());
        }
        if ((m_pDropList->GetWindow() == GetWindow()) && (m_pDropList->GetParent() == nullptr)) {
            m_pDropList->ChangeDpiScale(nOldDpiScale, nNewDpiScale);
        }
    }

    __super::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void CheckCombo::SetDropBoxAttributeList(const DString& pstrList)
{
    SetAttributeList(m_pDropList.get(), pstrList);
}

void CheckCombo::SetDropboxItemClass(const DString& classValue)
{
    m_dropboxItemClass = classValue;
}

void CheckCombo::SetSelectedItemClass(const DString& classValue)
{
    m_selectedItemClass = classValue;
}

void CheckCombo::UpdateComboWndPos()
{
    if (m_pCheckComboWnd != nullptr) {
        m_pCheckComboWnd->UpdateComboWnd();
    }
}

void CheckCombo::ParseAttributeList(const DString& strList,
                                    std::vector<std::pair<DString, DString>>& attributeList) const
{
    if (strList.empty()) {
        return;
    }
    DString strValue = strList;
    //这个是手工写入的属性，以花括号{}代替双引号，编写的时候就不需要转义字符了；
    StringUtil::ReplaceAll(_T("{"), _T("\""), strValue);
    StringUtil::ReplaceAll(_T("}"), _T("\""), strValue);
    if (strValue.find(_T("\"")) != DString::npos) {
        AttributeUtil::ParseAttributeList(strValue, _T('\"'), attributeList);
    }
    else if (strValue.find(_T("\'")) != DString::npos) {
        AttributeUtil::ParseAttributeList(strValue, _T('\''), attributeList);
    }
}

void CheckCombo::SetAttributeList(Control* pControl, const DString& classValue)
{
    std::vector<std::pair<DString, DString>> attributeList;
    ParseAttributeList(classValue, attributeList);
    if (!attributeList.empty()) {
        //按属性列表设置
        for (const auto& attribute : attributeList) {
            pControl->SetAttribute(attribute.first, attribute.second);
        }
    }
    else if (!classValue.empty()) {
        //按Class名称设置
        pControl->SetClass(classValue);
    }
}

const UiSize& CheckCombo::GetDropBoxSize() const
{
    return m_szDropBox;
}

void CheckCombo::SetDropBoxSize(UiSize szDropBox, bool bNeedScaleDpi)
{
    ASSERT(szDropBox.cy > 0);
    if (szDropBox.cy <= 0) {
        return;
    }
    szDropBox.Validate();
    if (bNeedScaleDpi) {
        Dpi().ScaleSize(szDropBox);
    }
    m_szDropBox = szDropBox;
}

bool CheckCombo::OnSelectItem(const ui::EventArgs& args)
{
    if (args.pSender == nullptr) {
        return true;
    }
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(args.pSender);
    if (pCheckBox == nullptr) {
        return true;
    }
    DString itemText = pCheckBox->GetText();
    if (itemText.empty()) {
        return true;
    }

    Label* item = new Label(m_pList->GetWindow());
    SetAttributeList(item, m_selectedItemClass.c_str());
    item->SetText(itemText);
    m_pList->AddItem(item);
    UpdateSelectedListHeight();
    return true;
}

bool CheckCombo::OnUnSelectItem(const ui::EventArgs& args)
{
    if (args.pSender == nullptr) {
        return true;
    }
    CheckBox* pCheckBox = dynamic_cast<CheckBox*>(args.pSender);
    if (pCheckBox == nullptr) {
        return true;
    }
    DString itemText = pCheckBox->GetText();
    if (itemText.empty()) {
        return true;
    }

    size_t itemCount = m_pList->GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        Label* pLabel = dynamic_cast<Label*>(m_pList->GetItemAt(index));
        if (pLabel != nullptr) {
            if (pLabel->GetText() == itemText) {
                m_pList->RemoveItem(pLabel);
                break;
            }
        }
    }
    UpdateSelectedListHeight();
    return true;
}

void CheckCombo::UpdateSelectedListHeight()
{
    UiFixedInt oldFixedHeight = m_pList->GetFixedHeight();
    m_pList->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
    UiSize szAvailable(GetWidth(), 0);
    UiEstSize estSize = m_pList->EstimateSize(szAvailable);
    m_pList->SetFixedHeight(oldFixedHeight, false, false);
    if (estSize.cy.IsInt32()) {
        SetFixedHeight(ui::UiFixedInt(estSize.cy.GetInt32()), true, false);
    }
    else {
        SetFixedHeight(ui::UiFixedInt((int)m_pList->GetItemCount() * m_iOrgHeight), true, true);
    }
}

void CheckCombo::GetSelectedText(std::vector<DString>& selectedText) const
{
    size_t itemCount = m_pList->GetItemCount();
    for (size_t index = 0; index < itemCount; ++index) {
        Label* pLabel = dynamic_cast<Label*>(m_pList->GetItemAt(index));
        if (pLabel != nullptr) {
            selectedText.push_back(pLabel->GetText());
        }
    }
}

void CheckCombo::ClearAll()
{
    m_pList->RemoveAllItems();
    m_pDropList->RemoveAllItems();
    SetFixedHeight(ui::UiFixedInt(m_iOrgHeight), true, true);
}

bool CheckCombo::OnListButtonDown(const ui::EventArgs& /*args*/)
{
    Activate();
    return true;
}

} //namespace ui
