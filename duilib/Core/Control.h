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
	class ControlLoading;
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
public:
	Control();
	Control(const Control& r) = delete;
	Control& operator=(const Control& r) = delete;
    virtual ~Control();

    /** 获取控件类型
	*/
	virtual std::wstring GetType() const override;

    /// 图形相关
	/**@brief 获取背景颜色
	 * @return 返回背景颜色的字符串，该值在 global.xml 中定义
	 */
	std::wstring GetBkColor() const { return m_strBkColor.c_str(); }

	/** 设置背景颜色
	 * @param [in] strColor 要设置的背景颜色值，该值必须在 global.xml 中存在
	 */
	void SetBkColor(const std::wstring& strColor);

	/** 设置背景颜色
	 * @param [in] color 要设置的背景颜色值
	 */
	void SetBkColor(const UiColor& color);

	/**
	 * @brief 获取某个状态下的控件颜色
	 * @param[in] stateType 要获取何种状态下的颜色值，参考 ControlStateType 枚举
	 * @return 指定状态下设定的颜色字符串，对应 global.xml 中指定色值
	 */
	std::wstring GetStateColor(ControlStateType stateType) const;

	/**
	 * @brief 设置某个状态下的控件颜色
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

public:
    /** 设置loading图片
    * @param[in] strImage 要设置的图片路径及属性
    */
    void SetLoadingImage(const std::wstring& strImage);

    /** 设置loading背景色
    * @param[in] strColor 背景色
    */
    void SetLoadingBkColor(const std::wstring& strColor);

	/** 开启loading状态
	* @param[in] start_angle loading图片旋转的角度
	*/
	void StartLoading(int32_t fStartAngle = -1);

	/** 关闭loading状态
	* @param[in] frame 播放完成停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifFrameType 枚举
	*/
	void StopLoading(GifFrameType frame = kGifFrameFirst);

