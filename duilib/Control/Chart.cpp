#include "Chart.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace ui
{

namespace
{
    /** 默认折线粗细
    */
    constexpr int32_t kDefaultLineWidth = 2;

    /** 默认 Y 轴刻度分段数
    */
    constexpr int32_t kDefaultAxisDivisions = 4;

    /** 圆周率
    */
    constexpr double kPi = 3.14159265358979323846;

    /** 自动系列配色（色相均匀分布，共 8 色循环）
    */
    const UiColor kAutoSeriesColors[] = {
        UiColor(0xFF4F8BFF), //蓝
        UiColor(0xFF34C77B), //绿
        UiColor(0xFFFF9F43), //橙
        UiColor(0xFFE74C3C), //红
        UiColor(0xFF9B59B6), //紫
        UiColor(0xFF16A085), //青
        UiColor(0xFFF39C12), //黄
        UiColor(0xFF34495E), //深蓝灰
    };
    constexpr size_t kAutoSeriesColorCount = sizeof(kAutoSeriesColors) / sizeof(kAutoSeriesColors[0]);
}

Chart::Chart(Window* pWindow) :
    BaseClass(pWindow),
    m_chartType(ChartType::kLine),
    m_strSeriesColor(_T("color_accent")),
    m_strAxisColor(_T("border_control_normal")),
    m_strLabelColor(_T("text_default")),
    m_bShowValue(true),
    m_bLegendVisible(false),
    m_bShowGrid(true),
    m_bShowAxisValues(true),
    m_nAxisDivisions(kDefaultAxisDivisions),
    m_nLineWidth(kDefaultLineWidth),
    m_lineMode(ChartLineMode::kStraight),
    m_bAreaFill(false),
    m_bShowDataPoints(true),
    m_barMode(ChartBarMode::kGrouped),
    m_bDonut(false),
    m_bShowPercent(false)
{
    //折线/柱状默认不显示图例（单系列时无意义），饼图默认显示
    //图例默认值在 SetAttribute 时由 chart_type 决定；此处给 false，饼图默认在构造后由外部显式开启
    //为避免歧义，饼图图例默认在解析 chart_type 时自动开启（见 SetAttribute）
}

Chart::~Chart()
{
}

DString Chart::GetType() const
{
    return DUI_CTR_CHART;
}

void Chart::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("chart_type")) {
        if (strValue == _T("bar")) {
            SetChartType(ChartType::kBar);
        }
        else if (strValue == _T("pie")) {
            SetChartType(ChartType::kPie);
        }
        else {
            SetChartType(ChartType::kLine);
        }
    }
    else if (strName == _T("data")) {
        std::vector<double> data;
        ParseDataString(strValue, data);
        SetData(data);
    }
    else if (strName == _T("data_labels")) {
        std::vector<DString> labels;
        ParseLabelsString(strValue, labels);
        SetDataLabels(labels);
    }
    else if ((strName == _T("series_color")) || (strName == _T("seriescolor"))) {
        SetSeriesColor(strValue);
    }
    else if ((strName == _T("axis_color")) || (strName == _T("axiscolor"))) {
        SetAxisColor(strValue);
    }
    else if ((strName == _T("label_color")) || (strName == _T("labelcolor"))) {
        SetLabelColor(strValue);
    }
    else if (strName == _T("title")) {
        SetTitle(strValue);
    }
    else if ((strName == _T("x_axis_title")) || (strName == _T("xaxistitle"))) {
        SetXAxisTitle(strValue);
    }
    else if ((strName == _T("y_axis_title")) || (strName == _T("yaxistitle"))) {
        SetYAxisTitle(strValue);
    }
    else if (strName == _T("show_value")) {
        SetShowValue(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("legend_visible")) || (strName == _T("legendvisible"))) {
        SetLegendVisible(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("show_grid")) || (strName == _T("showgrid"))) {
        SetShowGrid(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("show_axis_values")) || (strName == _T("showaxisvalues"))) {
        SetShowAxisValues(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("axis_divisions")) || (strName == _T("axisdivisions"))) {
        SetAxisDivisions(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("line_width")) || (strName == _T("linewidth"))) {
        SetLineWidth(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("line_mode")) || (strName == _T("linemode"))) {
        if (strValue == _T("curve")) {
            SetLineMode(ChartLineMode::kCurve);
        }
        else {
            SetLineMode(ChartLineMode::kStraight);
        }
    }
    else if ((strName == _T("area_fill")) || (strName == _T("areafill"))) {
        SetAreaFill(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("show_data_points")) || (strName == _T("showdatapoints"))) {
        SetShowDataPoints(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("bar_mode")) || (strName == _T("barmode"))) {
        if (strValue == _T("stacked")) {
            SetBarMode(ChartBarMode::kStacked);
        }
        else {
            SetBarMode(ChartBarMode::kGrouped);
        }
    }
    else if (strName == _T("donut")) {
        SetDonut(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("show_percent")) || (strName == _T("showpercent"))) {
        SetShowPercent(StringUtil::IsValueTrue(strValue));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Chart::SetChartType(ChartType chartType)
{
    if (m_chartType != chartType) {
        m_chartType = chartType;
        //饼图默认开启图例，折线/柱状默认关闭
        if (chartType == ChartType::kPie) {
            m_bLegendVisible = true;
        }
        Invalidate();
    }
}

void Chart::SetTitle(const DString& strTitle)
{
    if (m_strTitle != strTitle) {
        m_strTitle = strTitle;
        Invalidate();
    }
}

void Chart::SetXAxisTitle(const DString& strTitle)
{
    if (m_strXAxisTitle != strTitle) {
        m_strXAxisTitle = strTitle;
        Invalidate();
    }
}

void Chart::SetYAxisTitle(const DString& strTitle)
{
    if (m_strYAxisTitle != strTitle) {
        m_strYAxisTitle = strTitle;
        Invalidate();
    }
}

void Chart::SetData(const std::vector<double>& data)
{
    //单系列接口：映射到 m_series[0]
    if (m_series.empty()) {
        m_series.resize(1);
    }
    m_series[0].data = data;
    if (m_series[0].color.empty()) {
        m_series[0].color = m_strSeriesColor;
    }
    Invalidate();
}

void Chart::AddData(double value)
{
    if (m_series.empty()) {
        m_series.resize(1);
    }
    if (m_series[0].color.empty()) {
        m_series[0].color = m_strSeriesColor;
    }
    m_series[0].data.push_back(value);
    Invalidate();
}

void Chart::ClearData()
{
    m_series.clear();
    Invalidate();
}

const std::vector<double>& Chart::GetData() const
{
    static const std::vector<double> kEmptyData;
    if (m_series.empty()) {
        return kEmptyData;
    }
    return m_series[0].data;
}

void Chart::SetDataLabels(const std::vector<DString>& labels)
{
    m_dataLabels = labels;
    Invalidate();
}

void Chart::SetSeriesData(const std::vector<Series>& series)
{
    m_series = series;
    Invalidate();
}

void Chart::AddSeries(const Series& series)
{
    m_series.push_back(series);
    Invalidate();
}

void Chart::SetSeriesColor(const DString& strColor)
{
    m_strSeriesColor = strColor;
    //同步到第一个系列（如果其颜色未显式设置或为空）
    if (!m_series.empty() && (m_series[0].color.empty() || m_series[0].color == _T("color_accent"))) {
        m_series[0].color = strColor;
    }
    Invalidate();
}

void Chart::SetAxisColor(const DString& strColor)
{
    m_strAxisColor = strColor;
    Invalidate();
}

void Chart::SetLabelColor(const DString& strColor)
{
    m_strLabelColor = strColor;
    Invalidate();
}

void Chart::SetShowValue(bool bShowValue)
{
    if (m_bShowValue != bShowValue) {
        m_bShowValue = bShowValue;
        Invalidate();
    }
}

void Chart::SetLegendVisible(bool bLegendVisible)
{
    if (m_bLegendVisible != bLegendVisible) {
        m_bLegendVisible = bLegendVisible;
        Invalidate();
    }
}

void Chart::SetShowGrid(bool bShowGrid)
{
    if (m_bShowGrid != bShowGrid) {
        m_bShowGrid = bShowGrid;
        Invalidate();
    }
}

void Chart::SetShowAxisValues(bool bShow)
{
    if (m_bShowAxisValues != bShow) {
        m_bShowAxisValues = bShow;
        Invalidate();
    }
}

void Chart::SetAxisDivisions(int32_t nDivisions)
{
    if (nDivisions < 1) {
        nDivisions = 1;
    }
    if (nDivisions > 10) {
        nDivisions = 10;
    }
    if (m_nAxisDivisions != nDivisions) {
        m_nAxisDivisions = nDivisions;
        Invalidate();
    }
}

void Chart::SetLineWidth(int32_t nLineWidth)
{
    if (nLineWidth < 1) {
        nLineWidth = 1;
    }
    if (m_nLineWidth != nLineWidth) {
        m_nLineWidth = nLineWidth;
        Invalidate();
    }
}

void Chart::SetLineMode(ChartLineMode lineMode)
{
    if (m_lineMode != lineMode) {
        m_lineMode = lineMode;
        Invalidate();
    }
}

void Chart::SetAreaFill(bool bAreaFill)
{
    if (m_bAreaFill != bAreaFill) {
        m_bAreaFill = bAreaFill;
        Invalidate();
    }
}

void Chart::SetShowDataPoints(bool bShow)
{
    if (m_bShowDataPoints != bShow) {
        m_bShowDataPoints = bShow;
        Invalidate();
    }
}

void Chart::SetBarMode(ChartBarMode barMode)
{
    if (m_barMode != barMode) {
        m_barMode = barMode;
        Invalidate();
    }
}

void Chart::SetDonut(bool bDonut)
{
    if (m_bDonut != bDonut) {
        m_bDonut = bDonut;
        Invalidate();
    }
}

void Chart::SetShowPercent(bool bShowPercent)
{
    if (m_bShowPercent != bShowPercent) {
        m_bShowPercent = bShowPercent;
        Invalidate();
    }
}

void Chart::ParseDataString(const DString& strData, std::vector<double>& data)
{
    data.clear();
    if (strData.empty()) {
        return;
    }
    std::list<DString> parts = StringUtil::Split(strData, _T(","));
    for (const DString& part : parts) {
        DString s = part;
        StringUtil::Trim(s);
        if (s.empty()) {
            continue;
        }
        data.push_back(StringUtil::StringToDouble(s));
    }
}

void Chart::ParseLabelsString(const DString& strLabels, std::vector<DString>& labels)
{
    labels.clear();
    if (strLabels.empty()) {
        return;
    }
    std::list<DString> parts = StringUtil::Split(strLabels, _T(","));
    for (const DString& part : parts) {
        DString s = part;
        StringUtil::Trim(s);
        labels.push_back(s);
    }
}

UiColor Chart::GetSeriesColor(size_t index) const
{
    //优先用系列显式指定的颜色
    if (index < m_series.size() && !m_series[index].color.empty()) {
        UiColor c = GetUiColor(m_series[index].color);
        if (!c.IsEmpty()) {
            return c;
        }
    }
    //回退到默认系列色
    UiColor c = GetUiColor(m_strSeriesColor);
    if (!c.IsEmpty()) {
        return c;
    }
    //最后自动分配
    return MakeSeriesColor(index, m_series.size());
}

UiColor Chart::MakeSeriesColor(size_t index, size_t /*count*/)
{
    return kAutoSeriesColors[index % kAutoSeriesColorCount];
}

UiColor Chart::MakeSliceColor(UiColor baseColor, size_t index, size_t count)
{
    //基于基准色，通过亮度变化生成同色系的多扇区颜色
    if (count <= 1) {
        return baseColor;
    }
    uint8_t a = baseColor.GetA();
    uint8_t r = baseColor.GetR();
    uint8_t g = baseColor.GetG();
    uint8_t b = baseColor.GetB();

    double t = (count <= 1) ? 0.0 : (static_cast<double>(index) / static_cast<double>(count - 1));
    //亮度在 0.65 ~ 1.0 之间变化，保持同一色相
    double brightness = 1.0 - 0.35 * t;
    r = static_cast<uint8_t>(r * brightness);
    g = static_cast<uint8_t>(g * brightness);
    b = static_cast<uint8_t>(b * brightness);
    return UiColor(UiColor::MakeARGB(a, r, g, b));
}

DString Chart::FormatValue(double value)
{
    //根据数值大小选择合适精度：整数直接显示，小数保留2位
    if (std::fabs(value - std::round(value)) < 1e-9) {
        return StringUtil::Printf(_T("%lld"), static_cast<long long>(value));
    }
    return StringUtil::Printf(_T("%.2f"), value);
}

void Chart::DrawChartText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat)
{
    if ((pRender == nullptr) || strText.empty()) {
        return;
    }
    UiColor textColor = GetUiColor(m_strLabelColor);
    if (textColor.IsEmpty()) {
        textColor = UiColor(0xFF000000);
    }
    IFont* pFont = GetIFontById(_T("system_regular_12"));
    DrawStringParam param;
    param.textRect = rc;
    param.dwTextColor = textColor;
    param.pFont = pFont;
    param.uFormat = uFormat;
    pRender->DrawString(strText, param);
}

