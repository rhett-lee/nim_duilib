#ifndef UI_CORE_CONTROL_H_
#define UI_CORE_CONTROL_H_

#pragma once

#include "duilib/Core/PlaceHolder.h"
#include "duilib/Core/BoxShadow.h"
#include "duilib/Utils/Delegate.h"
#include <map>
#include <memory>

namespace ui 
{
	class Control;
	class Image;
	class IMatrix;
	class StateColorMap;
	class StateImageMap;
	class AnimationManager;
	class IRender;
	class IPath;

	typedef Control* (CALLBACK* FINDCONTROLPROC)(Control*, LPVOID);

class UILIB_API Control : 
	public PlaceHolder
{
	typedef std::map<int, CEventSource> GifEventMap;
public:
	Control();
	Control(const Control& r) = delete;
	Control& operator=(const Control& r) = delete;
    virtual ~Control();

	virtual std::wstring GetType() const;

    /// 图形相关
	/**@brief 获取背景颜色
	 * @return 返回背景颜色的字符串，该值在 global.xml 中定义
	 */
	const std::wstring& GetBkColor() const { return m_strBkColor; }

	/** @brief 设置背景颜色
	 * @param[in] strColor 要设置的背景颜色值，该值必须在 global.xml 中存在
	 * @return 无
	 */
	void SetBkColor(const std::wstring& strColor);

	/**
	 * @brief 获取某个状态下的字体颜色
	 * @param[in] stateType 要获取何种状态下的颜色值，参考 ControlStateType 枚举
	 * @return 指定状态下设定的颜色字符串，对应 global.xml 中指定色值
	 */
	std::wstring GetStateColor(ControlStateType stateType) const;

	/**
	 * @brief 设置某个状态下的字体颜色
	 * @param[in] stateType 要设置何种状态下的颜色值，参考 ControlStateType 枚举
	 * @param[in] strColor 要设置的颜色值，该值必须在 global.xml 中存在
	 * @return 无
	 */
	void SetStateColor(ControlStateType stateType, const std::wstring& strColor);

	/**
	 * @brief 获取背景图片位置
	 * @return 背景图片位置  
	 */
    std::wstring GetBkImage() const;

	/**
	 * @brief 获取 UTF8 格式的背景图片位置
	 * @return 背景图片位置
	 */
	std::string GetUTF8BkImage() const;

	/**
	 * @brief 设置背景图片
	 * @param[in] strImage 要设置的图片路径
	 * @return 无
	 */
	void SetBkImage(const std::wstring& strImage);

	/**
	 * @brief 设置背景图片（UTF8 格式字符串）
	 * @param[in] strImage 要设置的图片路径
	 * @return 无
	 */
	void SetUTF8BkImage(const std::string& strImage);

    /**
    * @brief 获取loading状态图片位置
    * @return loading图片位置
    */
    std::wstring GetLoadingImage() const;

    /**
    * @brief 设置loading图片
    * @param[in] strImage 要设置的图片路径
    * @return 无
    */
    void SetLoadingImage(const std::wstring& strImage);

    /**
    * @brief 设置loading背景色
    * @param[in] strColor 背景色
    * @return 无
    */
    void SetLoadingBkColor(const std::wstring& strColor);

	/**
	 * @brief 获取指定状态下的图片位置
	 * @param[in] 要获取何种状态下的图片，参考 ControlStateType 枚举
	 * @return 指定状态下的图片位置
	 */
	std::wstring GetStateImage(ControlStateType stateType);

	/**
	 * @brief 设置某个状态下的图片
	 * @param[in] stateType 要设置何种状态下的图片，参考 ControlStateType 枚举
	 * @param[in] strImage 要设置的图片路径
	 * @return 无
	 */
	void SetStateImage(ControlStateType stateType, const std::wstring& strImage);

	/**
	 * @brief 获取指定状态下的前景图片
	 * @param[in] 要获取何种状态下的图片，参考 `ControlStateType` 枚举
	 * @return 指定状态下前景图片位置
	 */
	std::wstring GetForeStateImage(ControlStateType stateType);

	/**
	 * @brief 设置某个状态下前景图片
	 * @param[in] stateType 要设置何种状态下的图片，参考 `ControlStateType` 枚举
	 * @param[in] strImage 要设置的前景图片路径
	 * @return 无
	 */
	void SetForeStateImage(ControlStateType stateType, const std::wstring& strImage);

	/**@brief 获取控件状态
	 * @return 控件的状态，请参考 `ControlStateType` 枚举
	 */
	ControlStateType GetState() const;

	/**@brief 设置控件状态
	 * @param[in] controlState 要设置的控件状态，请参考 `ControlStateType` 枚举
	 */
	void SetState(ControlStateType controlState);

	/** 获取用于估算控件大小（宽和高）图片接口
	 */
	virtual Image* GetEstimateImage();

	/// 边框相关
	/**@brief 获取边框颜色
	 * @return 边框的颜色字符串，对应 global.xml 中的具体颜色值
	 */
	const std::wstring& GetBorderColor() const;

	/**
	 * @brief 设置边框颜色
	 * @param[in] strBorderColor 设置边框的颜色字符串值，该值必须在 global.xml 中存在
	 * @return 无
	 */
	void SetBorderColor(const std::wstring& strBorderColor);

	/** @brief 设置边框的大小(left、top、right、bottom分别对应左边框大小，上边框大小，右边框大小、下边框大小)
	 * @param[in] rc 一个 `UiRect` 结构的边框大小集合
	 */
	void SetBorderSize(UiRect rc);

	/**
	 * @brief 获取左侧边框大小
	 * @return 左侧边框的大小  
	 */
	int GetLeftBorderSize() const;

	/**
	 * @brief 设置左侧边框大小
	 * @param[in] nSize 要设置的左侧边框大小
	 * @return 无
	 */
	void SetLeftBorderSize(int nSize);

	/**
	 * @brief 获取顶部边框大小
	 * @return 顶部边框大小
	 */
	int GetTopBorderSize() const;

	/**
	 * @brief 设置顶部边框大小
	 * @param[in] nSize 要设置的上方边框大小
	 * @return 无
	 */
	void SetTopBorderSize(int nSize);

	/**
	 * @brief 获取右侧边框大小
	 * @return 右侧的边框大小
	 */
	int GetRightBorderSize() const;

	/**
	 * @brief 设置右侧边框大小
	 * @param[in] nSize 要设置的右侧边框大小
	 * @return 无
	 */
	void SetRightBorderSize(int nSize);

	/**
	 * @brief 获取下方边框大小
	 * @return 下方边框大小
	 */
	int GetBottomBorderSize() const;

	/**@brief 设置下方边框大小
	 * @param[in] nSize 要设置的下方边框大小
	 */
	void SetBottomBorderSize(int nSize);

	/**@brief 获取边框大小
	 * @return 四个方向的边框大小
	 */
	const UiSize& GetBorderRound() const;

	/**
	 * @brief 设置边框大小
	 * @param[in] cxyRound 一个 UiSize 结构表示了四个方向边框的大小
	 * @return 无
	 */
	void SetBorderRound(UiSize cxyRound);

	/**
	 * @brief 设置边框阴影
	 * @param[in] 要设置的阴影属性
	 * @return 无
	 */
	void SetBoxShadow(const std::wstring& strShadow);

    /// 鼠标相关
	/**
	 * @brief 获取鼠标指针类型
	 */
	virtual CursorType GetCursorType() const;

	/**@brief 设置当前鼠标指针类型
	 * @param[in] cursorType 要设置的鼠标类型，参考 CursorType 枚举
	 */
	void SetCursorType(CursorType cursorType);

	/**
	 * @brief 获取控件在鼠标悬浮状态下的提示文本
	 * @return 返回当前鼠标悬浮状态提示的文本
	 */
    virtual std::wstring GetToolTipText() const;

	/**
	 * @brief 获取控件在鼠标悬浮状态下的提示文本（UTF8 格式）
	 * @return 返回当前鼠标悬浮状态提示的文本
	 */
	virtual std::string GetUTF8ToolTipText() const;

	/**
	 * @brief 设置鼠标悬浮到控件显示的提示文本
	 * @param[in] strText 要设置的文本
	 * @return 无
	 */
	virtual void SetToolTipText(const std::wstring& strText);

	/**
	 * @brief 设置鼠标悬浮到控件显示的提示文本（UTF8 格式）
	 * @param[in] strText 要设置的文本
	 * @return 无
	 */
	virtual void SetUTF8ToolTipText(const std::string& strText);

	/**
	 * @brief 设置鼠标悬浮到控件显示的提示文本在语言文件中对应的文字
	 * @param[in] strTextId 在语言文件中对应的提示文字 ID
	 * @return 无
	 */
	virtual void SetToolTipTextId(const std::wstring& strTextId);

	/**
	 * @brief 设置鼠标悬浮到控件显示的提示文本在语言文件中对应的文字（UTF8 格式）
	 * @param[in] strTextId 在语言文件中对应的提示文字 ID
	 * @return 无
	 */
	virtual void SetUTF8ToolTipTextId(const std::string& strTextId);

	/**
	 * @brief 设置鼠标悬浮到控件上提示的文本单行最大宽度
	 * @param[in] nWidth 要设置的宽度值
	 * @return 无
	 */
	virtual void SetToolTipWidth(int nWidth);

	/**
	 * @brief 获取鼠标悬浮到控件上提示的文本单行最大宽度
	 * @return 当前提示文本最大宽度
	 */
	virtual int GetToolTipWidth(void) const;

    /// 菜单
	/**
	 * @brief 控件是否响应上下文菜单消息
	 * @return 返回结果表示了是否响应上下文菜单消息，true 响应上下文菜单消息，false 不响应上下文菜单消息
	 */
	virtual bool IsContextMenuUsed() const { return m_bContextMenuUsed; };

	/**
	 * @brief 设置控件响应上下文菜单消息
	 * @param[in] bMenuUsed 是否响应上下文菜单消息，true 为是，false 为否
	 * @return 无
	 */
    virtual void SetContextMenuUsed(bool bMenuUsed);

    /// 用户数据，辅助函数，供用户使用
	/**
	 * @brief 获取用户绑定到控件的数据字符串
	 * @return 返回具体数据字符串
	 */
    virtual std::wstring GetDataID() const;

	/**
	 * @brief 获取用户绑定到控件的数据字符串（UTF8 格式）
	 * @return 返回具体数据字符串
	 */
	virtual std::string GetUTF8DataID() const;				

	/**
	 * @brief 绑定一个字符串数据到控件
	 * @param[in] strText 要绑定的字符串数据
	 * @return 无
	 */
    virtual void SetDataID(const std::wstring& strText);

	/**
	 * @brief 绑定一个字符串数据到控件（UTF8 格式）
	 * @param[in] strText 要绑定的字符串数据
	 * @return 无
	 */
	virtual void SetUTF8DataID(const std::string& strText);	

    /// 一些重要的属性
	/**
	 * @brief 以淡入淡出等动画形式设置控件是否可见, 调用的结果与SetVisible相同，只是过程包含了动画效果。
	          调用SetFadeVisible以后，不需要再调用SetVisible函数修改可见属性。
			  该函数内部会调用SetVisible这个虚函数。
	 * @param[in] bVisible 为 true 时控件可见，为 false 时控件被隐藏
	 * @return 无
	 */
    virtual void SetFadeVisible(bool bVisible);

	/** 设置控件是否可见
	 * @param[in] @param[in] bVisible 为 true 时控件可见，为 false 时控件被隐藏
	 */
	virtual void SetVisible(bool bVisible) override;

	/**
	 * @brief 检查控件是否可用
	 * @return 控件可用状态，返回 true 控件可用，否则为 false
	 */
	virtual bool IsEnabled() const { return m_bEnabled; };

	/**
	 * @brief 设置控件可用状态
	 * @param[in] bEnable 为 true 时控件可用，为 false 时控件为禁用状态则不可用
	 * @return 无
	 */
    virtual void SetEnabled(bool bEnable = true);

	/**
	 * @brief 检查控件是否响应鼠标事件
	 * @return 返回控件是否响应鼠标事件，返回 true 响应鼠标事件，false 为不响应
	 */
	virtual bool IsMouseEnabled() const { return m_bMouseEnabled; };

	/**
	 * @brief 设置控件是否响应鼠标事件
	 * @param[in] bEnable 为 true 响应鼠标事件，为 false 时不响应鼠标事件
	 * @return 无
	 */
    virtual void SetMouseEnabled(bool bEnable);

	/**
	 * @brief 检查控件是否响应键盘事件
	 * @return 返回控件是否响应键盘事件，返回 true 响应键盘事件，false 不响应键盘事件
	 */
	virtual bool IsKeyboardEnabled() const { return m_bKeyboardEnabled; };

	/**
	 * @brief 设置控件是否响应键盘事件
	 * @param[in] bEnable 为 true 响应键盘事件，为 false 时不响应键盘事件
	 * @return 无
	 */
    virtual void SetKeyboardEnabled(bool bEnable);

	/**
	 * @brief 检查控件是否具有焦点
	 * @return 返回控件是否具有检点，为 true 时是当前具有焦点，为 false 时控件没有焦点
	 */
    virtual bool IsFocused() const;

	/**
	 * @brief 让控件获取焦点
	 * @return 无
	 */
    virtual void SetFocus();

	/**
	 * @brief 返回控件的标识，用于判断是否可以响应 TAB 切换事件
	 * @return 返回控件的标识类型
	 */
	virtual UINT GetControlFlags() const;

	/**
	 * @brief 让控件设置永远获取不到焦点
	 * @return 无
	 */
	void SetNoFocus(); // 控件永远不要焦点，与KillFocus不一样

	/**
	 * @brief 判断当前鼠标焦点是否在控件上
	 * @return 返回鼠标焦点是否在控件上，true 鼠标焦点在控件上，false 鼠标焦点不在控件上
	 */
	virtual bool IsMouseFocused() const { return m_bMouseFocused;}

	/**
	 * @brief 设置是否将鼠标焦点到控件上
	 * @param[in] bMouseFocused 为 true 时设置鼠标焦点到控件上，为 false 时让控件失去鼠标焦点
	 * @return 无
	 */
	virtual void SetMouseFocused(bool bMouseFocused) { m_bMouseFocused = bMouseFocused; }

	/**
	 * @brief 判断控件当前是否是可激活状态
	 * @return 返回控件状态，true 控件当前是可激活状态，可见并可用，false 控件当前非可激活状态，可能不可见或被禁用
	 */
	virtual bool IsActivatable() const;

	/**
	 * @brief 激活控件，如点击、选中、展开等操作
	 * @param[in] 待补充
	 * @return 待补充
	 */
	virtual void Activate();

	/// 控件搜索
	/**
	 * @brief 根据坐标查找指定控件
	 * @param[in] Proc
	 * @param[in] pData
	 * @param[in] uFlags
	 * @param[in] scrollPos
	 * @return 返回控件的指针
	 */
    virtual Control* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, UiPoint scrollPos = UiPoint());

