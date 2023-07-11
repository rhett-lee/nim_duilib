#ifndef UI_CORE_SCROLLBOX_H_
#define UI_CORE_SCROLLBOX_H_

#pragma once

#include "duilib/Core/Box.h"
#include "duilib/Control/ScrollBar.h"
#include "duilib/Animation/AnimationPlayer.h"

namespace ui 
{

 /// 带有垂直或水平滚动条的容器，使容器可以容纳更多内容
class UILIB_API ScrollBox : public Box
{
public:
	explicit ScrollBox(Layout* pLayout);
	ScrollBox(const ScrollBox& r) = delete;
	ScrollBox& operator=(const ScrollBox& r) = delete;

	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& pstrName, const std::wstring& pstrValue) override;
	virtual void SetPos(UiRect rc) override;
	virtual void HandleEvent(const EventArgs& event) override;
	virtual bool MouseEnter(const EventArgs& msg) override;
	virtual bool MouseLeave(const EventArgs& msg) override;
	virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;
	virtual void SetMouseEnabled(bool bEnable = true) override;
	virtual void SetWindow(Window* pManager, Box* pParent, bool bInit) override;
	virtual Control* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, UiPoint scrollPos = UiPoint()) override;
	virtual void ClearImageCache() override;

	/**
	 * @brief 获取滚动条位置
	 * @return 返回滚动条的位置信息
	 */
	virtual UiSize GetScrollPos() const;

	/**
	 * @brief 获取滚动条的范围
	 * @return 返回滚动条的范围信息
	 */
	virtual UiSize GetScrollRange() const;

	/**
	 * @brief 设置滚动条位置
	 * @param[in] szPos 要设置的位置数据
	 * @return 无
	 */
	virtual void SetScrollPos(UiSize szPos);

	/**
	 * @brief 设置滚动条 Y 轴坐标
	 * @param[in] y 要设置的 Y 轴坐标数值
	 * @return 无
	 */
	virtual void SetScrollPosY(int y);
	
	/**
	 * @brief 设置滚动条 X 轴坐标
	 * @param[in] x 要设置的 X 轴坐标数值
	 * @return 无
	 */
    virtual void SetScrollPosX(int x);
	
	/**
	 * @brief 向上滚动滚动条
	 * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
 	 * @param[in] withAnimation 是否附带动画效果，默认为 true
	 * @return 无
	 */
	virtual void LineUp(int detaValue = DUI_NOSET_VALUE, bool withAnimation = true);
	
	/**
	 * @brief 向下滚动滚动条
	 * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
 	 * @param[in] withAnimation 是否附带动画效果，默认为 true
	 * @return 无
	 */
	virtual void LineDown(int detaValue = DUI_NOSET_VALUE, bool withAnimation = true);
	
	/**
	 * @brief 向左滚动滚动条
	 * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
	 * @return 无
	 */
    virtual void LineLeft(int detaValue = DUI_NOSET_VALUE);
	
	/**
	 * @brief 向右滚动滚动条
	 * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
	 * @return 无
	 */
    virtual void LineRight(int detaValue = DUI_NOSET_VALUE);
	/**
	 * @brief 向上滚动一个页面大小的距离
	 * @return 无
	 */
	virtual void PageUp();

	/**
	 * @brief 向下滚动一个页面大小的距离
	 * @return 无
	 */
	virtual void PageDown();

	/**
	 * @brief 回到滚动条最上方
	 * @return 无
	 */
	virtual void HomeUp();

	/**
	 * @brief 滚动到最下方位置
	 * @param[in] arrange 是否重置滚动条位置，默认为 true
	 * @param[in] withAnimation 是否包含动画特效，默认为 true
	 * @return 无
	 */
	virtual void EndDown(bool arrange = true, bool withAnimation = true);
	/**
	 * @brief 向左滚动一个页面大小的距离
	 * @return 无
	 */
	virtual void PageLeft();

	/**
	 * @brief 向右滚动一个页面大小的距离
	 * @return 无
	 */
	virtual void PageRight();

	/**
	 * @brief 滚动到最左侧
	 * @return 无
	 */
	virtual void HomeLeft();

	/**
	 * @brief 滚动到最右侧
	 * @return 无
	 */
	virtual void EndRight();

	/**
	 * @brief 触摸向上滚动（响应 WM_TOUCH 消息）
	 * @param[in] deltaValue 滚动距离
	 * @return 无
	 */
	virtual void TouchUp(int deltaValue);

	/**
	 * @brief 触摸向下滚动（响应 WM_TOUCH 消息）
	 * @param[in] deltaValue 滚动距离
	 * @return 无
	 */
	virtual void TouchDown(int deltaValue);

	/**
	 * @brief 启用滚动条
	 * @param[in] bEnableVertical 是否启用垂直滚动条，默认为 true
	 * @param[in] bEnableHorizontal 是否启用水平滚动条，默认为 true
	 * @return 无
	 */
	virtual void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);

	/**
	 * @brief 获取垂直滚动条对象指针
	 * @return 返回垂直滚动条对象指针
	 */
	virtual ScrollBar* GetVerticalScrollBar() const;

	/**
	 * @brief 获取水平滚动条对象指针
	 * @return 返回水平滚动条对象指针
	 */
	virtual ScrollBar* GetHorizontalScrollBar() const;

	/**
	 * @brief 待补充
	 * @param[in] 待补充
	 * @return 待补充
	 */
	virtual void ProcessVScrollBar(UiRect rc, int cyRequired);

	/**
	 * @brief 待补充
	 * @param[in] 待补充
	 * @return 待补充
	 */
	virtual void ProcessHScrollBar(UiRect rc, int cxRequired);

	/**
	 * @brief 判断垂直滚动条是否有效
	 * @return 返回 true 表示有效，否则 false 为无效
	 */
	bool IsVScrollBarValid() const;

	/**
	 * @brief 判断水平滚动条是否有效
	 * @return 返回 true 表示有效，否则 false 为无效
	 */
	bool IsHScrollBarValid() const;

	/**
	 * @brief 待补充
	 * @param[in] 待补充
	 * @return 待补充
	 */
	void ReomveLastItemAnimation();

	/**
	 * @brief 待补充
	 * @param[in] 待补充
	 * @return 待补充
	 */
	void PlayRenderOffsetYAnimation(int nRenderY);

	/**
	 * @brief 是否已经在底部
	 * @return 返回 true 表示已经在底部，否则为 false
	 */
	bool IsAtEnd() const;

	/**
	 * @brief 是否锁定到底部
	 * @return 回 true 表示锁定在底部，否则为 false
	 */
	bool IsHoldEnd() const;

	/**
	 * @brief 设置滚动条是否始终锁定到底部位置
	 * @param[in] bHoldEnd 设置 true 表示锁定，false 为不锁定
	 * @return 无
	 */
	void SetHoldEnd(bool bHoldEnd);
	
	/**
	 * @brief 获取垂直滚动条滚动步长
	 * @return 返回滚动步长
	 */
	int GetVerScrollUnitPixels() const;

	/**
	 * @brief 设置垂直滚动条滚动步长
	 * @param[in] nUnitPixels 要设置的步长
	 * @return 无
	 */
	void SetVerScrollUnitPixels(int nUnitPixels);

	/**
	* @brief 获取横向滚动条滚动步长
	* @return 返回滚动步长
	*/
	int GetHorScrollUnitPixels() const;

	/**
	* @brief 设置横向滚动条滚动步长
	* @param[in] nUnitPixels 要设置的步长
	* @return 无
	*/
	void SetHorScrollUnitPixels(int nUnitPixels);

	/**
	 * @brief 获取容器的滚动条是否悬浮在子控件上面
	 * @return 返回 true 表示悬浮在滚动条上，否则为 false
	 */
	bool GetScrollBarFloat() const;

	/**
	 * @brief 设置容器的滚动条是否悬浮在子控件上面
	 * @param[in] bScrollBarFloat true 表示悬浮在滚动条上，false 表示不悬浮在控件上
	 * @return 无
	 */
	void SetScrollBarFloat(bool bScrollBarFloat);

	/**
	 * @brief 获取容器的滚动条是否在左侧显示
	 * @return 返回 true 表示在左侧，false 为右侧
	 */
	bool GetVScrollBarLeftPos() const;

	/**
	* @brief 设置容器的滚动条是否在左侧显示
	* @param[in] bLeftPos true 表示在左侧，false 为右侧
	* @return 无
	*/
	void SetVScrollBarLeftPos(bool bLeftPos);

	/**
	 * @brief 获取滚动条的外边距
	 * @return 返回边距信息 
	 */
	UiRect GetScrollBarPadding() const;

	/**
	 * @brief 设置滚动条的外边距，可以让滚动条不占满容器
	 * @param[in] rcScrollBarPadding 要设置的边距
	 * @return 无
	 */
	void SetScrollBarPadding(UiRect rcScrollBarPadding);

	/**
	 * @brief 监听滚动条位置变化事件
	 * @param[in] callback 有变化后通知的回调函数
	 * @return 无
	 */
	void AttachScrollChange(const EventCallback& callback) { AttachEvent(kEventScrollChange, callback); }

