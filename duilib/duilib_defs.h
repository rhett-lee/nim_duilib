// Copyright (c) 2010-2011, duilib develop team(www.duilib.com).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or 
// without modification, are permitted provided that the 
// following conditions are met.
//
// Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above 
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials 
// provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef DUILIB_DEFS_H_
#define DUILIB_DEFS_H_ 1

#pragma once

#include "duilib/duilib_config.h"

#define DUI_NOSET_VALUE		-1

namespace ui
{
	//定义所有控件类型
	#define  DUI_CTR_CONTROL                         (L"Control")
	#define  DUI_CTR_CONTROL_DRAGABLE                (L"ControlDragable")
	#define  DUI_CTR_LABEL                           (L"Label")
	#define  DUI_CTR_RICHTEXT                        (L"RichText")
	#define  DUI_CTR_HYPER_LINK                      (L"HyperLink")
	#define  DUI_CTR_BUTTON                          (L"Button")
	#define  DUI_CTR_OPTION                          (L"Option")
	#define  DUI_CTR_CHECKBOX                        (L"CheckBox")
	#define  DUI_CTR_SPLIT							 (L"Split")

	#define  DUI_CTR_LABELBOX                        (L"LabelBox")
	#define  DUI_CTR_BUTTONBOX                       (L"ButtonBox")
	#define  DUI_CTR_OPTIONBOX                       (L"OptionBox")
	#define  DUI_CTR_CHECKBOXBOX                     (L"CheckBoxBox")
	#define  DUI_CTR_SPLITBOX					     (L"SplitBox")

	#define  DUI_CTR_BOX							 (L"Box")
	#define  DUI_CTR_HBOX							 (L"HBox")
	#define  DUI_CTR_VBOX                            (L"VBox")
	#define  DUI_CTR_VTILE_BOX					     (L"VTileBox")
	#define  DUI_CTR_HTILE_BOX					     (L"HTileBox")

	#define  DUI_CTR_BOX_DRAGABLE					 (L"BoxDragable")
	#define  DUI_CTR_HBOX_DRAGABLE				     (L"HBoxDragable")
	#define  DUI_CTR_VBOX_DRAGABLE                   (L"VBoxDragable")

	#define  DUI_CTR_GROUP_BOX                       (L"GroupBox")
	#define  DUI_CTR_GROUP_HBOX                      (L"GroupHBox")
	#define  DUI_CTR_GROUP_VBOX                      (L"GroupVBox")

	#define  DUI_CTR_SCROLLBOX						 (L"ScrollBox")
	#define  DUI_CTR_HSCROLLBOX						 (L"HScrollBox")
	#define  DUI_CTR_VSCROLLBOX						 (L"VScrollBox")
	#define  DUI_CTR_HTILE_SCROLLBOX				 (L"HTileScrollBox")
	#define  DUI_CTR_VTILE_SCROLLBOX				 (L"VTileScrollBox")

	#define  DUI_CTR_LISTBOX_ITEM					 (L"ListBoxItem")
	#define  DUI_CTR_HLISTBOX						 (L"HListBox")
	#define  DUI_CTR_VLISTBOX                        (L"VListBox")
	#define  DUI_CTR_HTILE_LISTBOX                   (L"HTileListBox")
	#define  DUI_CTR_VTILE_LISTBOX                   (L"VTileListBox")

	#define  DUI_CTR_LISTCTRL						 (L"ListCtrl")
	#define  DUI_CTR_PROPERTY_GRID					 (L"PropertyGrid")

	#define  DUI_CTR_VIRTUAL_VLISTBOX				 (L"VirtualVListBox")
	#define  DUI_CTR_VIRTUAL_HLISTBOX				 (L"VirtualHListBox")
	#define  DUI_CTR_VIRTUAL_HTILE_LISTBOX			 (L"VirtualHTileListBox")
	#define  DUI_CTR_VIRTUAL_VTILE_LISTBOX			 (L"VirtualVTileListBox")