	/// 位置相关
	/**
	 * @brief 获取控件位置
	 * @param[in] bContainShadow 是否包含阴影，默认为 true 则包含阴影范围，否则 false 为不包含
	 * @return 返回控件的位置信息
	 */
	virtual	UiRect GetPos(bool bContainShadow = true) const override;

	/**
	 * @brief 设置控件位置
	 * @param[in] rc 控件位置的描述信息
	 * @return 无
	 */
	virtual void SetPos(UiRect rc) override;

	/**
	 * @brief 获取控件的外边距
	 * @return 返回控件的外边距
	 */
	virtual UiRect GetMargin() const;

	/**
	 * @brief 设置控件的外边距
	 * @param[in] rcMargin 控件的外边距信息
	 * @param[in] bNeedDpiScale 是否让外边距根据 DPI 适配，默认为 true，false 不适配 DPI
	 * @return 无
	 */
	virtual void SetMargin(UiRect rcMargin, bool bNeedDpiScale);

	/** 计算控件大小(宽和高)
	    如果设置了图片并设置 width 或 height 任意一项为 auto，将根据图片来计算最终大小
	 * @param[in] szAvailable 可用的大小
	 * @return 控件估算大小
	 */
	virtual UiSize EstimateSize(UiSize szAvailable) override;

