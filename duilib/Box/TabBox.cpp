#include "TabBox.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

TabBox::TabBox(Window* pWindow, Layout* pLayout)
: Box(pWindow, pLayout)
, m_iCurSel(Box::InvalidIndex)
, m_iInitSel(Box::InvalidIndex)
, m_bFadeSwith(false)
{

}

DString TabBox::GetType() const { return DUI_CTR_TABBOX; }

void TabBox::SetAttribute(const DString& strName, const DString& strValue)
{
    if ((strName == _T("selected_id")) || (strName == _T("selectedid"))) {
        size_t iSel = (size_t)StringUtil::StringToInt32(strValue);
        if (IsInited()) {
            SelectItem(iSel);
        }
        else {
            m_iInitSel = iSel;
        }
    }
    else if ((strName == _T("fade_switch")) || (strName == _T("fadeswitch"))) {
        SetFadeSwitch(strValue == _T("true"));
    }
    else {
        Box::SetAttribute(strName, strValue);
    }
}

void TabBox::OnSetVisible(bool bChanged)
{
    BaseClass::OnSetVisible(bChanged);

    if (!IsInited() || !IsVisible()) {
        //未初始化或者隐藏时，不处理
        return;
    }

    //显示时，只能显示一个页面，其他页面需要隐藏    
    size_t nCurSel = GetCurSel();
    bool bSelected = false;
    if (Box::IsValidItemIndex(nCurSel)) {
        bSelected = DoSelectItem(nCurSel, false, false);
    }
    if (!bSelected && (GetItemCount() > 0) && (GetCurSel() > GetItemCount())) {
        //默认选择第一个页面，避免所有页面都同时显示出来
        DoSelectItem(0, false, false);
    }
}

void TabBox::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    bool bSelected = false;
    if (Box::IsValidItemIndex(m_iInitSel)) {
        bSelected = DoSelectItem(m_iInitSel, false, true);
    }

    if (!bSelected && (GetItemCount() > 0) && (m_iCurSel > GetItemCount())) {
        //初始化时，默认选择第一个页面，避免所有页面都同时显示出来
        DoSelectItem(0, false, true);
    }
}

bool TabBox::AddItem(Control* pControl)
{
    return AddItemAt(pControl, GetItemCount());
}

