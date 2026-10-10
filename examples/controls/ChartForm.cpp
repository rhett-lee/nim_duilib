#include "ChartForm.h"
#include <cstdlib>
#include <ctime>

using namespace ui;

namespace
{
    /** 从语言文件读取指定 STRID 的文本（支持中英双语）
    */
    DString ChartLang(const DString& strId)
    {
        return GlobalManager::Instance().Lang().GetStringByID(strId);
    }
}

ChartForm::ChartForm()
{
}

ChartForm::~ChartForm()
{
}

DString ChartForm::GetSkinFolder()
{
    return _T("controls");
}

DString ChartForm::GetSkinFile()
{
    return _T("chart.xml");
}

void ChartForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //折线图（多系列）：演示 C++ 动态追加数据点 + 第二系列
    ui::Button* pLineAddBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_line_add")));
    ui::Button* pLineClearBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_line_clear")));
    ui::Chart* pLineChart = dynamic_cast<ui::Chart*>(FindControl(_T("chart_line")));
    if (pLineChart != nullptr) {
        //初始化：把 XML 里的单系列数据，扩展为"双系列"对比
        OnInitMultiSeriesLine(pLineChart);
    }
    if ((pLineAddBtn != nullptr) && (pLineChart != nullptr)) {
        pLineAddBtn->AttachClick([this, pLineChart](const ui::EventArgs& /*args*/) {
            AppendRandomLineData();
            return true;
            });
    }
    if ((pLineClearBtn != nullptr) && (pLineChart != nullptr)) {
        pLineClearBtn->AttachClick([pLineChart](const ui::EventArgs& /*args*/) {
            pLineChart->ClearData();
            pLineChart->SetDataLabels({});
            return true;
            });
    }

    //柱状图：演示切换数据源 + 分组/堆叠模式切换
    ui::Button* pBarSwitchBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_bar_switch")));
    if (pBarSwitchBtn != nullptr) {
        pBarSwitchBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            SwitchBarData();
            return true;
            });
    }

    //饼图：演示切换数据源 + 环形/实心切换
    ui::Button* pPieSwitchBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_pie_switch")));
    if (pPieSwitchBtn != nullptr) {
        pPieSwitchBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            SwitchPieData();
            return true;
            });
    }

    //交互演示：数据点点击选中回调（折线/柱状/饼图均支持）
    AttachChartPointClick(pLineChart, _T("chart_line"));
    AttachChartPointClick(dynamic_cast<ui::Chart*>(FindControl(_T("chart_bar"))), _T("chart_bar"));
    AttachChartPointClick(dynamic_cast<ui::Chart*>(FindControl(_T("chart_pie"))), _T("chart_pie"));
}

void ChartForm::AttachChartPointClick(ui::Chart* pChart, const DString& strChartName)
{
    if (pChart == nullptr) {
        return;
    }
    pChart->AttachPointClick([pChart, strChartName](const ui::EventArgs& args) {
        ui::Chart::HitResult hit = ui::Chart::GetHitFromEvent(args);
        if (hit.type != ui::Chart::HitType::kNone) {
            //演示：打印命中的元素信息（Tooltip 已由控件自动显示，这里演示回调用法）
            (void)pChart;
            (void)strChartName;
        }
        return true;
        });
}

void ChartForm::OnInitMultiSeriesLine(ui::Chart* pLineChart)
{
    if (pLineChart == nullptr) {
        return;
    }
    //把单系列扩展为双系列：系列1 保留原数据，新增系列2 做对比
    const std::vector<double>& data1 = pLineChart->GetData();
    std::vector<ui::Chart::Series> series;
    ui::Chart::Series s1;
    s1.name = ChartLang(_T("STRID_CHART_SERIES_THISYEAR"));
    s1.data = data1;
    s1.color = _T("color_accent");
    series.push_back(s1);

    ui::Chart::Series s2;
    s2.name = ChartLang(_T("STRID_CHART_SERIES_LASTYEAR"));
    s2.data = { 28.0, 40.0, 22.0, 55.0, 45.0, 36.0, 50.0, 28.0, 40.0, 47.0 };
    s2.color = _T("color_warning"); //若语义色不存在则自动分配
    series.push_back(s2);

    pLineChart->SetSeriesData(series);
}

