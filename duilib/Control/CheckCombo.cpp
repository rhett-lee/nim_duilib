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

	virtual std::wstring GetWindowClassName() const override;
	virtual void OnFinalMessage(HWND hWnd) override;
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
	CreateWnd(pOwner->GetWindow()->GetHWND(), L"", WS_POPUP, WS_EX_TOOLWINDOW);
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
	rc.top = rc.bottom + 1;		    // 父窗口left、bottom位置作为弹出窗口起点
	rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
	if (szDrop.cx > 0) {
		rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度
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

std::wstring CCheckComboWnd::GetWindowClassName() const
{
	return L"CCheckComboWnd";
}

void CCheckComboWnd::OnFinalMessage(HWND /*hWnd*/)
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
	m_pOwner->SetState(ui::kControlStateNormal);
	m_pOwner->Invalidate();
	delete this;
}

void CCheckComboWnd::OnSeleteItem()
{
	PostMsg(WM_KILLFOCUS);
}

LRESULT CCheckComboWnd::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	bHandled = false;
	if (uMsg == WM_CREATE) {
		this->InitWnd(GetHWND());
		Box* pRoot = new Box;
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

CheckCombo::CheckCombo() :
	m_pCheckComboWnd(nullptr),
	m_szDropBox(0, 150),
	m_bPopupTop(false),
	m_iOrgHeight(20)
{
	//需要调用设置函数，内部有DPI自适应的逻辑调整大小
	SetDropBoxSize({ 0, 150 });

	m_pDropList.reset(new ui::ListBox(new ui::VLayout));
	m_pDropList->EnableScrollBar();
	
	m_pList.reset(new ui::ListBox(new ui::VTileLayout));
	m_pList->AttachButtonDown(std::bind(&CheckCombo::OnListButtonDown, this, std::placeholders::_1));
	m_pList->SetMouseChildEnabled(false);
	m_pList->EnableScrollBar();

	Box::AddItem(m_pList.get());
	SetMaxHeight(m_iOrgHeight * 3);
	SetMinHeight(m_iOrgHeight);
}

CheckCombo::~CheckCombo()
{
	Box::RemoveItem(m_pList.get());
	m_pList.reset();
	m_pDropList.reset();
}

std::wstring CheckCombo::GetType() const { return DUI_CTR_CHECK_COMBO; }

bool CheckCombo::AddItem(Control* pControl)
{
	ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(pControl);
	if (pCheckBox) {
		pCheckBox->AttachSelect(std::bind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
		pCheckBox->AttachUnSelect(std::bind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
	}
	else	{
		ui::CheckBoxBox* pCheckBoxBox = dynamic_cast<ui::CheckBoxBox*>(pControl);
		if (pCheckBoxBox) {
			pCheckBoxBox->AttachSelect(std::bind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
			pCheckBoxBox->AttachUnSelect(std::bind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
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
		pCheckBox->AttachSelect(std::bind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
		pCheckBox->AttachUnSelect(std::bind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
	}
	else {
		ui::CheckBoxBox* pCheckBoxBox = dynamic_cast<ui::CheckBoxBox*>(pControl);
		if (pCheckBoxBox) {
			pCheckBoxBox->AttachSelect(std::bind(&CheckCombo::OnSelectItem, this, std::placeholders::_1));
			pCheckBoxBox->AttachUnSelect(std::bind(&CheckCombo::OnUnSelectItem, this, std::placeholders::_1));
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

bool CheckCombo::AddTextItem(const std::wstring& itemText)
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

	CheckBox* item = new CheckBox;
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

void CheckCombo::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"dropbox") {
		SetDropBoxAttributeList(strValue);
	}
	else if (strName == L"dropbox_item_class") {		
		SetDropboxItemClass(strValue);
	}
	else if (strName == L"selected_item_class") {
		SetSelectedItemClass(strValue);
	}
	else if (strName == L"vscrollbar") {
	}
	else if ((strName == L"dropbox_size") || (strName == L"dropboxsize")) {
		ui::UiSize szDropBoxSize;
		ui::AttributeUtil::ParseSizeValue(strValue.c_str(), szDropBoxSize);
		SetDropBoxSize(szDropBoxSize);
	}
	else if ((strName == L"popup_top") || (strName == L"popuptop")) {
		SetPopupTop(strValue == L"true");
	}
	else if (strName == L"height") {
		__super::SetAttribute(strName, strValue);
		if (strValue != L"stretch" && strValue != L"auto") {
			ASSERT(_wtoi(strValue.c_str()) >= 0);
			m_iOrgHeight = _wtoi(strValue.c_str());
			SetMaxHeight(m_iOrgHeight * 3);
			SetMinHeight(m_iOrgHeight);
		}
	}
	else {
		__super::SetAttribute(strName, strValue);
	}
}

void CheckCombo::SetDropBoxAttributeList(const std::wstring& pstrList)
{
	SetAttributeList(m_pDropList.get(), pstrList);
}

void CheckCombo::SetDropboxItemClass(const std::wstring& classValue)
{
	m_dropboxItemClass = classValue;
}

void CheckCombo::SetSelectedItemClass(const std::wstring& classValue)
{
	m_selectedItemClass = classValue;
}

void CheckCombo::UpdateComboWndPos()
{
	if (m_pCheckComboWnd != nullptr) {
		m_pCheckComboWnd->UpdateComboWnd();
	}
}

void CheckCombo::ParseAttributeList(const std::wstring& strList,
								    std::vector<std::pair<std::wstring, std::wstring>>& attributeList) const
{
	if (strList.empty()) {
		return;
	}
	std::wstring strValue = strList;
	//这个是手工写入的属性，以花括号{}代替双引号，编写的时候就不需要转义字符了；
	StringHelper::ReplaceAll(L"{", L"\"", strValue);
	StringHelper::ReplaceAll(L"}", L"\"", strValue);
	if (strValue.find(L"\"") != std::wstring::npos) {
		AttributeUtil::ParseAttributeList(strValue, L'\"', attributeList);
	}
	else if (strValue.find(L"\'") != std::wstring::npos) {
		AttributeUtil::ParseAttributeList(strValue, L'\'', attributeList);
	}
}

void CheckCombo::SetAttributeList(Control* pControl, const std::wstring& classValue)
{
	std::vector<std::pair<std::wstring, std::wstring>> attributeList;
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

ui::UiSize CheckCombo::GetDropBoxSize() const
{
	return m_szDropBox;
}

void CheckCombo::SetDropBoxSize(ui::UiSize szDropBox)
{
	ui::GlobalManager::Instance().Dpi().ScaleSize(szDropBox);
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
	std::wstring itemText = pCheckBox->GetText();
	if (itemText.empty()) {
		return true;
	}

	ui::Label* item = new ui::Label;
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
	std::wstring itemText = pCheckBox->GetText();
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

void CheckCombo::GetSelectedText(std::vector<std::wstring>& selectedText) const
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
