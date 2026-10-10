#ifndef UI_CONTROL_CHART_H_
#define UI_CONTROL_CHART_H_

#include "duilib/Core/Control.h"
#include <vector>

namespace ui {

/** Chart 图表类型
*/
enum class ChartType
{
    kLine = 0,  //折线图
    kBar  = 1,  //柱状图
    kPie  = 2   //饼图
};

/** 柱状图排列模式
*/
enum class ChartBarMode
{
    kGrouped = 0,  //分组（多系列并排）
    kStacked = 1   //堆叠（多系列累加）
};

/** 折线图绘制模式
*/
enum class ChartLineMode
{
    kStraight = 0,  //折线（直线连接）
    kCurve    = 1   //平滑曲线（贝塞尔）
};

/** Chart 轻量图表控件（继承Control，纯自绘，对标 Qt Charts 精简版）
 *  支持折线图（line）、柱状图（bar）、饼图（pie）三种形态，数据通过 XML 属性或 C++ 接口绑定。
 *
 *  特性：
 *  - 多系列：折线/柱状支持多组数据叠加对比，自动分配系列颜色
 *  - 坐标轴：Y 轴刻度值、X/Y 轴标题、图表标题、网格线
 *  - 柱状图：支持负值（基线自动调整）、分组/堆叠两种模式
 *  - 折线图：折线/平滑曲线、面积填充、数据点标记开关
 *  - 饼图：环形（donut）模式、百分比显示、图例
 *
 *  XML 属性（单系列，向后兼容）：
 *  - chart_type: "line"/"bar"/"pie"，默认 "line"
 *  - data / data_labels: 逗号分隔的数值 / 类别标签
 *  - series_color / axis_color / label_color: 系列/坐标轴/文本颜色（语义色名或颜色值）
 *  - title: 图表标题
 *  - x_axis_title / y_axis_title: X/Y 轴标题
 *  - show_value: 是否显示数据值标签，默认 true
 *  - show_grid: 是否显示网格线，默认 true
 *  - show_axis_values: 是否显示 Y 轴刻度值，默认 true
 *  - axis_divisions: Y 轴刻度分段数（1~10），默认 4
 *  - legend_visible: 是否显示图例（饼图默认 true，折线/柱状默认 false）
 *  - line_width: 折线粗细，默认 2
 *  - line_mode: "straight"/"curve"，折线/曲线，默认 straight
 *  - area_fill: 折线下方是否填充面积，默认 false
 *  - show_data_points: 是否绘制数据点圆点（折线），默认 true
 *  - bar_mode: "grouped"/"stacked"，柱状排列，默认 grouped
 *  - donut: 饼图是否环形，默认 false
 *  - show_percent: 饼图是否显示百分比，默认 false
 *
 *  颜色均使用语义色名（global.xml 的 ThemeColor），自动适配深浅色主题。
 */
class DUILIB_API Chart : public Control
{
    typedef Control BaseClass;

public:
    /** 一个数据系列
    */
    struct Series
    {
        /** 系列名称（图例显示用，可为空）
        */
        DString name;

        /** 系列数据
        */
        std::vector<double> data;

        /** 系列主色（语义色名或颜色值，空则自动分配）
        */
        DString color;
    };

public:
    explicit Chart(Window* pWindow);
    Chart(const Chart& r) = delete;
    Chart& operator=(const Chart& r) = delete;
    ~Chart() override;

    /** 获取控件类型
    */
    DString GetType() const override;

    /** 设置XML属性
    */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 绘制图表内容（坐标轴、系列、图例、数据标签）
    */
    void PaintStateImages(IRender* pRender) override;

public:
    /** 设置图表类型
    */
    void SetChartType(ChartType chartType);

    /** 获取图表类型
    */
    ChartType GetChartType() const { return m_chartType; }

    /** 设置图表标题
    */
    void SetTitle(const DString& strTitle);

    /** 获取图表标题
    */
    const DString& GetTitle() const { return m_strTitle; }

    /** 设置 X 轴标题
    */
    void SetXAxisTitle(const DString& strTitle);

    /** 设置 Y 轴标题
    */
    void SetYAxisTitle(const DString& strTitle);

    /** 设置系列数据（单系列，替代原有数据），兼容旧接口
    */
    void SetData(const std::vector<double>& data);

    /** 追加一个数据点（单系列）
    */
    void AddData(double value);

    /** 清空所有系列数据
    */
    void ClearData();

    /** 获取单系列数据（兼容旧接口，取第一个系列）
    */
    const std::vector<double>& GetData() const;

    /** 设置数据对应的类别标签（X 轴类目，所有系列共用）
    */
    void SetDataLabels(const std::vector<DString>& labels);

    /** 获取数据标签
    */
    const std::vector<DString>& GetDataLabels() const { return m_dataLabels; }

    /** 设置多系列数据（替代全部系列）
    */
    void SetSeriesData(const std::vector<Series>& series);

    /** 添加一个系列
    */
    void AddSeries(const Series& series);

    /** 获取全部系列
    */
    const std::vector<Series>& GetSeries() const { return m_series; }

    /** 获取系列数量
    */
    size_t GetSeriesCount() const { return m_series.size(); }

    /** 设置系列主色（单系列，语义色名或颜色值）
    */
    void SetSeriesColor(const DString& strColor);

    /** 设置坐标轴/网格线颜色（语义色名或颜色值）
    */
    void SetAxisColor(const DString& strColor);

    /** 设置文本标签颜色（语义色名或颜色值）
    */
    void SetLabelColor(const DString& strColor);

    /** 设置是否显示数据值标签
    */
    void SetShowValue(bool bShowValue);

    /** 设置是否显示图例
    */
    void SetLegendVisible(bool bLegendVisible);

