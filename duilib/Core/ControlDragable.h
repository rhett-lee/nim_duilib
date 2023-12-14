#ifndef UI_CORE_CONTROL_DRAGABLE_H_
#define UI_CORE_CONTROL_DRAGABLE_H_

#pragma once

#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/HBox.h"

namespace ui
{
/** 主要功能：支持相同Box内的子控件通过拖动来调整顺序
*/
template<typename T = Control>
class UILIB_API ControlDragableT: public T
{
public:
    ControlDragableT();
    virtual ~ControlDragableT();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;	
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

	/** 设置是否支持拖动改变控件的顺序
	*/
	void SetEnableDragOrder(bool bEnable);

	/** 判断是否支持拖动改变控件的顺序
	*/
	virtual bool IsEnableDragOrder() const;

	/** 设置拖动顺序时，控件的透明度
	*/
	void SetDragAlpha(uint8_t nAlpha);

	/** 获取拖动顺序时，控件的透明度
	*/
	uint8_t GetDragAlpha() const;

	/** 当前是否处于拖拽操作中
	*/
	bool IsInDragging() const;

protected:
	/** @name 拖动相关的成员函数
	* @{ */

	virtual bool ButtonDown(const EventArgs& msg) override;
	virtual bool ButtonUp(const EventArgs& msg) override;
	virtual bool MouseMove(const EventArgs& msg) override;
	virtual bool OnWindowKillFocus(const EventArgs& msg) override;//控件所属的窗口失去焦点

protected:
	/** 子控件的状态
	*/
	struct ItemStatus
	{
		Control* m_pItem = nullptr;
		UiRect m_rcPos;
		uint8_t m_nPaintOrder = 0;
		size_t m_index = Box::InvalidIndex;
	};

	/** 根据鼠标位置调整各个控件的位置(拖动操作的一部分)
	* @param [in] pt 当前鼠标的位置
	* @param [in] ptMouseDown 鼠标按下时的位置
	* @param [in] rcItemList 子控件的列表
	*/
	virtual void AdjustItemPos(const UiPoint& pt, const UiPoint& ptMouseDown,
							   const std::vector<ItemStatus>& rcItemList) const;

	/** 交换两个控件的位置，完成顺序调整
	* @param [in] pt 当前鼠标的位置
	* @param [in] rcItemList 子控件的列表
	* @param [in] nOldItemIndex 原来的子项索引号
	* @param [in] nNewItemIndex 最新的子项索引号
	* @return 如果有顺序调整，返回true；否则返回false
	*/
	virtual bool AdjustItemOrders(const UiPoint& pt,
								  const std::vector<ItemStatus>& rcItemList,
								  size_t& nOldItemIndex,
								  size_t& nNewItemIndex);

	/** 控件位置拖动完成事件
	* @param [in] nOldItemIndex 原来的子项索引号
	* @param [in] nNewItemIndex 最新的子项索引号
	*/
	virtual void OnItemOrdersChanged(size_t nOldItemIndex, size_t nNewItemIndex);

	/** 清除拖动状态，恢复原状态
	*/
	void ClearDragStatus();

    /** 将当前鼠标位置，转换到子项所在区域有效的范围内
    */
    void GetValidPointInItemRects(UiPoint& pt) const;

	/** @} */

private:
	/** @name 拖动相关的成员变量
	* @{ */

	/** 是否支持拖动改变列的顺序
	*/
	bool m_bEnableDragOrder;

	/** 是否鼠标左键按下
	*/
	bool m_bMouseDown;

	/** 是否处于拖拽操作中
	*/
	bool m_bInDragging;

	/** 拖动顺序时，控件的透明度
	*/
	uint8_t m_nDragAlpha;

	/** 原来的透明度
	*/
	uint8_t m_nOldAlpha;

	/** 鼠标按下时的鼠标位置
	*/
	UiPoint m_ptMouseDown;

	/** 鼠标按下时的控件矩形区域
	*/
	UiRect m_rcMouseDown;

	/** 鼠标按下时，父容器中，每个控件的位置
	*/
	std::vector<ItemStatus> m_rcItemList;

	/** @} */
};

template<typename T>
ControlDragableT<T>::ControlDragableT():
    m_bEnableDragOrder(true),
    m_bMouseDown(false),
    m_bInDragging(false),
    m_nDragAlpha(216),
    m_nOldAlpha(255)
{
    
}

template<typename T>
ControlDragableT<T>::~ControlDragableT()
{
}

template<typename T>
inline std::wstring ControlDragableT<T>::GetType() const { return DUI_CTR_CONTROL_DRAGABLE; }

template<>
inline std::wstring ControlDragableT<Box>::GetType() const { return DUI_CTR_BOX_DRAGABLE; }

template<>
inline std::wstring ControlDragableT<HBox>::GetType() const { return DUI_CTR_HBOX_DRAGABLE; }

template<>
inline std::wstring ControlDragableT<VBox>::GetType() const { return DUI_CTR_VBOX_DRAGABLE; }

template<typename T>
void ControlDragableT<T>::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"drag_order") {
		SetEnableDragOrder(strValue == L"true");
	}
	else if (strName == L"drag_order") {
		SetDragAlpha((uint8_t)_wtoi(strValue.c_str()));
	}
	else {
		__super::SetAttribute(strName, strValue);
	}
}