bool TabBox::AddItemAt(Control* pControl, size_t iIndex)
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return false;
    }
    bool ret = Box::AddItemAt(pControl, iIndex);
    if (!ret) {
        return ret;
    }        

    if(!Box::IsValidItemIndex(m_iCurSel) && pControl->IsVisible()) {
        //如果当前无选择页，则选择新插入的页面
        size_t iCurSel = GetItemIndex(pControl);
        ASSERT(iCurSel == iIndex);
        ret = SelectItem(iCurSel);
    }
    else if(Box::IsValidItemIndex(m_iCurSel) && (iIndex <= m_iCurSel)) {
        //在当前选择页面前面插入一个新的页面
        m_iCurSel += 1;
    }

    if (m_iCurSel != iIndex) {
        //如果不是选择页面，则隐藏
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

    size_t index = GetItemIndex(pControl);
    ASSERT(Box::IsValidItemIndex(index));
    bool ret = Box::RemoveItem(pControl);
    if (!ret) {
        return false;
    }

    if( m_iCurSel == index)    {
        if (GetItemCount() > 0) {
            //移除当前选择的TAB页面后，选择被移除页面的前一个TAB页面
            size_t newSel = m_iCurSel > 0 ? m_iCurSel - 1 : 0;
            if (newSel >= GetItemCount()) {
                newSel = Box::InvalidIndex;
            }
            m_iCurSel = Box::InvalidIndex;
            ret = DoSelectItem(newSel, false, true);
        }
        else {
            //当前只有一个TAB页，被移除以后，更新选择为未选择
            m_iCurSel = Box::InvalidIndex;
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
    m_iCurSel = Box::InvalidIndex;
    Box::RemoveAllItems();
    ArrangeAncestor();
}

size_t TabBox::GetCurSel() const
{
    return m_iCurSel;
}
    
bool TabBox::DoSelectItem(size_t iIndex, bool bFadeSwith, bool bCheckChanged)
{
    ASSERT(iIndex < m_items.size());
    if (!Box::IsValidItemIndex(iIndex) || (iIndex >= m_items.size())) {
        return false;
    }
    if (bCheckChanged) {
        if (iIndex == m_iCurSel) {
            return true;
        }
    }

    const size_t iOldSel = m_iCurSel;
    m_iCurSel = iIndex;
    const size_t itemCount = m_items.size();
    for(size_t it = 0; it < itemCount; ++it ){
        Control* pItemControl = m_items.at(it);
        ASSERT(pItemControl != nullptr);
        if (it == iIndex) {
            //当前选择的TAB Item
            OnShowTabItem(it);

            if (!bFadeSwith) {
                pItemControl->SetFadeVisible(true);
            }
            else {
                pItemControl->SetVisible(true);
                int startValue = 0;
                int endValue = 0;
                if (m_iCurSel < iOldSel) {
                    startValue = GetPos().Width();
                    endValue = 0;
                }
                else {
                    startValue = -GetPos().Width();
                    endValue = 0;
                }

                auto player = pItemControl->GetAnimationManager().SetFadeInOutX(true, true);
                if (player != nullptr) {
                    player->SetStartValue(startValue);
                    player->SetEndValue(endValue);
                    player->SetSpeedUpfactorA(0.015);
                    CompleteCallback compelteCallback = UiBind(&TabBox::OnAnimationComplete, this, (size_t)it);
                    player->SetCompleteCallback(compelteCallback);
                    player->Start();
                }                
            }
        }
        else {
            //不是当前选择的TAB页面
            OnHideTabItem(it);
            if ((it == iOldSel) && bFadeSwith) {
                //对于原来选择的TAB页面，出发动画效果
                pItemControl->SetVisible(true);
                int startValue = 0;
                int endValue = 0;
                if (m_iCurSel < iOldSel) {
                    startValue = 0;
                    endValue = -GetPos().Width();
                }
                else {
                    startValue = 0;
                    endValue = GetPos().Width();
                }

                auto player = pItemControl->GetAnimationManager().SetFadeInOutX(true, true);
                if (player != nullptr) {
                    player->SetStartValue(startValue);
                    player->SetEndValue(endValue);
                    player->SetSpeedUpfactorA(0.015);
                    CompleteCallback compelteCallback = UiBind(&TabBox::OnAnimationComplete, this, it);
                    player->SetCompleteCallback(compelteCallback);
                    player->Start();
                }                
            }
            else {                
                pItemControl->SetFadeVisible(false);
            }
        }
    }
    if (bCheckChanged && (iOldSel != m_iCurSel)) {
        SendEvent(kEventTabSelect, m_iCurSel, iOldSel);
    }    
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
    ASSERT(index < m_items.size());
    if (index >= m_items.size()) {
        return;
    }
    Control* pContol = m_items.at(index);
    ASSERT(pContol != nullptr);
    if (pContol != nullptr) {
        pContol->SetRenderOffsetX(0);
        if (m_iCurSel != index) {
            pContol->SetVisible(false);
        }
    }
}

bool TabBox::SelectItem(size_t iIndex)
{
    return DoSelectItem(iIndex, IsFadeSwitch(), true);
}

bool TabBox::SelectItem( Control* pControl )
{
    size_t iIndex = GetItemIndex(pControl);
    if (!Box::IsValidItemIndex(iIndex)) {
        return false;
    }        
    else {
        return SelectItem(iIndex);
    }        
}

bool TabBox::SelectItem(const DString& pControlName)
{
    Control* pControl = FindSubControl(pControlName);
    ASSERT(pControl != nullptr);
    return SelectItem(pControl);
}

void TabBox::SetFadeSwitch(bool bFadeSwitch)
{
    m_bFadeSwith = bFadeSwitch;
}

}
