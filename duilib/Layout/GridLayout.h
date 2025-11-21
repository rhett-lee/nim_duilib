#ifndef UI_LAYOUT_GRIDLAYOUT_H_
#define UI_LAYOUT_GRIDLAYOUT_H_

#include "duilib/Layout/Layout.h"
#include <unordered_map>

namespace ui 
{
/** 网格布局
 */
class UILIB_API GridLayout : public Layout
{
    typedef Layout BaseClass;
public:
    GridLayout();

public:
    /** 获取行数(0表示自动计算)
     */
    int32_t GetRows() const;

    /** 设置行数(0表示自动计算)
     * @param [in] nRows 行数
     */
    void SetRows(int32_t nRows);

    /** 获取列数(0表示自动计算)
     */
    int32_t GetColumns() const;

    /** 设置列数(0表示自动计算)
     * @param [in] nCols 列数
     */
    void SetColumns(int32_t nCols);

    /** 设置网格单元格宽度(0表示自动计算)
    */
    void SetGridWidth(int32_t nGridWidth, bool bNeedDpiScale);

    /** 获取网格单元格宽度(0表示自动计算)
    * @return 返回网格宽度（已经做过DPI缩放）
    */
    int32_t GetGridWidth() const;

    /** 设置网格单元格高度(0表示自动计算)
    */
    void SetGridHeight(int32_t nGridHeight, bool bNeedDpiScale);

    /** 获取网格单元格高度(0表示自动计算)
    * @return 返回网格高度（已经做过DPI缩放）
    */
    int32_t GetGridHeight() const;

    /** 当控件内容超出边界时，是否按比例缩小
     *   true  使用子控件的大小，若超出网格大小，按比例缩小，以使控件内容完全显示在网格内
     *   false 忽略子控件自身的大小，子控件的大小与网格大小一致
     */
    void SetScaleDown(bool bScaleDown);

    /** 判断超出边界的时候，是否按比例缩小
     *@return 返回值 true  使用子控件的大小，若超出网格大小，按比例缩小，以使控件内容完全显示在网格内
     *              false 忽略子控件自身的大小，子控件的大小与网格大小一致
     */
    bool IsScaleDown() const;

public:
    /** 布局类型
    */
    virtual LayoutType GetLayoutType() const override { return LayoutType::GridLayout; }

    /** 设置布局属性
     * @param [in] strName 要设置的属性名
     * @param [in] strValue 要设置的属性值
     * @param [in] dpiManager DPI管理接口
     * @return true 设置成功，false 属性不存在
     */
    virtual bool SetAttribute(const DString& strName, 
                              const DString& strValue,
                              const DpiManager& dpiManager) override;

    /** DPI发生变化，更新控件大小和布局
     * @param [in] nOldDpiScale 旧的DPI缩放百分比
     * @param [in] dpiManager DPI缩放管理器
     */
    virtual void ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale) override;

    /** 按布局策略调整内部所有子控件的位置和大小
     * @param [in] items 子控件列表
     * @param [in] rc 当前容器位置与大小信息, 包含内边距，但不包含外边距
     * @param [in] bEstimateOnly true表示仅评估不调整控件的位置，false表示调整控件的位置
     * @return 返回排列后最终布局的宽度和高度信息，包含Box容器的内边距，但不包含Box容器本身的外边距(当容器支持滚动条时使用该返回值)
     */
    virtual UiSize64 ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly = false) override;

    /** 根据内部子控件大小估算容器布局大小（用于评估宽度或者高度为"auto"类型的控件大小，拉伸类型的子控件不计入大小估算）
     * @param [in] items 子控件列表
     * @param [in] szAvailable 容器的可用宽度和高度，包含分配给该容器的内边距，但不包含分配给容器的外边距
     * @return 返回排列后最终布局的大小信息（宽度和高度），包含Box容器本身的内边距，但不包含Box容器本身的外边距；
     */
    virtual UiSize64 EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable) override;

