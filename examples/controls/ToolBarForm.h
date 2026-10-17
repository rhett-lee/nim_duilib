#ifndef EXAMPLES_CONTROLS_TOOL_BAR_FORM_H_
#define EXAMPLES_CONTROLS_TOOL_BAR_FORM_H_

#include "duilib/duilib.h"

/** ToolBar 工具条容器演示窗口
*/
class ToolBarForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    ToolBarForm();
    virtual ~ToolBarForm() override;

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
    /** 工具条中被收纳的按钮被激活时的回调
    * @param [in] itemName 被激活按钮的名称
    */
    void OnOverflowItemActivated(const DString& itemName);
};

#endif // EXAMPLES_CONTROLS_TOOL_BAR_FORM_H_
