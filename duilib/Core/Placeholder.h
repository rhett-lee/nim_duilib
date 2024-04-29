#ifndef UI_CORE_PLACE_HOLDER_H_
#define UI_CORE_PLACE_HOLDER_H_

#pragma once

#include "base/callback/callback.h"
#include "duilib/Core/UiTypes.h"
#include <string>
#include <memory>

namespace ui 
{
	class Box;
	class Window;

/** 控件的位置、大小、外观形状的基本封装
*/
class UILIB_API PlaceHolder : public virtual nbase::SupportWeakCallback
{
public:
	PlaceHolder();
	PlaceHolder(const PlaceHolder& r) = delete;
	PlaceHolder& operator=(const PlaceHolder& r) = delete;
	virtual ~PlaceHolder();

	/** 控件类型
	*/
	virtual std::wstring GetType() const;

	/** 获取控件名称，对应 xml 中 name 属性
	 */
	std::wstring GetName() const;

	/** 获取控件名称，对应 xml 中 name 属性
	 * @return 返回控件名称（UTF8 编码）
	 */
	std::string GetUTF8Name() const;

	/** 设置控件名称，内存中设置不会写入 xml 中
	 * @param [in] strName 要设置的名称
	 */
	void SetName(const std::wstring& strName);

	/** 设置控件名称，内存中设置不会写入 xml 中（UTF8 编码）
	 * @param[in] strName 要设置的名称
	 */
	void SetUTF8Name(const std::string& strName);

	/** 判断控件名称是否相等
	*/
	bool IsNameEquals(const std::wstring& name) const;

	/** 根据名称获取祖先容器指针
	* @param [in] strName 要获取的祖先容器名称
	*/
	Box* GetAncestor(const std::wstring& strName);

	/** 设置容器所属窗口
	 * @param [in] pParent 父容器指针
	 */
	virtual void SetParent(Box* pParent);

	/** 设置容器所属窗口
	 * @param [in] pManager 窗口指针
	 */
	virtual void SetWindow(Window* pManager);

	/** 获取父容器指针
	*/
	Box* GetParent() const { return m_pParent; }

	/** 获取关联的窗口指针
	 * @return 返回关联窗口的指针
	 */
	Window* GetWindow() const { return m_pWindow; }

	/** 初始化函数(当该控件被添加到父控件的时候，调用该Init函数)
	 */
	virtual void Init();

	/** 是否已经初始化
	*/
	bool IsInited() const;

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

public:
	/** 获取控件设置的宽度和高度，宽高均包含内边距，但均不包含外边距
	*/
	const UiFixedSize& GetFixedSize() const;

	/** 获取设置的宽度（包含内边距，不包含外边距），对应 xml 中 width 属性; 如果未设置，默认值是拉伸
	 */
	const UiFixedInt& GetFixedWidth() const;

	/** 获取固定高度（包含内边距，不包含外边距），对应 xml 中 height 属性; 如果未设置，默认值是拉伸
	 */
	const UiFixedInt& GetFixedHeight() const;

	/**@brief 设置控件的宽度
	 * @param[in] cx 要设置的宽度（包含内边距，不包含外边距）
	 * @param[in] bArrange 是否重新排列，默认为 true
	 * @param[in] bNeedDpiScale 兼容 DPI 缩放，默认为 true
	 */
	void SetFixedWidth(UiFixedInt cx, bool bArrange, bool bNeedDpiScale);

	/** 设置控件的宽度（包含内边距，不包含外边距），供动画使用的接口
	*/
	void SetFixedWidth64(int64_t cx64);

	/**@brief 设置控件的高度
	 * @param[in] cy 要设置的固定高度（包含内边距，不包含外边距）
	 * @param[in] bArrange 是否重新排列，默认为 true
	 * @param[in] bNeedDpiScale 兼容 DPI 缩放，默认为 true
	 */
	void SetFixedHeight(UiFixedInt cy, bool bArrange, bool bNeedDpiScale);

	/** 设置控件的高度（包含内边距，不包含外边距），供动画使用的接口
	*/
	void SetFixedHeight64(int64_t cy64);

public:
	/** 判断是否需要重新评估大小
	* @param [in] szAvailable 估算时，区域矩形大小
	*/
	bool IsReEstimateSize(const UiSize& szAvailable) const;

	/** 设置是否需要重新评估大小
	* @param [in] bReEstimateSize 是否需要重新估算
	*/
	void SetReEstimateSize(bool bReEstimateSize);

	/** 获取控件的已估算大小（长度和宽度），相当于EstimateSize函数估算后的缓存值
	*/
	const UiEstSize& GetEstimateSize() const;

