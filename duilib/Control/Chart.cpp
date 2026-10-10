#include "Chart.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
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
    m_bShowPercent(false),
    m_nBarRadius(3),
    m_bBarGradient(true),
    m_bLineGlow(false),
    m_bAnimationEnabled(true),
    m_nAnimationDuration(300),
    m_nAnimProgress(100),
    m_bEnableTooltip(true),
    m_bEnableSelect(true),
    m_viewStart(0.0),
    m_viewEnd(-1.0),
    m_bPanning(false),
    m_bPanMoved(false),
    m_panStartX(0),
    m_panStartView(0.0)
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
    else if ((strName == _T("bar_radius")) || (strName == _T("barradius"))) {
        SetBarRadius(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("bar_gradient")) || (strName == _T("bargradient"))) {
        SetBarGradient(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("line_glow")) || (strName == _T("lineglow"))) {
        SetLineGlow(StringUtil::IsValueTrue(strValue));
    }

    else if ((strName == _T("animation_enabled")) || (strName == _T("animationenabled"))) {
        SetAnimationEnabled(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("animation_duration")) || (strName == _T("animationduration"))) {
        SetAnimationDuration(StringUtil::StringToInt32(strValue));
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
        //类型切换：命中类型含义变化，清除选中与悬停
        m_selected = HitResult();
        m_lastHover = HitResult();
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
    //数据变更：清除选中状态、重置缩放平移视图
    m_selected = HitResult();
    m_viewStart = 0.0;
    m_viewEnd = -1.0;
    StartDataAnimation();
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
    StartDataAnimation();
    Invalidate();
}

void Chart::ClearData()
{
    m_series.clear();
    m_selected = HitResult();
    m_viewStart = 0.0;
    m_viewEnd = -1.0;
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
    //数据变更：清除选中状态、重置缩放平移视图
    m_selected = HitResult();
    m_viewStart = 0.0;
    m_viewEnd = -1.0;
    StartDataAnimation();
    Invalidate();
}

