#ifndef UI_CORE_SCROLLBAR_H_
#define UI_CORE_SCROLLBAR_H_

#pragma once

#include "duilib/Core/Control.h"
#include "base/callback/callback.h"

namespace ui
{
	class ScrollBox;
	class StateImage;

/** 滚动条控件
*/
class UILIB_API ScrollBar: 
	public Control
{
public:
	ScrollBar();
	ScrollBar(const ScrollBar& r) = delete;
	ScrollBar& operator=(const ScrollBar& r) = delete;

	ScrollBox* GetOwner() const;
	void SetOwner(ScrollBox* pOwner);

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void SetEnabled(bool bEnable = true) override;
	virtual void SetFocus() override;
	virtual void SetVisible(bool bVisible) override;
	virtual bool ButtonUp(const EventArgs& msg) override;
	virtual bool HasHotState() override;
	virtual bool MouseEnter(const EventArgs& msg) override;
	virtual bool MouseLeave(const EventArgs& msg) override;

	virtual void SetPos(UiRect rc) override;
	virtual void HandleEvent(const EventArgs& msg) override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;
	virtual void ClearImageCache() override;

	/**
	 * @brief 是否是水平滚动条
	 * @return 返回 true 代表是水平滚动条，否则为 false 代表是垂直滚动条
	 */
	bool IsHorizontal();

	/**
	 * @brief 设置滚动条方向
	 * @param[in] 设置 true 代表是水平滚动条，设置 false 代表是垂直滚动条，默认为 true
	 * @return 无
	 */
	void SetHorizontal(bool bHorizontal);

	/**
	 * @brief 获取滚动范围，对应 XML 中 range 属性
	 * @return 返回滚动范围
	 */
	int64_t GetScrollRange() const;

	/**
	 * @brief 是设置滚动范围
	 * @param[in] nRange 要设置的滚动范围
	 * @return 无
	 */
	void SetScrollRange(int64_t nRange);

	/**
	 * @brief 判断是否有效
	 * @return true 为有效，否则为 false
	 */
	bool IsValid() { return GetScrollRange() != 0; }

	/**
	 * @brief 获取滚动条位置
	 * @return 返回滚动条位置
	 */
	int64_t GetScrollPos() const;

	/**
	 * @brief 设置滚动条位置
	 * @param[in] nPos 要设置的位置信息
	 * @return 无
	 */
	void SetScrollPos(int64_t nPos);

	/**
	 * @brief 获取滚动一行的大小
	 * @return 返回滚动一行的大小
	 */
	int GetLineSize() const;

	/**
	 * @brief 设置滚动一行的大小
	 * @param[in] nSize 要设置的大小数值
	 * @return 无
	 */
	void SetLineSize(int nSize);

	/**
	 * @brief 获取滑块的最小长度
	 * @return 返回滑块的最小长度
	 */
	int GetThumbMinLength() const;

	/**
	 * @brief 设置滑块的最小长度
	 * @param[in] nThumbMinLength 要设置的最小长度
	 * @return 无
	 */
	void SetThumbMinLength(int nThumbMinLength);

	/**
	 * @brief 是否显示左或上按钮
	 * @return 返回 true 为显示，false 为不显示
	 */
	bool IsShowButton1();

	/**
	 * @brief 设置是否显示左或上按钮
	 * @param[in] bShow 设置 true 为显示，false 为不显示
	 * @return 无
	 */
	void SetShowButton1(bool bShow);

	/**
	 * @brief 获取指定状态下的向左或向上按钮图片
	 * @param[in] stateType 要获取的状态，参考 ControlStateType 枚举
	 * @return 返回图片位置
	 */
	std::wstring GetButton1StateImage(ControlStateType stateType);

	/**
	 * @brief 设置指定状态下的向左或向上按钮图片
	 * @param[in] stateType 要设置哪种状态下的图片
	 * @param[in] pStrImage 图片位置
	 * @return 无
	 */
	void SetButton1StateImage(ControlStateType stateType, const std::wstring& pStrImage);

	/**
	 * @brief 是否显示右或下按钮
	 * @return 返回 true 为显示，false 为不显示
	 */
	bool IsShowButton2();

	/**
	 * @brief 设置是否显示右或下按钮
	 * @param[in] bShow 设置 true 为显示，false 为不显示
	 * @return 无
	 */
	void SetShowButton2(bool bShow);

	/**
	 * @brief 获取指定状态下的向右或向下按钮图片
	 * @param[in] stateType 要获取的状态，参考 ControlStateType 枚举
	 * @return 返回图片位置
	 */
	std::wstring GetButton2StateImage(ControlStateType stateType);

	/**
	 * @brief 设置指定状态下的向右或向下按钮图片
	 * @param[in] stateType 要设置哪种状态下的图片
	 * @param[in] pStrImage 图片位置
	 * @return 无
	 */
	void SetButton2StateImage(ControlStateType stateType, const std::wstring& pStrImage);

	/**
	 * @brief 获取滑块指定状态下的图片
	 * @param[in] stateType 要获取的状态标识，参考 ControlStateType 枚举
	 * @return 返回图片位置
	 */
	std::wstring GetThumbStateImage(ControlStateType stateType);

	/**
	 * @brief 设置滑块指定状态下的图片
	 * @param[in] stateType 要设置的状态标识，参考 ControlStateType 枚举
	 * @param[in] pStrImage 图片位置
	 * @return 无
	 */
	void SetThumbStateImage(ControlStateType stateType, const std::wstring& pStrImage);

	/**
	 * @brief 获取指定状态下滑块中间标识图片
	 * @param[in] stateType 要获取的状态标识，参考 ControlStateType 枚举
	 * @return 返回图片位置
	 */
	std::wstring GetRailStateImage(ControlStateType stateType);

	/**
	 * @brief 设置指定状态下滑块中间标识图片
	 * @param[in] stateType 要设置的状态标识，参考 ControlStateType 枚举
	 * @param[in] pStrImage 图片位置
	 * @return 无
	 */
	void SetRailStateImage(ControlStateType stateType, const std::wstring& pStrImage);

	/**
	 * @brief 获取指定状态下的背景图片
	 * @param[in] stateType 要获取的状态标识，参考 ControlStateType 枚举
	 * @return 返回图片位置
	 */
	std::wstring GetBkStateImage(ControlStateType stateType);

	/**
	 * @brief 设置指定状态下的背景图片
	 * @param[in] stateType 要设置的状态标识，参考 ControlStateType 枚举
	 * @param[in] pStrImage 图片位置
	 * @return 无
	 */
	void SetBkStateImage(ControlStateType stateType, const std::wstring& pStrImage);

	/**
	 * @brief 是否自动隐藏滚动条
	 * @return 返回 true 为是，否则为 false
	 */
	bool IsAutoHideScroll(){return m_bAutoHide;}

	/**
	 * @brief 设置是否自动隐藏滚动条
	 * @param[in] hide 设置为 true 为自动隐藏，false 为不自动隐藏
	 * @return 无
	 */
	void SetAutoHideScroll(bool hide);

	/**
	 * @brief 获取滑块状态
	 * @return 返回滑块状态，参考 ControlStateType 枚举
	 */
	ControlStateType GetThumbState() { return m_uThumbState; }

private:
	void ScrollTimeHandle();

	/// 绘制相关函数
	void PaintBk(IRender* pRender);
	void PaintButton1(IRender* pRender);
	void PaintButton2(IRender* pRender);
	void PaintThumb(IRender* pRender);
	void PaintRail(IRender* pRender);

	/** 获取一次快速滚动的偏移量
	*/
	int64_t CalcFastScrollOffset(int32_t posOffset) const;

private:
	enum
	{ 
		DEFAULT_SCROLLBAR_SIZE = 16,
	};

	//是否为水平滚动条：true表示水平滚动条，false表示垂直滚动条
	bool m_bHorizontal;

	//是否显示左或上按钮
	bool m_bShowButton1;

	//是否显示右或下按钮
	bool m_bShowButton2;

	//是否自动隐藏滚动条
	bool m_bAutoHide;

	//滚动范围
	int64_t m_nRange;

	//滚动条位置
	int64_t m_nScrollPos;

	//滚动一行的大小
	int m_nLineSize;

	//滚动条滑块的最小长度
	int m_nThumbMinLength;

	//上次的滚动条位置
	int64_t m_nLastScrollPos;

	//上次滚动条位置偏移
	int64_t m_nLastScrollOffset;

	//延迟滚动重复次数
	int m_nScrollRepeatDelay;
	
	//上次鼠标所在位置
	UiPoint m_ptLastMouse;

	//左或上按钮的位置和大小
	UiRect m_rcButton1;

	//右或下按钮的位置和大小
	UiRect m_rcButton2;

	//滑块的位置和大小
	UiRect m_rcThumb;

	//左或上按钮的状态
	ControlStateType m_uButton1State;

	//右或下按钮的状态
	ControlStateType m_uButton2State;

	//滑块的状态
	ControlStateType m_uThumbState;

	//图片的目标区域，绘制用
	std::wstring m_sImageModify;

	//背景各个状态的图片
	std::unique_ptr<StateImage> m_bkStateImage;

	//左或上按钮的状态图片
	std::unique_ptr<StateImage> m_button1StateImage;

	//右或下按钮的状态图片
	std::unique_ptr<StateImage> m_button2StateImage;

	//滑块的状态图片
	std::unique_ptr<StateImage> m_thumbStateImage;

	//指定状态下滑块中间标识图片
	std::unique_ptr<StateImage> m_railStateImage;

	//延迟滚动的取消机制
	nbase::WeakCallbackFlag m_weakFlagOwner;

	//支持滚动条的容器接口
	ScrollBox* m_pOwner;
};

}//namespace ui

#endif // UI_CORE_SCROLLBAR_H_