	#define  DUI_CTR_TABBOX                          (L"TabBox")

	#define  DUI_CTR_TREENODE                        (L"TreeNode")
	#define  DUI_CTR_TREEVIEW                        (L"TreeView")

	#define  DUI_CTR_RICHEDIT                        (L"RichEdit")
	#define  DUI_CTR_COMBO                           (L"Combo")
	#define  DUI_CTR_COMBO_BUTTON                    (L"ComboButton")
	#define  DUI_CTR_FILTER_COMBO                    (L"FilterCombo")
	#define  DUI_CTR_CHECK_COMBO                     (L"CheckCombo")
	#define  DUI_CTR_SLIDER                          (L"Slider")
	#define  DUI_CTR_PROGRESS                        (L"Progress")
	#define  DUI_CTR_CIRCLEPROGRESS                  (L"CircleProgress")
	#define  DUI_CTR_SCROLLBAR                       (L"ScrollBar")

	#define  DUI_CTR_MENU							 (L"Menu")
	#define  DUI_CTR_SUB_MENU				   		 (L"SubMenu")
	#define  DUI_CTR_MENUELEMENT					 (L"MenuElement")

	#define  DUI_CTR_DATETIME                        (L"DateTime")

	//颜色相关的部分控件
	#define  DUI_CTR_COLOR_CONTROL					 (L"ColorControl")
	#define  DUI_CTR_COLOR_SLIDER					 (L"ColorSlider")
	#define  DUI_CTR_COLOR_PICKER_REGULAR			 (L"ColorPickerRegular")
	#define  DUI_CTR_COLOR_PICKER_STANDARD			 (L"ColorPickerStatard")
	#define  DUI_CTR_COLOR_PICKER_STANDARD_GRAY		 (L"ColorPickerStatardGray")
	#define  DUI_CTR_COLOR_PICKER_CUSTOM			 (L"ColorPickerCustom")

	#define  DUI_CTR_LINE							 (L"Line")
	#define  DUI_CTR_IPADDRESS						 (L"IPAddress")
	#define  DUI_CTR_HOTKEY							 (L"HotKey")	
	#define	 DUI_CTR_TAB_CTRL						 (L"TabCtrl")
	#define	 DUI_CTR_TAB_CTRL_ITEM					 (L"TabCtrlItem")

	// 窗口标题栏按钮：最大化、最小化、关闭、还原、全屏窗口的名字，代码中写死的
	#define DUI_CTR_CAPTION_BAR						 (L"window_caption_bar")
	#define DUI_CTR_BUTTON_CLOSE					 (L"closebtn")
	#define DUI_CTR_BUTTON_MIN						 (L"minbtn")
	#define DUI_CTR_BUTTON_MAX						 (L"maxbtn")
	#define DUI_CTR_BUTTON_RESTORE					 (L"restorebtn")
	#define DUI_CTR_BUTTON_FULLSCREEN				 (L"fullscreenbtn")

	//EventType事件类型对应的字符串定义
	#define	EVENTSTR_ALL				(L"all")
	#define	EVENTSTR_KEYDOWN			(L"keydown")
	#define	EVENTSTR_KEYUP				(L"keyup")
	#define	EVENTSTR_CHAR				(L"char")
	
	#define EVENTSTR_SETFOCUS			(L"setfocus")
	#define EVENTSTR_KILLFOCUS			(L"killfocus")
	#define	EVENTSTR_SETCURSOR			(L"setcursor")

	#define EVENTSTR_MOUSEMOVE			(L"mousemove")
	#define	EVENTSTR_MOUSEENTER			(L"mouseenter")
	#define	EVENTSTR_MOUSELEAVE			(L"mouseleave")
	#define	EVENTSTR_MOUSEHOVER			(L"mousehover")

