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

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;
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

private:
	/** 获取基本参数：瓦片宽度和列数，这两个参数在布局排列前就确定，是固定值
	* @param [in] items 子控件列表
	* @param [in] szAvailable 可用区域的宽度和高度
	* @param [out] itemWidth 返回每个瓦片控件的宽度，包含了外边距Margin.left + Margin.right值
	* @param [out] columns 返回总列数
	*/
	void CalcTileWidthAndColumns(const std::vector<Control*>& items, UiSize szAvailable,
		                        int32_t& itemWidth, int32_t& columns) const;

	/** 获取基本参数：瓦片高度，布局排列过程中，在每行开始时，计算本行的高度
	* @param [in] items 子控件列表
	* @param [in] iterBegin 子控件开始的迭代器
	* @param [in] nColumns 总列数
	* @param [in] tileWidth 瓦片控件宽度
	* @param [in] tileBoxHeight 瓦片控件所在容器的总高度
	* @return 返回高度值，包含了外边距Margin.top + Margin.bottom值
	*/
	int32_t CalcTileHeight(const std::vector<Control*>& items,
		                   const std::vector<Control*>::const_iterator iterBegin, 
		                   int32_t nColumns,
		                   int32_t tileWidth,
		                   int32_t tileBoxHeight);

private:

	//显示几列数据
	int32_t m_nColumns;

	//子项大小
	UiSize m_szItem;
};

} // namespace ui

#endif // UI_BOX_TILELAYOUT_H_