int32_t Chart::MeasureTextWidth(IRender* pRender, const DString& strText) const
{
    if ((pRender == nullptr) || strText.empty()) {
        return 0;
    }
    MeasureStringParam param;
    param.pFont = GetIFontById(_T("system_regular_12"));
    param.uFormat = TEXT_SINGLELINE;
    UiRect rc = pRender->MeasureString(strText, param);
    return rc.Width();
}

int32_t Chart::EstimateTextWidth(const DString& strText) const
{
    //按字符数估算：全角字符 ≈ 字号宽，半角字符 ≈ 0.6×字号宽（保守取值）
    if (strText.empty()) {
        return 0;
    }
    const int32_t nFullChar = Dpi().GetScaleInt(13);
    const int32_t nHalfChar = Dpi().GetScaleInt(8);
    int32_t nWidth = 0;
    for (wchar_t ch : strText) {
        if (ch > 0xFF) {
            nWidth += nFullChar;
        }
        else {
            nWidth += nHalfChar;
        }
    }
    return nWidth;
}

int32_t Chart::GetSafeTextWidth(IRender* pRender, const DString& strText) const
{
    return std::max(MeasureTextWidth(pRender, strText), EstimateTextWidth(strText));
}

void Chart::CalcValueRange(double& dMin, double& dMax) const
{
    //计算所有系列数据的合并范围（不含余量）
    dMin = 0.0;
    dMax = 0.0;
    bool bFirst = true;
    for (const Series& s : m_series) {
        for (double v : s.data) {
            if (bFirst) {
                dMin = dMax = v;
                bFirst = false;
            }
            else {
                dMin = std::min(dMin, v);
                dMax = std::max(dMax, v);
            }
        }
    }
    if (bFirst) {
        dMin = 0.0;
        dMax = 1.0;
    }
    if (dMax <= dMin) {
        dMax = dMin + 1.0;
    }
}

