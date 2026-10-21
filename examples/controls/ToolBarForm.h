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

    /** 动态工具条的工具项点击回调
    * @param [in] itemName 工具项名称
    */
    void OnToolItemClick(const DString& itemName);

    /** 动态工具条的工具项勾选状态变化回调
    * @param [in] itemName 工具项名称
    * @param [in] bChecked 变化后的勾选状态
    */
    void OnToolItemCheckedChanged(const DString& itemName, bool bChecked);

    /** 更新窗口内的事件提示文本
    */
    void SetEventText(const DString& strText);

private:
    //动态工具条（由代码创建工具项）
    ui::ToolBar* m_pDynamicToolBar;
};

#endif // EXAMPLES_CONTROLS_TOOL_BAR_FORM_H_
