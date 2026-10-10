#ifndef UI_CONTROL_CHART_H_
#define UI_CONTROL_CHART_H_

#include "duilib/Core/Control.h"
#include "duilib/Animation/AnimationPlayer.h"
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

/** Chart 轻量图表控件（继承Control，纯自绘）
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

    /** 命中元素的类型
    */
    enum class HitType
    {
        kNone = 0,  //未命中
        kPoint = 1, //折线数据点
        kBar   = 2, //柱状图的柱
        kSlice = 3, //饼图扇区
        kLegend = 4 //图例项
    };

    /** 命中检测结果
    */
    struct HitResult
    {
        HitType type = HitType::kNone; //命中类型
        size_t seriesIndex = 0;        //系列索引（折线点/柱/图例有效）
        size_t dataIndex = 0;          //数据索引（折线点/柱/扇区有效）
        UiPoint ptHit;                 //命中位置（控件坐标）
    };

    /** 折线图布局参数（绘制与命中检测共用，保证坐标一致）
    */
    struct LineLayout
    {
        UiRect rcPlot;              //绘图区
        double dMin = 0.0;          //Y 轴下界
        double dMax = 0.0;          //Y 轴上界
        double dRange = 1.0;        //Y 轴范围
        int32_t nDivisions = 1;     //实际刻度分段数（美观刻度对齐后的值）
        double viewStart = 0.0;     //可视窗口起始索引（含小数）
        double viewEnd = 0.0;       //可视窗口结束索引（含小数）
        size_t nMaxCount = 0;       //最大数据点数

        /** 数据索引 → X 像素坐标
        */
        float XToPixel(double index) const {
            double t = (viewEnd <= viewStart) ? 0.5 : ((index - viewStart) / (viewEnd - viewStart));
            return static_cast<float>(rcPlot.left) + static_cast<float>(rcPlot.Width()) * static_cast<float>(t);
        }
        /** 数据值 → Y 像素坐标
        */
        float YToPixel(double value) const {
            double norm = (value - dMin) / dRange;
            return static_cast<float>(rcPlot.bottom) - static_cast<float>(rcPlot.Height()) * static_cast<float>(norm);
        }
        /** X 像素坐标 → 数据索引
        */
        double PixelToX(int32_t px) const {
            if (rcPlot.Width() <= 0) {
                return viewStart;
            }
            double t = static_cast<double>(px - rcPlot.left) / static_cast<double>(rcPlot.Width());
            return viewStart + t * (viewEnd - viewStart);
        }
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

    /** 语言切换时刷新标题/轴标题/类目标签（text_id 关联）
    */
    void OnLanguageChanged(bool bRedraw) override;

    // 鼠标交互（重写基类虚函数，实现 Tooltip / 选中 / 缩放平移）
    bool MouseMove(const EventArgs& msg) override;
    bool MouseLeave(const EventArgs& msg) override;
    bool ButtonDown(const EventArgs& msg) override;
    bool ButtonUp(const EventArgs& msg) override;
    bool MouseWheel(const EventArgs& msg) override;

    /** 设置是否启用数据点 Tooltip（悬停显示系列/类目/值），默认 true
    */
    void SetEnableTooltip(bool bEnable);

    /** 获取是否启用数据点 Tooltip
    */
    bool IsEnableTooltip() const { return m_bEnableTooltip; }

    /** 设置是否启用点击选中（饼图扇区高亮/偏移、柱状单柱高亮），默认 true
    */
    void SetEnableSelect(bool bEnable);

    /** 获取是否启用点击选中
    */
    bool IsEnableSelect() const { return m_bEnableSelect; }

    /** 获取当前选中的元素（无选中时 type == kNone）
    */
    HitResult GetSelected() const { return m_selected; }

    /** 清除选中状态
    */
    void ClearSelection();

    /** 数据点点击事件回调（参数 EventArgs::wParam 为命中元素打包指针，由 GetHitFromEvent 解析）
    */
    void AttachPointClick(const EventCallback& callback);

    /** 数据点悬停事件回调
    */
    void AttachPointHover(const EventCallback& callback);

    /** 从点击/悬停事件的 wParam 解析命中的元素
    *   @return 命中结果（args.wParam 为空时返回 kNone）
    */
    static HitResult GetHitFromEvent(const EventArgs& args);

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

    /** 设置图表标题的语言 ID（text_id，多语言支持）
    */
    void SetTitleId(const DString& strTitleId);

    /** 设置 X 轴标题的语言 ID
    */
    void SetXAxisTitleId(const DString& strTitleId);

    /** 设置 Y 轴标题的语言 ID
    */
    void SetYAxisTitleId(const DString& strTitleId);

    /** 设置类目标签的语言 ID（逗号分隔的多个 STRID，多语言支持）
    */
    void SetDataLabelsId(const DString& strLabelsId);

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

    /** 设置柱状图圆角半径（像素，自动 DPI 缩放），0 表示直角，默认 3
    */
    void SetBarRadius(int32_t nRadius);

    /** 设置柱状图是否渐变填充（顶部亮、底部暗），默认 true
    */
    void SetBarGradient(bool bGradient);

    /** 设置折线图是否绘制发光/阴影效果，默认 true
    */
    void SetLineGlow(bool bGlow);

    /** 设置数据更新动画是否启用（柱体生长/折线渐入），默认 true
    */
    void SetAnimationEnabled(bool bEnabled);

    /** 设置数据更新动画时长（毫秒），默认 300
    */
    void SetAnimationDuration(int32_t nMs);

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

    /** 生成渐变填充的暗色端（用于柱状渐变、折线发光）
    */
    static UiColor MakeDarkerColor(UiColor baseColor, uint8_t nScale);

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

    /** 启动数据更新动画（柱体生长/折线渐入）
    */
    void StartDataAnimation();

    /** 获取当前动画进度（0~1，1 表示动画完成或未启用）
    */
    double GetAnimProgress() const;

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

    /** 计算折线图布局参数（含可视窗口与 Y 轴范围）
    */
    LineLayout CalcLineLayout(IRender* pRender, const UiRect& rcChart) const;

    /** 命中检测：检测鼠标位置命中的元素
    *   @param pRender 渲染接口（用于获取字体测量）
    *   @param ptMouse 鼠标位置（控件坐标）
    *   @return 命中结果
    */
    HitResult HitTest(IRender* pRender, const UiPoint& ptMouse);

    /** 构建命中元素的 Tooltip 文本
    */
    DString BuildHitTooltip(const HitResult& hit) const;

    /** 构建命中元素的显示文本（"系列名: 值"）
    */
    DString BuildHitLabel(const HitResult& hit) const;

    /** 发送数据点点击事件（把命中结果打包进 wParam）
    */
    void SendPointClickEvent(const HitResult& hit);

    /** 发送数据点悬停事件
    */
    void SendPointHoverEvent(const HitResult& hit);

    /** 获取命中的扇区角度（饼图，返回 -1 表示未命中）
    *   @param ptMouse 鼠标位置（控件坐标）
    *   @param rcPie 饼图外接矩形
    *   @param nCenterX/nCenterY 圆心
    *   @param fRadius 半径
    *   @param data 数据
    *   @param dSum 数据总和（正值）
    */
    int32_t HitTestPieSlice(const UiPoint& ptMouse, const UiRect& rcPie,
                            int32_t nCenterX, int32_t nCenterY, float fRadius,
                            const std::vector<double>& data, double dSum) const;

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

    /** 图表标题的语言 ID（空表示不使用）
    */
    DString m_strTitleId;

    /** X 轴标题的语言 ID
    */
    DString m_strXAxisTitleId;

    /** Y 轴标题的语言 ID
    */
    DString m_strYAxisTitleId;

    /** 类目标签的语言 ID（逗号分隔多个 STRID）
    */
    DString m_strDataLabelsId;

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

    /** 柱状图圆角半径（像素，未 DPI 缩放）
    */
    int32_t m_nBarRadius;

    /** 柱状图是否渐变填充
    */
    bool m_bBarGradient;

    /** 折线图是否绘制发光/阴影
    */
    bool m_bLineGlow;

    /** 数据更新动画是否启用
    */
    bool m_bAnimationEnabled;

    /** 数据更新动画时长（毫秒）
    */
    int32_t m_nAnimationDuration;

    /** 数据更新动画播放接口
    */
    std::unique_ptr<AnimationPlayer> m_pAnimPlayer;

    /** 动画进度值（0~100）
    */
    int32_t m_nAnimProgress;

    /** 是否启用数据点 Tooltip
    */
    bool m_bEnableTooltip;

    /** 是否启用点击选中
    */
    bool m_bEnableSelect;

    /** 当前选中的元素（type == kNone 表示无选中）
    */
    HitResult m_selected;

    /** 上次悬停命中的元素（用于判断是否需要更新 Tooltip）
    */
    HitResult m_lastHover;

    /** 折线缩放平移：可视窗口（数据索引的浮点范围 [0, N-1]）
    */
    double m_viewStart;

    /** 折线可视窗口结束索引
    */
    double m_viewEnd;

    /** 是否处于拖拽平移状态
    */
    bool m_bPanning;

    /** 拖拽是否实际产生了位移（用于区分"按下"与"拖动"）
    */
    bool m_bPanMoved;

    /** 拖拽起始鼠标 X（控件坐标）
    */
    int32_t m_panStartX;

    /** 拖拽起始可视窗口 start
    */
    double m_panStartView;
};

} // namespace ui

#endif // UI_CONTROL_CHART_H_
