#ifndef UI_CORE_PLACE_HOLDER_H_
#define UI_CORE_PLACE_HOLDER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "duilib/Core/Define.h"
#include "base/callback/callback.h"
#include <string>
#include <memory>

namespace ui 
{
	class Box;
	class Window;

class UILIB_API PlaceHolder : public virtual nbase::SupportWeakCallback
{
public:
	PlaceHolder();
	PlaceHolder(const PlaceHolder& r) = delete;
	PlaceHolder& operator=(const PlaceHolder& r) = delete;
	virtual ~PlaceHolder();

	/**@brief 获取父容器指针
	 */
	Box* GetParent() const { return m_pParent;}

	/**@brief 根据名称获取祖先容器指针
	 * @param[in] strName 要获取的祖先容器名称
	 */
	Box* GetAncestor(const std::wstring& strName);

	/**@brief 获取控件名称，对应 xml 中 name 属性
	 */
	const std::wstring& GetName() const { return m_sName; }

	/**@brief 获取控件名称，对应 xml 中 name 属性
	 * @return 返回控件名称（UTF8 编码）
	 */
	std::string GetUTF8Name() const;

	/**@brief 设置控件名称，内存中设置不会写入 xml 中
	 * @param[in] strName 要设置的名称
	 */
	void SetName(const std::wstring& strName);

	/**@brief 设置控件名称，内存中设置不会写入 xml 中（UTF8 编码）
	 * @param[in] strName 要设置的名称
	 */
	void SetUTF8Name(const std::string& strName);

	/**@brief 获取关联的窗口指针
	 * @return 返回关联窗口的指针
	 */
	virtual Window* GetWindow() const { return m_pWindow; }

	/**@brief 设置容器所属窗口
	 * @param[in] pManager 窗口指针
	 * @param[in] pParent 父容器
	 * @param[in] bInit 设置后是否调用 Init 初始化控件
	 */
	virtual void SetWindow(Window* pManager, Box* pParent, bool bInit);

	/**@brief 设置容器所属窗口
	 * @param[in] pManager 窗口指针
	 */
	virtual void SetWindow(Window* pManager);

	/**@brief 初始化函数
	 */
	virtual void Init();

	/**@brief 由 Init 调用，功能与 Init 相同
	 */
	virtual void DoInit();

	/**@brief 设置该控件是否可见
	 */
	virtual void SetVisible(bool bVisible);

	/**@brief 判断是否可见
	 */
	virtual bool IsVisible() const { return m_bVisible; }

	/**@brief 判断控件是否浮动状态，对应 xml 中 float 属性
	 */
	bool IsFloat() const { return m_bFloat; }

	/**@brief 设置控件是否浮动
	 * @param[in] bFloat 设置为 true 为浮动，false 为不浮动
	 */
	void SetFloat(bool bFloat);

	/**@brief 获取固定宽度，对应 xml 中 width 属性
	 */
	int GetFixedWidth() const { return m_cxyFixed.cx; }

	/**@brief 设置控件固定宽度
	 * @param[in] cx 要设置的宽度
	 * @param[in] bArrange 是否重新排列，默认为 true
	 * @param[in] bNeedDpiScale 兼容 DPI 缩放，默认为 true
	 */
	void SetFixedWidth(int cx, bool bArrange, bool bNeedDpiScale);

	/**@brief 获取固定高度
	 */
	int GetFixedHeight() const { return m_cxyFixed.cy; }

	/**@brief 设置固定高度
	 * @param[in] cy 要设置的固定高度
	 * @param[in] bNeedDpiScale 兼容 DPI 缩放，默认为 true
	 */
	void SetFixedHeight(int cy, bool bNeedDpiScale);

	/**@brief 获取控件大小
	*/
	const UiSize& GetFixedSize() const { return m_cxyFixed; }

	/**@brief 获取最小宽度
	 */
	int GetMinWidth() const { return m_cxyMin.cx; }

	/**@brief 设置最小宽度
	 * @param[in] cx 要设置的最小宽度
	 */
	void SetMinWidth(int cx);

	/**@brief 获取最大宽度
	 */
	int GetMaxWidth() const { return m_cxyMax.cx; }

	/**@brief 设置最大宽度
	 * @param[in] cx 要设置的最大宽度
	 */
	void SetMaxWidth(int cx);

	/**@brief 获取最小高度
	 */
	int GetMinHeight() const { return m_cxyMin.cy; }

	/**@brief 设置最小高度
	 * @param[in] cy 要设置的最小高度
	 */
	void SetMinHeight(int cy);

	/**@brief 获取最大高度
	 */
	int GetMaxHeight() const { return m_cxyMax.cy; }

	/**@brief 设置最大高度
	 * @param[in] cy 要设置的最大高度
	 */
	void SetMaxHeight(int cy);

	/**@brief 获取实际宽度
	 */
	int GetWidth() const { return m_uiRect.GetWidth(); }

	/**@brief 获取实际高度
	 */
	int GetHeight() const { return m_uiRect.GetHeight(); }