    /** 设置是否显示网格线
    */
    void SetShowGrid(bool bShowGrid);

    /** 设置是否显示 Y 轴刻度值
    */
    void SetShowAxisValues(bool bShow);

    /** 设置 Y 轴刻度分段数（1~10）
    */
    void SetAxisDivisions(int32_t nDivisions);

    /** 设置折线粗细（折线图）
    */
    void SetLineWidth(int32_t nLineWidth);

    /** 设置折线绘制模式（折线/曲线）
    */
    void SetLineMode(ChartLineMode lineMode);

    /** 设置是否填充折线下方面积
    */
    void SetAreaFill(bool bAreaFill);

    /** 设置是否绘制数据点圆点（折线）
    */
    void SetShowDataPoints(bool bShow);

    /** 设置柱状图排列模式（分组/堆叠）
    */
    void SetBarMode(ChartBarMode barMode);

    /** 设置饼图是否环形
    */
    void SetDonut(bool bDonut);

    /** 设置饼图是否显示百分比
    */
    void SetShowPercent(bool bShowPercent);

private:
    /** 解析逗号分隔的数值字符串
    */
    static void ParseDataString(const DString& strData, std::vector<double>& data);

    /** 解析逗号分隔的标签字符串
    */
    static void ParseLabelsString(const DString& strLabels, std::vector<DString>& labels);

    /** 获取系列主色（第 index 个系列），失败则自动分配
    */
    UiColor GetSeriesColor(size_t index) const;

    /** 根据索引自动分配系列颜色（色相均匀分布）
    */
    static UiColor MakeSeriesColor(size_t index, size_t count);

    /** 根据索引生成同色系扇区颜色（用于饼图多扇区）
    */
    static UiColor MakeSliceColor(UiColor baseColor, size_t index, size_t count);

    /** 将值格式化为字符串（保留合适精度）
    */
    static DString FormatValue(double value);

    /** 绘制文本（简化封装）
    */
    void DrawChartText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat);

    /** 绘制标题与坐标轴标题
    */
    void PaintTitles(IRender* pRender, const UiRect& rcChart, const UiRect& rcPlot);

    /** 计算绘图区（扣除标题/图例/轴标题/Y轴刻度值空间）
    */
    UiRect CalcPlotRect(IRender* pRender, const UiRect& rcChart) const;

    /** 测量单行文本的宽度（像素，已含 DPI 缩放）
    */
    int32_t MeasureTextWidth(IRender* pRender, const DString& strText) const;

    /** 按字符数估算文本宽度（保守值，规避 MeasureString 对中文测量偏小的问题）
    */
    int32_t EstimateTextWidth(const DString& strText) const;

    /** 取测量与估算中的较大值，作为布局用的安全宽度
    */
    int32_t GetSafeTextWidth(IRender* pRender, const DString& strText) const;

    /** 测量 Y 轴刻度值区域所需的最大宽度（像素，已含 DPI 缩放）
    */
    int32_t MeasureAxisValueWidth(IRender* pRender) const;

    /** 计算所有系列数据的合并范围（不含余量）
    */
    void CalcValueRange(double& dMin, double& dMax) const;

    /** 计算美观的坐标轴刻度（步长取 1/2/5×10ⁿ，刻度值为整数或规整小数）
    *   @param dMin 数据最小值（传入时含余量，返回时对齐到规整刻度）
    *   @param dMax 数据最大值（同上）
    *   @param nDivisions 传入期望分段数，返回实际分段数（可能少于期望值）
    */
    void CalcNiceAxis(double& dMin, double& dMax, int32_t& nDivisions) const;

    /** 绘制折线图
    */
    void PaintLine(IRender* pRender, const UiRect& rcChart);

    /** 绘制柱状图
    */
    void PaintBar(IRender* pRender, const UiRect& rcChart);

    /** 绘制饼图
    */
    void PaintPie(IRender* pRender, const UiRect& rcChart);

    /** 绘制图例（折线/柱状，多系列时显示）
    */
    void PaintLegend(IRender* pRender, const UiRect& rcChart);

private:
    /** 图表类型
    */
    ChartType m_chartType;

    /** 图表标题
    */
    DString m_strTitle;

    /** X 轴标题
    */
    DString m_strXAxisTitle;

    /** Y 轴标题
    */
    DString m_strYAxisTitle;

    /** 类别标签（X 轴类目，所有系列共用）
    */
    std::vector<DString> m_dataLabels;

    /** 多系列数据（单系列接口内部也映射到 m_series[0]）
    */
    std::vector<Series> m_series;

    /** 系列主色（单系列默认色，语义色名）
    */
    DString m_strSeriesColor;

    /** 坐标轴/网格线颜色（语义色名）
    */
    DString m_strAxisColor;

    /** 文本标签颜色（语义色名）
    */
    DString m_strLabelColor;

    /** 是否显示数据值标签
    */
    bool m_bShowValue;

    /** 是否显示图例
    */
    bool m_bLegendVisible;

    /** 是否显示网格线
    */
    bool m_bShowGrid;

    /** 是否显示 Y 轴刻度值
    */
    bool m_bShowAxisValues;

    /** Y 轴刻度分段数
    */
    int32_t m_nAxisDivisions;

    /** 折线粗细
    */
    int32_t m_nLineWidth;

    /** 折线绘制模式
    */
    ChartLineMode m_lineMode;

    /** 是否填充折线下方面积
    */
    bool m_bAreaFill;

    /** 是否绘制数据点圆点
    */
    bool m_bShowDataPoints;

    /** 柱状排列模式
    */
    ChartBarMode m_barMode;

    /** 饼图是否环形
    */
    bool m_bDonut;

    /** 饼图是否显示百分比
    */
    bool m_bShowPercent;
};

} // namespace ui

#endif // UI_CONTROL_CHART_H_