protected:

	/**
	 * @brief 计算所需的尺寸
	 * @param[in] rc 当前位置信息
	 * @return 返回所需尺寸大小
	 */
	virtual UiSize CalcRequiredSize(const UiRect& rc);

	/**
	 * @brief 加载图片缓存，仅供 ScrollBox 内部使用
	 * @param[in] bFromTopLeft 暂无意义
	 * @return 无
	 */
	virtual void LoadImageCache(bool bFromTopLeft);

private:
	/**@brief 设置位置大小
	 */
	void SetPosInternally(UiRect rc);

protected:
	//垂直滚动条接口
	std::unique_ptr<ScrollBar> m_pVerticalScrollBar;

	//水平滚动条接口
	std::unique_ptr<ScrollBar> m_pHorizontalScrollBar;

	//垂直滚动条滚动步长
	int m_nVerScrollUnitPixels;

	//垂直滚动条滚动步长(默认)
	const int m_nVerScrollUnitPixelsDefault = 30;

	//水平滚动条滚动步长
    int m_nHerScrollUnitPixels;

	//水平滚动条滚动步长(默认)
	int m_nHerScrollUnitPixelsDefault = 30;

	// 防止SetPos循环调用
	bool m_bScrollProcess; 

	//是否锁定到底部
	bool m_bHoldEnd;

	//容器的滚动条是否悬浮在子控件上面
	bool m_bScrollBarFloat;

	//容器的滚动条是否在左侧显示
	bool m_bVScrollBarLeftPos;

	//滚动条的外边距
	UiRect m_rcScrollBarPadding;

	//滚动条动画效果支持
	std::unique_ptr<AnimationPlayer> m_scrollAnimation;

	//滚动条动画效果支持
	std::unique_ptr<AnimationPlayer> m_renderOffsetYAnimation;
};

} // namespace ui

#endif // UI_CORE_SCROLLBOX_H_
