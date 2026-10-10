#ifndef EXAMPLES_CHART_FORM_H_
#define EXAMPLES_CHART_FORM_H_

// duilib
#include "duilib/duilib.h"

/** Chart 轻量图表演示窗口
 *  演示折线图 / 柱状图 / 饼图三种形态，以及多系列、坐标轴、标题、
 *  曲线/面积、负值柱状、堆叠、环形饼图、百分比等全部能力，以及 C++ 动态数据接口。
*/
class ChartForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    ChartForm();
    virtual ~ChartForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

private:
    /** 初始化折线图为多系列（今年/去年对比）
    */
    void OnInitMultiSeriesLine(ui::Chart* pLineChart);

    /** 演示动态数据：向折线图两个系列追加随机数据点
    */
    void AppendRandomLineData();

    /** 演示切换柱状图数据源（分组含负值 <-> 堆叠多系列）
    */
    void SwitchBarData();

    /** 演示切换饼图数据源（含环形/实心切换）
    */
    void SwitchPieData();
};

#endif //EXAMPLES_CHART_FORM_H_
