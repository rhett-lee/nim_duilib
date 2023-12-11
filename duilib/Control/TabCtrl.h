#ifndef UI_CONTROL_TABCTRL_H_
#define UI_CONTROL_TABCTRL_H_

#pragma once

#include "duilib/Box/ListBox.h"

namespace ui
{
/** 多标签控件（类似浏览器的多标签）
*/
class TabCtrl: public ListBox
{
public:
	TabCtrl();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

protected:
	/** 初始化接口
	*/
	virtual void DoInit() override;

private:
	//初始化标志
	bool m_bInited;
};

/** 多标签控件的一个标签页
*/
class TabCtrlItem : public ListBoxItemH
{
public:
	TabCtrlItem();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual void SetVisible(bool bVisible) override;

public:
	/** 设置图标
	* @param [in] iconImageString 图标资源字符串
	*/
	void SetIcon(const std::wstring& iconImageString);

	/** 获取图标
	* @return 返回图标资源字符串
	*/
	std::wstring GetIcon() const;

	/** 设置文字内容
	*/
	void SetTitle(const std::wstring& title);

	/** 获取文字内容
	*/
	std::wstring GetTitle() const;

public:
	/** 获取图标控件
	*/
	Control* GetIconControl() const { return m_pIcon; }

	/** 获取文本控件
	*/
	Label* GetTextLabel() const { return m_pLabel; }

	/** 获取关闭按钮
	*/
	Button* GetCloseButton() const { return m_pCloseBtn; }

public:
	/** 图标控件的Class
	*/
	void SetIconClass(const std::wstring& iconClass);
	std::wstring GetIconClass() const;

	/** 文本控件的Class
	*/
	void SetTitleClass(const std::wstring& titleClass);
	std::wstring GetTitleClass() const;

	/** 关闭按钮控件的Class
	*/
	void SetCloseButtonClass(const std::wstring& closeButtonClass);
	std::wstring GetCloseButtonClass() const;

public:
	/** 设置选择状态的标签圆角大小
	* @param [in] szCorner 圆角的宽度和高度
	* @param [in] bNeedDpiScale 是否根据 DPI 自适应，默认为 true
	*/
	void SetSelectedRoundCorner(UiSize szCorner, bool bNeedDpiScale);

	/** 获取选择状态的标签圆角大小
	*/
	UiSize GetSelectedRoundCorner() const;

	/** 设置悬停状态的标签圆角大小
	* @param [in] szCorner 圆角的宽度和高度
	* @param [in] bNeedDpiScale 是否根据 DPI 自适应，默认为 true
	*/
	void SetHotRoundCorner(UiSize szCorner, bool bNeedDpiScale);

	/** 获取悬停状态的标签圆角大小
	*/
	UiSize GetHotRoundCorner() const;

	/** 设置Hot状态的背景色的内边距
	* @param [in] rcPadding 内边距数据
	* @param [in] bNeedDpiScale 是否根据 DPI 自适应，默认为 true
	*/
	void SetHotPadding(UiPadding rcPadding, bool bNeedDpiScale);

	/** 获取Hot状态的背景色的内边距
	*/
	UiPadding GetHotPadding() const;

	/** 设置关闭按钮是否自动隐藏
	* @param [in] bAutoHideCloseBtn true表示自动隐藏关闭按钮，false表示关闭按钮始终显示
	*/
	void SetAutoHideCloseButton(bool bAutoHideCloseBtn);

	/** 获取关闭按钮是否自动隐藏
	*/
	bool IsAutoHideCloseButton() const;

protected:
	/** 是否已经完成初始化
	*/
	bool IsInited() const;

	/** 初始化接口
	*/
	virtual void DoInit() override;

	/** 处理鼠标移入消息
	*/
	virtual bool MouseEnter(const EventArgs& msg) override;

	/** 处理鼠标移出消息
	*/
	virtual bool MouseLeave(const EventArgs& msg) override;

	/** 鼠标左键按下消息
	*/
	virtual bool ButtonDown(const EventArgs& msg) override;

	/** 绘制控件状态颜色的函数
	*/
	virtual void PaintStateColors(IRender* pRender) override;

	/** 选择状态变化事件(m_bSelected变量发生变化)
	*/
	virtual void OnPrivateSetSelected() override;

	/** 绘制标签页(选择状态)
	*/
	virtual void PaintTabItemSelected(IRender* pRender) ;

	/** 绘制标签页(悬停状态)
	*/
	virtual void PaintTabItemHot(IRender* pRender);

	/** 填充路径, 形成圆角矩形
	*/
	void AddTabItemPath(IPath* path, const UiRect& rect, UiSize roundSize) const;

private:
	/** 选择标签的圆角大小
	*/
	struct RoundCorner
	{
		uint8_t cx; //圆角的宽度
		uint8_t cy; //圆角的高度
	};

	/** Hot标签的状态Padding值
	*/
	struct HotPadding
	{
		uint8_t left;
		uint8_t top;
		uint8_t right;
		uint8_t bottom;
	};

	/** 选择状态的标签圆角大小
	*/
	RoundCorner m_rcSelected;

	/** 悬停状态的标签圆角大小
	*/
	RoundCorner m_rcHot;

	/** Hot标签的状态Padding值
	*/
	HotPadding m_hotPadding;

	/** 初始化标志
	*/
	bool m_bInited;

	/** 关闭按钮是否自动隐藏
	*/
	bool m_bAutoHideCloseBtn;

	/** 图标控件
	*/
	Control* m_pIcon;

	/** 文本控件
	*/
	Label* m_pLabel;

	/** 关闭按钮
	*/
	Button* m_pCloseBtn;

	/** 图标控件的Class
	*/
	UiString m_iconClass;

	/** 文本控件的Class
	*/
	UiString m_titleClass;

	/** 关闭按钮控件的Class
	*/
	UiString m_closeBtnClass;

	/** 文本内容
	*/
	UiString m_title;

	/** 图标资源字符串
	*/
	UiString m_iconImageString;
};

}//namespace ui

#endif //UI_CONTROL_TABCTRL_H_
