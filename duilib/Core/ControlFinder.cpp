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

void ControlFinder::Clear()
{
    m_pRoot = nullptr;
    m_mNameHash.clear();
}

Control* ControlFinder::FindControl(const UiPoint& pt) const
{
    ASSERT(m_pRoot != nullptr);
    if (m_pRoot != nullptr) {
        UiPoint ptLocal = pt;
        return m_pRoot->FindControl(FindControlFromPoint, &ptLocal, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST, pt);
    }
    return nullptr;
}

Control* ControlFinder::FindContextMenuControl(const UiPoint* pt) const
{
    Control* pControl = nullptr;
    if(m_pRoot != nullptr){
        if (pt != nullptr) {
            UiPoint ptLocal = *pt;
            pControl = m_pRoot->FindControl(FindContextMenuControl, &ptLocal, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_HITTEST | UIFIND_TOP_FIRST, ptLocal);
        }
        else {
            pControl = m_pRoot->FindControl(FindContextMenuControl, nullptr, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_TOP_FIRST);
        }
    }
    return pControl;
}

Box* ControlFinder::FindDroppableBox(const UiPoint& pt, uint8_t nDropInId) const
{
    Control* pControl = nullptr;
    ASSERT(m_pRoot != nullptr);
    if (m_pRoot != nullptr) {
        pControl = m_pRoot->FindControl(FindControlFromDroppableBox, (void*)(size_t)nDropInId, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST | UIFIND_DRAG_DROP, pt);
    }
    if (pControl != nullptr) {
        return dynamic_cast<Box*>(pControl);
    }
    return nullptr;
}

Control* ControlFinder::FindControl2(const DString& strName) const
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
        return pParent->FindControl(FindControlFromPoint, &ptLocal, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST, pt);
    }
    return nullptr;
}

Control* ControlFinder::FindSubControlByName(Control* pParent, const DString& strName) const
{
    if (pParent == nullptr) {
        pParent = m_pRoot;
    }
    ASSERT(pParent);
    if (pParent) {
        return pParent->FindControl(FindControlFromName, (void*)strName.c_str(), UIFIND_ALL);
    }
    return nullptr;
}

void ControlFinder::RemoveControl(Control* pControl)
{
    if (pControl == nullptr) {
        return;
    }
    const DString sName = pControl->GetName();
    if (!sName.empty()) {
        auto it = m_mNameHash.find(sName);
        if (it != m_mNameHash.end()) {
            m_mNameHash.erase(it);
        }
    }
}

void ControlFinder::AddControl(Control* pControl)
{
    if (pControl == nullptr) {
        return;
    }
    const DString sName = pControl->GetName();
    if (sName.empty()) {
        return;
    }
    auto iter = m_mNameHash.find(sName);
    if (iter != m_mNameHash.end()) {
        if (iter->second != pControl) {
            //控件名称相同的，覆盖
            iter->second = pControl;
        }
    }
    else {
        m_mNameHash[sName] = pControl;
    }
}

Control* ControlFinder::FindControlFromPoint(Control* pThis, void* pData)
{
    UiPoint* pPoint = static_cast<UiPoint*>(pData);
    if ((pPoint == nullptr) || (pThis == nullptr)) {
        return nullptr;
    }
    UiPoint pt(*pPoint);
    pt.Offset(pThis->GetScrollOffsetInScrollBox());
    UiRect rect = pThis->GetRect();
    return rect.ContainsPt(pt) ? pThis : nullptr;
}

Control* ControlFinder::FindControlFromTab(Control* pThis, void* pData)
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

Control* ControlFinder::FindControlFromUpdate(Control* pThis, void* /*pData*/)
{
    if (pThis == nullptr) {
        return nullptr;
    }
    return pThis->IsArranged() ? pThis : nullptr;
}

Control* ControlFinder::FindControlFromName(Control* pThis, void* pData)
{
    const DString::value_type* pstrName = static_cast<const DString::value_type*>(pData);
    if ((pstrName == nullptr) || (pThis == nullptr)) {
        return nullptr;
    }
    const DString sName = pThis->GetName();
    if (sName.empty()) {
        return nullptr;
    }
    return (StringUtil::StringICompare(sName.c_str(), pstrName) == 0) ? pThis : nullptr;
}

Control* ControlFinder::FindContextMenuControl(Control* pThis, void* /*pData*/)
{
    if (pThis != nullptr) {
        if (!pThis->IsContextMenuUsed()) {
            return nullptr;
        }
    }    
    return pThis;
}

Control* ControlFinder::FindControlFromDroppableBox(Control* pThis, void* pData)
{
    if (pThis != nullptr) {
        Box* pBox = dynamic_cast<Box*>(pThis);
        if (pBox != nullptr) {
            uint8_t nDropInId = (uint8_t)(size_t)pData;
            if (nDropInId == pBox->GetDropInId()) {
                return pThis;
            }            
        }
    }
    return nullptr;
}

} // namespace ui