	/**@brief 获取水平对齐方式， 参考 HorAlignType 枚举
	 */
	HorAlignType GetHorAlignType() const { return m_horAlignType; }

	/**@brief 设置水平对齐方式
	 * @param[in] horAlignType 要设置的对齐方式，参考 HorAlignType 枚举
	 */
	void SetHorAlignType(HorAlignType horAlignType);

	/**@brief 获取垂直对齐方式，参见 VerAlignType 枚举
	 */
	VerAlignType GetVerAlignType() const { return m_verAlignType; }

	/**@brief 设置垂直对齐方式
	 * @param[in] vorAlignType 要设置的对齐方式，参考 VerAlignType 枚举
	 */
	void SetVerAlignType(VerAlignType verAlignType);

	/**@brief 判断是否需要重新评估大小
	 */
	bool IsReEstimateSize() const { return m_bReEstimateSize; }

	/**@brief 设置是否需要重新评估大小
	 */
	void SetReEstimateSize(bool bReEstimateSize);

	/**@brief 获取控件的已估算大小（长度和宽度），相当于EstimateSize函数估算后的缓存值
	*/
	const UiSize& GetEstimateSize() const { return m_szEstimateSize; }

	/**@brief 设置控件的已估算大小（长度和宽度），相当于EstimateSize函数估算后的缓存值
	*/
	void SetEstimateSize(const UiSize& szEstimateSize) { m_szEstimateSize = szEstimateSize; }

	/**@brief 估算控件大小（基类中无具体实现, 返回设置的固定大小）
	*  @param [in] szAvailable 已有大小
	 */
	virtual UiSize EstimateSize(UiSize /*szAvailable*/) { return m_cxyFixed; }

	/**@brief 获取控件位置（子类可改变行为）
	 * @param[in] bContainShadow 是否包含阴影位置，默认为 true
	 */
	virtual	UiRect GetPos(bool /*bContainShadow*/) const { return m_uiRect; }

	/**@brief 设置控件位置（子类可改变行为）
	 * @param[in] rc 要设置的位置信息
	 */
	virtual void SetPos(UiRect rc) { m_uiRect = rc; }

	/**@brief 获取控件区域(单纯返回m_uiRect值)
	*/
	const UiRect& GetRect() const { return m_uiRect; }

	/**@brief 设置控件区域(单纯设置m_uiRect值)
	*/
	void SetRect(const UiRect& rc) { m_uiRect = rc; }

	/**@brief 进行布局重排
	 */
	virtual void Arrange();

	/**@brief 让父容器重排
	 */
	virtual void ArrangeAncestor();

	/**@brief 判断是否已经排列过
	 */
	bool IsArranged() const { return m_bIsArranged; }

	/**@brief 设置是否已经排列过
	 */
	void SetArranged(bool bArranged) { m_bIsArranged = bArranged; }

	/**@brief 设置是否使用缓存
	 */
	void SetUseCache(bool cache);

	/**@brief 判断是否使用缓存
	 */
	bool IsUseCache() { return m_bUseCache; }

	/**@brief 设置缓存脏标志位
	 */
	void SetCacheDirty(bool dirty);

	/**@brief 判断缓存脏标志位值
	 */
	bool IsCacheDirty() { return m_bUseCache && m_bCacheDirty; }

	/**@brief 重绘控件
	 */
	virtual void Invalidate();

	/**@brief 获取控件实际的位置（布局位置加外层滚动后的偏移位置）
	 * @param[in] bContainShadow 是否考虑窗口阴影尺寸
	 */
	UiRect GetPosWithScrollOffset(bool bContainShadow) const;

	/** @brief 获取外层滚动偏移
	 */
	UiPoint GetScrollOffset() const;

	/** @brief 判断两个控件是否存在父子/子孙关系
	 */
	bool IsChild(PlaceHolder* pAncestor, PlaceHolder* pChild) const;

protected:
	/** @brief 让自己重排
	 * @return void 无
	 */
	virtual void ArrangeSelf();

private:
	//控件名称，用于查找控件等操作
	std::wstring m_sName;

	//关联的窗口对象
	Window* m_pWindow;

	//父控件对象
	Box* m_pParent;

	//控件位置与大小
	UiRect m_uiRect;

	//控件的大小
	UiSize m_cxyFixed;

	//控件大小最小值
	UiSize m_cxyMin;

	//控件大小最大值
	UiSize m_cxyMax;

	//控件水平对齐方式
	HorAlignType m_horAlignType;

	//控件垂直对齐方式
	VerAlignType m_verAlignType;

	//控件是否为浮动属性
	bool m_bFloat;

	//是否需要重新评估大小
	bool m_bReEstimateSize;

	//控件的已估算大小（长度和宽度），相当于估算后的缓存值
	UiSize m_szEstimateSize;

	//是否需要布局重排
	bool m_bIsArranged;

	//是否使用缓存
	bool m_bUseCache;

	//缓存是否存在脏标志值
	bool m_bCacheDirty;

	//是否可见
	bool m_bVisible;
};

} // namespace ui

#endif // UI_CORE_PLACE_HOLDER_H_