void ChartForm::AppendRandomLineData()
{
    ui::Chart* pLineChart = dynamic_cast<ui::Chart*>(FindControl(_T("chart_line")));
    if (pLineChart == nullptr) {
        return;
    }
    auto series = pLineChart->GetSeries();
    if (series.empty()) {
        //清空后的全新起点：两个空系列从零开始同步追加
        ui::Chart::Series s1;
        s1.name = ChartLang(_T("STRID_CHART_SERIES_THISYEAR"));
        s1.color = _T("color_accent");
        series.push_back(s1);
        ui::Chart::Series s2;
        s2.name = ChartLang(_T("STRID_CHART_SERIES_LASTYEAR"));
        s2.color = _T("color_warning");
        series.push_back(s2);
    }
    //对齐各系列长度（取最大长度，短系列用末值补齐），保证系列间数据不错位
    size_t nMaxCount = 0;
    for (const auto& s : series) {
        nMaxCount = std::max(nMaxCount, s.data.size());
    }
    for (auto& s : series) {
        if (s.data.size() < nMaxCount) {
            double dFill = s.data.empty() ? 0.0 : s.data.back();
            s.data.resize(nMaxCount, dFill);
        }
    }
    //各追加一个随机数据点（0~100）
    series[0].data.push_back(static_cast<double>(std::rand() % 101));
    if (series.size() > 1) {
        series[1].data.push_back(static_cast<double>(std::rand() % 101));
    }
    pLineChart->SetSeriesData(series);

    //类目标签与数据点数对齐：已有标签保留（初始为 Q1~Q10），不足时按序号补 P11、P12...
    std::vector<DString> labels = pLineChart->GetDataLabels();
    const size_t nDataCount = series[0].data.size();
    while (labels.size() < nDataCount) {
        labels.push_back(ui::StringUtil::Printf(_T("P%zu"), labels.size() + 1));
    }
    pLineChart->SetDataLabels(labels);
}

void ChartForm::SwitchBarData()
{
    ui::Chart* pBarChart = dynamic_cast<ui::Chart*>(FindControl(_T("chart_bar")));
    if (pBarChart == nullptr) {
        return;
    }
    //在"分组（含负值）"与"堆叠（多系列）"之间切换
    static bool bToggle = false;
    bToggle = !bToggle;
    if (bToggle) {
        //堆叠模式：两个系列正向累加
        std::vector<ui::Chart::Series> series;
        ui::Chart::Series s1;
        s1.name = ChartLang(_T("STRID_CHART_SERIES_INCOME"));
        s1.data = { 45.0, 62.0, 38.0, 80.0, 55.0, 70.0 };
        s1.color = _T("color_accent");
        series.push_back(s1);
        ui::Chart::Series s2;
        s2.name = ChartLang(_T("STRID_CHART_SERIES_EXPENSE"));
        s2.data = { 30.0, 40.0, 25.0, 50.0, 35.0, 45.0 };
        s2.color = _T("color_warning");
        series.push_back(s2);

        pBarChart->SetBarMode(ui::ChartBarMode::kStacked);
        pBarChart->SetLegendVisible(true);
        pBarChart->SetSeriesData(series);
        pBarChart->SetDataLabels({ ChartLang(_T("STRID_CHART_MONTH_1")), ChartLang(_T("STRID_CHART_MONTH_2")),
                                   ChartLang(_T("STRID_CHART_MONTH_3")), ChartLang(_T("STRID_CHART_MONTH_4")),
                                   ChartLang(_T("STRID_CHART_MONTH_5")), ChartLang(_T("STRID_CHART_MONTH_6")) });
    }
    else {
        //分组模式：单系列，含负值
        pBarChart->SetBarMode(ui::ChartBarMode::kGrouped);
        pBarChart->SetData({ 45.0, -18.0, 62.0, -25.0, 38.0, 80.0, -15.0, 55.0, 70.0 });
        pBarChart->SetDataLabels({ _T("Q1"), _T("Q2"), _T("Q3"), _T("Q4"), _T("Q5"), _T("Q6"), _T("Q7"), _T("Q8"), _T("Q9") });
    }
}

void ChartForm::SwitchPieData()
{
    ui::Chart* pPieChart = dynamic_cast<ui::Chart*>(FindControl(_T("chart_pie")));
    if (pPieChart == nullptr) {
        return;
    }
    //在两组数据 + 环形/实心之间切换
    static bool bToggle = false;
    bToggle = !bToggle;
    if (bToggle) {
        pPieChart->SetData({ 30.0, 25.0, 20.0, 15.0, 10.0 });
        pPieChart->SetDataLabels({ ChartLang(_T("STRID_CHART_SLICE_SE")), ChartLang(_T("STRID_CHART_SLICE_DIRECT")),
                                   ChartLang(_T("STRID_CHART_SLICE_SOCIAL")), ChartLang(_T("STRID_CHART_SLICE_EMAIL")),
                                   ChartLang(_T("STRID_CHART_SLICE_OTHER")) });
        pPieChart->SetDonut(true);
    }
    else {
        pPieChart->SetData({ 40.0, 30.0, 30.0 });
        pPieChart->SetDataLabels({ ChartLang(_T("STRID_CHART_REGION_EAST")), ChartLang(_T("STRID_CHART_REGION_SOUTH")),
                                   ChartLang(_T("STRID_CHART_REGION_NORTH")) });
        pPieChart->SetDonut(false);
    }
}
