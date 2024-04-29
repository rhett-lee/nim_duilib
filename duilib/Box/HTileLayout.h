#ifndef UI_BOX_HTILE_LAYOUT_H_
#define UI_BOX_HTILE_LAYOUT_H_

#pragma once

#include "duilib/Box/Layout.h"

namespace ui 
{

/** 瓦片布局(横向)
*/
class UILIB_API HTileLayout : public Layout
{
public:
	HTileLayout();

	/** 布局类型
	*/
	virtual LayoutType GetLayoutType() const override { return LayoutType::HTileLayout; }

	/** 调整内部所有控件的位置信息
	 * @param[in] items 控件列表
	 * @param[in] rc 当前容器位置信息, 包含内边距，但不包含外边距
	 * @return 返回排列后最终盒子的宽度和高度信息
	 */
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

	/** 根据内部子控件大小估算容器自身大小，拉伸类型的子控件被忽略，不计入大小估算
	 * @param[in] items 子控件列表
	 * @param [in] szAvailable 可用大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
	 * @return 返回排列后最终布局的大小信息（宽度和高度）；
			   包含items中子控件的外边距，包含items中子控件的内边距；
			   包含Box控件本身的内边距；
			   不包含Box控件本身的外边距；
			   返回值中不包含拉伸类型的子控件大小。
	 */
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable);
	
	/** 设置布局属性
	 * @param[in] strName 要设置的属性名
	 * @param[in] strValue 要设置的属性值
	 * @return true 设置成功，false 属性不存在
	 */
	virtual bool SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

public:
	/** 获取子项大小，该宽度和高度，是包含了控件的外边距和内边距的
	 */
	const UiSize& GetItemSize() const;

	/** 设置子项大小
	 * @param[in] szItem 子项大小数据，该宽度和高度，是包含了控件的外边距和内边距的
	 */
	void SetItemSize(UiSize szItem, bool bNeedDpiScale = true);

	/** 获取行数量
	 */
	int32_t GetRows() const;

	/** 设置显示几行数据
	 * @param[in] nRows 要设置显示几行的数值
	 */
	void SetRows(int32_t nRows);

	/** 设置是否自动计算行
	 */
	void SetAutoCalcRows(bool bAutoCalcRows);

	/** 判断是否自动计算行数
	 */
	bool IsAutoCalcRows() const;

	/** 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内
	*/
	void SetScaleDown(bool bScaleDown);

	/** 判断超出边界的时候，是否按比例缩小
	*/
	bool IsScaleDown() const;

	/** 当前是否为自由布局
	*/
	bool IsFreeLayout() const;

private:
	/** 未处理的子控件接口和其宽高信息
	*/
	struct ItemSizeInfo
	{
		Control* pControl = nullptr; //子控件接口
		int32_t cx = 0;				 //子控件的宽度
		int32_t cy = 0;			     //子控件的高度
	};

	/** 获取估算大小时的可用宽高
	* @param [in] pControl 空间接口
	* @param [in] szItem 每个瓦片控件的宽度和高度(设置值)
	* @param [in] rc 瓦片控件所在容器的可用区域矩形
	* @return 返回该空间的估算大小（宽和高）
	*/
	static UiSize CalcEstimateSize(Control* pControl, const UiSize& szItem, UiRect rc);

	/** 获取基本参数：瓦片的列数
	* @param [in] normalItems 子控件列表
	* @param [in] rc 瓦片控件所在容器的矩形
	* @param [in] tileHeight 每个瓦片控件的宽度(配置值)
	* @param [in] childMarginX 子控件的X轴间隔
	* @param [in] childMarginY 子控件的Y轴间隔	
	* @param [out] nRows 返回总行数
	*/
	static void CalcTileRows(const std::vector<ItemSizeInfo>& normalItems, const UiRect& rc,
		                     int32_t tileHeight, int32_t childMarginX, int32_t childMarginY,
		                     int32_t& nRows);

	/** 估算浮动控件的大小
	*/
	static UiSize64 EstimateFloatSize(Control* pControl, UiRect rc);