void Chart::CalcNiceAxis(double& dMin, double& dMax, int32_t& nDivisions) const
{
    //计算规整刻度：步长取 1/2/5×10ⁿ，使刻度值整齐可读
    double dRange = dMax - dMin;
    if (dRange <= 0.0) {
        dRange = 1.0;
        dMax = dMin + 1.0;
    }
    int32_t nWantDivisions = std::max(1, nDivisions);
    double rough = dRange / nWantDivisions;
    if (rough <= 0.0) {
        rough = 1.0;
    }
    double mag = std::pow(10.0, std::floor(std::log10(rough)));
    double norm = rough / mag;
    double stepMul = (norm <= 1.5) ? 1.0 : (norm <= 3.5) ? 2.0 : (norm <= 7.5) ? 5.0 : 10.0;
    double step = stepMul * mag;
    double niceMin = std::floor(dMin / step) * step;
    double niceMax = std::ceil(dMax / step) * step;
    dMin = niceMin;
    dMax = niceMax;
    int32_t nActual = static_cast<int32_t>(std::round((niceMax - niceMin) / step));
    nDivisions = std::max(1, std::min(nActual, nWantDivisions * 2));
}

int32_t Chart::MeasureAxisValueWidth(IRender* pRender) const
{
    //测量 Y 轴刻度值文本的最大宽度（按美观刻度取值，与实际绘制一致）
    if (!m_bShowAxisValues || (m_chartType == ChartType::kPie)) {
        return 0;
    }
    double dMin = 0.0;
    double dMax = 0.0;
    CalcValueRange(dMin, dMax);
    double dRange = dMax - dMin;
    dMax = dMax + dRange * 0.1;
    dMin = dMin - dRange * 0.1;
    int32_t nDivisions = m_nAxisDivisions;
    CalcNiceAxis(dMin, dMax, nDivisions);
    dRange = dMax - dMin;

    int32_t nMaxWidth = 0;
    for (int32_t i = 0; i <= nDivisions; ++i) {
        double val = dMax - dRange * (static_cast<double>(i) / static_cast<double>(nDivisions));
        nMaxWidth = std::max(nMaxWidth, MeasureTextWidth(pRender, FormatValue(val)));
    }
    return nMaxWidth;
}

UiRect Chart::CalcPlotRect(IRender* pRender, const UiRect& rcChart) const
{
    UiRect rc = rcChart;
    //顶部：标题空间
    if (!m_strTitle.empty()) {
        rc.top += Dpi().GetScaleInt(26);
    }
    //左侧：Y 轴刻度值 + Y 轴标题空间（按实际文本宽度动态预留，避免刻度值被裁剪）
    int32_t nLeftMargin = Dpi().GetScaleInt(4);
    int32_t nAxisValueWidth = MeasureAxisValueWidth(pRender);
    if (nAxisValueWidth > 0) {
        nLeftMargin += nAxisValueWidth + Dpi().GetScaleInt(4);
    }
    if (!m_strYAxisTitle.empty()) {
        nLeftMargin += Dpi().GetScaleInt(18);
    }
    rc.left += nLeftMargin;
    //右侧：图例空间（折线/柱状多系列时）
    if ((m_chartType != ChartType::kPie) && m_bLegendVisible && (m_series.size() > 1)) {
        rc.right -= Dpi().GetScaleInt(110);
    }
    //底部：X 轴标题 + 类目标签空间
    rc.bottom -= Dpi().GetScaleInt(20);
    if (!m_strXAxisTitle.empty()) {
        rc.bottom -= Dpi().GetScaleInt(20);
    }
    return rc;
}

void Chart::PaintTitles(IRender* pRender, const UiRect& rcChart, const UiRect& rcPlot)
{
    //图表标题（居中）
    if (!m_strTitle.empty()) {
        UiRect rcTitle;
        rcTitle.left = rcChart.left;
        rcTitle.right = rcChart.right;
        rcTitle.top = rcChart.top;
        rcTitle.bottom = rcChart.top + Dpi().GetScaleInt(24);
        IFont* pTitleFont = GetIFontById(_T("system_bold_14"));
        DrawStringParam param;
        param.textRect = rcTitle;
        param.dwTextColor = GetUiColor(m_strLabelColor);
        param.pFont = pTitleFont;
        param.uFormat = TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE;
        pRender->DrawString(m_strTitle, param);
    }

    //X 轴标题（居中，底部）
    if (!m_strXAxisTitle.empty()) {
        UiRect rcAxis;
        rcAxis.left = rcPlot.left;
        rcAxis.right = rcPlot.right;
        rcAxis.top = rcPlot.bottom + Dpi().GetScaleInt(20);
        rcAxis.bottom = rcChart.bottom;
        DrawChartText(pRender, m_strXAxisTitle, rcAxis, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE);
    }

    //Y 轴标题（纵向显示在刻度值左侧，避免与刻度值重叠）
    if (!m_strYAxisTitle.empty()) {
        UiRect rcAxis;
        rcAxis.left = rcChart.left + Dpi().GetScaleInt(2);
        rcAxis.right = rcPlot.left - Dpi().GetScaleInt(4);
        if (m_bShowAxisValues && (m_chartType != ChartType::kPie)) {
            //扣除刻度值区域，标题只占最左侧一列
            rcAxis.right = rcAxis.left + Dpi().GetScaleInt(16);
            if (rcAxis.right > rcPlot.left - Dpi().GetScaleInt(4)) {
                rcAxis.right = rcPlot.left - Dpi().GetScaleInt(4);
            }
        }
        rcAxis.top = rcPlot.top;
        rcAxis.bottom = rcPlot.bottom;
        DrawChartText(pRender, m_strYAxisTitle, rcAxis, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE | TEXT_END_ELLIPSIS | TEXT_VERTICAL);
    }
}