private:
    /** 调整内部所有控件的位置信息
     * @param [in] items 控件列表
     * @param [in] rc 当前容器位置信息, 包含内边距，但不包含外边距
     * @param [in] bEstimateOnly true表示仅评估不调整控件的位置，false表示调整控件的位置
     * @return 返回排列后最终盒子的宽度和高度信息，包含Owner Box的内边距，不包含外边距
     */
    UiSize64 ArrangeChildInternal(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly) const;

    /** 计算实际行列数
    * @param [in] gridCount 格子总数
    * @param [in,out] rows 输入当前设置的行数，输出计算得到的有效行数
    * @param [in,out] cols 输入当前设置的列数，输出计算得到的有效列数
    */
    void CalcActualGridSize(int32_t gridCount, int32_t& rows, int32_t& cols) const;

    /** 计算每列宽度（平均分配，扣除水平间距）
    * @param [in] contentRect 内容显示区的矩形范围(不含容器内边距)
    * @param [in] cols 列数
    * @return 返回计算得到的每列的宽度
    */
    std::vector<int32_t> CalcColumnWidths(const UiRect& contentRect, int32_t cols) const;

    /** 计算每行高度（平均分配，扣除垂直间距）
    * @param [in] contentRect 内容显示区的矩形范围(不含容器内边距)
    * @param [in] rows 行数
    * @return 返回计算得到的每行的高度
    */
    std::vector<int32_t> CalcRowHeights(const UiRect& contentRect, int32_t rows) const;

    /** 计算合并单元格的区域（处理rowSpan/columnSpan）
    * @param [in] controls 需要布局的子控件列表
    * @param [in] rows 行数
    * @param [in] cols 列数
    * @param [in] colWidths 列宽列表
    * @param [in] rowHeights 行高列表
    * @param [out] cellOccupied 标记合并范围内的单元格为已占用（二维数组）
    * @param [out] ctrlCellRects 列表控件的位置和大小
    */
    bool CalcMergedCellRects(const std::vector<Control*>& controls,
                             int32_t rows, int32_t cols,
                             const std::vector<int32_t>& colWidths,
                             const std::vector<int32_t>& rowHeights,
                             std::vector<std::vector<bool>>& cellOccupied,
                             std::unordered_map<Control*, UiRect>& ctrlCellRects) const;

    /** 计算网格整体尺寸
    * @param [in] colWidths 列宽列表
    * @param [in] rowHeights 行高列表
    * @return 返回网格的尺寸
    */
    UiSize CalcGridTotalSize(const std::vector<int32_t>& colWidths, const std::vector<int32_t>& rowHeights) const;

    /** 计算网格在容器中的位置（基于布局的对齐属性）
    * @param [in] contentRect 内容显示区的矩形范围(不含容器内边距)
    * @param [in] gridSize 网格大小
    */
    UiRect CalcGridPosition(const UiRect& contentRect, const UiSize& gridSize) const;

    /** 调整合并单元格位置（加上网格整体偏移）
    * @param [in] ctrlCellRects 列表控件的位置和大小
    * @param [in] gridRect 网格矩形范围
    */
    void AdjustMergedCellPositions(std::unordered_map<Control*, UiRect>& ctrlCellRects,
                                   const UiRect& gridRect) const;

    /** 排列控件到合并单元格（使用控件自身对齐属性）
    * @param [in] controls 需要布局的子控件列表
    * @param [out] ctrlCellRects 列表控件的位置和大小
    */
    void ArrangeControlsInMergedCells(const std::vector<Control*>& controls,
                                      const std::unordered_map<Control*, UiRect>& ctrlCellRects) const;

    /** 基于控件自身对齐属性计算位置
    * @param [in] contentRect 容器的矩形范围
    * @param [in] ctrlSize 控件大小
    * @param [in] ctrl 控件接口
    */
    UiRect CalcControlPosition(const UiRect& contentRect, const UiSize& ctrlSize, Control* ctrl) const;

private:
    /** 网格行数(0表示自动计算)
    */
    int32_t m_nRows;

    /** 网格列数(0表示自动计算)
    */
    int32_t m_nColumns;

    /** 网格单元格宽度(0表示自动计算)
    */
    int32_t m_nGridWidth;

    /** 网格单元格高度(0表示自动计算)
    */
    int32_t m_nGridHeight;

    /** 当控件内容超出边界时，是否按比例缩小
     *  true  使用子控件的大小，若超出网格大小，按比例缩小，以使控件内容完全显示在网格内
     *  false 忽略子控件自身的大小，子控件的大小与网格大小一致
     */
    bool m_bScaleDown;
};

} // namespace ui

#endif // UI_LAYOUT_GRIDLAYOUT_H_