	#define	EVENTSTR_BUTTONDOWN			(L"buttondown")
	#define	EVENTSTR_BUTTONUP			(L"buttonup")
	#define	EVENTSTR_DOUBLECLICK		(L"doubleclick")
	#define	EVENTSTR_RBUTTONDOWN		(L"rbuttondown")
	#define	EVENTSTR_RBUTTONUP			(L"rbuttonup")
	#define	EVENTSTR_RDOUBLECLICK		(L"rdoubleclick")

	#define EVENTSTR_SELECT				(L"select")
	#define EVENTSTR_UNSELECT			(L"unselect")

	#define EVENTSTR_CHECKED			(L"checked")
	#define EVENTSTR_UNCHECK			(L"uncheck")
	
	#define EVENTSTR_TAB_SELECT			(L"tab_select")

	#define	EVENTSTR_MENU				(L"menu")

	#define EVENTSTR_EXPAND				(L"expand")
	#define EVENTSTR_COLLAPSE			(L"collapse")

	#define	EVENTSTR_SCROLLWHEEL		(L"scrollwheel")
	#define EVENTSTR_SCROLLCHANGE		(L"scrollchange")

	#define EVENTSTR_VALUECHANGE		(L"valuechange")
	#define EVENTSTR_RETURN				(L"return")
	#define EVENTSTR_TAB				(L"tab")
	#define EVENTSTR_WINDOWCLOSE		(L"windowclose")

	//水平对齐方式
	enum HorAlignType
	{
		kHorAlignLeft,		// 靠左对齐
		kHorAlignCenter,	// 水平居中
		kHorAlignRight,		// 靠右对齐
	};

	//垂直对齐方式
	enum VerAlignType
	{
		kVerAlignTop,		// 顶端对齐
		kVerAlignCenter,	// 垂直居中
		kVerAlignBottom,	// 底部对齐
	};

	//控件图片类型
	enum StateImageType
	{
		kStateImageBk,					//背景图片（Control在用）
		kStateImageFore,				//前景图片（Control在用）
		kStateImageSelectedBk,			//选择时的背景图片（CheckBox在用）
		kStateImageSelectedFore,		//选择时的前景图片（CheckBox在用）
		kStateImagePartSelectedBk,		//部分选择时的背景图片（CheckBox在用）
		kStateImagePartSelectedFore,	//部分选择时的前景图片（CheckBox在用）
	};

	//控件状态
	enum ControlStateType
	{
		kControlStateNormal,	// 普通状态
		kControlStateHot,		// 悬停状态
		kControlStatePushed,	// 按下状态
		kControlStateDisabled	// 禁用状态
	};

	//控件动画类型
	enum class AnimationType
	{
		kAnimationNone,					//无动画
		kAnimationAlpha,				//透明度渐变动画
		kAnimationHeight,				//控件高度变化动画
		kAnimationWidth,				//控件宽度变化动画
		kAnimationHot,					//鼠标悬停状态的动画
		kAnimationInoutXFromLeft,		//控件的X坐标变化动画，从左侧
		kAnimationInoutXFromRight,		//控件的X坐标变化动画，从右侧
		kAnimationInoutYFromTop,		//控件的Y坐标变化动画，从上侧
		kAnimationInoutYFromBottom,		//控件的Y坐标变化动画，从下侧
	};

	//GIF图片帧类型
	enum GifFrameType
	{
		kGifFrameFirst   = 0,	// 第一帧	
		kGifFrameCurrent = 1,	// 当前帧		
		kGifFrameLast    = 2	// 最后一帧
	};

	//光标
	enum CursorType
	{
		kCursorArrow,		// 箭头
		kCursorHand,		// 手型
		kCursorHandIbeam,	// “I”形状
		kCursorSizeWE,		// 左右拖动
		kCursorSizeNS		// 上下拖动
	};

	//定义所有消息类型
	enum EventType
	{
		kEventNone,
		kEventFirst,				//第一个消息（暂时没用到）
		kEventAll,					//代表所有消息