void Chart::PaintStateImages(IRender* pRender)
{
    BaseClass::PaintStateImages(pRender);
    if (pRender == nullptr) {
        return;
    }

    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    if ((rc.Width() <= 0) || (rc.Height() <= 0)) {
        return;
    }
    if (m_series.empty()) {
        return;
    }
    //检查是否有任何数据
    bool bHasData = false;
    for (const Series& s : m_series) {
        if (!s.data.empty()) {
            bHasData = true;
            break;
        }
    }
    if (!bHasData) {
        return;
    }

    switch (m_chartType) {
    case ChartType::kLine:
        PaintLine(pRender, rc);
        break;
    case ChartType::kBar:
        PaintBar(pRender, rc);
        break;
    case ChartType::kPie:
        PaintPie(pRender, rc);
        break;
    default:
        break;
    }
}

void Chart::PaintLine(IRender* pRender, const UiRect& rcChart)
{
    UiRect rcPlot = CalcPlotRect(pRender, rcChart);
    PaintTitles(pRender, rcChart, rcPlot);
    if ((rcPlot.Width() <= 0) || (rcPlot.Height() <= 0)) {
        return;
    }

    UiColor axisColor = GetUiColor(m_strAxisColor);
    if (axisColor.IsEmpty()) {
        axisColor = UiColor(0xFFD0D0D0);
    }

    //计算所有系列的数据范围（合并）
    double dMin = 0.0;
    double dMax = 0.0;
    CalcValueRange(dMin, dMax);
    //上下各留 10% 余量
    double dRange = dMax - dMin;
    dMax = dMax + dRange * 0.1;
    dMin = dMin - dRange * 0.1;
    //美观刻度：步长取 1/2/5×10ⁿ，刻度值整齐可读
    int32_t nDiv = m_nAxisDivisions;
    CalcNiceAxis(dMin, dMax, nDiv);
    dRange = dMax - dMin;

    const int32_t nLeft = rcPlot.left;
    const int32_t nRight = rcPlot.right;
    const int32_t nTop = rcPlot.top;
    const int32_t nBottom = rcPlot.bottom;
    const float fPlotWidth = static_cast<float>(nRight - nLeft);
    const float fPlotHeight = static_cast<float>(nBottom - nTop);

    //Y 轴刻度值文本宽度（直接测量实际要绘制的刻度值，保证宽度精确一致）
    int32_t nAxisValueWidth = 0;
    if (m_bShowAxisValues) {
        for (int32_t i = 0; i <= nDiv; ++i) {
            double val = dMax - dRange * (static_cast<double>(i) / static_cast<double>(nDiv));
            nAxisValueWidth = std::max(nAxisValueWidth, GetSafeTextWidth(pRender, FormatValue(val)));
        }
    }

    //Y 轴刻度值 + 网格线
    if (m_bShowAxisValues || m_bShowGrid) {
        for (int32_t i = 0; i <= nDiv; ++i) {
            float fy = static_cast<float>(nTop) + fPlotHeight * static_cast<float>(i) / static_cast<float>(nDiv);
            if (m_bShowGrid && (i > 0) && (i < nDiv)) {
                pRender->DrawLine(UiPointF(static_cast<float>(nLeft), fy),
                                  UiPointF(static_cast<float>(nRight), fy),
                                  axisColor, 1.0f);
            }
            //Y 轴刻度值（靠右对齐在左侧）
            if (m_bShowAxisValues) {
                double val = dMax - dRange * (static_cast<double>(i) / static_cast<double>(nDiv));
                DString strVal = FormatValue(val);
                UiRect rcText;
                rcText.right = nLeft - Dpi().GetScaleInt(4);
                rcText.left = rcText.right - nAxisValueWidth - Dpi().GetScaleInt(4);
                rcText.top = static_cast<int32_t>(fy) - Dpi().GetScaleInt(8);
                rcText.bottom = rcText.top + Dpi().GetScaleInt(16);
                DrawChartText(pRender, strVal, rcText, TEXT_RIGHT | TEXT_VCENTER | TEXT_SINGLELINE);
            }
        }
    }

    //获取最大数据点数（用于 X 轴归一化）
    size_t nMaxCount = 0;
    for (const Series& s : m_series) {
        nMaxCount = std::max(nMaxCount, s.data.size());
    }
    if (nMaxCount == 0) {
        return;
    }

    //逐系列绘制
    IRenderFactory* pFactory = GlobalManager::Instance().GetRenderFactory();
    for (size_t si = 0; si < m_series.size(); ++si) {
        const std::vector<double>& data = m_series[si].data;
        if (data.empty()) {
            continue;
        }
        UiColor seriesColor = GetSeriesColor(si);
        const size_t nCount = data.size();

        //计算各点坐标
        std::vector<UiPointF> points;
        points.reserve(nCount);
        for (size_t i = 0; i < nCount; ++i) {
            double t = (nMaxCount == 1) ? 0.5 : (static_cast<double>(i) / static_cast<double>(nMaxCount - 1));
            float fx = static_cast<float>(nLeft) + fPlotWidth * static_cast<float>(t);
            double norm = (data[i] - dMin) / dRange;
            float fy = static_cast<float>(nBottom) - fPlotHeight * static_cast<float>(norm);
            points.emplace_back(fx, fy);
        }

        //面积填充（在折线下方填充到基线）
        if (m_bAreaFill && (nCount >= 2) && (pFactory != nullptr)) {
            IPath* pPath = pFactory->CreatePath();
            std::unique_ptr<IPath> spPath(pPath);
            if (pPath != nullptr) {
                float baseY = static_cast<float>(nBottom);
                pPath->AddLine(points[0].x, baseY, points[0].x, points[0].y);
                if (m_lineMode == ChartLineMode::kCurve) {
                    for (size_t i = 0; i + 1 < nCount; ++i) {
                        float cx1 = points[i].x + (points[i + 1].x - points[i].x) * 0.5f;
                        float cx2 = points[i + 1].x - (points[i + 1].x - points[i].x) * 0.5f;
                        pPath->AddBezier(points[i].x, points[i].y, cx1, points[i].y, cx2, points[i + 1].y, points[i + 1].x, points[i + 1].y);
                    }
                }
                else {
                    for (size_t i = 0; i + 1 < nCount; ++i) {
                        pPath->AddLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
                    }
                }
                pPath->AddLine(points[nCount - 1].x, points[nCount - 1].y, points[nCount - 1].x, baseY);
                pPath->Close();
                //半透明填充
                UiColor fillColor = UiColor(UiColor::MakeARGB(64, seriesColor.GetR(), seriesColor.GetG(), seriesColor.GetB()));
                IBrush* pBrush = pFactory->CreateBrush(fillColor);
                std::unique_ptr<IBrush> spBrush(pBrush);
                if (pBrush != nullptr) {
                    pRender->FillPath(pPath, pBrush);
                }
            }
        }

        //绘制折线/曲线
        if (nCount >= 2) {
            const float fLineWidth = static_cast<float>(Dpi().GetScaleInt(m_nLineWidth));
            if (m_lineMode == ChartLineMode::kCurve) {
                for (size_t i = 0; i + 1 < nCount; ++i) {
                    //用贝塞尔平滑：控制点取相邻中点水平延伸
                    float cx1 = points[i].x + (points[i + 1].x - points[i].x) * 0.5f;
                    float cx2 = points[i + 1].x - (points[i + 1].x - points[i].x) * 0.5f;
                    IPath* pPath = pFactory != nullptr ? pFactory->CreatePath() : nullptr;
                    std::unique_ptr<IPath> spPath(pPath);
                    if (pPath != nullptr) {
                        pPath->AddBezier(points[i].x, points[i].y, cx1, points[i].y, cx2, points[i + 1].y, points[i + 1].x, points[i + 1].y);
                        IPen* pPen = pFactory->CreatePen(seriesColor, fLineWidth);
                        std::unique_ptr<IPen> spPen(pPen);
                        if (pPen != nullptr) {
                            pRender->DrawPath(pPath, pPen);
                        }
                    }
                }
            }
            else {
                for (size_t i = 0; i + 1 < nCount; ++i) {
                    pRender->DrawLine(points[i], points[i + 1], seriesColor, fLineWidth);
                }
            }
        }

        //数据点 + 数值标签
        if (m_bShowDataPoints) {
            const float fRadius = static_cast<float>(Dpi().GetScaleInt(3));
            for (size_t i = 0; i < nCount; ++i) {
                pRender->FillCircle(points[i], fRadius, seriesColor, GetAlpha());
            }
        }
        if (m_bShowValue) {
            //多系列时上下错开：偶数系列标签在点上方，奇数系列在点下方，避免相互重叠
            const bool bBelow = (si % 2 == 1);
            //同一系列内相邻标签水平重叠时，跳过后一个
            int32_t nLastLabelRight = std::numeric_limits<int32_t>::min();
            for (size_t i = 0; i < nCount; ++i) {
                DString strValue = FormatValue(data[i]);
                int32_t nLabelLeft = static_cast<int32_t>(points[i].x) - Dpi().GetScaleInt(30);
                int32_t nLabelRight = static_cast<int32_t>(points[i].x) + Dpi().GetScaleInt(30);
                if (nLabelLeft < nLastLabelRight) {
                    //与同系列前一个标签水平重叠，跳过避免文字叠文字
                    continue;
                }
                UiRect rcText;
                rcText.left = std::max(nLabelLeft, rcPlot.left);
                rcText.right = std::min(nLabelRight, rcPlot.right);
                if (bBelow) {
                    rcText.top = static_cast<int32_t>(points[i].y) + Dpi().GetScaleInt(4);
                    rcText.bottom = rcText.top + Dpi().GetScaleInt(18);
                }
                else {
                    rcText.top = static_cast<int32_t>(points[i].y) - Dpi().GetScaleInt(20);
                    rcText.bottom = static_cast<int32_t>(points[i].y) - Dpi().GetScaleInt(2);
                }
                DrawChartText(pRender, strValue, rcText, TEXT_HCENTER | (bBelow ? TEXT_TOP : TEXT_BOTTOM) | TEXT_SINGLELINE);
                nLastLabelRight = nLabelRight;
            }
        }
    }

    //X 轴类目标签（在 X 轴标题下方）
    if (!m_dataLabels.empty()) {
        const size_t nLabelCount = std::min(m_dataLabels.size(), nMaxCount);
        int32_t nLabelTop = nBottom + Dpi().GetScaleInt(2);
        int32_t nLabelBottom = nBottom + Dpi().GetScaleInt(18);
        if (!m_strXAxisTitle.empty()) {
            nLabelTop = nBottom + Dpi().GetScaleInt(2);
            nLabelBottom = nBottom + Dpi().GetScaleInt(18);
        }
        //先计算所有标签矩形（实测宽度 + 钳位到图表区域内）
        std::vector<UiRect> labelRects(nLabelCount);
        for (size_t i = 0; i < nLabelCount; ++i) {
            double t = (nMaxCount == 1) ? 0.5 : (static_cast<double>(i) / static_cast<double>(nMaxCount - 1));
            float fx = static_cast<float>(nLeft) + fPlotWidth * static_cast<float>(t);
            int32_t nTextWidth = GetSafeTextWidth(pRender, m_dataLabels[i]);
            if (nTextWidth <= 0) {
                nTextWidth = Dpi().GetScaleInt(20);
            }
            UiRect rc;
            rc.left = std::max(static_cast<int32_t>(fx) - nTextWidth / 2, rcChart.left);
            rc.right = std::min(static_cast<int32_t>(fx) + nTextWidth / 2 + (nTextWidth % 2), rcChart.right);
            rc.top = nLabelTop;
            rc.bottom = nLabelBottom;
            labelRects[i] = rc;
        }
        //抽稀：贪心选择互不重叠的标签，但保证末标签始终绘制（与最后已选标签重叠时用末标签替换之）
        std::vector<size_t> drawn;
        int32_t nLastRight = std::numeric_limits<int32_t>::min();
        for (size_t i = 0; i < nLabelCount; ++i) {
            if (labelRects[i].left > nLastRight) {
                drawn.push_back(i);
                nLastRight = labelRects[i].right;
            }
        }
        if (!drawn.empty() && (drawn.back() != nLabelCount - 1)) {
            //末标签未绘制：若替换最后一个已选标签后不与更前的标签重叠，则替换
            UiRect rcLast = labelRects[nLabelCount - 1];
            bool bCanReplace = (drawn.size() == 1);
            if (!bCanReplace) {
                UiRect rcPrev = labelRects[drawn[drawn.size() - 2]];
                bCanReplace = (rcLast.left > rcPrev.right);
            }
            if (bCanReplace) {
                drawn.back() = nLabelCount - 1;
            }
        }
        for (size_t i : drawn) {
            DrawChartText(pRender, m_dataLabels[i], labelRects[i], TEXT_HCENTER | TEXT_TOP | TEXT_SINGLELINE | TEXT_END_ELLIPSIS);
        }
    }

    //多系列图例
    PaintLegend(pRender, rcChart);
}