public:
	/**
	 * @brief 获取指定状态下的图片位置
	 * @param[in] 要获取何种状态下的图片，参考 ControlStateType 枚举
	 * @return 指定状态下的图片位置
	 */
	std::wstring GetStateImage(ControlStateType stateType) const;

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
	std::wstring GetForeStateImage(ControlStateType stateType) const;

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

	/** 当前是否为Hot状态(这个状态下，业务逻辑较多)
	*/
	bool IsHotState() const;

	/** 获取用于估算控件大小（宽和高）图片接口
	 */
	virtual Image* GetEstimateImage();

	/// 边框相关
	/** 获取指定状态下的边框颜色
	 * @param [in] stateType 控件状态
	 * @return 边框的颜色字符串，对应 global.xml 中的具体颜色值
	 */
	virtual std::wstring GetBorderColor(ControlStateType stateType) const;

	/** 设置边框颜色，应用于所有状态
	 * @param [in] strBorderColor 设置边框的颜色字符串值，该值必须在 global.xml 中存在
	 */
	void SetBorderColor(const std::wstring& strBorderColor);

	/** 设置指定状态下的边框颜色
	 * @param [in] stateType 控件状态
	 * @param [in] strBorderColor 设置边框的颜色字符串值，该值必须在 global.xml 中存在
	 */
	void SetBorderColor(ControlStateType stateType, const std::wstring& strBorderColor);

	/** 设置焦点状态下的边框颜色
	*/
	void SetFocusBorderColor(const std::wstring& strBorderColor);

	/** 获取焦点状态下的边框颜色
	*/
	std::wstring GetFocusBorderColor() const;

	/** 设置边框的大小(left、top、right、bottom分别对应左边框大小，上边框大小，右边框大小、下边框大小)
	 * @param [in] rc 一个 `UiRect` 结构的边框大小集合
	 * @param [in] bNeedDpiScale 是否需要做DPI自适应
	 */
	void SetBorderSize(UiRect rc, bool bNeedDpiScale = true);

	/** 获取左侧边框大小
	 * @return 左侧边框的大小  
	 */
	int32_t GetLeftBorderSize() const;

	/** 设置左侧边框大小
	 * @param [in] nSize 要设置的左侧边框大小
	 * @param [in] bNeedDpiScale 是否需要做DPI自适应
	 */
	void SetLeftBorderSize(int32_t nSize, bool bNeedDpiScale = true);

	/** 获取顶部边框大小
	 */
	int32_t GetTopBorderSize() const;

	/** 设置顶部边框大小
	 * @param [in] nSize 要设置的上方边框大小
	 * @param [in] bNeedDpiScale 是否需要做DPI自适应
	 */
	void SetTopBorderSize(int32_t nSize, bool bNeedDpiScale = true);

	/** 获取右侧边框大小
	 * @return 右侧的边框大小
	 */
	int32_t GetRightBorderSize() const;

	/** 设置右侧边框大小
	 * @param [in] nSize 要设置的右侧边框大小
	 * @param [in] bNeedDpiScale 是否需要做DPI自适应
	 */
	void SetRightBorderSize(int32_t nSize, bool bNeedDpiScale = true);

	/** 获取下方边框大小
	 * @return 下方边框大小
	 */
	int32_t GetBottomBorderSize() const;

	/** 设置下方边框大小
	 * @param [in] nSize 要设置的下方边框大小
	 * @param [in] bNeedDpiScale 是否需要做DPI自适应
	 */
	void SetBottomBorderSize(int32_t nSize, bool bNeedDpiScale = true);

	/** 获取圆角大小
	 */
	const UiSize& GetBorderRound() const;

	/** 设置边框大小
	 * @param [in] cxyRound 一个 UiSize 结构表示圆角大小
	 */
	void SetBorderRound(UiSize cxyRound);

	/** 设置边框阴影
	 * @param[in] 要设置的阴影属性
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
	virtual void SetToolTipWidth(int32_t nWidth);

	/**
	 * @brief 获取鼠标悬浮到控件上提示的文本单行最大宽度
	 * @return 当前提示文本最大宽度
	 */
	virtual int32_t GetToolTipWidth(void) const;

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

	/** 获取用户绑定到控件的数据字符串
	 * @return 返回具体数据字符串
	 */
	std::wstring GetDataID() const;

	/** 获取用户绑定到控件的数据字符串（UTF8 格式）
	 * @return 返回具体数据字符串
	 */
	std::string GetUTF8DataID() const;

	/** 绑定一个字符串数据到控件
	 * @param[in] strText 要绑定的字符串数据
	 */
	void SetDataID(const std::wstring& strText);

	/** 绑定一个字符串数据到控件（UTF8 格式）
	 * @param[in] strText 要绑定的字符串数据
	 */
	void SetUTF8DataID(const std::string& strText);

	/** 绑定一个整型值数据到控件
	 * @param[in] dataID 要绑定的整型数据
	 */
	void SetUserDataID(size_t dataID);

	/** 获取控件绑定的整型值数据
	*/
	size_t GetUserDataID() const;

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

	/** 检查控件是否具有焦点
	 * @return 返回控件是否具有检点，为 true 时是当前具有焦点，为 false 时控件没有焦点
	 */
    virtual bool IsFocused() const;

	/** 让控件获取焦点
	 */
    virtual void SetFocus();

	/** 返回控件的标识，用于判断是否可以响应 TAB 切换事件
	 * @return 返回控件的标识类型
	 */
	virtual uint32_t GetControlFlags() const;

	/** 让控件设置永远获取不到焦点
	 */
	void SetNoFocus(); // 控件永远不要焦点，与KillFocus不一样

	/** 该控件是否不获取焦点
	*/
	bool IsNoFocus() const { return m_bNoFocus; }

	/** 设置是否显示焦点状态(一个虚线构成的矩形)
	*/
	void SetShowFocusRect(bool bShowFocusRect);

	/** 设置是否显示焦点状态(一个虚线构成的矩形)
	*/
	bool IsShowFocusRect() const;

	/** 设置焦点状态虚线矩形的颜色(线条的颜色)
	*/
	void SetFocusRectColor(const std::wstring& focusRectColor);

	/** 获取焦点状态虚线矩形的颜色(线条的颜色)
	*/
	std::wstring GetFocusRectColor() const;

	/** 判断当前鼠标焦点是否在控件上
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

	/** 根据坐标查找指定控件
	 * @param [in] Proc 查找的匹配函数
	 * @param [in] pProcData 查找函数的自定义数据
	 * @param [in] ptMouse 鼠标所在的坐标, 仅当含有UIFIND_HITTEST时有效
	 * @param [in] uFlags 查找标志，比如：UIFIND_ENABLED 等
	 * @param [in] scrollPos 滚动条的滚动位置
	 * @return 返回控件的指针
	 */
    virtual Control* FindControl(FINDCONTROLPROC Proc, LPVOID pProcData, uint32_t uFlags,								 
								 const UiPoint& ptMouse = UiPoint(),
								 const UiPoint& scrollPos = UiPoint());

	/// 位置相关
	/** 获取控件位置（子类可改变行为）
	* @return 返回控件的矩形区域，包含内边距，不包含外边距
	 */
	virtual	UiRect GetPos() const override;

	/** 设置控件位置（子类可改变行为）
	 * @param [in] rc 要设置的矩形区域信息，包含内边距，不包含外边距
	 */
	virtual void SetPos(UiRect rc) override;

	/** 计算控件大小(宽和高)
	    如果设置了图片并设置 width 或 height 任意一项为 auto，将根据图片大小和文本大小来计算最终大小
	 *  @param [in] szAvailable 可用大小，不包含外边距
	 *  @return 控件的估算大小，包含内边距(Box)，不包含外边距
	 */
	virtual UiEstSize EstimateSize(UiSize szAvailable);

	/** 计算文本区域大小（宽和高）
	 *  @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
	 *  @return 控件的文本估算大小，包含内边距(Box)，不包含外边距
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
	 * @param [in] pRender 绘制上下文
	 * @param [in] pImage 图片对象的接口
	 * @param [in] isLoadingImage true表示Loading Image，绘制时会旋转该图片;false表示正常图片
	 * @param [in] strModify 图片的附加属性
	 * @param [in] nFade 控件的透明度，如果启用动画效果该值在绘制时是不断变化的
	 * @param [in] pMatrix 绘制图片时使用的变换矩阵
	 * @param [in] pInRect 外部传入的图片绘制的目标区域，如果为nullptr，则内部使用GetRect()返回的区域
	 * @param [out] pPaintedRect 返回图片绘制的最终目标矩形区域
	 * @return 成功返回 true，失败返回 false
	 */
	bool PaintImage(IRender* pRender, Image* pImage,
				    const std::wstring& strModify = L"",
		            int32_t nFade = DUI_NOSET_VALUE,
		            IMatrix* pMatrix = nullptr,
		            UiRect* pInRect = nullptr,
		            UiRect* pPaintedRect = nullptr) const;

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
	void SetAlpha(int64_t alpha);

	/**
	 * @brief 获取控件透明度
	 * @return 返回控件的透明度
	 */
	int32_t GetAlpha() const { return m_nAlpha;	}

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
	void SetHotAlpha(int64_t nHotAlpha);

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
	int32_t GetHotAlpha() const { return m_nHotAlpha; }

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
	void SetRenderOffsetX(int64_t renderOffsetX);

	/**
	 * @brief 设置控件偏移的 Y 坐标
	 * @param[in] renderOffsetY Y 坐标值
	 * @return 无
	 */
	void SetRenderOffsetY(int64_t renderOffsetY);

	/// 动画图片
	/** 播放 GIF/WebP/APNG 动画
	 * @param [in] nStartFrame 从哪一帧开始播放，可设置第一帧、当前帧和最后一帧。请参考 GifFrameType 枚举
	 * @param [in] nPlayCount 指定播放次数, 如果是-1表示一直播放
	 */
	bool StartGifPlay(GifFrameType nStartFrame = kGifFrameFirst, int32_t nPlayCount = -1);

	/** 停止播放 GIF/WebP/APNG 动画
	 * @param [in] bTriggerEvent 是否将停止事件通知给订阅者，参考 AttachGifPlayStop 方法
	 * @param [in] nStopFrame 播放结束停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifFrameType 枚举
	 */
	void StopGifPlay(bool bTriggerEvent = false, GifFrameType nStopFrame = kGifFrameCurrent);

	/** 监听 GIF 播放完成通知
	 * @param[in] callback 要监听 GIF 停止播放的回调函数
	 */
	void AttachGifPlayStop(const EventCallback& callback);

	/** @brief 获取动画管理器接口
	 */
	AnimationManager& GetAnimationManager();

	/// 图片缓存
	/**@brief 根据图片路径, 加载图片信息到缓存中。
	 *        加载策略：如果图片没有加载则执行加载图片；如果图片路径发生变化，则重新加载该图片。
	 * @param[in，out] duiImage 传入时标注图片的路径信息，如果成功则会缓存图片并记录到该参数的成员中
	 */
	bool LoadImageData(Image& duiImage) const;

	/**@brief 清理图片缓存
	 */
	virtual void ClearImageCache();

	/** 屏幕坐标转换为客户区坐标
	*/
	virtual bool ScreenToClient(UiPoint& pt);

	/** 客户区坐标转换为屏幕坐标
	*/
	virtual bool ClientToScreen(UiPoint& pt);