template<typename T>
void ControlDragableT<T>::SetEnableDragOrder(bool bEnable)
{
    m_bEnableDragOrder = bEnable;
}

template<typename T>
bool ControlDragableT<T>::IsEnableDragOrder() const
{
    return m_bEnableDragOrder;
}

template<typename T>
void ControlDragableT<T>::SetDragAlpha(uint8_t nAlpha)
{
    m_nDragAlpha = nAlpha;
}

template<typename T>
uint8_t ControlDragableT<T>::GetDragAlpha() const
{
    return m_nDragAlpha;
}

template<typename T>
bool ControlDragableT<T>::IsInDragging() const
{
    return m_bInDragging;
}

template<typename T>
bool ControlDragableT<T>::ButtonDown(const EventArgs& msg)
{
    m_bMouseDown = false;
    bool bRet = __super::ButtonDown(msg);
    if (!IsEnableDragOrder()) {
        //当前列为固定列，不允许调整顺序
        return bRet;
    }
    Box* pParent = this->GetParent();
    if (pParent == nullptr) {
        return bRet;
    }
    Layout* pLayout = pParent->GetLayout();
    if ((pLayout == nullptr) || (!pLayout->IsHLayout() && !pLayout->IsVLayout())) {
        return bRet;
    }
    UiPoint pt(msg.ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());

    m_bMouseDown = true;
    m_ptMouseDown = pt;
    m_rcMouseDown = this->GetRect();

    m_rcItemList.clear();
    size_t nItemCount = pParent->GetItemCount();
    for (size_t index = 0; index < nItemCount; ++index) {
        ItemStatus itemStatus;
        itemStatus.m_index = index;
        itemStatus.m_pItem = pParent->GetItemAt(index);
        if (itemStatus.m_pItem != nullptr) {
            itemStatus.m_rcPos = itemStatus.m_pItem->GetRect();
            itemStatus.m_nPaintOrder = itemStatus.m_pItem->GetPaintOrder();
            m_rcItemList.push_back(itemStatus);
        }
        if (itemStatus.m_pItem == this) {
            //当前控件，绘制顺序需要放最后，避免被其他控件遮挡
            itemStatus.m_pItem->SetPaintOrder(255);
            ASSERT(itemStatus.m_rcPos.ContainsPt(m_ptMouseDown));
        }
    }
    return bRet;
}

template<typename T>
bool ControlDragableT<T>::MouseMove(const EventArgs& msg)
{
    bool bRet = __super::MouseMove(msg);
    if (!m_bMouseDown) {
        return bRet;
    }
    Box* pParent = this->GetParent();
    if (pParent == nullptr) {
        return bRet;
    }
    Layout* pLayout = pParent->GetLayout();
    if ((pLayout == nullptr) || (!pLayout->IsHLayout() && !pLayout->IsVLayout())) {
        return bRet;
    }

    UiPoint pt(msg.ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());
    GetValidPointInItemRects(pt);
    if (pLayout->IsHLayout()) {
        int32_t xOffset = pt.x - m_ptMouseDown.x;
        if (std::abs(xOffset) < GlobalManager::Instance().Dpi().GetScaleInt(3)) {
            return bRet;
        }
        //调整其他控件的位置
        AdjustItemPos(pt, m_ptMouseDown, m_rcItemList);

        UiRect rect = m_rcMouseDown;
        rect.left += xOffset;
        rect.right += xOffset;
        this->SetPos(rect);
    }
    else {
        int32_t yOffset = pt.y - m_ptMouseDown.y;
        if (std::abs(yOffset) < GlobalManager::Instance().Dpi().GetScaleInt(3)) {
            return bRet;
        }
        //调整其他控件的位置
        AdjustItemPos(pt, m_ptMouseDown, m_rcItemList);

        UiRect rect = m_rcMouseDown;
        rect.top += yOffset;
        rect.bottom += yOffset;
        this->SetPos(rect);
    }

    if (!m_bInDragging) {
        m_bInDragging = true;
        m_nOldAlpha = (uint8_t)this->GetAlpha();
        //设置为半透明的效果
        this->SetAlpha(m_nDragAlpha);
    }
    return bRet;
}