		//键盘消息
		kEventKeyBegin,
		kEventKeyDown,				//Window类：当收到WM_KEYDOWN消息时触发，发送给Focus控件
		kEventKeyUp,				//Window类：当收到WM_KEYUP消息时触发，发送给WM_KEYDOWN事件中的那个Focus控件
		kEventSysKeyDown,			//Window类：当收到WM_SYSKEYDOWN消息时触发，发送给Focus控件
		kEventSysKeyUp,				//Window类：当收到WM_SYSKEYUP消息时触发，发送给WM_KEYDOWN事件中的那个Focus控件
		kEventChar,					//Window类：当收到WM_CHAR消息时触发，发送给WM_KEYDOWN事件中的那个Focus控件
		kEventKeyEnd,

		//鼠标消息
		kEventMouseBegin,		
		kEventMouseEnter,			//Window类：当收到WM_MOUSEMOVE消息时触发，发送给MouseMove时，鼠标进入的那个控件，表明鼠标进入到这个控件内
		kEventMouseLeave,			//Window类：当收到WM_MOUSEMOVE消息时触发，发送给MouseMove时，鼠标进入的那个控件，表明鼠标已经离开这个控件
		kEventMouseMove,			//Window类：当收到WM_MOUSEMOVE消息时触发，发送给鼠标左键、右键、双击时记录的那个控件
		kEventMouseHover,			//Window类：当收到WM_MOUSEHOVER消息时触发，发送给MouseMove时，鼠标进入的那个控件，表明鼠标在这个控件内悬停
		kEventMouseWheel,			//Window类：当收到WM_MOUSEWHEEL消息时触发，发送给当前鼠标所在位置对应的控件
		kEventMouseButtonDown,		//Window类：当收到WM_LBUTTONDOWN消息时触发，发送给鼠标左键按下时对应的控件
		kEventMouseButtonUp,		//Window类：当收到WM_LBUTTONUP消息时触发，发送给鼠标左键按下时对应的控件
		kEventMouseDoubleClick,		//Window类：当收到WM_LBUTTONDBLCLK消息时触发，发送给当前鼠标所在位置对应的控件		
		kEventMouseRButtonDown,		//Window类：当收到WM_RBUTTONDOWN消息时触发，发送给鼠标右键按下时对应的控件
		kEventMouseRButtonUp,		//Window类：当收到WM_RBUTTONUP消息时触发，发送给鼠标右键按下时对应的控件
		kEventMouseRDoubleClick,	//Window类：当收到WM_RBUTTONDBLCLK消息时触发，发送给当前鼠标所在位置对应的控件	
		kEventMouseMenu,			//Window类：当收到WM_CONTEXTMENU消息时触发，发送给所有注册回调函数的控件
		kEventMouseEnd,

		//焦点/光标/输入法相关消息
		kEventSetFocus,				//Window类：发送给Focus控件，当控件获得焦点时触发事件（控件焦点不是窗口焦点，两者完全不同）
		kEventKillFocus,			//Window类：发送给Focus控件，当控件失去焦点时触发事件（控件焦点不是窗口焦点，两者完全不同）		
		kEventSetCursor,			//Window类：发送给当前鼠标所在控件，设置光标
		kEventImeStartComposition,	//Window类：发送给Focus控件，当收到系统WM_IME_STARTCOMPOSITION消息时触发
		kEventImeEndComposition,	//Window类：发送给Focus控件，当收到系统WM_IME_ENDCOMPOSITION消息时触发

		kEventWindowKillFocus,		//Window类：发送给鼠标左键、右键按下时记录的控件、焦点控件，当窗口失去焦点时触发事件（主要用于恢复一些内部状态）
		kEventWindowSize,			//Window类：发送给Focus控件，当窗口大小发生变化时触发事件
		kEventWindowMove,			//Window类：发送给Focus控件，当窗口大小发生变化时触发事件
		kEventWindowClose,			//Window类，Combo控件：当窗口关闭（或者Combo的下拉框窗口关闭）时触发

		//左键点击/右键点击事件
		kEventClick,				//Button类、ListBoxItem、Option、CheckBox等：当点击按钮（或者键盘回车）时触发
		kEventRClick,				//Control类：当鼠标右键弹起时触发