public:
	/** 监听控件所有事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachAllEvents(const EventCallback& callback)	{ AttachEvent(kEventAll, callback); }

	/** 监听鼠标进入事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachMouseEnter(const EventCallback& callback) { AttachEvent(kEventMouseEnter, callback); }

	/** 监听鼠标离开事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachMouseLeave(const EventCallback& callback) { AttachEvent(kEventMouseLeave, callback); }

	/** 监听鼠标悬浮事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachMouseHover(const EventCallback& callback) { AttachEvent(kEventMouseHover, callback); }

	/** 监听鼠标移动事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachMouseMove(const EventCallback& callback) { AttachEvent(kEventMouseMove, callback); }

	/** 监听鼠标按下事件(左键)
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachButtonDown(const EventCallback& callback) { AttachEvent(kEventMouseButtonDown, callback); }

	/** 监听鼠标弹起事件(左键)
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachButtonUp(const EventCallback& callback) { AttachEvent(kEventMouseButtonUp, callback); }

	/** 监听鼠标按下事件(右键)
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachRButtonDown(const EventCallback& callback) { AttachEvent(kEventMouseRButtonDown, callback); }

	/** 监听鼠标弹起事件(右键)
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachRButtonUp(const EventCallback& callback) { AttachEvent(kEventMouseRButtonUp, callback); }

	/** 监听获得焦点事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachSetFocus(const EventCallback& callback) { AttachEvent(kEventSetFocus, callback); }

	/** 监听失去焦点事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachKillFocus(const EventCallback& callback) { AttachEvent(kEventKillFocus, callback); }

	/** 监听窗口失去焦点事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachWindowKillFocus(const EventCallback& callback) { AttachEvent(kEventWindowKillFocus, callback); }

	/** 监听右键菜单事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachMenu(const EventCallback& callback) { AttachEvent(kEventMouseMenu, callback); }

	/** 监听控件大小改变事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachResize(const EventCallback& callback) { AttachEvent(kEventResize, callback); }

	/** 监听双击事件
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachDoubleClick(const EventCallback& callback) { AttachEvent(kEventMouseDoubleClick, callback); }

	/** 绑定鼠标点击处理函数
	* @param[in] callback 要绑定的回调函数
	*/
	void AttachClick(const EventCallback& callback) { AttachEvent(kEventClick, callback); }

	/** 绑定鼠标右键点击处理函数
	* @param[in] callback 要绑定的回调函数
	*/
	void AttachRClick(const EventCallback& callback) { AttachEvent(kEventRClick, callback); }

	/** 监听控件显示或隐藏事件
	* @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	*/
	void AttachVisibleChange(const EventCallback& callback) { AttachEvent(kEventVisibleChange, callback); }

	/** 监听控件状态变化事件
	* @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	*/
	void AttachStateChange(const EventCallback& callback) { AttachEvent(kEventStateChange, callback); }

	/** 监听控件关闭前最后一条消息
	* @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	*/
	void AttachLastEvent(const EventCallback& callback) { AttachEvent(kEventLast, callback); }

