#ifndef UI_CORE_BOX_H_
#define UI_CORE_BOX_H_

#pragma once

#include "duilib/Box/Layout.h"
#include "duilib/Core/Control.h"

namespace ui 
{

/////////////////////////////////////////////////////////////////////////////////////
//
class UILIB_API Box : public Control
{
public:
	explicit Box(Layout* pLayout = new Layout());
	Box(const Box& r) = delete;
	Box& operator=(const Box& r) = delete;
    virtual ~Box();

public:
	/** 无效的子项索引
	*/
	static constexpr auto InvalidIndex{ static_cast<size_t>(-1) };

	/** @brief 是否为有效的子控件索引
	*/
	static bool IsValidItemIndex(size_t index) { return index != Box::InvalidIndex; }

public:
	/// 重写父类接口，提供个性化功能。方法具体说明请查看 Control 控件
	virtual std::wstring GetType() const override;
	virtual void SetParent(Box* pParent) override;
	virtual void SetWindow(Window* pManager) override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;
	virtual void PaintFocusRect(IRender* pRender);
	virtual void SetEnabled(bool bEnabled) override;
	virtual void SetVisible(bool bVisible) override;
	virtual Control* FindControl(FINDCONTROLPROC Proc, LPVOID pProcData, uint32_t uFlags, 
								 const UiPoint& ptMouse = UiPoint(),
								 const UiPoint& scrollPos = UiPoint()) override;
	virtual void ClearImageCache() override;
	virtual uint32_t GetControlFlags() const override;

	/** 设置控件位置（子类可改变行为）
	 * @param [in] rc 要设置的矩形区域信息，包含内边距，不包含外边距
	 */
	virtual void SetPos(UiRect rc) override;

	/** 获取控件实际可用矩形区域
	*@return 返回控件的实际可用区域，即GetPos剪去Padding后的区域
	*/
	virtual UiRect GetPosWithoutPadding() const;

	/** 获取控件实际可用矩形区域
	*@return 返回控件的实际可用区域，即GetRect剪去Padding后的区域
	*/
	UiRect GetRectWithoutPadding() const;

	/** 计算控件大小(宽和高)
		如果设置了图片并设置 width 或 height 任意一项为 auto，将根据图片大小和文本大小来计算最终大小
	 *  @param [in] szAvailable 可用大小，不包含外边距
	 *  @return 控件的估算大小，包含内边距(Box)，不包含外边距
	 */
	virtual UiEstSize EstimateSize(UiSize szAvailable) override;

public:
	/** @name 操作子控件(item)相关的方法
	* @{
	*/
	/** 获取子控件数量
	 */
	virtual size_t GetItemCount() const;

	/** 根据索引查找指定控件
	 * @param[in] iIndex 控件索引
	 */
	virtual Control* GetItemAt(size_t iIndex) const;

	/** 根据控件指针获取索引
	 * @param[in] pControl 控件指针
	 * @return 返回 pControl 所指向的控件索引, 如果没找到，则返回 Box::InvalidIndex
	 */
	virtual size_t GetItemIndex(Control* pControl) const;

	/** 设置控件索引（内部会重新排列控件位置）
	 * @param[in] pControl 控件指针
	 * @param[in] iIndex 要设置的索引值
	 * @return 返回 true 设置成功，false 设置失败
	 */
	virtual bool SetItemIndex(Control* pControl, size_t iIndex);

	/** 添加一个控件到容器中
	 * @param[in] pControl 控件指针
	 * @return 返回 true 添加成功，false 为添加失败
	 */
	virtual bool AddItem(Control* pControl);

	/** 向指定位置添加一个控件
	 * @param[in] pControl 控件指针
	 * @param[in] iIndex 在该索引之后插入控件
	 * @return 返回 true 为添加成功，false 为添加失败
	 */
	virtual bool AddItemAt(Control* pControl, size_t iIndex);

	/** 根据控件指针从容器中移除一个控件
	 * @param[in] pControl 控件的指针
	 * @return 返回 true 为移除成功，false 为移除失败（控件可能不存在）
	 */
	virtual bool RemoveItem(Control* pControl);

	/** 根据控件索引从容器中移除一个控件
	 * @param[in] iIndex 要移除的控件索引
	 * @return 返回 true 为移除成功，false 为移除失败（索引值太小或超出了子控件总数）
	 */
	virtual bool RemoveItemAt(size_t iIndex);

	/** 移除所有子控件
	 */
	virtual void RemoveAllItems();

	/** @} */

public:
	/** 查找下一个可选控件的索引（面向 list、combo）
	 * @param[in] iIndex 指定要起始查找的索引
	 * @param[in] bForward true 为递增查找， false 为递减查找
	 * @return 下一个可选控件的索引，返回 Box::InvalidIndex 为没有可选控件
	 */
	size_t FindSelectable(size_t iIndex, bool bForward = true) const;

