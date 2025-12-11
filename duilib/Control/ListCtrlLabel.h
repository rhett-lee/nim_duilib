#ifndef UI_CONTROL_LIST_CTRL_LABEL_H_
#define UI_CONTROL_LIST_CTRL_LABEL_H_

#include "duilib/Control/CheckBox.h"
#include "duilib/Box/HBox.h"

namespace ui
{
/** 列表中使用的Label控件，用于显示文本，并提供文本编辑功能的事件
*/
class ListCtrlLabel: public CheckBoxTemplate<HBox>
{
    typedef CheckBoxTemplate<HBox> BaseClass;
public:
    explicit ListCtrlLabel(Window* pWindow);

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
    /** 设置文本所在位置的矩形区域
    */
    void SetTextRect(const UiRect& rect);

    /** 获取文本所在位置的矩形区域
    */
    UiRect GetTextRect() const;

    /** 设置关联的列表项
    */
    void SetListBoxItem(Control* pListBoxItem);

    /** 设置是否支持文本编辑
    */
    void SetEnableEdit(bool bEnableEdit);

    /** 获取是否支持文本编辑
    */
    bool IsEnableEdit() const;

private:
    /** 鼠标事件(来源于子控件的消息转发)
    * @param [in] msg 鼠标事件的内容
    */
    bool OnMouseEvent(const EventArgs& msg);

    /** 进入编辑状态
    */
    void OnItemEnterEditMode();

private:
    /** 关联的列表项
    */
    Control* m_pListBoxItem;

    /** 文本所在位置的矩形区域
    */
    UiRect m_textRect;

    /** 是否鼠标点击在控件范围内
    */
    bool m_bMouseDown;

    /** 是否支持文本编辑
    */
    bool m_bEnableEdit;
};
}//namespace ui

#endif //UI_CONTROL_LIST_CTRL_LABEL_H_