public:
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

	/** 获取颜色值对应的字符串, 返回该颜色对应的字符串
	* @param [in] color 颜色值
	* @return 返回颜色值对应的字符串，比如"#FF123456"
	*/
	std::wstring GetColorString(const UiColor& color) const;

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

	/**@brief (m_pOnEvent)监听指定事件
	 * @param[in] type 事件类型，见 EventType 枚举
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachEvent(EventType type, const EventCallback& callback);

	/**@brief (m_pOnEvent)取消监听指定事件
	 * @param[in] type 事件类型，见 EventType 枚举
	 */
	void DetachEvent(EventType type);

	/**@brief (m_pOnXmlEvent)通过XML中，配置<Event标签添加的响应事件，最终由Control::OnApplyAttributeList函数响应具体操作
	 * @param[in] type 事件类型，见 EventType 枚举
	 * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	 */
	void AttachXmlEvent(EventType eventType, const EventCallback& callback);

	/**@brief (m_pOnXmlEvent)取消监听指定事件
	 * @param[in] type 事件类型，见 EventType 枚举
	 */
	void DetachXmlEvent(EventType type);

	/**@brief (m_pOnBubbledEvent)绑定事件处理函数
	 * @param[in] eventType 事件类型
	 * @param[in] callback 指定回调函数
	 */
	void AttachBubbledEvent(EventType eventType, const EventCallback& callback);

	/**@brief (m_pOnBubbledEvent)解绑事件处理函数
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

	/** 触发事件，向所有容器的监听者发送事件（m_pOnEvent，m_pOnXmlEvent，m_pOnBubbledEvent， m_pOnXmlBubbledEvent）
	* @param [in] msg 消息内容
	* @return 如果所有监听者回调函数返回true，则该函数返回true；否则返回false
	*/
	bool FireAllEvents(const EventArgs& msg);

	/** @} */