	/** 计算文本区域大小（宽和高）
	   @param[in] szAvailable 可用的大小
	 * @return 控件估算大小
	 */
	virtual UiSize EstimateText(UiSize szAvailable);

	/**
	 * @brief 检查指定坐标是否在滚动条当前滚动位置的范围内
	 * @param[in] point 具体坐标
	 * @return 返回是否在范围内，true 在滚动条当前滚动位置范围内，false 不在滚动条当前滚动位置范围内
	 */
	virtual bool IsPointInWithScrollOffset(const UiPoint& point) const;

	// 消息处理
	/** @brief 控件统一的消息处理入口，将传统 Windows 消息转换为自定义格式的消息
	 * @param[in] eventType	消息内容
	 * @param[in] wParam	消息附加内容
	 * @param[in] lParam	消息附加内容
	 * @param[in] tChar		按键信息
	 * @param[in] mousePos	鼠标信息
	 */
	virtual void SendEvent(EventType eventType,
			        	   WPARAM wParam = 0, 
						   LPARAM lParam = 0, 
						   TCHAR tChar = 0, 
						   const UiPoint& mousePos = UiPoint());

	/** @brief 将转换后的消息派发到消息处理函数
	 * @param[in] msg 消息内容
	 */
    virtual void SendEvent(const EventArgs& msg);

