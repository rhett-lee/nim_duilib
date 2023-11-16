#ifndef UI_CONTROL_LIST_CTRL_ITEM_H_
#define UI_CONTROL_LIST_CTRL_ITEM_H_

#pragma once

#include "duilib/Box/ListBox.h"
#include "duilib/Control/Label.h"

namespace ui
{
/** ListCtrl列表数据项UI控件（行）
*    基本结构: <ListCtrlItem> <ListCtrlSubItem/> ... <ListCtrlSubItem/>  </ListCtrlItem>
     附加说明: 1. ListCtrlItem 是HBox的子类;   
              2. 每一列，放置一个ListCtrlSubItem控件
              3. ListCtrlSubItem 是LabelBox的子类
*/
class ListCtrl;
class ListCtrlSubItem;
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

    /** 设置是否可以选择(置顶数据不可选择，其他数据可选择)
    */
    void SetSelectableType(bool bSelectable);

public:
    /** 设置关联的ListCtrl接口
    */
    void SetListCtrl(ListCtrl* pListCtrl);

    /** 获取关联的ListCtrl接口
    */
    ListCtrl* GetListCtrl() const;

    /** 获取子控件的个数
    */
    size_t GetSubItemCount() const;

    /** 获取第columnIndex个子控件
    * @param [in] columnIndex 列索引序号：[0, GetSubItemCount())
    */
    ListCtrlSubItem* GetSubItem(size_t columnIndex) const;

    /** 设置是否在行首显示CheckBox
    * @param [in] bShow true表示在行首显示CheckBox，false表示不显示
    */
    bool SetShowCheckBox(bool bShow);

    /** 判断是否行首显示了CheckBox
    */
    bool IsShowCheckBox() const;

protected:
    virtual void Activate();
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool ButtonDoubleClick(const EventArgs& msg) override;
    virtual bool RButtonDown(const EventArgs& msg) override;
    virtual bool RButtonUp(const EventArgs& msg) override;
    virtual bool RButtonDoubleClick(const EventArgs& msg) override;

    /** 是否支持勾选模式（目前是TreeView/ListCtrl在使用这个模式）
        勾选模式是指：
        （1）只有点击在CheckBox图片上的时候，勾选框图片才是选择状态（非勾选模式下，是点击在控件矩形内就选择）
        （2）勾选状态和选择状态分离，是两个不同的状态
    */
    virtual bool SupportCheckedMode() const override;

protected:
    /** 执行选择功能
    * @param [in] vkFlag 按键标志, 取值范围参见 enum VKFlag 的定义
    */
    void SelectItem(uint64_t vkFlag);

private:
    /** 是否可以选择（影响方向键切换选择项）
    */
    bool m_bSelectable;

    /** 关联的ListCtrl接口
    */
    ListCtrl* m_pListCtrl;

    /** 设置的CheckBox宽度值
    */
    int32_t m_nCheckBoxPadding;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_ITEM_H_
