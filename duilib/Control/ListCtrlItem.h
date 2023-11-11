#ifndef UI_CONTROL_LIST_CTRL_ITEM_H_
#define UI_CONTROL_LIST_CTRL_ITEM_H_

#pragma once

#include "duilib/Box/ListBox.h"

namespace ui
{
/** ListCtrl列表数据项UI控件（行）
*/
class ListCtrlItem : public ListBoxItemH
{
public:
    ListCtrlItem();
    virtual ~ListCtrlItem();

    /** 获取控件类型
    */
    virtual std::wstring GetType() const override;

    /** 判断控件类型是否为可选择的
     * @return 默认返回false
     */
    virtual bool IsSelectableType() const override;

    /** 设置控件是否选择状态
     * @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
     * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。默认为 false
     */
    virtual void Selected(bool bSelect, bool bTriggerEvent) override;

    /** 设置是否可以选择
    */
    void SetSelectableType(bool bSelectable);

protected:
    virtual void Activate();
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool ButtonDoubleClick(const EventArgs& msg) override;
    virtual bool RButtonDown(const EventArgs& msg) override;
    virtual bool RButtonUp(const EventArgs& msg) override;
    virtual bool RButtonDoubleClick(const EventArgs& msg) override;

protected:
    /** 执行选择功能
    * @param [in] vkFlag 按键标志, 取值范围参见 enum VKFlag 的定义
    */
    void SelectItem(uint64_t vkFlag);

private:
    /** 是否可以选择（影响方向键切换选择项）
    */
    bool m_bSelectable;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_ITEM_H_
