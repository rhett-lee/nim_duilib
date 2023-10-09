#ifndef UI_CONTROL_COLORPICKER_H_
#define UI_CONTROL_COLORPICKER_H_

#pragma once

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Control/Label.h"

namespace ui
{

/** 拾色器，独立窗口
*/
class Control;
class ColorPickerRegular;
class ColorPickerStatard;
class ColorPickerStatardGray;
class ColorPickerCustom;
class ColorPicker : public WindowImplBase
{
public:
	ColorPicker();
	virtual ~ColorPicker();

	/** 以下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
	 * GetSkinFolder		接口设置你要绘制的窗口皮肤资源路径
	 * GetSkinFile			接口设置你要绘制的窗口的 xml 描述文件
	 * GetWindowClassName	接口设置窗口唯一的类名称
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;

	/** 窗口类名
	*/
	static const std::wstring kClassName;

public:
	/** 设置所选颜色
	*/
	void SetSelectedColor(const UiColor& color);

	/** 获取窗口关闭时最终选择的颜色
	*/
	UiColor GetSelectedColor() const;

	/** 监听选择颜色的事件
	* @param[in] callback 选择颜色变化时的回调函数
	*            参数说明:
						wParam: 当前新选择的颜色值，可以用UiColor((uint32_t)wParam)生成颜色
						lParam: 原来旧选择的颜色值，可以用UiColor((uint32_t)lParam)生成颜色
	*/
	void AttachSelectColor(const EventCallback& callback);

	/** 监听窗口关闭事件
	 * @param[in] callback 指定关闭后的回调函数, 在这个回调中，可以调用GetSelectedColor()函数获取选择的颜色值
	                       参数的wParam代表窗口关闭的触发情况：
	 *                     0 - 表示 "确认" 关闭本窗口
						   1 - 表示点击窗口的 "关闭" 按钮关闭本窗口(默认值)
				           2 - 表示 "取消" 关闭本窗口
	 */
	void AttachWindowClose(const EventCallback& callback);

private:
	/** 收到 WM_CREATE 消息时该函数会被调用，通常做一些控件初始化的操作
	*/
	virtual void OnInitWindow() override;

	/** 收到 WM_CLOSE 消息时该函数会被调用
	 */
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

	/** 内部选择了新颜色
	*/
	void OnSelectColor(const UiColor& newColor);

	/** 屏幕取色
	*/
	void OnPickColorFromScreen();

private:
	/** 新选择的颜色控件接口
	*/
	Label* m_pNewColor;

	/** 原来的颜色控件接口
	*/
	Label* m_pOldColor;

	/** 常用颜色控件接口
	*/
	ColorPickerRegular* m_pRegularPicker;

	/** 标准颜色控件接口
	*/
	ColorPickerStatard* m_pStatardPicker;

	/** 标准颜色控件接口(灰色)
	*/
	ColorPickerStatardGray* m_pStatardGrayPicker;

	/** 自定义颜色控件接口
	*/
	ColorPickerCustom* m_pCustomPicker;

	/** 选择的颜色
	*/
	UiColor m_selectedColor;

	/** 选择颜色的监听事件
	*/
	EventCallback m_colorCallback;
};

} // namespace ui

#endif //UI_CONTROL_COLORPICKER_H_