	/**
	 * @brief 查找指定子控件
	 * @param[in] pstrSubControlName 子控件名称
	 * @return 返回子控件指针
	 */
	Control* FindSubControl(const std::wstring& pstrSubControlName);

	/**
	 * @brief 判断是否自动销毁
	 * @return true 为自动销毁，false 为不自动销毁
	 */
    bool IsAutoDestroyChild() const { return m_bAutoDestroyChild; }

	/**
	 * @brief 设置控件是否自动销毁
	 * @param[in] bAuto true 为自动销毁，false 为不自动销毁
	 * @return 无
	 */
    void SetAutoDestroyChild(bool bAuto) { m_bAutoDestroyChild = bAuto; }

	/**
	 * @brief 判断窗口关闭后是否自动销毁
	 * @return true 为自动销毁，false 为不自动销毁
	 */
    bool IsDelayedDestroy() const { return m_bDelayedDestroy; }

	/**
	 * @brief 设置窗口关闭后是否自动销毁
	 * @param[in] bDelayedDestroy true 为自动销毁，false 为不自动销毁
	 * @return 无
	 */
	void SetDelayedDestroy(bool bDelayedDestroy) { m_bDelayedDestroy = bDelayedDestroy; }

	/**
	 * @brief 获取容器是否响应鼠标操作
	 * @return true 为响应，false 为不响应
	 */
    bool IsMouseChildEnabled() const { return m_bMouseChildEnabled; }

	/**
	 * @brief 设置容器响应鼠标操作
	 * @param[in] bEnable 设置为 true 为响应鼠标操作，设置为 false 为不响应，默认为 true
	 */
	void SetMouseChildEnabled(bool bEnable) { m_bMouseChildEnabled = bEnable; }

	/**
	 * @brief 获取容器布局对象指针
	 * @return 返回容器关联的布局对象指针
	 */
	Layout* GetLayout() const { return m_pLayout; }

	/**
	 * @brief 重新关联布局对象
	 * @param[in] pLayout 布局对象指针
	 * @return 无
	 */
	void ReSetLayout(Layout* pLayout);

public:
	/** 设置是否支持拖拽投放进入该容器: 如果不等于0，支持拖入，否则不支持拖入(从DragOutId==DropInId的容器拖入到该容器)
	*/
	void SetDropInId(uint8_t nDropInId);

	/** 获取是否支持拖拽投放进入该容器: 如果不等于0，支持拖入，否则不支持拖入
	*/
	uint8_t GetDropInId() const;

	/** 设置是否支持拖拽拖出该容器：如果不等于0，支持拖出，否则不支持拖出（拖出到DropInId==DragOutId的容器）
	*/
	void SetDragOutId(uint8_t nDragOutId);

	/** 获取是否支持拖拽拖出该容器：如果不等于0，支持拖出，否则不支持拖出
	*/
	uint8_t GetDragOutId() const;

protected:

	/** 查找控件, 子控件列表由外部传入
	*/
	Control* FindControlInItems(const std::vector<Control*>& items, 
								FINDCONTROLPROC Proc, LPVOID pProcData,
								uint32_t uFlags, 
							    const UiPoint& ptMouse, 
							    const UiPoint& scrollPos);

private:
	/**@brief 向指定位置添加一个控件
	 * @param[in] pControl 控件指针
	 * @param[in] iIndex 在该索引之后插入控件
	 */
	bool DoAddItemAt(Control* pControl, size_t iIndex);

	/**@brief 根据控件指针从容器中移除一个控件
	 * @param[in] pControl 控件的指针
	 */
	bool DoRemoveItem(Control* pControl);

protected:

	//容器中的子控件列表
	std::vector<Control*> m_items;

private:

	//是否自动删除item的对象（如果为true：在从m_items移除元素时，会delete掉这个对象；如果为false，不delete）
	bool m_bAutoDestroyChild;

	//是否延迟删除item对象，如果为true，则元素移除后，会放到Window对象中，延迟delete这个对象，仅当m_bAutoDestroyChild时有效
	bool m_bDelayedDestroy;
	
	//布局管理接口
	Layout* m_pLayout;

	//是否允许响应子控件的鼠标消息
	bool m_bMouseChildEnabled;

	//是否支持拖拽投放进入该容器: 如果不等于0，支持拖入，否则不支持拖入(从DragOutId==DropInId的容器拖入到该容器)
	uint8_t m_nDropInId;

	//是否支持拖拽拖出该容器：如果不等于0，支持拖出，否则不支持拖出（拖出到DropInId==DragOutId的容器）
	uint8_t m_nDragOutId;
};

} // namespace ui

#endif // UI_CORE_BOX_H_
