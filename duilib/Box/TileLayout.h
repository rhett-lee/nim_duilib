#ifndef UI_BOX_TILELAYOUT_H_
#define UI_BOX_TILELAYOUT_H_

#pragma once

#include "duilib/Box/Layout.h"

namespace ui 
{

/** 瓦片布局
*/
class UILIB_API TileLayout : public Layout
{
public:
	TileLayout();

	/** 调整内部所有控件的位置信息
	 * @param[in] items 控件列表
	 * @param[in] rc 当前容器位置信息, 外部调用时，不需要扣除内边距
	 * @return 返回排列后最终盒子的宽度和高度信息
	 */
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

	/** 根据内部子控件大小估算容器自身大小，拉伸类型的子控件被忽略，不计入大小估算
	 * @param[in] items 子控件列表
	 * @param[in] szAvailable 子控件允许的最大宽度和高度
	 * @return 返回排列后最终布局的大小信息（宽度和高度），不包含拉伸类型的子控件大小
	 */
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;

	/** 设置布局属性
	 * @param[in] strName 要设置的属性名
	 * @param[in] strValue 要设置的属性值
	 * @return true 设置成功，false 属性不存在
	 */
	virtual bool SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

	/** 获取子项大小
	 */
	const UiSize& GetItemSize() const;

	/** 设置子项大小
	 * @param[in] szItem 子项大小数据
	 */
	void SetItemSize(UiSize szItem, bool bNeedDpiScale = true);

	/** 获取列数量
	 */
	int32_t GetColumns() const;

	/** 设置显示几列数据
	 * @param[in] nCols 要设置显示几列的数值
	 */
	void SetColumns(int32_t nCols);

	/** 设置是否自动计算列数
	 */
	void SetAutoCalcColumns(bool bAutoCalcColumns);

	/** 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内
	*/
	void SetScaleDown(bool bScaleDown);

private:
	/** 获取估算大小时的可用宽高
	* @param [in] pControl 空间接口
	* @param [in] szItem 每个瓦片控件的宽度和高度(设置值)
	* @param [in] rc 瓦片控件所在容器的可用区域矩形
	* @return 返回该空间的估算大小（宽和高）
	*/
	static UiSize CalcEstimateSize(Control* pControl, const UiSize& szItem, UiRect rc);

	/** 获取基本参数：瓦片的列数
	* @param [in] items 子控件列表
	* @param [in] rc 瓦片控件所在容器的矩形
	* @param [in] itemWidth 每个瓦片控件的宽度(配置值)
	* @param [in] childMarginX 子控件的X轴间隔
	* @param [in] childMarginY 子控件的Y轴间隔	
	* @param [out] columns 返回总列数
	*/
	static void CalcTileColumns(const std::vector<Control*>& items, const UiRect& rc,
		                        int32_t itemWidth, int32_t childMarginX, int32_t childMarginY,
		                        int32_t& nColumns);

	/** 获取基本参数：瓦片宽度和高度的最大值, 作为拉伸类型控件的宽高值
	* @param [in] items 子控件列表
	* @param [in] rc 瓦片控件所在容器的矩形
	* @param [in] childMarginX 子控件的X轴间隔
	* @param [in] childMarginY 子控件的Y轴间隔	
	* @param [in] columns 总列数
	* @param [out] szMaxItem 返回瓦片控件的宽度和高度最大值，包含了外边距Margin.left + Margin.right值
	*/
	static void CalcStretchTileSize(const std::vector<Control*>& items, UiRect rc,
		                            int32_t childMarginX, int32_t childMarginY,
		                            int32_t nColumns, UiSize& szMaxItem);

	/** 估算浮动控件的大小
	*/
	static UiSize64 EstimateFloatSize(Control* pControl, UiRect rc);

	/** 获取基本参数：瓦片高度，布局排列过程中，在每行开始时，计算本行的高度
	* @param [in] items 子控件列表
	* @param [in] iterBegin 子控件开始的迭代器
	* @param [in] nColumns 总列数
	* @param [in] szItem 瓦片控件宽度和高度（设置值）
	* @param [in] rc 瓦片控件所在容器的矩形
	* @return 返回高度值，包含了外边距Margin.top + Margin.bottom值
	*/
	static int32_t CalcTileLineHeight(const std::vector<Control*>& items,
									  const std::vector<Control*>::const_iterator iterBegin,
								 	  int32_t nColumns,
								      const UiSize& szItem,
									  const UiRect& rc);

	/** 计算瓦片控件的显示坐标和大小
	* @param [in] pControl 瓦片控件的接口
	* @param [in] itemWidth 配置的瓦片控件宽度
	* @param [in] itemHeight 配置的瓦片控件高度（取行高）
	* @param [in] szStretchItem 拉伸类型的瓦片控件宽度和高度
	* @param [in] rcContainer 外部可用矩形大小
	* @param [in] ptTile 当前瓦片控件左上角的坐标
	* @param [in] bScaleDown 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内
	* @param [out] szTilePos 瓦片控件的显示坐标、宽度和高度
	* @return 返回瓦片控件目标区域的大小（宽和高）
	*/
	static UiSize CalcTilePosition(Control* pControl,
								   int32_t itemWidth,
								   int32_t itemHeight,
								   const UiSize& szStretchItem, 
								   const UiRect& rcContainer,
								   const UiPoint& ptTile,
		                           bool bScaleDown, 
								   UiRect& szTilePos);


	/** 对子控件布局的内部实现函数
	* @param [in] items 子控件列表
	* @param [in] rect 外部可用矩形大小
	* @param [in] isCalcOnly 如果为true表示仅计算区域，对控件位置不做调整；如果为false，表示对控件位置做调整。
	* @param [in] inColumnWidths 每列的宽度值，可用为空
	* @param [out] outColumnWidths 本次布局，使用的每列宽度值
	* @return 返回区域的宽度和高度
	*/
	UiSize64 ArrangeChildNormal(const std::vector<Control*>& items, 
								UiRect rect,
							    bool isCalcOnly,
		                        const std::vector<int32_t>& inColumnWidths,
		                        std::vector<int32_t>& outColumnWidths) const;

	/** 使用自由布局排列控件(无固定列数，尽量充分利用展示空间，显示尽可能多的内容)
	* @param [in] items 子控件列表
	* @param [in] rect 外部可用矩形大小
	* @param [in] isCalcOnly 如果为true表示仅计算区域，对控件位置不做调整；如果为false，表示对控件位置做调整。
	* @return 返回区域的宽度和高度
	*/
	UiSize64 ArrangeChildFreeLayout(const std::vector<Control*>& items, 
									UiRect rect, 
									bool isCalcOnly) const;

	/** 当前是否为自由布局
	*/
	bool IsFreeLayout() const;

private:

	//显示几列数据
	int32_t m_nColumns;

	//子项大小
	UiSize m_szItem;

	//自动计算列数
	bool m_bAutoCalcColumns;

	//当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内
	bool m_bScaleDown;
};

} // namespace ui

#endif // UI_BOX_TILELAYOUT_H_
