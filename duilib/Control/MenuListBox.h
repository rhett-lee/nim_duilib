#ifndef UI_BOX_MENU_LIST_BOX_H_
#define UI_BOX_MENU_LIST_BOX_H_

#include "duilib/Box/ListBox.h"

namespace ui
{

/** 用在菜单中的ListBox控件
*/
class UILIB_API MenuListBox : public VListBox
{
    typedef VListBox BaseClass;
public:
    explicit MenuListBox(Window* pWindow) :
        VListBox(pWindow)
    {
    }

    virtual DString GetType() const override { return DUI_CTR_MENU_LISTBOX; }

    /** 计算本页里面显示几个子项
    * @param [in] bIsHorizontal 当前布局是否为水平布局
    * @param [out] nColumns 返回列数
    * @param [out] nRows 返回行数
    * @return 返回可视区域显示的记录数
    */
    virtual size_t GetDisplayItemCount(bool bIsHorizontal, size_t& nColumns, size_t& nRows) const override
    {
        size_t nCount = 0;
        nRows = 0;
        nColumns = 1;
        if (bIsHorizontal) {
            //目前没有这种情况: 已经固定纵向布局
            nCount = BaseClass::GetDisplayItemCount(bIsHorizontal, nColumns, nRows);
            ASSERT(0);
        }
        else {
            const size_t nItemCount = GetItemCount();
            for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
                Control* pControl = GetItemAt(nItemIndex);
                if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
                    continue;
                }
                ++nCount;
            }
        }
        nRows = nCount;
        return nCount;
    }

    /** 响应KeyDown消息
    * @return 返回true表示成功处理，返回false表示未处理此消息
    */
    virtual bool OnListBoxKeyDown(const EventArgs& msg) override
    {
        ASSERT(msg.eventType == kEventKeyDown);
        bool bHandled = false;
        bool bArrowKeyDown = (msg.eventType == kEventKeyDown) && ((msg.vkCode == kVK_UP) || (msg.vkCode == kVK_DOWN));
        if (!bArrowKeyDown || (GetItemCount() == 0) || IsMultiSelect() || IsHorizontalScrollBar()) {
            return BaseClass::OnListBoxKeyDown(msg);
        }
        //上下键盘切换菜单项的支持
        bHandled = true;
        switch (msg.vkCode) {
        case kVK_UP:
            //向上1行
            {
                size_t nColumns = 0;
                size_t nRows = 0;
                GetDisplayItemCount(false, nColumns, nRows);
                size_t nCurSel = GetCurSel();
                if (nCurSel >= GetItemCount()) {
                    //无选中时，选中最后一条
                    SelectItem(GetItemCount() - 1);
                }
                else if (nCurSel == 0) {
                    //选中第一条时，选中最后一条，循环选择
                    SelectItem(GetItemCount() - 1);
                }
                else if ((nCurSel < GetItemCount()) && IsSelectableItem(nCurSel) && (GetItemCountBefore(nCurSel) >= nColumns)) {
                    //可以向上滚动1行
                    SelectItemCountN(true, true, false, nColumns);
                }
            }
            break;
        case kVK_DOWN:
            //向下1行
            {
                size_t nColumns = 0;
                size_t nRows = 0;
                GetDisplayItemCount(false, nColumns, nRows);
                size_t nCurSel = GetCurSel();
                if (nCurSel >= GetItemCount()) {
                    //无选中时，选中第一条
                    SelectItem(0);
                }
                else if (nCurSel == (GetItemCount() - 1)) {
                    //选中最后一条时，选中第一条，循环选择
                    SelectItem(0);
                }
                else if ((nCurSel < GetItemCount()) && IsSelectableItem(nCurSel) && (GetItemCountAfter(nCurSel) >= nColumns)) {
                    SelectItemCountN(true, true, true, nColumns);
                }
            }
            break;
        default:
            break;
        }
        return bHandled;
    }
};

}
#endif // UI_BOX_MENU_LIST_BOX_H_
