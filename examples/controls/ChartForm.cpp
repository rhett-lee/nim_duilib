#include "ChartForm.h"
#include <cstdlib>
#include <ctime>

using namespace ui;

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
    s1.name = _T("今年");
    s1.data = data1;
    s1.color = _T("color_accent");
    series.push_back(s1);

    ui::Chart::Series s2;
    s2.name = _T("去年");
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
    //向两个系列各追加一个随机数据点（0~100）
    double value1 = static_cast<double>(std::rand() % 101);
    double value2 = static_cast<double>(std::rand() % 101);

    auto series = pLineChart->GetSeries();
    if (series.size() < 2) {
        //若尚未初始化多系列，则先初始化
        OnInitMultiSeriesLine(pLineChart);
        series = pLineChart->GetSeries();
    }
    series[0].data.push_back(value1);
    if (series.size() > 1) {
        series[1].data.push_back(value2);
    }
    pLineChart->SetSeriesData(series);

    //同步追加类目标签
    std::vector<DString> labels = pLineChart->GetDataLabels();
    labels.push_back(ui::StringUtil::Printf(_T("P%zu"), labels.size() + 1));
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
        s1.name = _T("收入");
        s1.data = { 45.0, 62.0, 38.0, 80.0, 55.0, 70.0 };
        s1.color = _T("color_accent");
        series.push_back(s1);
        ui::Chart::Series s2;
        s2.name = _T("支出");
        s2.data = { 30.0, 40.0, 25.0, 50.0, 35.0, 45.0 };
        s2.color = _T("color_warning");
        series.push_back(s2);

        pBarChart->SetBarMode(ui::ChartBarMode::kStacked);
        pBarChart->SetLegendVisible(true);
        pBarChart->SetSeriesData(series);
        pBarChart->SetDataLabels({ _T("一月"), _T("二月"), _T("三月"), _T("四月"), _T("五月"), _T("六月") });
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
        pPieChart->SetDataLabels({ _T("搜索引擎"), _T("直接访问"), _T("社交媒体"), _T("邮件推广"), _T("其他") });
        pPieChart->SetDonut(true);
    }
    else {
        pPieChart->SetData({ 40.0, 30.0, 30.0 });
        pPieChart->SetDataLabels({ _T("华东"), _T("华南"), _T("华北") });
        pPieChart->SetDonut(false);
    }
}