	/**@brief 设置控件的已估算大小（长度和宽度），相当于EstimateSize函数估算后的缓存值
	*@param [in] szEstimateSize 估算的结果，作为缓存保存下来
	*@param [in] szAvailable szAvailable 估算时，区域矩形大小
	*/
	void SetEstimateSize(const UiEstSize& szEstimateSize, const UiSize& szAvailable);

public:
	/** 获取最小宽度
	* @return 返回最小宽度（包含内边距，不包含外边距）（返回值 >= 0）
	*/
	int32_t GetMinWidth() const;

	/**@brief 设置最小宽度
	 * @param [in] cx 要设置的最小宽度（包含内边距，不包含外边距）（值 >= 0）
	 * @param [in] bNeedDpiScale 是否需要做DPI自适应
	 */
	void SetMinWidth(int32_t cx, bool bNeedDpiScale = true);

	/** 获取最大宽度
	* @return 返回最大宽度（包含内边距，不包含外边距）（返回值 >= 0）
	*/
	int32_t GetMaxWidth() const;

	/** 设置最大宽度
	 * @param [in] cx 要设置的最大宽度（包含内边距，不包含外边距）（值 >= 0）
	 * @param [in] bNeedDpiScale 是否需要做DPI自适应
	 */
	void SetMaxWidth(int32_t cx, bool bNeedDpiScale = true);

	/** 获取最小高度
	* @return 返回最小高度（包含内边距，不包含外边距）（返回值 >= 0）
	*/
	int32_t GetMinHeight() const;

	/** 设置最小高度
	 * @param [in] cy 要设置的最小高度（包含内边距，不包含外边距）（值 >= 0）
	 * @param [in] bNeedDpiScale 是否需要做DPI自适应
	 */
	void SetMinHeight(int32_t cy, bool bNeedDpiScale = true);

	/** 获取最大高度
	* @return 返回最大高度（包含内边距，不包含外边距）（返回值 >= 0）
	*/
	int32_t GetMaxHeight() const;

	/** 设置最大高度
	 * @param[in] cy 要设置的最大高度（包含内边距，不包含外边距）（值 >= 0）
	 * @param [in] bNeedDpiScale 是否需要做DPI自适应
	 */
	void SetMaxHeight(int32_t cy, bool bNeedDpiScale = true);

	/** 获取实际宽度（包括内边距，不包括外边距）
	 */
	int32_t GetWidth() const { return m_uiRect.Width(); }

	/** 获取实际高度（包括内边距，不包括外边距）
	 */
	int32_t GetHeight() const { return m_uiRect.Height(); }

	/**@brief 获取水平对齐方式， 参考 HorAlignType 枚举
	 */
	HorAlignType GetHorAlignType() const;

	/**@brief 设置水平对齐方式
	 * @param[in] horAlignType 要设置的对齐方式，参考 HorAlignType 枚举
	 */
	void SetHorAlignType(HorAlignType horAlignType);

	/**@brief 获取垂直对齐方式，参见 VerAlignType 枚举
	 */
	VerAlignType GetVerAlignType() const;

	/**@brief 设置垂直对齐方式
	 * @param[in] vorAlignType 要设置的对齐方式，参考 VerAlignType 枚举
	 */
	void SetVerAlignType(VerAlignType verAlignType);

	/** 获取控件的外边距
	 */
	UiMargin GetMargin() const;

	/** 设置控件的外边距
	 * @param[in] rcMargin 控件的外边距信息
	 * @param[in] bNeedDpiScale 是否让外边距根据 DPI 适配，false 不适配 DPI
	 */
	void SetMargin(UiMargin rcMargin, bool bNeedDpiScale);

	/** 获取内边距
	 * @return 返回内边距四边的大小（Rect的四个参数分别代表四个边的内边距）
	 */
	UiPadding GetPadding() const;

	/** 设置内边距
	 * @param[in] rcPadding 内边距数据
	 * @param[in] bNeedDpiScale 是否根据 DPI 自适应，默认为 true
	 */
	void SetPadding(UiPadding rcPadding, bool bNeedDpiScale);

	/** 是否允许控件自身运用内边距
	* @param [in] bEnable true表示控件本身也运用内边距（Box和Control）；
						  false表示控件本身不运用内边距，内边距仅用于布局子控件（Box的情况）
	*/
	void SetEnableControlPadding(bool bEnable);

	/** 判断控件自身是否允许运用内边距
	* @return true表示控件本身也运用内边距（Box和Control）；
		      false表示控件本身不运用内边距，内边距仅用于布局子控件（Box的情况）
	*/
	bool IsEnableControlPadding() const;

	/** 获取控件自身的内边距（如果设置为禁止运用内边距，则返回空）
	* @return 返回内边距四边的大小（Rect的四个参数分别代表四个边的内边距）
	*/
	UiPadding GetControlPadding() const;

