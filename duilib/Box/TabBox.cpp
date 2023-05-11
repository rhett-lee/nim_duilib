#include "TabBox.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Core/Window.h"

namespace ui
{

TabBox::TabBox(Layout* pLayout) 
: Box(pLayout)
, m_iCurSel(-1)
, m_iInitSel(-1)
, m_bIsInit(false)
, m_bFadeSwith(false)
{

}

void TabBox::DoInit()
{
	m_bIsInit = true;
	if (m_iInitSel != -1) {
		SelectItem(m_iInitSel);
	}
}

std::wstring TabBox::GetType() const { return DUI_CTR_TABBOX; }

bool TabBox::AddItem(Control* pControl)
{
	return AddItemAt(pControl, GetItemCount());
}

bool TabBox::AddItemAt(Control* pControl, size_t iIndex)
{
	bool ret = Box::AddItemAt(pControl, iIndex);
	if (!ret) {
		return ret;
	}		

	if(m_iCurSel == -1 && pControl->IsVisible()) {
		int iCurSel = GetItemIndex(pControl);
		ASSERT(iCurSel == (int)iIndex);
		ret = SelectItem(iCurSel);
	}
	else if( m_iCurSel != -1 && iIndex <= (size_t)m_iCurSel ) {
		m_iCurSel += 1;
	}

	if (m_iCurSel != iIndex) {
		pControl->SetFadeVisible(false);
		OnHideTabItem(iIndex);
	}
	return ret;
}

bool TabBox::RemoveItem(Control* pControl)
{
	if (pControl == nullptr) {
		return false;
	}

	int index = GetItemIndex(pControl);
	ASSERT(index >= 0);
	bool ret = Box::RemoveItem(pControl);
	if (!ret) {
		return false;
	}

	if( m_iCurSel == index)	{
		if( GetItemCount() > 0 ) {
			//移除当前选择的TAB页面后，选择被移除页面的前一个TAB页面
			int newSel = m_iCurSel - 1;
			if (newSel >= GetItemCount()) {
				newSel = -1;
			}
			if (newSel < 0) {
				newSel = 0;
			}
			m_iCurSel = -1;
			ret = SelectItem(newSel);
		}
		else {
			//当前只有一个TAB页，被移除以后，更新选择为未选择
			m_iCurSel = -1;
		}
		ArrangeAncestor();
	}
	else if( m_iCurSel > index ) {
		//更新当前选择页面的下标值，使其保持不变
		m_iCurSel -= 1;
	}
	return ret;
}

bool TabBox::RemoveItemAt(size_t iIndex)
{
	Control* pControl = GetItemAt(iIndex);
	if (pControl == nullptr) {
		return false;
	}
	return RemoveItem(pControl);
}

void TabBox::RemoveAllItems()
{
	m_iCurSel = -1;
	Box::RemoveAllItems();
	ArrangeAncestor();
}

int TabBox::GetCurSel() const
{
	return m_iCurSel;
}
	
bool TabBox::SelectItem(int iIndex)
{
	if ((iIndex < 0) || ((size_t)iIndex >= m_items.size())) {
		return false;
	}
	if (iIndex == m_iCurSel) {
		return true;
	}

	const int iOldSel = m_iCurSel;
	m_iCurSel = iIndex;
	const int itemCount = (int)m_items.size();
	for( int it = 0; it < itemCount; ++it ){
		Control* pItemControl = m_items.at(it);
		ASSERT(pItemControl != nullptr);
		if (it == iIndex) {
			//当前选择的TAB Item
			OnShowTabItem((size_t)it);

			if (!IsFadeSwitch()) {
				pItemControl->SetFadeVisible(true);
			}
			else {
				pItemControl->SetVisible(true);
				int startValue = 0;
				int endValue = 0;
				if (m_iCurSel < iOldSel) {
					startValue = GetPos().GetWidth();
					endValue = 0;
				}
				else {
					startValue = -GetPos().GetWidth();
					endValue = 0;
				}

				auto player = pItemControl->GetAnimationManager().SetFadeInOutX(true, true);
				if (player != nullptr) {
					player->SetStartValue(startValue);
					player->SetEndValue(endValue);
					player->SetSpeedUpfactorA(0.015);
					CompleteCallback compelteCallback = nbase::Bind(&TabBox::OnAnimationComplete, this, (size_t)it);
					player->SetCompleteCallback(compelteCallback);
					player->Start();
				}				
			}
		}
		else {
			//不是当前选择的TAB页面
			OnHideTabItem((size_t)it);
			if ((it == iOldSel) && IsFadeSwitch()) {
				//对于原来选择的TAB页面，出发动画效果
				pItemControl->SetVisible(true);
				int startValue = 0;
				int endValue = 0;
				if (m_iCurSel < iOldSel) {
					startValue = 0;
					endValue = -GetPos().GetWidth();
				}
				else {
					startValue = 0;
					endValue = GetPos().GetWidth();
				}

				auto player = pItemControl->GetAnimationManager().SetFadeInOutX(true, true);
				if (player != nullptr) {
					player->SetStartValue(startValue);
					player->SetEndValue(endValue);
					player->SetSpeedUpfactorA(0.015);
					CompleteCallback compelteCallback = nbase::Bind(&TabBox::OnAnimationComplete, this, it);
					player->SetCompleteCallback(compelteCallback);
					player->Start();
				}				
			}
			else {				
				pItemControl->SetFadeVisible(false);
			}
		}
	}		

	if( GetWindow() != nullptr ) {
		GetWindow()->SetNextTabControl();
	}
	SendEvent(kEventSelect, m_iCurSel, iOldSel);
	return true;
}

void TabBox::OnHideTabItem(size_t index)
{
	ASSERT(index < m_items.size());
	if (index >= m_items.size()) {
		return;
	}
	Control* pContol = m_items.at(index);
	ASSERT(pContol != nullptr);
	if (pContol == nullptr) {
		return;
	}
	pContol->SetMouseEnabled(false);
	Box* pBox = dynamic_cast<Box*>(pContol);
	if (pBox != nullptr) {
		pBox->SetMouseChildEnabled(false);
	}
}

void TabBox::OnShowTabItem(size_t index)
{
	ASSERT(index < m_items.size());
	if (index >= m_items.size()) {
		return;
	}
	Control* pContol = m_items.at(index);
	ASSERT(pContol != nullptr);
	if (pContol == nullptr) {
		return;
	}
	pContol->SetMouseEnabled(true);
	Box* pBox = dynamic_cast<Box*>(pContol);
	if (pBox != nullptr) {
		pBox->SetMouseChildEnabled(true);
	}
}

void TabBox::OnAnimationComplete(size_t index)
{
	ASSERT((size_t)index < m_items.size());
	if ((size_t)index >= m_items.size()) {
		return;
	}
	Control* pContol = m_items.at(index);
	ASSERT(pContol != nullptr);
	if (pContol != nullptr) {
		pContol->SetRenderOffsetX(0);
		if (m_iCurSel != (int)index) {
			pContol->SetVisible(false);
		}
	}
}

bool TabBox::SelectItem( Control* pControl )
{
	int iIndex = GetItemIndex(pControl);
	if (iIndex == -1) {
		return false;
	}		
	else {
		return SelectItem(iIndex);
	}		
}

bool TabBox::SelectItem(const std::wstring& pControlName)
{
	Control* pControl = FindSubControl(pControlName);
	ASSERT(pControl != nullptr);
	return SelectItem(pControl);
}

void TabBox::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"selectedid"){
		int iSel = _wtoi(strValue.c_str());
		if (m_bIsInit) {
			SelectItem(iSel);
		}
		else {
			m_iInitSel = iSel;
		}
	}
	else if (strName == L"fadeswitch") {
		SetFadeSwitch(strValue == L"true");
	}
	else {
		Box::SetAttribute(strName, strValue);
	}
}

void TabBox::SetFadeSwitch(bool bFadeSwitch)
{
	m_bFadeSwith = bFadeSwitch;
}

}