void Chart::PaintBar(IRender* pRender, const UiRect& rcChart)
{
    UiRect rcPlot = CalcPlotRect(pRender, rcChart);
    PaintTitles(pRender, rcChart, rcPlot);
    if ((rcPlot.Width() <= 0) || (rcPlot.Height() <= 0)) {
        return;
    }

    UiColor axisColor = GetUiColor(m_strAxisColor);
    if (axisColor.IsEmpty()) {
        axisColor = UiColor(0xFFD0D0D0);
    }

    //计算数据范围（柱状图支持负值）
    double dMin = 0.0;
    double dMax = 0.0;
    CalcValueRange(dMin, dMax);
    //堆叠模式需要考虑堆叠后的正负最大值
    if (m_barMode == ChartBarMode::kStacked) {
        dMin = 0.0;
        dMax = 0.0;
        size_t nCount = 0;
        for (const Series& s : m_series) {
            nCount = std::max(nCount, s.data.size());
        }
        for (size_t i = 0; i < nCount; ++i) {
            double posSum = 0.0;
            double negSum = 0.0;
            for (const Series& s : m_series) {
                if (i < s.data.size()) {
                    double v = s.data[i];
                    if (v >= 0.0) {
                        posSum += v;
                    }
                    else {
                        negSum += v;
                    }
                }
            }
            dMax = std::max(dMax, posSum);
            dMin = std::min(dMin, negSum);
        }
    }
    //确保范围有效，并留余量 + 美观刻度
    if (dMax <= dMin) {
        dMax = dMin + 1.0;
    }
    double dRange = dMax - dMin;
    dMax = dMax + dRange * 0.05;
    dMin = dMin - dRange * 0.05;
    int32_t nDiv = m_nAxisDivisions;
    CalcNiceAxis(dMin, dMax, nDiv);
    dRange = dMax - dMin;

    const int32_t nLeft = rcPlot.left;
    const int32_t nRight = rcPlot.right;
    const int32_t nTop = rcPlot.top;
    const int32_t nBottom = rcPlot.bottom;
    const float fPlotWidth = static_cast<float>(nRight - nLeft);
    const float fPlotHeight = static_cast<float>(nBottom - nTop);

    //零基线 Y 坐标
    double zeroNorm = (0.0 - dMin) / dRange;
    float fZeroY = static_cast<float>(nBottom) - fPlotHeight * static_cast<float>(zeroNorm);
    fZeroY = std::max(static_cast<float>(nTop), std::min(fZeroY, static_cast<float>(nBottom)));

    //Y 轴刻度值文本宽度（直接测量实际要绘制的刻度值，保证宽度精确一致）
    int32_t nAxisValueWidth = 0;
    if (m_bShowAxisValues) {
        for (int32_t i = 0; i <= nDiv; ++i) {
            double val = dMax - dRange * (static_cast<double>(i) / static_cast<double>(nDiv));
            nAxisValueWidth = std::max(nAxisValueWidth, GetSafeTextWidth(pRender, FormatValue(val)));
        }
    }

    //Y 轴刻度 + 网格
    if (m_bShowAxisValues || m_bShowGrid) {
        for (int32_t i = 0; i <= nDiv; ++i) {
            float fy = static_cast<float>(nTop) + fPlotHeight * static_cast<float>(i) / static_cast<float>(nDiv);
            if (m_bShowGrid && (i > 0) && (i < nDiv)) {
                pRender->DrawLine(UiPointF(static_cast<float>(nLeft), fy),
                                  UiPointF(static_cast<float>(nRight), fy),
                                  axisColor, 1.0f);
            }
            if (m_bShowAxisValues) {
                double val = dMax - dRange * (static_cast<double>(i) / static_cast<double>(nDiv));
                DString strVal = FormatValue(val);
                UiRect rcText;
                rcText.right = nLeft - Dpi().GetScaleInt(4);
                rcText.left = rcText.right - nAxisValueWidth - Dpi().GetScaleInt(4);
                rcText.top = static_cast<int32_t>(fy) - Dpi().GetScaleInt(8);
                rcText.bottom = rcText.top + Dpi().GetScaleInt(16);
                DrawChartText(pRender, strVal, rcText, TEXT_RIGHT | TEXT_VCENTER | TEXT_SINGLELINE);
            }
        }
    }

    //零基线（如有负值）
    if (dMin < 0.0) {
        pRender->DrawLine(UiPointF(static_cast<float>(nLeft), fZeroY),
                          UiPointF(static_cast<float>(nRight), fZeroY),
                          axisColor, 1.0f);
    }

    //获取类目数量
    size_t nCategoryCount = 0;
    for (const Series& s : m_series) {
        nCategoryCount = std::max(nCategoryCount, s.data.size());
    }
    if (nCategoryCount == 0) {
        return;
    }

    const size_t nSeriesCount = m_series.size();
    const float fSlotWidth = fPlotWidth / static_cast<float>(nCategoryCount);

    if (m_barMode == ChartBarMode::kStacked) {
        //堆叠模式：每个类目一根柱子，各系列累加
        const float fBarWidth = fSlotWidth * 0.6f;
        for (size_t i = 0; i < nCategoryCount; ++i) {
            float fCenterX = static_cast<float>(nLeft) + fSlotWidth * (static_cast<float>(i) + 0.5f);
            float fBarLeft = fCenterX - fBarWidth / 2.0f;
            //正堆叠（从零基线向上）
            float posTop = fZeroY;
            for (size_t si = 0; si < nSeriesCount; ++si) {
                if (i >= m_series[si].data.size()) {
                    continue;
                }
                double v = m_series[si].data[i];
                if (v >= 0.0) {
                    float h = fPlotHeight * static_cast<float>(v / dRange);
                    float segTop = posTop - h;
                    UiRectF rcBar(fBarLeft, segTop, fBarLeft + fBarWidth, posTop);
                    pRender->FillRect(rcBar, GetSeriesColor(si), GetAlpha());
                    posTop = segTop;
                }
            }
            //负堆叠（从零基线向下）
            float negBottom = fZeroY;
            for (size_t si = 0; si < nSeriesCount; ++si) {
                if (i >= m_series[si].data.size()) {
                    continue;
                }
                double v = m_series[si].data[i];
                if (v < 0.0) {
                    float h = fPlotHeight * static_cast<float>((-v) / dRange);
                    float segBottom = negBottom + h;
                    UiRectF rcBar(fBarLeft, negBottom, fBarLeft + fBarWidth, segBottom);
                    pRender->FillRect(rcBar, GetSeriesColor(si), GetAlpha());
                    negBottom = segBottom;
                }
            }
            //数值标签（显示堆叠总值）
            if (m_bShowValue) {
                double total = 0.0;
                for (size_t si = 0; si < nSeriesCount; ++si) {
                    if (i < m_series[si].data.size()) {
                        total += m_series[si].data[i];
                    }
                }
                DString strValue = FormatValue(total);
                UiRect rcText;
                rcText.left = static_cast<int32_t>(fCenterX) - Dpi().GetScaleInt(40);
                rcText.right = static_cast<int32_t>(fCenterX) + Dpi().GetScaleInt(40);
                rcText.top = static_cast<int32_t>(posTop) - Dpi().GetScaleInt(18);
                rcText.bottom = static_cast<int32_t>(posTop);
                DrawChartText(pRender, strValue, rcText, TEXT_HCENTER | TEXT_BOTTOM | TEXT_SINGLELINE);
            }
        }
    }
    else {
        //分组模式：每个类目内多根柱子并排
        const float fGroupWidth = fSlotWidth * 0.8f;
        const float fBarWidth = fGroupWidth / static_cast<float>(nSeriesCount);
        for (size_t si = 0; si < nSeriesCount; ++si) {
            for (size_t i = 0; i < m_series[si].data.size(); ++i) {
                double v = m_series[si].data[i];
                float fCenterX = static_cast<float>(nLeft) + fSlotWidth * (static_cast<float>(i) + 0.5f);
                float fGroupLeft = fCenterX - fGroupWidth / 2.0f;
                float fBarLeft = fGroupLeft + fBarWidth * static_cast<float>(si);
                float fBarTop, fBarBottom;
                if (v >= 0.0) {
                    float h = fPlotHeight * static_cast<float>(v / dRange);
                    fBarTop = fZeroY - h;
                    fBarBottom = fZeroY;
                }
                else {
                    float h = fPlotHeight * static_cast<float>((-v) / dRange);
                    fBarTop = fZeroY;
                    fBarBottom = fZeroY + h;
                }
                UiRectF rcBar(fBarLeft, fBarTop, fBarLeft + fBarWidth, fBarBottom);
                pRender->FillRect(rcBar, GetSeriesColor(si), GetAlpha());

                //数值标签
                if (m_bShowValue) {
                    DString strValue = FormatValue(v);
                    UiRect rcText;
                    rcText.left = static_cast<int32_t>(fBarLeft) - Dpi().GetScaleInt(20);
                    rcText.right = static_cast<int32_t>(fBarLeft + fBarWidth) + Dpi().GetScaleInt(20);
                    if (v >= 0.0) {
                        rcText.top = static_cast<int32_t>(fBarTop) - Dpi().GetScaleInt(18);
                        rcText.bottom = static_cast<int32_t>(fBarTop);
                    }
                    else {
                        rcText.top = static_cast<int32_t>(fBarBottom);
                        rcText.bottom = static_cast<int32_t>(fBarBottom) + Dpi().GetScaleInt(18);
                    }
                    DrawChartText(pRender, strValue, rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE);
                }
            }
        }
    }

    //X 轴类目标签
    if (!m_dataLabels.empty()) {
        for (size_t i = 0; (i < m_dataLabels.size()) && (i < nCategoryCount); ++i) {
            float fCenterX = static_cast<float>(nLeft) + fSlotWidth * (static_cast<float>(i) + 0.5f);
            UiRect rcLabel;
            rcLabel.left = static_cast<int32_t>(fCenterX) - static_cast<int32_t>(fSlotWidth / 2.0f);
            rcLabel.right = static_cast<int32_t>(fCenterX) + static_cast<int32_t>(fSlotWidth / 2.0f);
            rcLabel.top = nBottom + Dpi().GetScaleInt(2);
            rcLabel.bottom = nBottom + Dpi().GetScaleInt(18);
            DrawChartText(pRender, m_dataLabels[i], rcLabel, TEXT_HCENTER | TEXT_TOP | TEXT_SINGLELINE | TEXT_END_ELLIPSIS);
        }
    }

    //多系列图例
    PaintLegend(pRender, rcChart);
}