protected:

	//处理放弃控件焦点相关逻辑 
	void EnsureNoFocus();

	/** 判断消息是否为应过滤掉的消息, 辅助函数
	*   如果当前控件是 !IsEnabled() || !IsMouseEnabled() || !IsKeyboardEnabled() 状态，
	    并且消息是鼠标、键盘消息，返回true，否则返回false
	*/
	bool IsDisabledEvents(const EventArgs& msg) const;

	//消息处理的保护成员函数，不允许外部直接调用

	/** 消息处理函数
	* @param [in] msg 消息内容
	*/
	virtual void HandleEvent(const EventArgs& msg);

	//鼠标消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
	virtual bool MouseEnter(const EventArgs& msg);
	virtual bool MouseLeave(const EventArgs& msg);
	virtual bool ButtonDown(const EventArgs& msg);
	virtual bool ButtonUp(const EventArgs& msg);
	virtual bool ButtonDoubleClick(const EventArgs& msg);
	virtual bool RButtonDown(const EventArgs& msg);
	virtual bool RButtonUp(const EventArgs& msg);
	virtual bool RButtonDoubleClick(const EventArgs& msg);
	virtual bool MouseMove(const EventArgs& msg);
	virtual bool MouseHover(const EventArgs& msg);
	virtual bool MouseWheel(const EventArgs& msg);
	virtual bool MouseMenu(const EventArgs& msg);

    //键盘消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
	virtual bool OnChar(const EventArgs& msg);
	virtual bool OnKeyDown(const EventArgs& msg);
	virtual bool OnKeyUp(const EventArgs& msg);
	virtual bool OnSysKeyDown(const EventArgs& msg);
	virtual bool OnSysKeyUp(const EventArgs& msg);

    //光标与焦点相关消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
	virtual bool OnSetCursor(const EventArgs& msg);
	virtual bool OnSetFocus(const EventArgs& msg);
	virtual bool OnKillFocus(const EventArgs& msg); //控件失去焦点
	virtual bool OnWindowKillFocus(const EventArgs& msg);//控件所属的窗口失去焦点
	virtual bool OnImeStartComposition(const EventArgs& msg);
	virtual bool OnImeEndComposition(const EventArgs& msg);

	/// 绘制相关保护成员函数，不允许外部直接调用
	virtual void PaintShadow(IRender* pRender);
	virtual void PaintBkColor(IRender* pRender);
	virtual void PaintBkImage(IRender* pRender);
	virtual void PaintStateColors(IRender* pRender);
	virtual void PaintStateImages(IRender* pRender);
	virtual void PaintText(IRender* pRender);
	virtual void PaintBorder(IRender* pRender);
	virtual void PaintFocusRect(IRender* pRender);
	virtual void PaintLoading(IRender* pRender);

