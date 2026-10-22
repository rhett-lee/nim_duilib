#ifndef EXAMPLES_TEST_FORM_H_
#define EXAMPLES_TEST_FORM_H_

// duilib
#include "duilib/duilib.h"

class TestForm : public ui::WindowImplBase
{
public:
    TestForm();
    virtual ~TestForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

private:
    /** 显示进度条控件测试窗口（独立演示窗口）
    */
    void ShowProgressWindow();

    /** 显示标签栏控件测试窗口（垂直标签 / 固定标签 / 溢出滚动）
    */
    void ShowTabCtrlWindow();

    /** 显示 RangeSlider 双滑块范围控件演示窗口
    */
    void ShowRangeSliderWindow();

    /** 显示 Stepper 向导步骤条演示窗口
    */
    void ShowStepperWindow();

    /** 显示 Rating 星级评分演示窗口
    */
    void ShowRatingWindow();

    /** 显示 Carousel 图片轮播 / Pagination 分页器演示窗口
    */
    void ShowCarouselWindow();

    /** 显示 TreeGrid 树表格演示窗口
    */
    void ShowTreeGridWindow();

    /** 显示 Breadcrumb 面包屑导航演示窗口
    */
    void ShowBreadcrumbWindow();

    /** 显示 Flyout 浮层演示
    * @param [in] pAnchor 锚点按钮（事件发送者）
    * @param [in] placement 期望弹出方位
    */
    void ShowFlyoutDemo(ui::Control* pAnchor, ui::Flyout::Placement placement);
};

#endif //EXAMPLES_TEST_FORM_H_