void Chart::PaintPie(IRender* pRender, const UiRect& rcChart)
{
    UiRect rcPlot = CalcPlotRect(pRender, rcChart);
    PaintTitles(pRender, rcChart, rcPlot);
    if ((rcPlot.Width() <= 0) || (rcPlot.Height() <= 0)) {
        return;
    }

    //饼图只用第一个系列
    if (m_series.empty() || m_series[0].data.empty()) {
        return;
    }
    const std::vector<double>& data = m_series[0].data;
    const size_t nCount = data.size();

    UiColor seriesColor = GetSeriesColor(0);

    //计算总和（忽略负值）
    double dSum = 0.0;
    for (double v : data) {
        if (v > 0.0) {
            dSum += v;
        }
    }
    if (dSum <= 0.0) {
        return;
    }

    //预先构建图例文本（测量与绘制共用，避免两处逻辑不一致）
    std::vector<DString> legendTexts;
    legendTexts.reserve(nCount);
    for (size_t i = 0; i < nCount; ++i) {
        DString strLabel = (i < m_dataLabels.size()) ? m_dataLabels[i] : DString();
        if (strLabel.empty()) {
            strLabel = FormatValue(data[i]);
        }
        else {
            strLabel = strLabel + _T("  ") + FormatValue(data[i]);
            if (m_bShowPercent) {
                double percent = data[i] / dSum * 100.0;
                strLabel = strLabel + StringUtil::Printf(_T(" (%.1f%%)"), percent);
            }
        }
        legendTexts.push_back(strLabel);
    }

    //饼图绘制区域：右侧按图例文本安全宽度预留（上限为绘图区宽度的 60%），避免文字被截断
    int32_t nLegendWidth = 0;
    if (m_bLegendVisible) {
        const int32_t nColorBoxSize = Dpi().GetScaleInt(12);
        int32_t nMaxTextWidth = 0;
        for (const DString& strLabel : legendTexts) {
            nMaxTextWidth = std::max(nMaxTextWidth, GetSafeTextWidth(pRender, strLabel));
        }
        nLegendWidth = nColorBoxSize + Dpi().GetScaleInt(6) + nMaxTextWidth + Dpi().GetScaleInt(10);
        const int32_t nMaxLegendWidth = std::max(rcPlot.Width() * 60 / 100, Dpi().GetScaleInt(80));
        nLegendWidth = std::min(nLegendWidth, nMaxLegendWidth);
    }
    UiRect rcPieArea = rcPlot;
    rcPieArea.right -= nLegendWidth;
    if ((rcPieArea.Width() <= 0) || (rcPieArea.Height() <= 0)) {
        return;
    }

    //内切正方形 + 边距
    const int32_t nSide = std::min(rcPieArea.Width(), rcPieArea.Height());
    const int32_t nCenterX = rcPieArea.left + rcPieArea.Width() / 2;
    const int32_t nCenterY = rcPieArea.top + rcPieArea.Height() / 2;
    UiRect rcPie;
    rcPie.left = nCenterX - nSide / 2;
    rcPie.top = nCenterY - nSide / 2;
    rcPie.right = rcPie.left + nSide;
    rcPie.bottom = rcPie.top + nSide;

    IRenderFactory* pFactory = GlobalManager::Instance().GetRenderFactory();
    const float fRadius = static_cast<float>(nSide) / 2.0f;
    const float fInnerRadius = m_bDonut ? fRadius * 0.55f : 0.0f; //环形内半径

    //绘制各扇区（从 270° 即正上方开始，顺时针）
    float fStartAngle = 270.0f;
    for (size_t i = 0; i < nCount; ++i) {
        double v = data[i];
        if (v <= 0.0) {
            continue;
        }
        float fSweep = static_cast<float>(360.0 * (v / dSum));
        UiColor sliceColor = MakeSliceColor(seriesColor, i, nCount);

        if (pFactory != nullptr) {
            IPath* pPath = pFactory->CreatePath();
            std::unique_ptr<IPath> spPath(pPath);
            if (pPath != nullptr) {
                if (m_bDonut) {
                    //环形：外弧 + 内弧，用两条弧线闭合（外弧顺时针，内弧逆时针）
                    pPath->AddArc(UiRectF(static_cast<float>(rcPie.left), static_cast<float>(rcPie.top),
                                          static_cast<float>(rcPie.right), static_cast<float>(rcPie.bottom)),
                                  fStartAngle, fSweep);
                    //内弧反向走
                    UiRectF rcInner(static_cast<float>(nCenterX) - fInnerRadius,
                                    static_cast<float>(nCenterY) - fInnerRadius,
                                    static_cast<float>(nCenterX) + fInnerRadius,
                                    static_cast<float>(nCenterY) + fInnerRadius);
                    pPath->AddArc(rcInner, fStartAngle + fSweep, -fSweep);
                    pPath->Close();
                }
                else {
                    //完整扇形：圆心 -> 弧起点 -> 弧 -> 闭合
                    double startRad = fStartAngle * kPi / 180.0;
                    float startX = static_cast<float>(nCenterX) + fRadius * static_cast<float>(std::cos(startRad));
                    float startY = static_cast<float>(nCenterY) + fRadius * static_cast<float>(std::sin(startRad));
                    pPath->AddLine(static_cast<float>(nCenterX), static_cast<float>(nCenterY), startX, startY);
                    pPath->AddArc(UiRectF(static_cast<float>(rcPie.left), static_cast<float>(rcPie.top),
                                          static_cast<float>(rcPie.right), static_cast<float>(rcPie.bottom)),
                                  fStartAngle, fSweep);
                    pPath->Close();
                }

                IBrush* pBrush = pFactory->CreateBrush(sliceColor);
                std::unique_ptr<IBrush> spBrush(pBrush);
                if (pBrush != nullptr) {
                    pRender->FillPath(pPath, pBrush);
                }
            }
        }
        fStartAngle += fSweep;
    }

    //数据标签（各扇区中心方向外侧）
    if (m_bShowValue || m_bShowPercent) {
        fStartAngle = 270.0f;
        for (size_t i = 0; i < nCount; ++i) {
            double v = data[i];
            if (v <= 0.0) {
                continue;
            }
            float fSweep = static_cast<float>(360.0 * (v / dSum));
            float fMidAngle = fStartAngle + fSweep / 2.0f;
            double rad = fMidAngle * kPi / 180.0;
            float fLabelRadius = fRadius * (m_bDonut ? 0.78f : 0.7f);
            float fLabelX = static_cast<float>(nCenterX) + fLabelRadius * static_cast<float>(std::cos(rad));
            float fLabelY = static_cast<float>(nCenterY) + fLabelRadius * static_cast<float>(std::sin(rad));

            //标签文本：数值 + 可选百分比
            DString strValue;
            if (m_bShowPercent) {
                double percent = v / dSum * 100.0;
                strValue = StringUtil::Printf(_T("%.1f%%"), percent);
            }
            else {
                strValue = FormatValue(v);
            }
            UiRect rcText;
            rcText.left = static_cast<int32_t>(fLabelX) - Dpi().GetScaleInt(30);
            rcText.right = static_cast<int32_t>(fLabelX) + Dpi().GetScaleInt(30);
            rcText.top = static_cast<int32_t>(fLabelY) - Dpi().GetScaleInt(10);
            rcText.bottom = static_cast<int32_t>(fLabelY) + Dpi().GetScaleInt(10);
            DrawChartText(pRender, strValue, rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE);
            fStartAngle += fSweep;
        }
    }

    //图例
    if (m_bLegendVisible) {
        const int32_t nLegendLeft = rcPieArea.right + Dpi().GetScaleInt(12);
        int32_t nLegendTop = rcPlot.top + Dpi().GetScaleInt(8);
        const int32_t nColorBoxSize = Dpi().GetScaleInt(12);
        for (size_t i = 0; i < nCount; ++i) {
            UiColor sliceColor = MakeSliceColor(seriesColor, i, nCount);
            UiRectF rcColor;
            rcColor.left = static_cast<float>(nLegendLeft);
            rcColor.top = static_cast<float>(nLegendTop) + nColorBoxSize / 2.0f;
            rcColor.right = rcColor.left + nColorBoxSize;
            rcColor.bottom = rcColor.top + nColorBoxSize;
            pRender->FillRoundRect(rcColor, 2.0f, 2.0f, sliceColor, GetAlpha());

            UiRect rcText;
            rcText.left = nLegendLeft + nColorBoxSize + Dpi().GetScaleInt(6);
            rcText.right = rcPieArea.right + nLegendWidth;
            rcText.top = nLegendTop;
            rcText.bottom = nLegendTop + Dpi().GetScaleInt(20);
            DrawChartText(pRender, legendTexts[i], rcText, TEXT_LEFT | TEXT_VCENTER | TEXT_SINGLELINE | TEXT_END_ELLIPSIS);

            nLegendTop += Dpi().GetScaleInt(22);
        }
    }
}