	/**
	 * @brief 判断控件是否有 HOT 状态
	 * @return 返回 true 有 HOT 状态， 否则为 false
	 */
	virtual bool HasHotState();

	// 属性设置
	/**
	 * @brief 设置控件指定属性
	 * @param[in] strName 要设置的属性名称（如 width）
	 * @param[in] strValue 要设置的属性值（如 100）
	 * @return 无
	 */
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue);

	/**
	 * @brief 设置控件的 class 全局属性
	 * @param[in] strClass 要设置的 class 名称，该名称必须在 global.xml 中存在
	 * @return 无
	 */
	void SetClass(const std::wstring& strClass);

	/**
	 * @brief 应用一套属性列表
	 * @param[in] strList 属性列表的字符串表示，如 `width="100" height="30"`
	 * @return 无
	 */
    void ApplyAttributeList(const std::wstring& strList);

	/**
	 * @brief 待补充
	 * @param[in] 待补充
	 * @return 待补充
	 */
	bool OnApplyAttributeList(const std::wstring& strReceiver, const std::wstring& strList, const EventArgs& eventArgs);

	/// 绘制操作
	/**
	 * @brief 绘制图片
	 * @param[in] pRender 绘制上下文
	 * @param[in] duiImage 图片对象
	 * @param[in] isLoadingImage true表示Loading Image，绘制时会旋转该图片;false表示正常图片
	 * @param[in] strModify 图片的附加属性
	 * @param[in] nFade 控件的透明度，如果启用动画效果该值在绘制时是不断变化的
	 * @param[in] pMatrix 绘制图片时使用的变换矩阵
	 * @return 成功返回 true，失败返回 false
	 */
	bool PaintImage(IRender* pRender, Image& duiImage,
				    const std::wstring& strModify = L"",
		            int nFade = DUI_NOSET_VALUE,
		            IMatrix* pMatrix = nullptr);

	/**
	* @brief 获取绘制上下文对象
	* @return 返回绘制上下文对象
	*/
	IRender* GetRender();

	/**
	* @brief 清理绘制上下文对象
	* @return 无
	*/
	void ClearRender();

	/**
	 * @brief 待补充
	 * @param[in] 待补充
	 * @return 待补充
	 */
	virtual void AlphaPaint(IRender* pRender, const UiRect& rcPaint);
	
	/**
	* @brief 绘制控件的入口函数
	* @param[in] pRender 指定绘制区域
	* @param[in] rcPaint 指定绘制坐标
	* @return 无
	*/
	virtual void Paint(IRender* pRender, const UiRect& rcPaint);

	/**
	* @brief 绘制控件子项入口函数
	* @param[in] pRender 指定绘制区域
	* @param[in] rcPaint 指定绘制坐标
	* @return 无
	*/
	virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) { (void)pRender; (void)rcPaint; };

	/**
	* @brief 设置是否对绘制范围做剪裁限制
	* @param[in] clip 设置 true 为需要，否则为不需要，见绘制函数
	* @return 无
	*/
	void SetClip(bool clip) { m_bClip = clip; };

	/**
	* @brief 判断是否对绘制范围做剪裁限制
	* @return 返回 true 为需要，false 为不需要
	*/
	bool IsClip() const { return m_bClip; }

	/**
	 * @brief 设置控件透明度
	 * @param[in] alpha 0 ~ 255 的透明度值，255 为不透明
	 * @return 无
	 */
	void SetAlpha(int alpha);

	/**
	 * @brief 获取控件透明度
	 * @return 返回控件的透明度
	 */
	int GetAlpha() const { return m_nAlpha;	}

	/**
	 * @brief 检查控件是否有透明属性
	 * @return 返回控件是否透明，true 控件当前有透明属性，false 控件没有透明属性
	 */
	bool IsAlpha() const { return m_nAlpha != 255; }

	/**
	 * @brief 设置焦点状态透明度
	 * @param[in] alpha 0 ~ 255 的透明度值，255 为不透明
	 * @return 无
	 */
	void SetHotAlpha(int nHotAlpha);

	/**
	 * @brief 设置是否接受TAB键切换焦点
	 * @param[in] enable
	 * @return 无
	 */
	void SetTabStop(bool enable);

	/**
	 * @brief 检查是否接受TAB键切换焦点
	 * @return 返回控件是否接受TAB键切换焦点
	 */
	bool IsAllowTabStop() const { return m_bAllowTabstop; }

	/**
	 * @brief 获取焦点状态透明度
	 * @return 返回控件焦点状态的透明度
	 */
	int GetHotAlpha() const { return m_nHotAlpha; }

	/**
	 * @brief 获取控件绘制偏移量
	 * @return 返回当前控件的绘制偏移量
	 */
	UiPoint GetRenderOffset() const { return m_renderOffset;	}

	/**
	 * @brief 设置控件绘制偏移量
	 * @param[in] renderOffset 控件偏移数据
	 * @return 无
	 */
	void SetRenderOffset(UiPoint renderOffset);

	/**
	 * @brief 设置控件偏移的 X 坐标
	 * @param[in] renderOffsetX X 坐标值
	 * @return 无
	 */
	void SetRenderOffsetX(int renderOffsetX);

	/**
	 * @brief 设置控件偏移的 Y 坐标
	 * @param[in] renderOffsetY Y 坐标值
	 * @return 无
	 */
	void SetRenderOffsetY(int renderOffsetY);

	/// Gif图片
	/**
	 * @brief 播放 GIF
	 * @param[in] 播放完成停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifStopType 枚举
	 * @return 无
	 */
	void StartGifPlayForUI(GifStopType frame = kGifStopFirst,int playcount = -1);

	/**
	 * @brief 停止播放 GIF
	 * @param[in] transfer 是否将停止事件通知给订阅者，参考 AttachGifPlayStop 方法
	 * @param[frame] frame 播放结束停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifStopType 枚举
	 * @return 无
	 */
	void StopGifPlayForUI(bool transfer = false, GifStopType frame = kGifStopCurrent);

	/**
	 * @brief 监听 GIF 播放完成通知
	 * @param[in] callback 要监听 GIF 停止播放的回调函数
	 * @return 无
	 */
	void AttachGifPlayStop(const EventCallback& callback){ m_OnGifEvent[m_nVirtualEventGifStop] += callback; };

    /**
    * @brief 开启loading状态
    * @param[in] start_angle loading图片旋转的角度
    * @return 无
    */
    void StartLoading(int fStartAngle = -1);

    /**
    * @brief 关闭loading状态
    * @param[in] frame 播放完成停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifStopType 枚举
    * @return 无
    */
    void StopLoading(GifStopType frame = kGifStopFirst);

    /**
    * @brief 计算loading图片的旋转角度
    * @return 无
    */
    void Loading();
    /**
    * @brief 是否正在loading
    * @return 在loading返回true, 反之返回false
    */
    bool IsLoading();

	/** @brief 获取动画管理器接口
	 */
	AnimationManager& GetAnimationManager() const;

	/// 图片缓存
	/**@brief 根据图片路径, 加载图片信息到缓存中。
	 *        加载策略：如果图片没有加载则执行加载图片；如果图片路径发生变化，则重新加载该图片。
	 * @param[in，out] duiImage 传入时标注图片的路径信息，如果成功则会缓存图片并记录到该参数的成员中
	 */
	bool LoadImageData(Image& duiImage) const;

	/**@brief 加载图片到缓存
	 */
	virtual void InvokeLoadImageCache();

	/**@brief 取消加载图片缓存
	 */
	virtual void UnLoadImageCache();

	/**@brief 清理图片缓存
	 */
	virtual void ClearImageCache();

	/**
	 * @brief 监听控件所有事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachAllEvents(const EventCallback& callback)	{ AttachEvent(kEventAll, callback); }

	/**
	 * @brief 监听鼠标进入事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachMouseEnter(const EventCallback& callback) { AttachEvent(kEventMouseEnter, callback); }

	/**
	 * @brief 监听鼠标离开事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachMouseLeave(const EventCallback& callback) { AttachEvent(kEventMouseLeave, callback); }

	/**
	 * @brief 监听鼠标悬浮事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachMouseHover(const EventCallback& callback) { AttachEvent(kEventMouseHover, callback); }

	/**
	 * @brief 监听鼠标按下事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachButtonDown(const EventCallback& callback) { AttachEvent(kEventMouseButtonDown, callback); }

	/**
	 * @brief 监听鼠标弹起事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachButtonUp(const EventCallback& callback) { AttachEvent(kEventMouseButtonUp, callback); }

	/**
	 * @brief 监听获得焦点事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachSetFocus(const EventCallback& callback) { AttachEvent(kEventSetFocus, callback); }

	/**
	 * @brief 监听失去焦点事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachKillFocus(const EventCallback& callback) { AttachEvent(kEventKillFocus, callback); }

	/**
	 * @brief 监听右键菜单事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachMenu(const EventCallback& callback) { AttachEvent(kEventMouseMenu, callback); }

	/**
	 * @brief 监听控件大小改变事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachResize(const EventCallback& callback) { AttachEvent(kEventResize, callback); }

	/**
	 * @brief 监听双击事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 * @return 无
	 */
	void AttachDoubleClick(const EventCallback& callback) { AttachEvent(kEventMouseDoubleClick, callback); }

	/**
	* @brief 绑定鼠标点击处理函数
	* @param[in] callback 要绑定的回调函数
	* @return 无
	*/
	void AttachClick(const EventCallback& callback) { AttachEvent(kEventClick, callback); }

	/**
	* @brief 监听控件关闭前最后一条消息
	* @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	* @return 无
	*/
	void AttachLastEvent(const EventCallback& callback) { AttachEvent(kEventLast, callback); }

	/**
	* @brief 监听控件显示或隐藏事件
	* @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	* @return 无
	*/
	void AttachVisibleChange(const EventCallback& callback) { AttachEvent(kEventVisibleChange, callback); }

	/** 判断颜色定义是否有效
	* @param [in] colorName 颜色的名称，有效的颜色名称可以是以下定义（按获取优先级顺序）：
	*           (1) 优先级1：以'#'字符开头，直接指定颜色值，举例：#FFFFFFFF       
	*		    (2) 优先级2：在配置XML中的<Window>节点中定义子节点，举例：<TextColor name="wnd_darkcolor" value="#FF00BB96"/>
	*           (3) 优先级3：在global.xml中的<Global>节点中定义子节点，举例：<TextColor name="white" value="#FFFFFFFF"/>                    
	*           (4) 优先级4：参见ui::UiColors::UiColorConsts函数中的定义
	*/
	bool HasUiColor(const std::wstring& colorName) const;

	/** 获取某个颜色对应的值
	* @param [in] colorName 颜色的名称，有效的颜色名称可以是以下定义（按获取优先级顺序）：
	*           (1) 优先级1：以'#'字符开头，直接指定颜色值，举例：#FFFFFFFF
	*		    (2) 优先级2：在配置XML中的<Window>节点中定义子节点，举例：<TextColor name="wnd_darkcolor" value="#FF00BB96"/>
	*           (3) 优先级3：在global.xml中的<Global>节点中定义子节点，举例：<TextColor name="white" value="#FFFFFFFF"/>                    
	*           (4) 优先级4：直接指定预定义的颜色别名，参见ui::UiColors::UiColorConsts函数中的定义
	* @return ARGB颜色值
	*/
    UiColor GetUiColor(const std::wstring& colorName) const;

	/** 判断控件类型是否为可选择的
	 * @return 默认返回false
	 */
	virtual bool IsSelectableType() const;

	/** 判断是否接受 TAB 按键消息
	 * @return 返回 true 表示接受，false 表示不接受， 默认返回false
	 */
	virtual bool IsWantTab() const;

	/** 该控件是否可以放置在标题栏上（以用于处理NC消息响应）
	 * @return 返回 true 表示可以，false 表示不可以， 默认返回false
	 */
	virtual bool CanPlaceCaptionBar() const;

	/** 当前控件以及级联父容器是否可见（从当前控件到最顶层父控件，只要有一个Visible为false，则返回false）
	* @return true 表示当前控件以及所有父控件，均是可见状态
	*         false 表示当前控件或者所有父控件中，有非可见状态的控件
	 */
	bool CheckVisibleAncestor(void) const;

