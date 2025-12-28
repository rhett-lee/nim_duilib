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

Box* ControlFinder::GetRoot() const
{
    return m_pRoot;
}

void ControlFinder::Clear()
{
    m_pRoot = nullptr;
    m_controlNameMap.clear();
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

Control* ControlFinder::FindToolTipControl(const UiPoint& pt) const
{
    ASSERT(m_pRoot != nullptr);
    if (m_pRoot != nullptr) {
        UiPoint ptLocal = pt;
        return m_pRoot->FindControl(FindControlFromPoint, &ptLocal, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST | UIFIND_TOOLTIP, pt);
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
    if (strName.empty()) {
        return nullptr;
    }
    //优先从缓存中查找
    Control* pFindControl = FindControlInCache(pParent, strName);
    if (pFindControl != nullptr) {
        return pFindControl;
    }
    if (pParent == nullptr) {
        pParent = m_pRoot;
    }
    ASSERT(pParent != nullptr);
    if (pParent != nullptr) {
        return pParent->FindControl(FindControlFromName, (void*)strName.c_str(), UIFIND_ALL);
    }
    return nullptr;
}

Control* ControlFinder::FindControlInCache(Control* pAncestor, const DString& strName) const
{
    if (strName.empty()) {
        return nullptr;
    }
    if ((pAncestor != nullptr) && pAncestor->IsNameEquals(strName)) {
        return pAncestor;
    }
    Control* pFindedControl = nullptr;
    auto iter = m_controlNameMap.find(strName);
    if (iter != m_controlNameMap.end()) {
        const std::vector<ControlPtr>& controlList = iter->second;
        for (const ControlPtr& spControl : controlList) {
            if (pAncestor != nullptr) {
                if ((spControl != nullptr) &&
                    (pAncestor->GetWindow() == spControl->GetWindow()) &&
                    spControl->IsNameEquals(strName) &&
                    PlaceHolder::IsControlRelated(pAncestor, spControl.get())) {
                    //在同一个窗口下，并且与pAncestor是父祖关系
                    pFindedControl = spControl.get();
                    break;
                }
            }
            else {
                if ((spControl != nullptr) && spControl->IsNameEquals(strName)) {
                    pFindedControl = spControl.get();
                    break;
                }
            }
        }
    }
    return pFindedControl;
}

void ControlFinder::RemoveControl(Control* pControl)
{
    if ((pControl == nullptr) || !pControl->HasName()) {
        return;
    }
    const DString sName = pControl->GetName();
    if (!sName.empty()) {
        auto iter = m_controlNameMap.find(sName);
        if (iter != m_controlNameMap.end()) {
            std::vector<ControlPtr>& controlList = iter->second;
            for (auto pos = controlList.begin(); pos != controlList.end(); ++pos) {
                if (pControl == *pos) {
                    controlList.erase(pos);
                    break;
                }
            }
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
    //控件的名字允许重复(Control的name属性), 但查找时，相同父控件下只命中第一个控件
    auto iter = m_controlNameMap.find(sName);
    if (iter != m_controlNameMap.end()) {
        std::vector<ControlPtr>& controlList = iter->second;
        for (auto pos = controlList.begin(); pos != controlList.end(); ++pos) {
            if (pControl == *pos) {
                controlList.erase(pos);
                break;
            }
        }
        const size_t nMaxControlCount = (size_t)10; //名字重复的控件，最多只保存10个，避免过多导致性能降低
        if (controlList.size() < nMaxControlCount) {
            controlList.push_back(ControlPtr(pControl));
        }
        else {
            //超过数量，不添加
        }
    }
    else {
        m_controlNameMap[sName].push_back(ControlPtr(pControl));
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
    
    //比较控件的名称，区分大小写
    return (pThis->IsNameEquals(pstrName)) ? pThis : nullptr;
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
