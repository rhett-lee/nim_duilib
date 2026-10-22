#ifndef EXAMPLES_CONTROLS_BREADCRUMB_FORM_H_
#define EXAMPLES_CONTROLS_BREADCRUMB_FORM_H_

#include "duilib/duilib.h"

/** Breadcrumb 面包屑导航控件演示窗口
*/
class BreadcrumbForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    BreadcrumbForm();
    virtual ~BreadcrumbForm() override;

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
    /** 把当前项回显到标签
    * @param [in] pBreadcrumb 面包屑控件
    */
    void UpdateCurrentText(ui::Breadcrumb* pBreadcrumb);
};

#endif // EXAMPLES_CONTROLS_BREADCRUMB_FORM_H_