		//鼠标点击控件发生变化，发给旧的鼠标点击事件
		kEventMouseClickChanged,	//Window类：当收到鼠标按下相关消息时触发，发送给上次鼠标按下时的控件
		kEventMouseClickEsc,	    //Window类：当收到键盘ESC键消息时触发，发送给鼠标按下时的控件

		//选择/取消选择
		kEventSelect,				//CheckBox类、Option类、ListBox类、Combo类：当变成选中状态时触发
		kEventUnSelect,			    //CheckBox类、Option类、ListBox类、Combo类：当变成非选中状态时触发

		//勾选/取消勾选
		kEventChecked,				//CheckBox类：当变成Checked状态时触发
		kEventUnCheck,			    //CheckBox类：当变成UnCheck状态时触发

		//TAB页面选择
		kEventTabSelect,			//TabBox类，选中TAB页面, WPARAM是新页面ID，LPARAM是旧页面ID

		//树节点展开/收起
		kEventExpand,				//TreeNode类：当树节点展开时触发
		kEventCollapse,				//TreeNode类：当树节点收起时触发

		kEventZoom,					//RichEdit类：当缩放比例发生变化时，按缩放比例分子/分母显示的缩放：1/64 < (wParam / lParam) < 64
		kEventTextChange,			//RichEdit类：当文本内容发生变化时触发
		kEventSelChange,			//ListCtrl类：选择项发生变化，RichEdit类：当文本选择内容发生变化时触发
		kEventReturn,				//ListCtrl、VirtualListBox、ListBoxItem、RichEdit类：当收到回车键时触发
		kEventTab,					//RichEdit类：在WantTab为false时，当收到TAB键时触发
		kEventLinkClick,		    //RichEdit类、RichText类：当点击到超级链接的数据上时触发, 可以通过WPARAM获取点击的URL，类型为const wchar_t*

		kEventScrollChange,			//ScrollBox类：当滚动条位置发生变化时触发
		kEventValueChange,			//DateTime、Slider类：当值发生变化时触发, Slider类：WPARAM是新值，LPARAM是旧值
		kEventResize,				//Control类：当控件的大小和位置发生变化时触发
		kEventVisibleChange,		//Control类：当控件的Visible属性发生变化时触发
		kEventStateChange,			//Control类：当控件的State属性发生变化时触发, WPARAM是新状态，LPARAM是旧状态
		kEventSelectColor,			//ColorPicker类：当选择了新颜色时触发, WPARAM是新的颜色值，LPARAM是旧的颜色值

		kEventSplitDraged,			//Split类：通过拖动操作调整了控件的大小，WPARAM是第一个控件接口，LPARAM是第二个控件接口

		kEventEnterEdit,			//ListCtrl类：进入编辑状态, wParam 是接口：ListCtrlEditParam*
		kEventLeaveEdit,			//ListCtrl类：退出编辑状态, wParam 是接口：ListCtrlEditParam*

		kEventLast					//Control类：该控件的最后一个消息，当这个控件对象销毁时触发
	};

	/** 热键组合键标志位
	*/
	enum HotKeyModifiers
	{
		kHotKey_Shift = 0x01,	//Shift键
		kHotKey_Contrl = 0x02,	//Contrl键
		kHotKey_Alt = 0x04,		//Alt键
		kHotKey_Ext = 0x08		//扩展键
	};

	/** 鼠标操作的标志
	*/
#ifdef UILIB_IMPL_WINSDK
	enum VKFlag
	{
		kVkControl = MK_CONTROL,	//按下了 CTRL 键
		kVkShift   = MK_SHIFT,		//按下了 SHIFT 键
		kVkLButton = MK_LBUTTON,	//按下了鼠标左键
		kVkRButton = MK_RBUTTON		//按下了鼠标右键
	};
#endif

}// namespace ui

#endif //DUILIB_DEFS_H_


