#ifndef UI_CONTROL_COLOR_CONTROL_H_
#define UI_CONTROL_COLOR_CONTROL_H_

#pragma once

#include "duilib/Core/Control.h"
#include "duilib/Render/IRender.h"

namespace ui
{
/** 自定义颜色控件
*/
class ColorControl: public Control
{
public:
	ColorControl();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;

	/** 选择颜色
	*/
	void SelectColor(const UiColor& selColor);

	/** 监听选择颜色的事件
	* @param[in] callback 选择颜色变化时的回调函数
	*            参数说明:
						wParam: 当前新选择的颜色值，可以用UiColor((uint32_t)wParam)生成颜色
						lParam: 原来旧选择的颜色值，可以用UiColor((uint32_t)lParam)生成颜色
	*/
	void AttachSelectColor(const EventCallback& callback) { AttachEvent(kEventSelectColor, callback); }

protected:
	/** 绘制背景图片的入口函数
	* @param[in] pRender 指定绘制区域
	*/
	virtual void PaintBkImage(IRender* pRender) override;

	/** 鼠标左键按下
	*/
	virtual bool ButtonDown(const EventArgs& msg) override;

	/** 鼠标移动
	*/
	virtual bool MouseMove(const EventArgs& msg) override;

	/** 鼠标左键弹起
	*/
	virtual bool ButtonUp(const EventArgs& msg) override;

private:
	/** 获取绘制的颜色位图接口
	* @param [in] rect 显示区域大小信息
	*/
	IBitmap* GetColorBitmap(const UiRect& rect);

	/** 选择位置发生变化
	*/
	void OnSelectPosChanged(const UiRect& rect, const UiPoint& pt);

	/** 设置鼠标捕获
	*/
	void SetMouseCapture(bool bCapture);

private:
	/** 颜色位图
	*/
	std::unique_ptr<IBitmap> m_spBitmap;

	/** 鼠标点击位置
	*/
	UiPoint m_lastPt;

	/** 鼠标是否按下
	*/
	bool m_bMouseDown;
};

}//namespace ui

#endif //UI_CONTROL_COLOR_CONTROL_H_
