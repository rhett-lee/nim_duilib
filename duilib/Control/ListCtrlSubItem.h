#ifndef UI_CONTROL_LIST_CTRL_SUB_ITEM_H_
#define UI_CONTROL_LIST_CTRL_SUB_ITEM_H_

#pragma once

#include "duilib/Box/ListBox.h"
#include "duilib/Control/Label.h"

namespace ui
{
/** 列表项的子项
*/
class ListCtrlItem;
class ListCtrlCheckBox;
class ListCtrlSubItem : public LabelBox
{
public:
    ListCtrlSubItem();

    /** 设置关联的Item接口
    */
    void SetListCtrlItem(ListCtrlItem* pItem);

    /** 获取关联的Item接口
    */
    ListCtrlItem* GetListCtrlItem() const;

    /** 设置是否显示CheckBox
    * @param [in] bVisible true表示显示，false表示隐藏
    */
    bool SetCheckBoxVisible(bool bVisible);

    /** 判断当前CheckBox是否处于显示状态
    @return 返回true表示CheckBox存在，并且可见； 如果不含CheckBox，返回false
    */
    bool IsCheckBoxVisible() const;

    /** 设置CheckBox的勾选状态
    * @param [in] bSelected true表示勾选，false表示不勾选
    * @param [in] 如果bSelected和bPartSelect同时为true，表示部分选择
    */
    bool SetCheckBoxSelect(bool bSelected, bool bPartSelect);

    /** 获取CheckBox的勾选状态
    * @param [out] bSelected true表示勾选，false表示不勾选
    * @param [out] 如果bSelected和bPartSelect同时为true，表示部分选择
    */
    bool GetCheckBoxSelect(bool& bSelected, bool& bPartSelect) const;

    /** 获取CheckBox接口
    */
    ListCtrlCheckBox* GetCheckBox() const;

private:
    /** 关联的Item接口
    */
    ListCtrlItem* m_pItem;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_SUB_ITEM_H_