public:
	/**@name 事件监听相关接口
	* @{
	*/

	/**@brief (m_OnEvent)监听指定事件
	 * @param[in] type 事件类型，见 EventType 枚举
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachEvent(EventType type, const EventCallback& callback);

	/**@brief (m_OnEvent)取消监听指定事件
	 * @param[in] type 事件类型，见 EventType 枚举
	 */
	void DetachEvent(EventType type);

	/**@brief (m_OnXmlEvent)通过XML中，配置<Event标签添加的响应事件，最终由Control::OnApplyAttributeList函数响应具体操作
	 * @param[in] type 事件类型，见 EventType 枚举
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachXmlEvent(EventType eventType, const EventCallback& callback);

	/**@brief (m_OnXmlEvent)取消监听指定事件
	 * @param[in] type 事件类型，见 EventType 枚举
	 */
	void DetachXmlEvent(EventType type);

	/**@brief (m_OnBubbledEvent)绑定事件处理函数
	 * @param[in] eventType 事件类型
	 * @param[in] callback 指定回调函数
	 */
	void AttachBubbledEvent(EventType eventType, const EventCallback& callback);

	/**@brief (m_OnBubbledEvent)解绑事件处理函数
	 * @param[in] eventType 事件类型
	 */
	void DetachBubbledEvent(EventType eventType);

	/** @brief 绑定 XML 中编写的 Event 和 BubbleEvent 事件的处理函数
	 * @param[in] eventType 事件类型
	 * @param[in] callback 指定回调函数
	 */
	void AttachXmlBubbledEvent(EventType eventType, const EventCallback& callback);

	/** @brief 解绑XML事件处理函数
	 * @param[in] eventType 事件类型
	 */
	void DetachXmlBubbledEvent(EventType eventType);

	/** 触发事件，向所有容器的监听者发送事件（m_OnEvent，m_OnXmlEvent，m_OnBubbledEvent， m_OnXmlBubbledEvent）
	* @param [in] msg 消息内容
	* @return 如果所有监听者回调函数返回true，则该函数返回true；否则返回false
	*/
	bool FireAllEvents(const EventArgs& msg);

	/** @} */

