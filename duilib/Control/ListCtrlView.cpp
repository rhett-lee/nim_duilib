#include "ListCtrlView.h" 
#include "duilib/Control/ListCtrl.h"
#include "duilib/Control/ListCtrlData.h"
#include "duilib/Core/Keyboard.h"

namespace ui
{
ListCtrlView::ListCtrlView(Window* pWindow, Layout* pLayout):
    VirtualListBox(pWindow, pLayout)
{
}

ListCtrlView::~ListCtrlView()
{
}

void ListCtrlView::SendEventMsg(const EventArgs& msg)
{
    BaseClass::SendEventMsg(msg);
    if ((msg.eventType == kEventSelect) || (msg.eventType == kEventUnSelect)) {
        SendEvent(kEventSelChange);
    }
}

size_t ListCtrlView::GetTopElementIndex() const
{
    std::vector<size_t> itemIndexList;
    GetDisplayElements(itemIndexList);
    if (itemIndexList.empty()) {
        return Box::InvalidIndex;
    }
    else {
        return itemIndexList.front();
    }
}

bool ListCtrlView::IsDataItemDisplay(size_t itemIndex) const
{
    std::vector<size_t> itemIndexList;
    GetDisplayElements(itemIndexList);
    return std::find(itemIndexList.begin(), itemIndexList.end(), itemIndex) != itemIndexList.end();
}

void ListCtrlView::GetDisplayDataItems(std::vector<size_t>& itemIndexList) const
{
    itemIndexList.clear();
    GetDisplayElements(itemIndexList);
}

bool ListCtrlView::EnsureDataItemVisible(size_t itemIndex, bool bToTop)
{
    EnsureVisible(itemIndex, bToTop);
    return true;
}

void ListCtrlView::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //禁止随鼠标滚轮的滚动改变选中项
    SetScrollSelect(false);
}

void ListCtrlView::HandleEvent(const EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            BaseClass::HandleEvent(msg);
        }
        return;
    }
    bool bHandled = false;
    if (msg.eventType == kEventKeyDown) {
        //跳过基类的功能开关，直接调用ListCtrl的快捷键处理函数
        bHandled = OnListCtrlKeyDown(msg);
    }
    if (!bHandled) {
        BaseClass::HandleEvent(msg);
    }
}

bool ListCtrlView::SelectItem(size_t iIndex, bool bTakeFocus, bool bTriggerEvent, uint64_t vkFlag)
{
    //跳过基类的IsSelectLikeListCtrl()功能开发, 直接使用ListCtrl的选择模式
    return ListCtrlSelectItem(iIndex, bTakeFocus, bTriggerEvent, vkFlag);
}

}//namespace ui

