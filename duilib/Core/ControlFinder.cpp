#include "ControlFinder.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Control.h"

namespace ui
{

ControlFinder::ControlFinder():
	m_pRoot(nullptr)
{
}

ControlFinder::~ControlFinder()
{
}

void ControlFinder::SetRoot(Box* pRoot)
{
	m_pRoot = pRoot;
}

Control* ControlFinder::FindControl(const UiPoint& pt) const
{
	ASSERT(m_pRoot != nullptr);
	if (m_pRoot != nullptr) {
		UiPoint ptLocal = pt;
		return m_pRoot->FindControl(__FindControlFromPoint, &ptLocal, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
	}
	return nullptr;
}

Control* ControlFinder::FindContextMenuControl(const UiPoint* pt) const
{
	Control* pControl = nullptr;
	if(m_pRoot != nullptr){
		if (pt != nullptr) {
			UiPoint ptLocal = *pt;
			pControl = m_pRoot->FindControl(__FindContextMenuControl, &ptLocal, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_HITTEST | UIFIND_TOP_FIRST);
		}
		else {
			pControl = m_pRoot->FindControl(__FindContextMenuControl, nullptr, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_TOP_FIRST);
		}
	}
	return pControl;
}

Control* ControlFinder::FindControl2(const std::wstring& strName) const
{
	Control* pFindedControl = nullptr;
	auto it = m_mNameHash.find(strName);
	if (it != m_mNameHash.end()) {
		pFindedControl = it->second;
	}
	return pFindedControl;
}

Control* ControlFinder::FindSubControlByPoint(Control* pParent, const UiPoint& pt) const
{
	if (pParent == nullptr) {
		pParent = m_pRoot;
	}
	ASSERT(pParent);
	if (pParent != nullptr) {
		UiPoint ptLocal = pt;
		return pParent->FindControl(__FindControlFromPoint, &ptLocal, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
	}
	return nullptr;
}

Control* ControlFinder::FindSubControlByName(Control* pParent, const std::wstring& strName) const
{
	if (pParent == nullptr) {
		pParent = m_pRoot;
	}
	ASSERT(pParent);
	if (pParent) {
		return pParent->FindControl(__FindControlFromName, (LPVOID)strName.c_str(), UIFIND_ALL);
	}
	return nullptr;
}

void ControlFinder::ReapObjects(Control* pControl)
{
	if (pControl == nullptr) {
		return;
	}
	const std::wstring sName = pControl->GetName();
	if (!sName.empty()) {
		auto it = m_mNameHash.find(sName);
		if (it != m_mNameHash.end())
		{
			m_mNameHash.erase(it);
		}
	}
}

Control* CALLBACK ControlFinder::__FindControlFromNameHash(Control* pThis, LPVOID pData)
{
	const std::wstring sName = pThis->GetName();
	if (sName.empty()) {
		return nullptr;
	}
	// Add this control to the hash list
	ControlFinder* pManager = static_cast<ControlFinder*>(pData);
	if (pManager != nullptr) {
		auto iter = pManager->m_mNameHash.find(sName);
		if (iter != pManager->m_mNameHash.end()){
			//ASSERT(iter->second == pThis);
			if (iter->second != pThis) {
				iter->second = pThis;
			}
		}
		else {
			pManager->m_mNameHash[sName] = pThis;
		}
	}	
	return nullptr; // Attempt to add all controls
}

Control* CALLBACK ControlFinder::__FindControlFromCount(Control* /*pThis*/, LPVOID pData)
{
	int* pnCount = static_cast<int*>(pData);
	if (pnCount != nullptr)	{
		(*pnCount)++;
	}	
	return nullptr;  // Count all controls
}

Control* CALLBACK ControlFinder::__FindControlFromPoint(Control* pThis, LPVOID pData)
{
	UiPoint* pPoint = static_cast<UiPoint*>(pData);
	if ((pPoint == nullptr) || (pThis == nullptr)) {
		return nullptr;
	}
	UiRect pos = pThis->GetPos();
	return pos.ContainsPt(*pPoint) ? pThis : nullptr;
}

Control* CALLBACK ControlFinder::__FindControlFromTab(Control* pThis, LPVOID pData)
{
	if (pThis == nullptr) {
		return nullptr;
	}
	FINDTABINFO* pInfo = static_cast<FINDTABINFO*>(pData);
	if (pInfo == nullptr) {
		return nullptr;
	}
	if (pInfo->pFocus == pThis) {
		if (pInfo->bForward) {
			pInfo->bNextIsIt = true;
		}
		return pInfo->bForward ? nullptr : pInfo->pLast;
	}
	if ((pThis->GetControlFlags() & UIFLAG_TABSTOP) == 0) {
		return nullptr;
	}
	pInfo->pLast = pThis;
	if (pInfo->bNextIsIt) {
		return pThis;
	}
	if (pInfo->pFocus == nullptr) {
		return pThis;
	}
	return nullptr;  // Examine all controls
}

Control* CALLBACK ControlFinder::__FindControlFromUpdate(Control* pThis, LPVOID /*pData*/)
{
	if (pThis == nullptr) {
		return nullptr;
	}
	return pThis->IsArranged() ? pThis : nullptr;
}

Control* CALLBACK ControlFinder::__FindControlFromName(Control* pThis, LPVOID pData)
{
	LPCTSTR pstrName = static_cast<LPCTSTR>(pData);
	if ((pstrName == nullptr) || (pThis == nullptr)) {
		return nullptr;
	}
	const std::wstring sName = pThis->GetName();
	if (sName.empty()) {
		return nullptr;
	}
	return (_wcsicmp(sName.c_str(), pstrName) == 0) ? pThis : nullptr;
}

Control* CALLBACK ControlFinder::__FindContextMenuControl(Control* pThis, LPVOID /*pData*/)
{
	if (pThis != nullptr) {
		if (!pThis->IsContextMenuUsed()) {
			return nullptr;
		}
	}	
	return pThis;
}

} // namespace ui
