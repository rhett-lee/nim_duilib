#ifndef UI_CONTROL_LIST_CTRL_ICON_H_
#define UI_CONTROL_LIST_CTRL_ICON_H_

#include "duilib/Control/CheckBox.h"
#include "duilib/Box/HBox.h"

namespace ui
{
/** 列表中使用的图标控件，用于显示图标
*/
class ListCtrlIcon: public CheckBoxTemplate<HBox>
{
    typedef CheckBoxTemplate<HBox> BaseClass;
public:
    explicit ListCtrlIcon(Window* pWindow);

    /** 获取控件类型
    */
    virtual DString GetType() const override;

    /** 消息处理
    */
    virtual void HandleEvent(const EventArgs& msg) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

public:
    /** 设置关联的列表项
    */
    void SetListBoxItem(Control* pListBoxItem);

private:
    /** 鼠标事件(来源于子控件的消息转发)
    * @param [in] msg 鼠标事件的内容
    */
    bool OnMouseEvent(const EventArgs& msg);

private:
    /** 关联的列表项
    */
    Control* m_pListBoxItem;
};

}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_ICON_H_