template<typename T>
bool ControlDragableT<T>::ButtonUp(const EventArgs& msg)
{
    bool bRet = __super::ButtonUp(msg);
    if (!m_bInDragging) {
        //没有处于拖动改变列顺序的状态
        ClearDragStatus();
        return bRet;
    }
    UiPoint pt(msg.ptMouse);
    pt.Offset(this->GetScrollOffsetInScrollBox());
    GetValidPointInItemRects(pt);

    size_t nOldItemIndex = Box::InvalidIndex;
    size_t nNewItemIndex = Box::InvalidIndex;
    bool bOrderChanged = AdjustItemOrders(pt, m_rcItemList, nOldItemIndex, nNewItemIndex);
    ClearDragStatus();
    if (bOrderChanged) {
        //触发列交换事件
        OnItemOrdersChanged(nOldItemIndex, nNewItemIndex);
    }
    return bRet;
}

template<typename T>
void ControlDragableT<T>::GetValidPointInItemRects(UiPoint& pt) const
{
    Box* pParent = this->GetParent();
    if (pParent == nullptr) {
        return;
    }

    UiRect boxRect = pParent->GetRect();
    UiRect rcItemRects = boxRect;
    for (size_t nIndex = 0; nIndex < m_rcItemList.size(); ++nIndex) {
        const ItemStatus& itemStatus = m_rcItemList[nIndex];
        if ((itemStatus.m_pItem != nullptr) && !itemStatus.m_pItem->IsFloat()) {
            if (itemStatus.m_rcPos.left > rcItemRects.left) {
                rcItemRects.left = itemStatus.m_rcPos.left;
            }
            if (itemStatus.m_rcPos.top > rcItemRects.top) {
                rcItemRects.top = itemStatus.m_rcPos.top;
            }
            break;
        }
    }
    for (int32_t nIndex = (int32_t)m_rcItemList.size() - 1; nIndex >= 0; --nIndex) {
        const ItemStatus& itemStatus = m_rcItemList[nIndex];
        if ((itemStatus.m_pItem != nullptr) && !itemStatus.m_pItem->IsFloat()) {
            if (itemStatus.m_rcPos.right < rcItemRects.right) {
                rcItemRects.right = itemStatus.m_rcPos.right;
            }
            if (itemStatus.m_rcPos.bottom < rcItemRects.bottom) {
                rcItemRects.bottom = itemStatus.m_rcPos.bottom;
            }
            break;
        }
    }
    if (pt.x < rcItemRects.left) {
        pt.x = rcItemRects.left;
    }
    if (pt.x >= rcItemRects.right) {
        pt.x = rcItemRects.right - 1;
    }
    if (pt.y < rcItemRects.top) {
        pt.y = rcItemRects.top;
    }
    if (pt.y >= rcItemRects.bottom) {
        pt.y = rcItemRects.bottom - 1;
    }
}

template<typename T>
bool ControlDragableT<T>::OnWindowKillFocus(const EventArgs& msg)
{
    bool bRet = __super::OnWindowKillFocus(msg);
    ClearDragStatus();
    return bRet;
}

template<typename T>
void ControlDragableT<T>::AdjustItemPos(const UiPoint& pt, const UiPoint& ptMouseDown,
                                        const std::vector<ItemStatus>& rcItemList) const
{
    Control* pMouseItem = nullptr;
    size_t nMouseItemIndex = Box::InvalidIndex;
    size_t nMouseDownItemIndex = Box::InvalidIndex;
    for (const ItemStatus& itemStatus : rcItemList) {
        if (itemStatus.m_rcPos.ContainsPt(pt)) {
            pMouseItem = itemStatus.m_pItem;
            nMouseItemIndex = itemStatus.m_index;
        }
        if (itemStatus.m_pItem == this) {
            nMouseDownItemIndex = itemStatus.m_index;
        }
    }
    if ((pMouseItem == nullptr) ||
        (nMouseItemIndex == Box::InvalidIndex) ||
        (nMouseDownItemIndex == Box::InvalidIndex)) {
        return;
    }
    ControlDragableT<T>* pItem = dynamic_cast<ControlDragableT<T>*>(pMouseItem);
    if (pItem == nullptr) {
        //鼠标不在可拖动控件上
        return;
    }
    if (!pItem->IsEnableDragOrder()) {
        //当前列为固定，不允许调整顺序
        return;
    }

    const size_t itemCount = rcItemList.size();
    int32_t xOffset = pt.x - ptMouseDown.x;
    if (pMouseItem == this) {
        //当前鼠标位置：在自身的位置，恢复各个控件的实际位置
        for (const ItemStatus& item : rcItemList) {
            if (item.m_pItem == this) {
                continue;
            }
            if (item.m_pItem->GetRect() != item.m_rcPos) {
                item.m_pItem->SetPos(item.m_rcPos);
            }
        }
    }
    else if (xOffset < 0) {
        //当前鼠标位置：在按下点的左侧，向右侧移动控件
        for (size_t index = 0; index < itemCount; ++index) {
            const ItemStatus& item = rcItemList[index];
            if ((item.m_index >= nMouseItemIndex) && (item.m_index < nMouseDownItemIndex)) {
                //向右侧移动
                if ((index + 1) < itemCount) {
                    const ItemStatus& nextItem = rcItemList[index + 1];
                    item.m_pItem->SetPos(nextItem.m_rcPos);
                }
                else {
                    if (item.m_pItem->GetRect() != item.m_rcPos) {
                        item.m_pItem->SetPos(item.m_rcPos);
                    }
                }
            }
            else {
                //恢复原位置
                if (item.m_pItem->GetRect() != item.m_rcPos) {
                    item.m_pItem->SetPos(item.m_rcPos);
                }
            }
        }
    }
    else {
        //当前鼠标位置：在按下点的右侧，向左侧移动控件
        for (size_t index = 0; index < itemCount; ++index) {
            const ItemStatus& item = rcItemList[index];
            if ((item.m_index > nMouseDownItemIndex) && (item.m_index <= nMouseItemIndex)) {
                //向左侧移动
                if ((index - 1) < itemCount) {
                    const ItemStatus& nextItem = rcItemList[index - 1];
                    item.m_pItem->SetPos(nextItem.m_rcPos);
                }
                else {
                    if (item.m_pItem->GetRect() != item.m_rcPos) {
                        item.m_pItem->SetPos(item.m_rcPos);
                    }
                }
            }
            else {
                //恢复原位置
                if (item.m_pItem->GetRect() != item.m_rcPos) {
                    item.m_pItem->SetPos(item.m_rcPos);
                }
            }
        }
    }
}