	/** 处理浮动子控件，并返回未处理的子控件列表
	* @param [in] items 子控件列表
	* @param [in] rect 外部可用矩形大小
	* @param [in] szItem 每个瓦片控件的宽度和高度(设置值)
	* @param [in] isCalcOnly 如果为true表示仅计算区域，对控件位置不做调整；如果为false，表示对控件位置做调整。
	* @param [out] normalItems 返回未处理的子控件列表，及其大小信息
	* @return 返回浮动控件所占的区域宽度和高度
	*/
	static UiSize64 ArrangeFloatChild(const std::vector<Control*>& items,
							          UiRect rc,
		                              const UiSize& szItem,
		                              bool isCalcOnly, 
		                              std::vector<ItemSizeInfo>& normalItems);

	/** 获取基本参数：瓦片高度，布局排列过程中，在每列开始时，计算本列的宽度
	* @param [in] normalItems 子控件列表
	* @param [in] iterBegin 子控件开始的迭代器
	* @param [in] nRows 总行数
	* @param [in] szItem 瓦片控件宽度和高度（设置值）
	* @return 返回高宽值，包含了外边距Margin.top + Margin.bottom值
	*/
	static int32_t CalcTileColumnWidth(const std::vector<ItemSizeInfo>& normalItems,
									   const std::vector<ItemSizeInfo>::const_iterator iterBegin,
								 	   int32_t nRows,
								       const UiSize& szItem);

	/** 计算瓦片控件的显示坐标和大小
	* @param [in] itemSizeInfo 瓦片控件的接口, 及控件的大小信息
	* @param [in] tileWidth 配置的瓦片控件宽度
	* @param [in] tileHeight 配置的瓦片控件高度（取行高）
	* @param [in] ptTile 当前瓦片控件左上角的坐标
	* @param [in] bScaleDown 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内
	* @param [out] szTilePos 瓦片控件的显示坐标、宽度和高度
	* @return 返回瓦片控件目标区域的大小（宽和高），宽度和高度包含了控件的外边距
	*/
	static UiSize CalcTilePosition(const ItemSizeInfo& itemSizeInfo,
								   int32_t tileWidth,
								   int32_t tileHeight,
								   const UiPoint& ptTile,
		                           bool bScaleDown, 
								   UiRect& szTilePos);

private:
	/** 对子控件布局的内部实现函数
	* @param [in] items 子控件列表
	* @param [in] rect 可用矩形大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
	* @param [in] isCalcOnly 如果为true表示仅计算区域，对控件位置不做调整；如果为false，表示对控件位置做调整。
	* @param [in] inRowHeights 每行的高度值，可用为空
	* @param [out] outRowHeights 本次布局，使用的每行高度值
	* @return 返回区域的宽度和高度
	*/
	UiSize64 ArrangeChildNormal(const std::vector<Control*>& items, 
								UiRect rect,
							    bool isCalcOnly,
		                        const std::vector<int32_t>& inRowHeights,
		                        std::vector<int32_t>& outRowHeights) const;

	/** 使用自由布局排列控件(无固定列数，尽量充分利用展示空间，显示尽可能多的内容)
	* @param [in] items 子控件列表
	* @param [in] rect 可用矩形大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
	* @param [in] isCalcOnly 如果为true表示仅计算区域，对控件位置不做调整；如果为false，表示对控件位置做调整。
	* @return 返回区域的宽度和高度
	*/
	UiSize64 ArrangeChildFreeLayout(const std::vector<Control*>& items, 
									UiRect rect, 
									bool isCalcOnly) const;

private:

	//显示几行数据
	int32_t m_nRows;

	//子项大小, 该宽度和高度，是包含了控件的外边距和内边距的
	UiSize m_szItem;

	//自动计算行数
	bool m_bAutoCalcRows;

	//当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内
	bool m_bScaleDown;
};

} // namespace ui

#endif // UI_BOX_HTILE_LAYOUT_H_