	/** 获取控件位置（子类可改变行为）
	* @return 返回控件的矩形区域，包含内边距，不包含外边距
	 */
	virtual	UiRect GetPos() const { return m_uiRect; }

	/** 设置控件位置（子类可改变行为）
	 * @param [in] rc 要设置的矩形区域信息，包含内边距，不包含外边距
	 */
	virtual void SetPos(UiRect rc);

	/** 获取控件矩形区域(单纯返回m_uiRect值)，包含内边距，不包含外边距
	*/
	const UiRect& GetRect() const { return m_uiRect; }

	/** 设置控件矩形区域(单纯设置m_uiRect值)，包含内边距，不包含外边距
	*/
	void SetRect(const UiRect& rc);

	/** 重绘控件
	*/
	virtual void Invalidate();

	/** 重绘控件的部分区域
	* @param [in] rc 需要重绘的区域
	*/
	virtual void InvalidateRect(const UiRect& rc);

	/** 控件布局重排
	 */
	virtual void Arrange();

	/** 让父容器进行布局重排
	 */
	virtual void ArrangeAncestor();

	/** 根据控件属性，判断需要重排布局或者只是重绘
	*/
	void RelayoutOrRedraw();

	/** 判断是否已经排列过
	 */
	bool IsArranged() const { return m_bIsArranged; }

	/** 设置是否已经排列过
	 */
	void SetArranged(bool bArranged);

	/** 设置是否使用缓存
	 */
	void SetUseCache(bool cache);

	/** 判断是否使用缓存
	 */
	bool IsUseCache() { return m_bUseCache; }

	/** 设置缓存脏标志位
	 */
	void SetCacheDirty(bool dirty);

	/** 判断缓存脏标志位值
	 */
	bool IsCacheDirty() { return m_bCacheDirty; }

	/** 获取外层滚动偏移
	 */
	UiPoint GetScrollOffsetInScrollBox() const;

	/** 判断两个控件是否存在父子/子孙关系
	 */
	bool IsChild(PlaceHolder* pAncestor, PlaceHolder* pChild) const;

protected:
	/** 让自己重排
	 */
	virtual void ArrangeSelf();

	/** 执行初始化函数的事件（每个控件在初始化时，会调用该函数，并且只调用一次）
	 *  该函数执行时，IsInited()的值为false，如果IsInited()为true，表示OnInit()函数重复执行了。
	 */
	virtual void OnInit();

private:
	//控件名称，用于查找控件等操作
	UiString m_sName;

	//关联的窗口对象
	Window* m_pWindow;

	//父控件对象
	Box* m_pParent;

	//控件位置与大小
	UiRect m_uiRect;

	//外部设置的控件大小
	UiFixedSize m_cxyFixed;

	//估算控件大小的结果
	UiEstResult m_estResult;

	//控件大小最小值
	UiSize m_cxyMin;

	//控件大小最大值
	UiSize m_cxyMax;

	//控件水平对齐方式(HorAlignType)
	int8_t m_horAlignType;

	//控件垂直对齐方式(VerAlignType)
	int8_t m_verAlignType;

	//控件的外边距属性（上，下，左，右边距），外边距是m_uiRect以外的空间，不包含在m_uiRect以内
	UiMargin16 m_rcMargin;

	//内边距四边的大小（上，下，左，右边距），内边距是控件矩形以内的空间，是包含在控件矩形以内的
	UiPadding16 m_rcPadding;

	//是否允许控件本身设置内边距
	//(原来的逻辑：Control自身无内边距，Box的Layout有内边距，所以Box自身的背景图片等是不应用内边距的，只有子控件应用内边距)
	//此开关默认为true，提供关闭选项是为了兼容原来的逻辑，比如阴影的实现，就不能开启内边距，否则阴影绘制异常
	bool m_bEnableControlPadding;

	//控件是否为浮动属性
	bool m_bFloat;

	//是否需要布局重排
	bool m_bIsArranged;

	//是否使用绘制缓存
	// 如果为true，每个控件自己保存一份绘制缓存，会占用较多内存，理论上会提升绘制性能，但实际未测试出效果）
	// 如果为false，表示无绘制缓存，内存占用比较少。
	// TODO: 这个模式下内存占有率很高，对绘制性能提升不明显，未来可能会删除掉这个逻辑，以简化代码。
	bool m_bUseCache;

	//缓存是否存在脏标志值
	bool m_bCacheDirty;

	//是否可见
	bool m_bVisible;

	//是否已经完成初始化
	bool m_bInited;
};

} // namespace ui

#endif // UI_CORE_PLACE_HOLDER_H_