protected:
	/** 是否状态图片, 只要含有任意状态图片，即返回true
	*/
	bool HasStateImages(void) const;

	/** 是否含有指定类型的图片
	*/
	bool HasStateImage(StateImageType stateImageType) const;

	/** 获取指定状态下的图片位置
	 */
	std::wstring GetStateImage(StateImageType imageType, ControlStateType stateType) const;

	/** 设置某个状态下的图片
	 */
	void SetStateImage(StateImageType imageType, ControlStateType stateType, const std::wstring& strImage);

	/** 绘制指定类型、指定状态的图片
	* @param [in] pRender 绘制接口
	* @param [in] stateImageType 图片类型
	* @param [in] stateType 控件状态，用于选择绘制哪个图片
	* @param [in] sImageModify 图片的附加属性
	* @param [out] pDestRect 返回图片绘制的最终目标矩形区域
	* @return 绘制成功返回true, 否则返回false
	*/
	bool PaintStateImage(IRender* pRender, 
						 StateImageType stateImageType, 
					     ControlStateType stateType, 
						 const std::wstring& sImageModify = L"",
		                 UiRect* pDestRect = nullptr);

	/** 清除所有状态图片属性
	*/
	void ClearStateImages();

	/** 设置状态图片的内边距left值(所有状态图片的内边距一起调整)
	* @param [in] leftOffset 要设置的图片内边距偏移量，正数表示增加，负数表示减少
	* @param [in] bNeedDpiScale 兼容 DPI 缩放，默认为 true
	* @return 成功返回true，失败返回false
	*/
	bool AdjustStateImagesPaddingLeft(int32_t leftOffset, bool bNeedDpiScale = true);

	/** 获取背景图片的内边距
	 */
	UiPadding GetBkImagePadding() const;

	/** 设置背景图片的内边距
	 * @param[in] rcPadding 要设置的图片内边距
	 * @param[in] bNeedDpiScale 兼容 DPI 缩放，默认为 true
	 */
	bool SetBkImagePadding(UiPadding rcPadding, bool bNeedDpiScale = true);

	/** 判断是否禁用背景图片绘制
	*/
	bool IsBkImagePaintEnabled() const;
 
	/** 设置是否禁止背景图片绘制
	*/
	void SetBkImagePaintEnabled(bool bEnable);

	/** 获取背景图片路径（不含属性）
	*/
	std::wstring GetBkImagePath() const;

	/** 获取背景图片大小(按需加载图片)
	*/
	UiSize GetBkImageSize() const;

protected:
	/** 绘制指定状态的颜色
	*/
	void PaintStateColor(IRender* pRender, const UiRect& rcPaint, ControlStateType stateType) const;

	/** @brief 获取控件的绘制区域
	*/
	const UiRect& GetPaintRect() const { return m_rcPaint; }

	/** @brief 设置控件的绘制区域
	*/
	void SetPaintRect(const UiRect& rect);

	/** 绘制焦点状态的矩形（虚线组成的矩形）
	*/
	void DoPaintFocusRect(IRender* pRender);

	/** 停止播放GIF动画(背景图片的动画等)
	*/
	void CheckStopGifPlay();

private:
	/** 绘制边框：根据条件判断绘制圆角矩形边框还是普通矩形边框
	*/
	void PaintBorders(IRender* pRender, UiRect rcDraw, 
		              int32_t nBorderSize, UiColor dwBorderColor) const;

    /** 绘制圆角矩形
	*/
	void DrawRoundRect(IRender* pRender, const UiRect& rc, const UiSize& roundSize, UiColor dwBorderColor, int32_t nBorderSize) const;

	/** 填充圆角矩形
	*/
	void FillRoundRect(IRender* pRender, const UiRect& rc, const UiSize& roundSize, UiColor dwColor) const;
  
    /** 填充路径, 形成圆角矩形
	*/
	void AddRoundRectPath(IPath* path, const UiRect& rc, UiSize roundSize) const;

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

	/** 获取指定状态下的图片大小(按需加载图片)
	*/
	UiSize GetStateImageSize(StateImageType imageType, ControlStateType stateType);

	/** 设置光标
	* @param [in] cursorType 光标类型
	*/
	void SetCursor(CursorType cursorType);

	/** 设置绘制顺序
	* @param [in] nPaintOrder 0 表示常规绘制，非0表示指定绘制顺序，值越大表示越晚绘制
	*/
	void SetPaintOrder(uint8_t nPaintOrder);

	/** 获取绘制顺序
	*/
	uint8_t GetPaintOrder() const;

