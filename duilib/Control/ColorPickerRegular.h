#ifndef UI_CONTROL_COLORPICKER_REGULAR_H_
#define UI_CONTROL_COLORPICKER_REGULAR_H_

#pragma once

#include "duilib/Box/VirtualListBox.h"

namespace ui
{
/** 颜色选择器：常用颜色
*/
class ColorPickerRegularProvider;
class ColorPickerRegular : public VirtualVTileListBox
{
public:
    ColorPickerRegular();

    /** 获取控件类型
    */
    virtual std::wstring GetType() const override;

    /** 选择一个颜色
    */
    void SelectColor(const UiColor& color);

    /** 获取当前选择的颜色
    */
    UiColor GetSelectedColor() const;

    /** 监听选择颜色的事件
    * @param[in] callback 选择颜色变化时的回调函数
    *            参数说明:
                        wParam: 当前新选择的颜色值，可以用UiColor((uint32_t)wParam)生成颜色
                        lParam: 原来旧选择的颜色值，可以用UiColor((uint32_t)lParam)生成颜色
    */
    void AttachSelectColor(const EventCallback& callback) { AttachEvent(kEventSelectColor, callback); }

    /** 设置控件位置
    */
    virtual void SetPos(UiRect rc) override;

    /** 设置显示列数
    */
    void SetColumns(int32_t nColumns);

    /** 设置属性
    */
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

private:
    /** 常用颜色数据提供者
    */
    std::unique_ptr<ColorPickerRegularProvider> m_regularColors;

    /** 原来设置的元素控件大小
    */
    UiSize m_szItem;
};

}//namespace ui

#endif //UI_CONTROL_COLORPICKER_REGULAR_H_