void Chart::AddSeries(const Series& series)
{
    m_series.push_back(series);
    StartDataAnimation();
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

void Chart::SetBarRadius(int32_t nRadius)
{
    if (nRadius < 0) {
        nRadius = 0;
    }
    if (m_nBarRadius != nRadius) {
        m_nBarRadius = nRadius;
        Invalidate();
    }
}

void Chart::SetBarGradient(bool bGradient)
{
    if (m_bBarGradient != bGradient) {
        m_bBarGradient = bGradient;
        Invalidate();
    }
}

void Chart::SetLineGlow(bool bGlow)
{
    if (m_bLineGlow != bGlow) {
        m_bLineGlow = bGlow;
        Invalidate();
    }
}

void Chart::SetAnimationEnabled(bool bEnabled)
{
    m_bAnimationEnabled = bEnabled;
    if (!bEnabled) {
        m_pAnimPlayer.reset();
        m_nAnimProgress = 100;
    }
}

void Chart::SetAnimationDuration(int32_t nMs)
{
    if (nMs < 0) {
        nMs = 0;
    }
    m_nAnimationDuration = nMs;
}

void Chart::StartDataAnimation()
{
    if (!m_bAnimationEnabled || (m_nAnimationDuration <= 0) || !IsInited()) {
        m_nAnimProgress = 100;
        return;
    }
    //停止旧动画
    if ((m_pAnimPlayer != nullptr) && m_pAnimPlayer->IsPlaying()) {
        m_pAnimPlayer->SetCompleteCallback(nullptr);
        m_pAnimPlayer->Stop();
    }
    m_nAnimProgress = 0;
    m_pAnimPlayer.reset(new AnimationPlayer);
    m_pAnimPlayer->SetAnimationType(AnimationType::kAnimationNone);
    m_pAnimPlayer->SetTotalMillSeconds(m_nAnimationDuration);
    m_pAnimPlayer->SetFrameIntervalMillSeconds(16);
    m_pAnimPlayer->SetEasingFunctionType(EaseOutQuad);
    m_pAnimPlayer->SetStartValue(0);
    m_pAnimPlayer->SetEndValue(100);

    std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
    m_pAnimPlayer->SetPlayCallback([this, weakFlag](int32_t nValue) {
        if (weakFlag.expired()) {
            return;
        }
        m_nAnimProgress = nValue;
        Invalidate();
    });
    m_pAnimPlayer->SetCompleteCallback([this, weakFlag]() {
        if (weakFlag.expired()) {
            return;
        }
        m_nAnimProgress = 100;
        Invalidate();
    });
    m_pAnimPlayer->Start();
}

double Chart::GetAnimProgress() const
{
    return static_cast<double>(m_nAnimProgress) / 100.0;
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

UiColor Chart::MakeDarkerColor(UiColor baseColor, uint8_t nScale)
{
    //生成更暗的颜色（nScale 为百分比 0~255，如 180 = 约 70% 亮度）
    uint8_t r = static_cast<uint8_t>(static_cast<uint32_t>(baseColor.GetR()) * nScale / 255);
    uint8_t g = static_cast<uint8_t>(static_cast<uint32_t>(baseColor.GetG()) * nScale / 255);
    uint8_t b = static_cast<uint8_t>(static_cast<uint32_t>(baseColor.GetB()) * nScale / 255);
    return UiColor(UiColor::MakeARGB(baseColor.GetA(), r, g, b));
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

Chart::LineLayout Chart::CalcLineLayout(IRender* pRender, const UiRect& rcChart) const
{
    LineLayout layout;
    layout.rcPlot = CalcPlotRect(pRender, rcChart);
    if ((layout.rcPlot.Width() <= 0) || (layout.rcPlot.Height() <= 0)) {
        return layout;
    }
    //计算所有系列的数据范围（合并）
    double dMin = 0.0;
    double dMax = 0.0;
    CalcValueRange(dMin, dMax);
    double dRange = dMax - dMin;
    dMax = dMax + dRange * 0.1;
    dMin = dMin - dRange * 0.1;
    int32_t nDiv = m_nAxisDivisions;
    CalcNiceAxis(dMin, dMax, nDiv);
    layout.dMin = dMin;
    layout.dMax = dMax;
    layout.dRange = dMax - dMin;
    layout.nDivisions = std::max(1, nDiv);

    //最大数据点数
    size_t nMaxCount = 0;
    for (const Series& s : m_series) {
        nMaxCount = std::max(nMaxCount, s.data.size());
    }
    layout.nMaxCount = nMaxCount;

    //可视窗口（用于缩放平移）
    if (nMaxCount == 0) {
        layout.viewStart = 0.0;
        layout.viewEnd = 0.0;
    }
    else if ((m_viewEnd <= m_viewStart) || (m_viewEnd <= 0.0)) {
        //未设置或已重置：显示全部
        layout.viewStart = 0.0;
        layout.viewEnd = (nMaxCount > 1) ? static_cast<double>(nMaxCount - 1) : 0.0;
    }
    else {
        layout.viewStart = m_viewStart;
        layout.viewEnd = m_viewEnd;
        //钳位
        double dMaxIndex = (nMaxCount > 1) ? static_cast<double>(nMaxCount - 1) : 0.0;
        if (layout.viewStart < 0.0) {
            layout.viewEnd -= layout.viewStart;
            layout.viewStart = 0.0;
        }
        if (layout.viewEnd > dMaxIndex) {
            layout.viewStart -= (layout.viewEnd - dMaxIndex);
            layout.viewEnd = dMaxIndex;
            if (layout.viewStart < 0.0) {
                layout.viewStart = 0.0;
            }
        }
        if (layout.viewEnd <= layout.viewStart) {
            layout.viewStart = 0.0;
            layout.viewEnd = dMaxIndex;
        }
    }
    return layout;
}

void Chart::SetEnableTooltip(bool bEnable)
{
    m_bEnableTooltip = bEnable;
    if (!bEnable) {
        m_lastHover = HitResult();
    }
}

void Chart::SetEnableSelect(bool bEnable)
{
    m_bEnableSelect = bEnable;
    if (!bEnable) {
        m_selected = HitResult();
        Invalidate();
    }
}

void Chart::ClearSelection()
{
    if (m_selected.type != HitType::kNone) {
        m_selected = HitResult();
        Invalidate();
    }
}

void Chart::AttachPointClick(const EventCallback& callback)
{
    AttachEvent(kEventClick, callback, 0);
}

void Chart::AttachPointHover(const EventCallback& callback)
{
    AttachEvent(kEventMouseHover, callback, 0);
}

Chart::HitResult Chart::GetHitFromEvent(const EventArgs& args)
{
    HitResult result;
    if (args.wParam != 0) {
        const HitResult* pHit = reinterpret_cast<const HitResult*>(args.wParam);
        if (pHit != nullptr) {
            result = *pHit;
        }
    }
    return result;
}

void Chart::SendPointClickEvent(const HitResult& hit)
{
    EventArgs args;
    args.SetSender(this);
    args.wParam = reinterpret_cast<WPARAM>(&hit);
    SendEvent(kEventClick, args);
}

void Chart::SendPointHoverEvent(const HitResult& hit)
{
    EventArgs args;
    args.SetSender(this);
    args.wParam = reinterpret_cast<WPARAM>(&hit);
    SendEvent(kEventMouseHover, args);
}

DString Chart::BuildHitLabel(const HitResult& hit) const
{
    DString strResult;
    if (hit.type == HitType::kLegend) {
        if (hit.seriesIndex < m_series.size()) {
            strResult = m_series[hit.seriesIndex].name;
        }
        return strResult;
    }
    if (hit.seriesIndex >= m_series.size()) {
        return strResult;
    }
    const Series& series = m_series[hit.seriesIndex];
    //系列名
    DString strName = series.name;
    if (strName.empty()) {
        strName = StringUtil::Printf(_T("系列 %zu"), hit.seriesIndex + 1);
    }
    //类目
    DString strCategory;
    if (hit.dataIndex < m_dataLabels.size()) {
        strCategory = m_dataLabels[hit.dataIndex];
    }
    //数值
    double value = 0.0;
    if (hit.dataIndex < series.data.size()) {
        value = series.data[hit.dataIndex];
    }
    if (!strCategory.empty()) {
        strResult = strName + _T(" / ") + strCategory + _T(": ") + FormatValue(value);
    }
    else {
        strResult = strName + _T(": ") + FormatValue(value);
    }
    return strResult;
}

DString Chart::BuildHitTooltip(const HitResult& hit) const
{
    //饼图扇区/柱/折线点：显示 "系列名\n类目: 值"
    if (hit.type == HitType::kSlice || hit.type == HitType::kBar || hit.type == HitType::kPoint) {
        return BuildHitLabel(hit);
    }
    if (hit.type == HitType::kLegend) {
        return BuildHitLabel(hit);
    }
    return DString();
}

int32_t Chart::HitTestPieSlice(const UiPoint& ptMouse, const UiRect& rcPie,
                               int32_t nCenterX, int32_t nCenterY, float fRadius,
                               const std::vector<double>& data, double dSum) const
{
    //先判断是否在圆内（含环形内圈之外）
    double dx = static_cast<double>(ptMouse.x - nCenterX);
    double dy = static_cast<double>(ptMouse.y - nCenterY);
    double dist = std::sqrt(dx * dx + dy * dy);
    if (dist > fRadius) {
        return -1;
    }
    if (m_bDonut) {
        float fInnerRadius = fRadius * 0.55f;
        if (dist < fInnerRadius) {
            return -1;
        }
    }
    //计算角度（从正上方 270° 顺时针）
    double angleDeg = std::atan2(dy, dx) * 180.0 / kPi; //-180~180，0°=右，90°=下
    if (angleDeg < 0.0) {
        angleDeg += 360.0;
    }
    //转换为从 270° 起顺时针的角度
    double relAngle = angleDeg - 270.0;
    if (relAngle < 0.0) {
        relAngle += 360.0;
    }
    //遍历扇区
    double fStartAngle = 0.0;
    for (size_t i = 0; i < data.size(); ++i) {
        double v = data[i];
        if (v <= 0.0) {
            continue;
        }
        double fSweep = 360.0 * (v / dSum);
        if (relAngle >= fStartAngle && relAngle < (fStartAngle + fSweep)) {
            return static_cast<int32_t>(i);
        }
        fStartAngle += fSweep;
    }
    return -1;
}

Chart::HitResult Chart::HitTest(IRender* pRender, const UiPoint& ptMouse)
{
    HitResult hit;
    if (pRender == nullptr) {
        return hit;
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);

    if (m_chartType == ChartType::kLine) {
        LineLayout layout = CalcLineLayout(pRender, rc);
        if ((layout.rcPlot.Width() <= 0) || (layout.rcPlot.Height() <= 0)) {
            return hit;
        }
        //遍历所有系列的可见数据点，找最近命中
        const int32_t nHitRadius = Dpi().GetScaleInt(8);
        double dBestDist = nHitRadius * nHitRadius;
        for (size_t si = 0; si < m_series.size(); ++si) {
            const std::vector<double>& data = m_series[si].data;
            if (data.empty()) {
                continue;
            }
            for (size_t i = 0; i < data.size(); ++i) {
                double idx = static_cast<double>(i);
                if ((idx < layout.viewStart) || (idx > layout.viewEnd)) {
                    continue; //不可见数据点
                }
                float fx = layout.XToPixel(idx);
                float fy = layout.YToPixel(data[i]);
                double dx = static_cast<double>(ptMouse.x) - fx;
                double dy = static_cast<double>(ptMouse.y) - fy;
                double dist = dx * dx + dy * dy;
                if (dist <= dBestDist) {
                    dBestDist = dist;
                    hit.type = HitType::kPoint;
                    hit.seriesIndex = si;
                    hit.dataIndex = i;
                    hit.ptHit = UiPoint(static_cast<int32_t>(fx), static_cast<int32_t>(fy));
                }
            }
        }
        return hit;
    }

    if (m_chartType == ChartType::kBar) {
        //柱状图命中：复用绘制时的布局计算
        UiRect rcPlot = CalcPlotRect(pRender, rc);
        if ((rcPlot.Width() <= 0) || (rcPlot.Height() <= 0)) {
            return hit;
        }
        if (m_series.empty()) {
            return hit;
        }
        //计算数据范围（含堆叠逻辑，与 PaintBar 一致）
        double dMin = 0.0;
        double dMax = 0.0;
        CalcValueRange(dMin, dMax);
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
        double zeroNorm = (0.0 - dMin) / dRange;
        float fZeroY = static_cast<float>(nBottom) - fPlotHeight * static_cast<float>(zeroNorm);
        fZeroY = std::max(static_cast<float>(nTop), std::min(fZeroY, static_cast<float>(nBottom)));

        size_t nCategoryCount = 0;
        for (const Series& s : m_series) {
            nCategoryCount = std::max(nCategoryCount, s.data.size());
        }
        if (nCategoryCount == 0) {
            return hit;
        }
        const size_t nSeriesCount = m_series.size();
        const float fSlotWidth = fPlotWidth / static_cast<float>(nCategoryCount);

        if (m_barMode == ChartBarMode::kStacked) {
            const float fBarWidth = fSlotWidth * 0.6f;
            for (size_t i = 0; i < nCategoryCount; ++i) {
                float fCenterX = static_cast<float>(nLeft) + fSlotWidth * (static_cast<float>(i) + 0.5f);
                float fBarLeft = fCenterX - fBarWidth / 2.0f;
                float fBarRight = fBarLeft + fBarWidth;
                if ((ptMouse.x >= fBarLeft) && (ptMouse.x <= fBarRight)) {
                    //命中该柱，确定系列（正堆叠从顶向下，负堆叠从底向上）
                    float posTop = fZeroY;
                    for (size_t si = 0; si < nSeriesCount; ++si) {
                        if (i >= m_series[si].data.size()) {
                            continue;
                        }
                        double v = m_series[si].data[i];
                        if (v >= 0.0) {
                            float h = fPlotHeight * static_cast<float>(v / dRange);
                            float segTop = posTop - h;
                            if ((ptMouse.y >= segTop) && (ptMouse.y <= posTop)) {
                                hit.type = HitType::kBar;
                                hit.seriesIndex = si;
                                hit.dataIndex = i;
                                hit.ptHit = UiPoint(static_cast<int32_t>(fCenterX), static_cast<int32_t>((segTop + posTop) / 2.0f));
                                return hit;
                            }
                            posTop = segTop;
                        }
                    }
                    float negBottom = fZeroY;
                    for (size_t si = 0; si < nSeriesCount; ++si) {
                        if (i >= m_series[si].data.size()) {
                            continue;
                        }
                        double v = m_series[si].data[i];
                        if (v < 0.0) {
                            float h = fPlotHeight * static_cast<float>((-v) / dRange);
                            float segBottom = negBottom + h;
                            if ((ptMouse.y >= negBottom) && (ptMouse.y <= segBottom)) {
                                hit.type = HitType::kBar;
                                hit.seriesIndex = si;
                                hit.dataIndex = i;
                                hit.ptHit = UiPoint(static_cast<int32_t>(fCenterX), static_cast<int32_t>((negBottom + segBottom) / 2.0f));
                                return hit;
                            }
                            negBottom = segBottom;
                        }
                    }
                    return hit;
                }
            }
        }
        else {
            const float fGroupWidth = fSlotWidth * 0.8f;
            const float fBarWidth = fGroupWidth / static_cast<float>(nSeriesCount);
            for (size_t si = 0; si < nSeriesCount; ++si) {
                for (size_t i = 0; i < m_series[si].data.size(); ++i) {
                    double v = m_series[si].data[i];
                    float fCenterX = static_cast<float>(nLeft) + fSlotWidth * (static_cast<float>(i) + 0.5f);
                    float fGroupLeft = fCenterX - fGroupWidth / 2.0f;
                    float fBarLeft = fGroupLeft + fBarWidth * static_cast<float>(si);
                    float fBarRight = fBarLeft + fBarWidth;
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
                    if ((ptMouse.x >= fBarLeft) && (ptMouse.x <= fBarRight) &&
                        (ptMouse.y >= fBarTop) && (ptMouse.y <= fBarBottom)) {
                        hit.type = HitType::kBar;
                        hit.seriesIndex = si;
                        hit.dataIndex = i;
                        hit.ptHit = UiPoint(static_cast<int32_t>(fCenterX), static_cast<int32_t>((fBarTop + fBarBottom) / 2.0f));
                        return hit;
                    }
                }
            }
        }
        return hit;
    }

    if (m_chartType == ChartType::kPie) {
        UiRect rcPlot = CalcPlotRect(pRender, rc);
        if ((rcPlot.Width() <= 0) || (rcPlot.Height() <= 0)) {
            return hit;
        }
        if (m_series.empty() || m_series[0].data.empty()) {
            return hit;
        }
        const std::vector<double>& data = m_series[0].data;
        const size_t nCount = data.size();
        double dSum = 0.0;
        for (double v : data) {
            if (v > 0.0) {
                dSum += v;
            }
        }
        if (dSum <= 0.0) {
            return hit;
        }
        //计算图例宽度（与 PaintPie 一致，用于确定饼图区域）
        int32_t nLegendWidth = 0;
        if (m_bLegendVisible) {
            const int32_t nColorBoxSize = Dpi().GetScaleInt(12);
            int32_t nMaxTextWidth = 0;
            for (size_t i = 0; i < nCount; ++i) {
                DString strLabel = (i < m_dataLabels.size()) ? m_dataLabels[i] : DString();
                if (strLabel.empty()) {
                    strLabel = FormatValue(data[i]);
                }
                else {
                    strLabel = strLabel + _T("  ") + FormatValue(data[i]);
                    if (m_bShowPercent) {
                        strLabel = strLabel + StringUtil::Printf(_T(" (%.1f%%)"), data[i] / dSum * 100.0);
                    }
                }
                nMaxTextWidth = std::max(nMaxTextWidth, GetSafeTextWidth(pRender, strLabel));
            }
            nLegendWidth = nColorBoxSize + Dpi().GetScaleInt(6) + nMaxTextWidth + Dpi().GetScaleInt(10);
            const int32_t nMaxLegendWidth = std::max(rcPlot.Width() * 60 / 100, Dpi().GetScaleInt(80));
            nLegendWidth = std::min(nLegendWidth, nMaxLegendWidth);
        }
        UiRect rcPieArea = rcPlot;
        rcPieArea.right -= nLegendWidth;
        const int32_t nSide = std::min(rcPieArea.Width(), rcPieArea.Height());
        const int32_t nCenterX = rcPieArea.left + rcPieArea.Width() / 2;
        const int32_t nCenterY = rcPieArea.top + rcPieArea.Height() / 2;
        UiRect rcPie;
        rcPie.left = nCenterX - nSide / 2;
        rcPie.top = nCenterY - nSide / 2;
        rcPie.right = rcPie.left + nSide;
        rcPie.bottom = rcPie.top + nSide;
        const float fRadius = static_cast<float>(nSide) / 2.0f;

        int32_t nSlice = HitTestPieSlice(ptMouse, rcPie, nCenterX, nCenterY, fRadius, data, dSum);
        if (nSlice >= 0) {
            hit.type = HitType::kSlice;
            hit.seriesIndex = 0;
            hit.dataIndex = static_cast<size_t>(nSlice);
            hit.ptHit = ptMouse;
        }
        return hit;
    }

    return hit;
}

void Chart::PaintLine(IRender* pRender, const UiRect& rcChart)
{
    LineLayout layout = CalcLineLayout(pRender, rcChart);
    UiRect rcPlot = layout.rcPlot;
    PaintTitles(pRender, rcChart, rcPlot);
    if ((rcPlot.Width() <= 0) || (rcPlot.Height() <= 0)) {
        return;
    }

    UiColor axisColor = GetUiColor(m_strAxisColor);
    if (axisColor.IsEmpty()) {
        axisColor = UiColor(0xFFD0D0D0);
    }

    const double dMin = layout.dMin;
    const double dMax = layout.dMax;
    const double dRange = layout.dRange;
    const int32_t nDiv = layout.nDivisions; //美观刻度对齐后的实际分段数

    const int32_t nLeft = rcPlot.left;
    const int32_t nRight = rcPlot.right;
    const int32_t nTop = rcPlot.top;
    const int32_t nBottom = rcPlot.bottom;
    const float fPlotWidth = static_cast<float>(nRight - nLeft);
    const float fPlotHeight = static_cast<float>(nBottom - nTop);

    //数据更新动画进度（折线从绘图区底部渐入）
    const double dProgress = GetAnimProgress();

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

    if (layout.nMaxCount == 0) {
        return;
    }

    //逐系列绘制（裁剪到绘图区：缩放/平移时窗口边界外扩的补点不会画出绘图区）
    IRenderFactory* pFactory = GlobalManager::Instance().GetRenderFactory();
    std::unique_ptr<AutoClip> spPlotClip = CreateRectClip(pRender, rcPlot, true);
    for (size_t si = 0; si < m_series.size(); ++si) {
        const std::vector<double>& data = m_series[si].data;
        if (data.empty()) {
            continue;
        }
        UiColor seriesColor = GetSeriesColor(si);
        const size_t nCount = data.size();

        //计算各点坐标（仅可视窗口内的点，含首尾各扩一点保持连线连续）
        std::vector<size_t> visibleIdx;
        visibleIdx.reserve(nCount);
        for (size_t i = 0; i < nCount; ++i) {
            double idx = static_cast<double>(i);
            if ((idx >= layout.viewStart) && (idx <= layout.viewEnd)) {
                visibleIdx.push_back(i);
            }
            else if (!visibleIdx.empty() && (visibleIdx.back() == i - 1) && (i > 0)) {
                //可视窗口边界：补上窗口外紧邻的一个点，保证连线连续
                visibleIdx.push_back(i);
            }
        }
        //若可视窗口从中间开始，补一个窗口起点左侧的点
        if (!visibleIdx.empty() && (visibleIdx.front() > 0)) {
            visibleIdx.insert(visibleIdx.begin(), visibleIdx.front() - 1);
        }
        std::vector<UiPointF> points;
        points.reserve(visibleIdx.size());
        for (size_t i : visibleIdx) {
            float fx = layout.XToPixel(static_cast<double>(i));
            float fyTarget = layout.YToPixel(data[i]);
            //渐入动画：从绘图区底部向目标值插值
            float fy = static_cast<float>(nBottom) - (static_cast<float>(nBottom) - fyTarget) * static_cast<float>(dProgress);
            points.emplace_back(fx, fy);
        }
        const size_t nVisible = points.size();
        if (nVisible == 0) {
            continue;
        }

        //面积填充（在折线下方填充到基线）
        if (m_bAreaFill && (nVisible >= 2) && (pFactory != nullptr)) {
            IPath* pPath = pFactory->CreatePath();
            std::unique_ptr<IPath> spPath(pPath);
            if (pPath != nullptr) {
                float baseY = static_cast<float>(nBottom);
                pPath->AddLine(points[0].x, baseY, points[0].x, points[0].y);
                if (m_lineMode == ChartLineMode::kCurve) {
                    for (size_t i = 0; i + 1 < nVisible; ++i) {
                        float cx1 = points[i].x + (points[i + 1].x - points[i].x) * 0.5f;
                        float cx2 = points[i + 1].x - (points[i + 1].x - points[i].x) * 0.5f;
                        pPath->AddBezier(points[i].x, points[i].y, cx1, points[i].y, cx2, points[i + 1].y, points[i + 1].x, points[i + 1].y);
                    }
                }
                else {
                    for (size_t i = 0; i + 1 < nVisible; ++i) {
                        pPath->AddLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
                    }
                }
                pPath->AddLine(points[nVisible - 1].x, points[nVisible - 1].y, points[nVisible - 1].x, baseY);
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
        if (nVisible >= 2) {
            const float fLineWidth = static_cast<float>(Dpi().GetScaleInt(m_nLineWidth));
            //发光/阴影：先画一条更粗、低透明度的同色线，再叠主线
            const bool bGlow = m_bLineGlow && (pFactory != nullptr);
            const float fGlowWidth = fLineWidth * 2.0f + 1.0f;
            const UiColor glowColor = UiColor(UiColor::MakeARGB(48, seriesColor.GetR(), seriesColor.GetG(), seriesColor.GetB()));
            if (m_lineMode == ChartLineMode::kCurve) {
                for (size_t i = 0; i + 1 < nVisible; ++i) {
                    //用贝塞尔平滑：控制点取相邻中点水平延伸
                    float cx1 = points[i].x + (points[i + 1].x - points[i].x) * 0.5f;
                    float cx2 = points[i + 1].x - (points[i + 1].x - points[i].x) * 0.5f;
                    if (bGlow) {
                        IPath* pGlowPath = pFactory->CreatePath();
                        std::unique_ptr<IPath> spGlowPath(pGlowPath);
                        IPen* pGlowPen = pFactory->CreatePen(glowColor, fGlowWidth);
                        std::unique_ptr<IPen> spGlowPen(pGlowPen);
                        if ((pGlowPath != nullptr) && (pGlowPen != nullptr)) {
                            pGlowPath->AddBezier(points[i].x, points[i].y, cx1, points[i].y, cx2, points[i + 1].y, points[i + 1].x, points[i + 1].y);
                            pRender->DrawPath(pGlowPath, pGlowPen);
                        }
                    }
                    IPath* pPath = pFactory->CreatePath();
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
                for (size_t i = 0; i + 1 < nVisible; ++i) {
                    if (bGlow) {
                        pRender->DrawLine(points[i], points[i + 1], glowColor, fGlowWidth);
                    }
                    pRender->DrawLine(points[i], points[i + 1], seriesColor, fLineWidth);
                }
            }
        }

        //数据点 + 数值标签（只绘制可视窗口内的点）
        if (m_bShowDataPoints) {
            const float fRadius = static_cast<float>(Dpi().GetScaleInt(3));
            for (size_t i = 0; i < nVisible; ++i) {
                //边界外扩的点不画圆点（避免重复/越界视觉）
                size_t realIdx = visibleIdx[i];
                if ((realIdx < static_cast<size_t>(std::ceil(layout.viewStart))) ||
                    (realIdx > static_cast<size_t>(std::floor(layout.viewEnd)))) {
                    continue;
                }
                pRender->FillCircle(points[i], fRadius, seriesColor, GetAlpha());
            }
        }
        if (m_bShowValue) {
            //多系列时上下错开：偶数系列标签在点上方，奇数系列在点下方，避免相互重叠
            //按数据索引间隔抽稀（而非像素防重叠）：拖拽平移时标签集合随窗口平滑移动，不会忽隐忽现
            const bool bBelow = (si % 2 == 1);
            double dVisibleCount = std::max(1.0, layout.viewEnd - layout.viewStart + 1.0);
            //最小标签间距约 5 个半角字符宽，据此计算抽稀步长
            int32_t nMinGap = Dpi().GetScaleInt(44);
            int32_t nStep = std::max(1, static_cast<int32_t>(std::ceil(dVisibleCount * nMinGap / static_cast<double>(rcPlot.Width()))));
            for (size_t i = 0; i < nVisible; ++i) {
                size_t realIdx = visibleIdx[i];
                if ((realIdx < static_cast<size_t>(std::ceil(layout.viewStart))) ||
                    (realIdx > static_cast<size_t>(std::floor(layout.viewEnd)))) {
                    continue;
                }
                //索引间隔抽稀：固定索引子集，平移时平滑跟随
                if ((realIdx % static_cast<size_t>(nStep)) != 0) {
                    continue;
                }
                DString strValue = FormatValue(data[realIdx]);
                int32_t nTextWidth = GetSafeTextWidth(pRender, strValue);
                if (nTextWidth <= 0) {
                    nTextWidth = Dpi().GetScaleInt(20);
                }
                //按实际文本宽度布局，只平移不缩窄（保持文字与数据点对齐，不被钳位挤压偏移）
                int32_t nLabelLeft = static_cast<int32_t>(points[i].x) - nTextWidth / 2;
                nLabelLeft = std::max(nLabelLeft, rcPlot.left);
                nLabelLeft = std::min(nLabelLeft, rcPlot.right - nTextWidth);
                UiRect rcText;
                rcText.left = nLabelLeft;
                rcText.right = nLabelLeft + nTextWidth;
                if (bBelow) {
                    rcText.top = static_cast<int32_t>(points[i].y) + Dpi().GetScaleInt(4);
                    rcText.bottom = rcText.top + Dpi().GetScaleInt(18);
                }
                else {
                    rcText.top = static_cast<int32_t>(points[i].y) - Dpi().GetScaleInt(20);
                    rcText.bottom = static_cast<int32_t>(points[i].y) - Dpi().GetScaleInt(2);
                }
                DrawChartText(pRender, strValue, rcText, TEXT_LEFT | (bBelow ? TEXT_TOP : TEXT_BOTTOM) | TEXT_SINGLELINE);
            }
        }
    }
    spPlotClip.reset(); //恢复裁剪：X 轴类目标签绘制在绘图区下方

    //X 轴类目标签（跟随可视窗口）
    if (!m_dataLabels.empty()) {
        const size_t nLabelCount = std::min(m_dataLabels.size(), layout.nMaxCount);
        int32_t nLabelTop = nBottom + Dpi().GetScaleInt(2);
        int32_t nLabelBottom = nBottom + Dpi().GetScaleInt(18);
        //先计算所有标签矩形（实测宽度 + 钳位到图表区域内）
        std::vector<UiRect> labelRects(nLabelCount);
        for (size_t i = 0; i < nLabelCount; ++i) {
            double idx = static_cast<double>(i);
            if ((idx < layout.viewStart) || (idx > layout.viewEnd)) {
                labelRects[i] = UiRect(); //不可见，宽度为0
                continue;
            }
            float fx = layout.XToPixel(idx);
            int32_t nTextWidth = GetSafeTextWidth(pRender, m_dataLabels[i]);
            if (nTextWidth <= 0) {
                nTextWidth = Dpi().GetScaleInt(20);
            }
            //保持文本完整宽度，只平移钳位（避免边缘标签被挤压变形）
            UiRect rc;
            rc.left = std::max(static_cast<int32_t>(fx) - nTextWidth / 2, rcChart.left);
            rc.left = std::min(rc.left, rcChart.right - nTextWidth);
            rc.right = rc.left + nTextWidth;
            rc.top = nLabelTop;
            rc.bottom = nLabelBottom;
            labelRects[i] = rc;
        }
        //抽稀：贪心选择互不重叠的标签（含最小间距），但保证末标签始终绘制（与最后已选标签重叠时用末标签替换之）
        std::vector<size_t> drawn;
        const int32_t nMinGap = Dpi().GetScaleInt(6); //相邻标签最小间距
        int32_t nLastRight = std::numeric_limits<int32_t>::min();
        for (size_t i = 0; i < nLabelCount; ++i) {
            //跳过可视窗口外的标签（空矩形）
            if ((labelRects[i].Width() <= 0) || (labelRects[i].Height() <= 0)) {
                continue;
            }
            if (labelRects[i].left > nLastRight + nMinGap) {
                drawn.push_back(i);
                nLastRight = labelRects[i].right;
            }
        }
        if (!drawn.empty() && (drawn.back() != nLabelCount - 1)) {
            //末标签未绘制：若它可见且替换最后一个已选标签后不与更前的标签重叠，则替换
            UiRect rcLast = labelRects[nLabelCount - 1];
            if ((rcLast.Width() > 0) && (rcLast.Height() > 0)) {
                bool bCanReplace = (drawn.size() == 1);
                if (!bCanReplace) {
                    UiRect rcPrev = labelRects[drawn[drawn.size() - 2]];
                    bCanReplace = (rcLast.left > rcPrev.right);
                }
                if (bCanReplace) {
                    drawn.back() = nLabelCount - 1;
                }
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

    //数据更新动画进度（柱体生长）
    const double dProgress = GetAnimProgress();
    const float fRadius = static_cast<float>(Dpi().GetScaleInt(m_nBarRadius));
    const uint8_t nDarkerScale = 210; //渐变暗端约 82% 亮度（过暗会显得脏）

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
                    float h = fPlotHeight * static_cast<float>(v / dRange) * static_cast<float>(dProgress);
                    float segTop = posTop - h;
                    UiRectF rcBar(fBarLeft, segTop, fBarLeft + fBarWidth, posTop);
                    UiColor barColor = GetSeriesColor(si);
                    //堆叠模式：分段不做圆角（分段圆角会产生缝隙感）
                    if (m_bBarGradient) {
                        pRender->FillRoundRect(rcBar, 0.0f, 0.0f, barColor,
                                               MakeDarkerColor(barColor, nDarkerScale), 2, GetAlpha());
                    }
                    else {
                        pRender->FillRect(rcBar, barColor, GetAlpha());
                    }
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
                    float h = fPlotHeight * static_cast<float>((-v) / dRange) * static_cast<float>(dProgress);
                    float segBottom = negBottom + h;
                    UiRectF rcBar(fBarLeft, negBottom, fBarLeft + fBarWidth, segBottom);
                    UiColor barColor = GetSeriesColor(si);
                    if (m_bBarGradient) {
                        pRender->FillRoundRect(rcBar, 0.0f, 0.0f, barColor,
                                               MakeDarkerColor(barColor, nDarkerScale), 2, GetAlpha());
                    }
                    else {
                        pRender->FillRect(rcBar, barColor, GetAlpha());
                    }
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
                    float h = fPlotHeight * static_cast<float>(v / dRange) * static_cast<float>(dProgress);
                    fBarTop = fZeroY - h;
                    fBarBottom = fZeroY;
                }
                else {
                    float h = fPlotHeight * static_cast<float>((-v) / dRange) * static_cast<float>(dProgress);
                    fBarTop = fZeroY;
                    fBarBottom = fZeroY + h;
                }
                UiRectF rcBar(fBarLeft, fBarTop, fBarLeft + fBarWidth, fBarBottom);
                UiColor barColor = GetSeriesColor(si);
                if (m_bBarGradient) {
                    //顶部亮 → 底部暗渐变（方向 2 = 上→下）
                    pRender->FillRoundRect(rcBar, fRadius, fRadius, barColor,
                                           MakeDarkerColor(barColor, nDarkerScale), 2, GetAlpha());
                }
                else {
                    pRender->FillRoundRect(rcBar, fRadius, fRadius, barColor, GetAlpha());
                }
                //选中柱描边高亮
                if (m_bEnableSelect && (m_selected.type == HitType::kBar) &&
                    (m_selected.seriesIndex == si) && (m_selected.dataIndex == i)) {
                    UiColor hlColor = GetUiColor(_T("text_default"));
                    if (hlColor.IsEmpty()) {
                        hlColor = UiColor(0xFF000000);
                    }
                    pRender->DrawRoundRect(rcBar, fRadius, fRadius, hlColor, 2.0f);
                }

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

    IRenderFactory* pFactory = GlobalManager::Instance().GetRenderFactory();
    //选中扇区偏移量固定为外接半径的 8%；有效绘制半径相应缩小，保证偏移后弧不超出饼图区域
    const float fOuterRadius = static_cast<float>(nSide) / 2.0f;
    const float fSelOffset = fOuterRadius * 0.08f;
    const float fRadius = fOuterRadius - fSelOffset;
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

        //选中扇区：向圆心外偏移绘制，形成分离高亮
        int32_t nOffsetX = 0;
        int32_t nOffsetY = 0;
        if (m_bEnableSelect && (m_selected.type == HitType::kSlice) && (m_selected.dataIndex == i)) {
            float fMidAngle = fStartAngle + fSweep / 2.0f;
            double rad = fMidAngle * kPi / 180.0;
            nOffsetX = static_cast<int32_t>(fSelOffset * static_cast<float>(std::cos(rad)));
            nOffsetY = static_cast<int32_t>(fSelOffset * static_cast<float>(std::sin(rad)));
        }
        const int32_t nCenterXOff = nCenterX + nOffsetX;
        const int32_t nCenterYOff = nCenterY + nOffsetY;

        if (pFactory != nullptr) {
            //有效绘制半径的外接矩形（中心可带选中偏移）
            UiRectF rcOuter(static_cast<float>(nCenterXOff) - fRadius, static_cast<float>(nCenterYOff) - fRadius,
                            static_cast<float>(nCenterXOff) + fRadius, static_cast<float>(nCenterYOff) + fRadius);
            IPath* pPath = pFactory->CreatePath();
            std::unique_ptr<IPath> spPath(pPath);
            if (pPath != nullptr) {
                if (m_bDonut) {
                    //环形：外弧 + 内弧，用两条弧线闭合（外弧顺时针，内弧逆时针）
                    pPath->AddArc(rcOuter, fStartAngle, fSweep);
                    //内弧反向走
                    UiRectF rcInner(static_cast<float>(nCenterXOff) - fInnerRadius,
                                    static_cast<float>(nCenterYOff) - fInnerRadius,
                                    static_cast<float>(nCenterXOff) + fInnerRadius,
                                    static_cast<float>(nCenterYOff) + fInnerRadius);
                    pPath->AddArc(rcInner, fStartAngle + fSweep, -fSweep);
                    pPath->Close();
                }
                else {
                    //完整扇形：圆心 -> 弧起点 -> 弧 -> 闭合
                    double startRad = fStartAngle * kPi / 180.0;
                    float startX = static_cast<float>(nCenterXOff) + fRadius * static_cast<float>(std::cos(startRad));
                    float startY = static_cast<float>(nCenterYOff) + fRadius * static_cast<float>(std::sin(startRad));
                    pPath->AddLine(static_cast<float>(nCenterXOff), static_cast<float>(nCenterYOff), startX, startY);
                    pPath->AddArc(rcOuter, fStartAngle, fSweep);
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

            //标签文本：数值 + 可选百分比
            DString strValue;
            if (m_bShowPercent) {
                double percent = v / dSum * 100.0;
                strValue = StringUtil::Printf(_T("%.1f%%"), percent);
            }
            else {
                strValue = FormatValue(v);
            }

            //选中扇区：标签移到扇区外，画引导线连接（小扇区百分比不再被挤在内部）
            const bool bSelectedSlice = m_bEnableSelect && (m_selected.type == HitType::kSlice) && (m_selected.dataIndex == i);
            if (bSelectedSlice) {
                const float fCos = static_cast<float>(std::cos(rad));
                const float fSin = static_cast<float>(std::sin(rad));
                const float fStartR = fOuterRadius * 1.05f;
                const float fEndR = fOuterRadius + Dpi().GetScaleInt(12);
                UiColor lineColor = GetUiColor(m_strLabelColor);
                if (lineColor.IsEmpty()) {
                    lineColor = UiColor(0xFF000000);
                }
                //引导线：从扇区边缘延伸到标签
                pRender->DrawLine(UiPointF(static_cast<float>(nCenterX) + fStartR * fCos, static_cast<float>(nCenterY) + fStartR * fSin),
                                  UiPointF(static_cast<float>(nCenterX) + fEndR * fCos, static_cast<float>(nCenterY) + fEndR * fSin),
                                  lineColor, 1.0f);
                //外部标签：沿半径方向外侧放置，按角度决定对齐
                float fLabelX = static_cast<float>(nCenterX) + (fEndR + Dpi().GetScaleInt(6)) * fCos;
                float fLabelY = static_cast<float>(nCenterY) + (fEndR + Dpi().GetScaleInt(6)) * fSin;
                UiRect rcText;
                rcText.left = static_cast<int32_t>(fLabelX) - Dpi().GetScaleInt(40);
                rcText.right = static_cast<int32_t>(fLabelX) + Dpi().GetScaleInt(40);
                rcText.top = static_cast<int32_t>(fLabelY) - Dpi().GetScaleInt(10);
                rcText.bottom = static_cast<int32_t>(fLabelY) + Dpi().GetScaleInt(10);
                DrawChartText(pRender, strValue, rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE);
            }
            else {
                float fLabelRadius = fOuterRadius * (m_bDonut ? 0.78f : 0.7f);
                float fLabelX = static_cast<float>(nCenterX) + fLabelRadius * static_cast<float>(std::cos(rad));
                float fLabelY = static_cast<float>(nCenterY) + fLabelRadius * static_cast<float>(std::sin(rad));
                UiRect rcText;
                rcText.left = static_cast<int32_t>(fLabelX) - Dpi().GetScaleInt(30);
                rcText.right = static_cast<int32_t>(fLabelX) + Dpi().GetScaleInt(30);
                rcText.top = static_cast<int32_t>(fLabelY) - Dpi().GetScaleInt(10);
                rcText.bottom = static_cast<int32_t>(fLabelY) + Dpi().GetScaleInt(10);
                DrawChartText(pRender, strValue, rcText, TEXT_HCENTER | TEXT_VCENTER | TEXT_SINGLELINE);
            }
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

bool Chart::MouseMove(const EventArgs& msg)
{
    if (!IsEnabled()) {
        return BaseClass::MouseMove(msg);
    }
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    if (pRender == nullptr) {
        return BaseClass::MouseMove(msg);
    }
    //折线图拖拽平移
    if (m_bPanning && (m_chartType == ChartType::kLine)) {
        UiRect rc = GetRect();
        UiPadding rcPadding = GetControlPadding();
        rc.Deflate(rcPadding);
        LineLayout layout = CalcLineLayout(pRender, rc);
        if ((layout.rcPlot.Width() > 0) && (layout.nMaxCount > 1)) {
            int32_t dx = msg.ptMouse.x - m_panStartX;
            if (dx != 0) {
                m_bPanMoved = true;
                double dMaxIndex = static_cast<double>(layout.nMaxCount - 1);
                double dSpan = layout.viewEnd - layout.viewStart;
                double dIndexDelta = (dSpan / static_cast<double>(layout.rcPlot.Width())) * static_cast<double>(dx);
                double dNewStart = m_panStartView - dIndexDelta;
                double dNewEnd = dNewStart + dSpan;
                if (dNewStart < 0.0) {
                    dNewEnd -= dNewStart;
                    dNewStart = 0.0;
                }
                if (dNewEnd > dMaxIndex) {
                    dNewStart -= (dNewEnd - dMaxIndex);
                    dNewEnd = dMaxIndex;
                    if (dNewStart < 0.0) {
                        dNewStart = 0.0;
                    }
                }
                m_viewStart = dNewStart;
                m_viewEnd = dNewEnd;
                Invalidate();
            }
        }
        return true;
    }
    if (!m_bEnableTooltip) {
        return BaseClass::MouseMove(msg);
    }
    //msg.ptMouse 为控件内坐标
    HitResult hit = HitTest(pRender, msg.ptMouse);
    if (hit.type == HitType::kNone) {
        if (m_lastHover.type != HitType::kNone) {
            m_lastHover = HitResult();
            //清除 Tooltip
            if (GetWindow() != nullptr) {
                SetToolTipText(_T(""));
            }
        }
        return BaseClass::MouseMove(msg);
    }
    //命中变化才更新（避免高频重设 Tooltip）
    bool bChanged = (hit.type != m_lastHover.type) ||
                    (hit.seriesIndex != m_lastHover.seriesIndex) ||
                    (hit.dataIndex != m_lastHover.dataIndex);
    m_lastHover = hit;
    if (bChanged) {
        SetToolTipText(BuildHitTooltip(hit));
    }
    return true; //消费消息
}

bool Chart::MouseLeave(const EventArgs& msg)
{
    m_lastHover = HitResult();
    if (m_bPanning) {
        m_bPanning = false;
        m_bPanMoved = false;
    }
    return BaseClass::MouseLeave(msg);
}

bool Chart::ButtonDown(const EventArgs& msg)
{
    if (!IsEnabled()) {
        return BaseClass::ButtonDown(msg);
    }
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    if (pRender == nullptr) {
        return BaseClass::ButtonDown(msg);
    }
    HitResult hit = HitTest(pRender, msg.ptMouse);

    //折线图：左键按在空白处开始平移；按在数据点上则走选中逻辑
    if (m_chartType == ChartType::kLine) {
        UiRect rc = GetRect();
        UiPadding rcPadding = GetControlPadding();
        rc.Deflate(rcPadding);
        LineLayout layout = CalcLineLayout(pRender, rc);
        if ((hit.type == HitType::kNone) &&
            (layout.rcPlot.Width() > 0) && (layout.rcPlot.Height() > 0) &&
            (msg.ptMouse.x >= layout.rcPlot.left) && (msg.ptMouse.x <= layout.rcPlot.right) &&
            (msg.ptMouse.y >= layout.rcPlot.top) && (msg.ptMouse.y <= layout.rcPlot.bottom)) {
            m_bPanning = true;
            m_bPanMoved = false;
            m_panStartX = msg.ptMouse.x;
            m_panStartView = layout.viewStart;
            if (GetWindow() != nullptr) {
                GetWindow()->SetCapture();
            }
            return true;
        }
    }

    //点击选中
    if (m_bEnableSelect && (hit.type != HitType::kNone)) {
        bool bSame = (hit.type == m_selected.type) &&
                     (hit.seriesIndex == m_selected.seriesIndex) &&
                     (hit.dataIndex == m_selected.dataIndex);
        if (bSame) {
            m_selected = HitResult(); //再次点击取消选中
        }
        else {
            m_selected = hit;
        }
        Invalidate();
        SendPointClickEvent(hit);
        return true;
    }
    return BaseClass::ButtonDown(msg);
}

bool Chart::ButtonUp(const EventArgs& msg)
{
    if (m_bPanning) {
        m_bPanning = false;
        m_bPanMoved = false;
        Invalidate(); //恢复数值标签显示
        if (GetWindow() != nullptr) {
            GetWindow()->ReleaseCapture();
        }
        return true;
    }
    return BaseClass::ButtonUp(msg);
}

bool Chart::MouseWheel(const EventArgs& msg)
{
    if (!IsEnabled() || (m_chartType != ChartType::kLine)) {
        return BaseClass::MouseWheel(msg);
    }
    //折线图滚轮缩放（围绕鼠标 X 位置）
    IRender* pRender = (GetWindow() != nullptr) ? GetWindow()->GetRender() : nullptr;
    if (pRender == nullptr) {
        return BaseClass::MouseWheel(msg);
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    LineLayout layout = CalcLineLayout(pRender, rc);
    if ((layout.rcPlot.Width() <= 0) || (layout.nMaxCount <= 1)) {
        return BaseClass::MouseWheel(msg);
    }
    int32_t wheelDelta = msg.eventData; //正数=向上（放大），负数=向下（缩小）
    const double dMaxIndex = static_cast<double>(layout.nMaxCount - 1);
    double dSpan = layout.viewEnd - layout.viewStart;
    //缩放系数：滚轮一格缩放 20%
    double dFactor = (wheelDelta > 0) ? 0.8 : 1.25;
    double dNewSpan = dSpan * dFactor;
    double dMinSpan = std::max(2.0, dMaxIndex * 0.02);
    double dMaxSpan = dMaxIndex;
    dNewSpan = std::max(dMinSpan, std::min(dNewSpan, dMaxSpan));
    //围绕鼠标位置缩放
    double dAnchor = layout.PixelToX(msg.ptMouse.x);
    double dAnchorRatio = (dSpan > 0.0) ? ((dAnchor - layout.viewStart) / dSpan) : 0.5;
    double dNewStart = dAnchor - dNewSpan * dAnchorRatio;
    double dNewEnd = dNewStart + dNewSpan;
    if (dNewStart < 0.0) {
        dNewEnd -= dNewStart;
        dNewStart = 0.0;
    }
    if (dNewEnd > dMaxIndex) {
        dNewStart -= (dNewEnd - dMaxIndex);
        dNewEnd = dMaxIndex;
        if (dNewStart < 0.0) {
            dNewStart = 0.0;
        }
    }
    m_viewStart = dNewStart;
    m_viewEnd = dNewEnd;
    Invalidate();
    return true;
}

} // namespace ui
