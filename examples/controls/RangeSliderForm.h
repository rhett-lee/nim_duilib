#ifndef EXAMPLES_CONTROLS_RANGE_SLIDER_FORM_H_
#define EXAMPLES_CONTROLS_RANGE_SLIDER_FORM_H_

#include "duilib/duilib.h"

/** RangeSlider 双滑块范围控件演示窗口
*/
class RangeSliderForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    RangeSliderForm();
    virtual ~RangeSliderForm() override;

    /** 加载资源文件的目录，位于 bin/resources/ 目录下
    */
    virtual DString GetSkinFolder() override;

    /** 窗口使用的 XML 布局文件
    */
    virtual DString GetSkinFile() override;

    /** 窗口初始化
    */
    virtual void OnInitWindow() override;

private:
    /** 把当前范围（下限~上限）回显到标签
    * @param [in] pRange 范围控件
    */
    void UpdateRangeText(ui::RangeSlider* pRange);
};

#endif // EXAMPLES_CONTROLS_RANGE_SLIDER_FORM_H_