protected:
	
	/// Gif等多帧图片，播放动画
	bool GifPlay();
	void StopGifPlay(GifStopType frame = kGifStopCurrent);

	//处理放弃控件焦点相关逻辑
	void EnsureNoFocus();
	
	/// 消息处理的保护成员函数，不允许外部直接调用
	virtual void HandleEvent(const EventArgs& msg);
	virtual bool MouseEnter(const EventArgs& msg);
	virtual bool MouseLeave(const EventArgs& msg);
	virtual bool ButtonDown(const EventArgs& msg);
	virtual bool ButtonUp(const EventArgs& msg);

	/// 绘制相关保护成员函数，不允许外部直接调用
	virtual void PaintShadow(IRender* pRender);
	virtual void PaintBkColor(IRender* pRender);
	virtual void PaintBkImage(IRender* pRender);
	virtual void PaintStatusColor(IRender* pRender);
	virtual void PaintStatusImage(IRender* pRender);
	virtual void PaintText(IRender* pRender);
	virtual void PaintBorder(IRender* pRender);
	virtual void PaintLoading(IRender* pRender);

protected:
	/** @brief 获取控件图片类型与状态图片的映射接口
	*/
	StateImageMap* GetImageMap() const { return m_imageMap.get(); }

	/** @brief 获取控件状态与颜色值的映射接口
	*/
	StateColorMap* GetColorMap() const { return m_colorMap.get(); }

	/** @brief 获取控件的绘制区域
	*/
	const UiRect& GetPaintRect() const { return m_rcPaint; }

	/** @brief 设置控件的绘制区域
	*/
	void SetPaintRect(const UiRect& rect);