private:

	/** 获取颜色名称对应的颜色值
	*/
	UiColor GetUiColorByName(const std::wstring& colorName) const;

	/** 是否含有BoxShadow
	*/
	bool HasBoxShadow() const;

	/** 设置控件状态的值，并触发状态变化事件
	 * @param[in] controlState 要设置的控件状态，请参考 `ControlStateType` 枚举
	 */
	void PrivateSetState(ControlStateType controlState);

private:
	/** 边框圆角大小(与m_rcBorderSize联合应用)或者阴影的圆角大小(与m_boxShadow联合应用)
	    仅当 m_rcBorderSize 四个边框值都有效, 并且都相同时
	*/
	UiSize m_cxyBorderRound;

	//控件阴影，其圆角大小通过m_cxyBorderRound变量控制
	BoxShadow* m_pBoxShadow;

	/** 焦点状态下的边框颜色
	*/
	UiString m_focusBorderColor;

	/** 边框颜色, 每个状态可以指定不同的边框颜色
	*/
	std::unique_ptr<StateColorMap> m_pBorderColorMap;

	//控件四边的边框大小（可分别设置top/bottom/left/right四个边的值）
	UiRect m_rcBorderSize;

private:
	//控件的背景颜色
	UiString m_strBkColor;

	//控件的背景图片
	std::shared_ptr<Image> m_pBkImage;

private:
	/** 控件状态(ControlStateType)
	*/
	int8_t m_controlState;

	/** 状态与颜色值MAP，每个状态可以指定不同的颜色
	*/
	std::unique_ptr<StateColorMap> m_pColorMap;

	/** 控件图片类型与状态图片的MAP
	*/
	std::unique_ptr<StateImageMap> m_pImageMap;

private:
	/** 控件"加载中"逻辑的实现接口
	*/
	ControlLoading* m_pLoading;

private:
	//控件动画播放管理器
	std::unique_ptr<AnimationManager> m_animationManager;

	//控件播放动画时的渲染偏移(X坐标偏移和Y坐标偏移)
	UiPoint m_renderOffset;
	
	//控件的透明度（0 - 255，0为完全透明，255为不透明）
	uint8_t m_nAlpha;

	//控件为Hot状态时的透明度（0 - 255，0为完全透明，255为不透明）
	uint8_t m_nHotAlpha;

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
	uint16_t m_nTooltipWidth;

	//ToolTip的文本内容
	UiString m_sToolTipText;

	//ToolTip的文本ID
	UiString m_sToolTipTextId;

private:
	
	//用户数据ID(字符串)
	UiString m_sUserDataID;

	//用户数据ID(整型值)
	size_t m_uUserDataID;

private:
	//通过AttachXXX接口，添加的监听事件
	EventMap* m_pOnEvent;

	//通过XML中，配置<Event标签添加的响应事件，最终由Control::OnApplyAttributeList函数响应具体操作
	EventMap* m_pOnXmlEvent;

	//通过AttachBubbledEvent接口添加的事件
	EventMap* m_pOnBubbledEvent;

	//通过XML中，配置<BubbledEvent标签添加的响应事件，最终由Control::OnApplyAttributeList函数响应具体操作
	EventMap* m_pOnXmlBubbledEvent;

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

	//控件的光标类型(CursorType)
	int8_t m_cursorType;

	//是否显示焦点状态(一个虚线构成的矩形)
	bool m_bShowFocusRect;

	//焦点状态虚线矩形的颜色
	UiString m_focusRectColor;

	//绘制顺序: 0 表示常规绘制，非0表示指定绘制顺序，值越大表示绘制越晚绘制
	uint8_t m_nPaintOrder;
};

} // namespace ui

#endif // UI_CORE_CONTROL_H_