void Chart::PaintLegend(IRender* pRender, const UiRect& rcChart)
{
    //折线/柱状：多系列且开启图例时显示
    if ((m_chartType == ChartType::kPie) || !m_bLegendVisible || (m_series.size() <= 1)) {
        return;
    }
    const int32_t nLegendLeft = rcChart.right - Dpi().GetScaleInt(108);
    int32_t nLegendTop = rcChart.top + Dpi().GetScaleInt(8);
    const int32_t nColorBoxSize = Dpi().GetScaleInt(12);
    for (size_t si = 0; si < m_series.size(); ++si) {
        UiColor seriesColor = GetSeriesColor(si);
        UiRectF rcColor;
        rcColor.left = static_cast<float>(nLegendLeft);
        rcColor.top = static_cast<float>(nLegendTop) + nColorBoxSize / 2.0f;
        rcColor.right = rcColor.left + nColorBoxSize;
        rcColor.bottom = rcColor.top + nColorBoxSize;
        pRender->FillRoundRect(rcColor, 2.0f, 2.0f, seriesColor, GetAlpha());

        DString strLabel = m_series[si].name;
        if (strLabel.empty()) {
            strLabel = StringUtil::Printf(_T("系列 %zu"), si + 1);
        }
        UiRect rcText;
        rcText.left = nLegendLeft + nColorBoxSize + Dpi().GetScaleInt(6);
        rcText.right = rcChart.right;
        rcText.top = nLegendTop;
        rcText.bottom = nLegendTop + Dpi().GetScaleInt(20);
        DrawChartText(pRender, strLabel, rcText, TEXT_LEFT | TEXT_VCENTER | TEXT_SINGLELINE | TEXT_END_ELLIPSIS);

        nLegendTop += Dpi().GetScaleInt(22);
    }
}

} // namespace ui