private:
	void BroadcastGifEvent(int nVirtualEvent);
	size_t GetGifFrameIndex(GifStopType frame);

	/** 绘制边框：根据条件判断绘制圆角矩形边框还是普通矩形边框
	*/
	void PaintBorders(IRender* pRender, UiRect rcDraw, 
		              int nBorderSize, UiColor dwBorderColor) const;

	/** 绘制圆角矩形
	*/
	void DrawRoundRect(IRender* pRender, const UiRect& rc, const UiSize& roundSize, UiColor dwBorderColor, int nBorderSize) const;

	/** 填充圆角矩形
	*/
	void FillRoundRect(IRender* pRender, const UiRect& rc, const UiSize& roundSize, UiColor dwColor) const;

	/** 填充路径, 形成圆角矩形
	*/
	void AddRoundRectPath(IPath* path, const UiRect& rc, const UiSize& roundSize) const;

	/** 当前控件是否为窗口的Root节点
	*/
	bool IsRootBox() const;

	/** 当前Window是否为圆角的(用于确定是否采用与Windows一样的圆角绘制方式)
	*/
	bool IsWindowRoundRect() const;

	/** 判断是否需要采用圆角矩形绘制边框
	*/
	bool ShouldBeRoundRectBorders() const;

public:
	/** 判断是否需要采用圆角矩形填充背景色
	*/
	bool ShouldBeRoundRectFill() const;