template<typename T>
bool ControlDragableT<T>::AdjustItemOrders(const UiPoint& pt,
                                           const std::vector<ItemStatus>& rcItemList,
                                           size_t& nOldItemIndex,
                                           size_t& nNewItemIndex)
{
    nOldItemIndex = Box::InvalidIndex;
    nNewItemIndex = Box::InvalidIndex;
    Box* pParent = this->GetParent();
    if (pParent == nullptr) {
        return false;
    }

    bool bOrderChanged = false;
    const size_t itemCount = pParent->GetItemCount();
    size_t nMouseItemIndex = Box::InvalidIndex;
    size_t nCurrentItemIndex = Box::InvalidIndex;
    for (const ItemStatus& itemStatus : rcItemList) {
        if (itemStatus.m_rcPos.ContainsPt(pt)) {
            nMouseItemIndex = itemStatus.m_index;
            ControlDragableT<T>* pItem = dynamic_cast<ControlDragableT<T>*>(itemStatus.m_pItem);
            if ((pItem != nullptr) && !pItem->IsEnableDragOrder()) {
                //当前控件为固定，不允许调整顺序
                nMouseItemIndex = Box::InvalidIndex;
            }
        }
        if ((itemStatus.m_pItem == this) && !itemStatus.m_rcPos.ContainsPt(pt)) {
            nCurrentItemIndex = itemStatus.m_index;
        }
    }
    if ((nMouseItemIndex < itemCount) && (nCurrentItemIndex < itemCount)) {
        //调整控件的位置
        pParent->SetItemIndex(this, nMouseItemIndex);
        nOldItemIndex = nCurrentItemIndex;
        nNewItemIndex = nMouseItemIndex;
        bOrderChanged = true;
    }
    return bOrderChanged;
}

template<typename T>
void ControlDragableT<T>::OnItemOrdersChanged(size_t /*nOldItemIndex*/, size_t /*nNewItemIndex*/)
{
}

template<typename T>
void ControlDragableT<T>::ClearDragStatus()
{
    if (m_bInDragging) {
        this->SetAlpha(m_nOldAlpha);
        m_nOldAlpha = 255;
        m_bInDragging = false;
    }
    m_bMouseDown = false;
    if (!m_rcItemList.empty()) {
        //恢复子控件的绘制顺序
        for (const ItemStatus& itemStatus : m_rcItemList) {
            if ((itemStatus.m_pItem != nullptr) &&
                (itemStatus.m_nPaintOrder != itemStatus.m_pItem->GetPaintOrder())) {
                itemStatus.m_pItem->SetPaintOrder(itemStatus.m_nPaintOrder);
            }
        }
        m_rcItemList.clear();
        //对父控件重绘
        Control* pParent = this->GetParent();
        if (pParent != nullptr) {
            pParent->Invalidate();
            pParent->SetPos(pParent->GetPos());
        }
    }
}

typedef ControlDragableT<Control> ControlDragable;
typedef ControlDragableT<Box> BoxDragable;
typedef ControlDragableT<HBox> HBoxDragable;
typedef ControlDragableT<VBox> VBoxDragable;

}

#endif // UI_CORE_CONTROL_DRAGABLE_H_