private:

	/** 获取颜色名称对应的颜色值
	*/
	UiColor GetUiColorByName(const std::wstring& colorName) const;

private:
	//控件的外边距属性（上，下，左，右边距）
	UiRect m_rcMargin;

private:
	/** 边框圆角大小(与m_rcBorderSize联合应用)或者阴影的圆角大小(与m_boxShadow联合应用)
	    仅当 m_rcBorderSize 四个边框值都有效, 并且都相同时
	*/
	UiSize m_cxyBorderRound;

	//控件阴影，其圆角大小通过m_cxyBorderRound变量控制
	BoxShadow m_boxShadow;

	//边框颜色
	std::wstring m_strBorderColor;

	//控件四边的边框大小（可分别设置top/bottom/left/right四个边的值）
	UiRect m_rcBorderSize;

private:
	//控件的背景颜色
	std::wstring m_strBkColor;

	//控件的背景图片
	std::unique_ptr<Image> m_bkImage;

	//加载中状态图片(m_bkImage)的生命周期管理、取消机制
	nbase::WeakCallbackFlag m_loadBkImageWeakFlag;

	//是否为播放GIF的状态（当背景图片m_bkImage是GIF文件时，触发此逻辑）
	bool m_bGifPlay;

	//GIF背景图片播放的取消机制
	nbase::WeakCallbackFlag m_gifWeakFlag;

    //GIF播放事件的回调注册管理容器(目前只有播放完成一个事件)
	GifEventMap m_OnGifEvent;

	//GIF背景图片播放完成事件的ID
	const int m_nVirtualEventGifStop = 1;	

private:
	/** 控件状态
	*/
	ControlStateType m_controlState;

	/** 状态与颜色值MAP
	*/
	std::unique_ptr<StateColorMap> m_colorMap;

	/** 控件图片类型与状态图片的MAP
	*/
	std::unique_ptr<StateImageMap> m_imageMap;

private:
	//是否处于加载中的状态
	bool m_bIsLoading;

	//加载中图片旋转的角度（0-360）
	int m_fCurrrentAngele;

	//加载中状态时显示的图片
	std::unique_ptr<Image> m_loadingImage;

	//加载中状态时的背景颜色
	std::wstring m_strLoadingBkColor;

	//加载中状态图片(m_loadingImage)的生命周期管理、取消机制
	nbase::WeakCallbackFlag m_loadingImageFlag;

private:
	//控件动画播放管理器
	std::unique_ptr<AnimationManager> m_animationManager;

	//控件播放动画时的渲染偏移(X坐标偏移和Y坐标偏移)
	UiPoint m_renderOffset;
	
	//控件的透明度（0 - 255，0为完全透明，255为不透明）
	int m_nAlpha;

	//控件为Hot状态时的透明度（0 - 255，0为完全透明，255为不透明）
	int m_nHotAlpha;

	//是否对绘制范围做剪裁限制
	bool m_bClip;

	//控件的绘制区域
	UiRect m_rcPaint;

	//绘制渲染引擎接口
	std::unique_ptr<IRender> m_render;

	//box-shadow是否已经绘制（由于box-shadow绘制会超过GetRect()范围，所以需要特殊处理）
	bool m_isBoxShadowPainted;

private:
	//ToolTip的宽度
	int m_nTooltipWidth;

	//ToolTip的文本内容
	std::wstring m_sToolTipText;

	//ToolTip的文本ID
	std::wstring m_sToolTipTextId;
	
	//用户数据ID
	std::wstring m_sUserDataID;

private:
	//通过AttachXXX接口，添加的监听事件
	EventMap m_OnEvent;

	//通过XML中，配置<Event标签添加的响应事件，最终由Control::OnApplyAttributeList函数响应具体操作
	EventMap m_OnXmlEvent;

	//通过AttachBubbledEvent接口添加的事件
	EventMap m_OnBubbledEvent;

	//通过XML中，配置<BubbledEvent标签添加的响应事件，最终由Control::OnApplyAttributeList函数响应具体操作
	EventMap m_OnXmlBubbledEvent;

private:
	//控件的Enable状态（当为false的时候，不响应鼠标、键盘等输入消息）
	bool m_bEnabled;

	//鼠标消息的Enable状态（当为false的时候，不响应鼠标消息）
	bool m_bMouseEnabled;

	//键盘消息的Enable状态（当为false的时候，不响应键盘消息）
	bool m_bKeyboardEnabled;

	//鼠标焦点是否在控件上
	bool m_bMouseFocused;

	//控件是否响应上下文菜单
	bool m_bContextMenuUsed;

	//控件不需要焦点（如果为true，则控件不会获得焦点）
	bool m_bNoFocus;

	//是否允许TAB切换焦点
	bool m_bAllowTabstop;

	//控件的光标类型
	CursorType m_cursorType;
};

} // namespace ui

#endif // UI_CORE_CONTROL_H_
