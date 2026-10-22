#ifndef EXAMPLES_CONTROLS_CASCADING_SELECT_FORM_H_
#define EXAMPLES_CONTROLS_CASCADING_SELECT_FORM_H_

#include "duilib/duilib.h"

/** CascadingSelect 级联选择器演示窗口
 */
class CascadingSelectForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    CascadingSelectForm();
    virtual ~CascadingSelectForm() override;

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
    /** 把当前选择回显到标签
     * @param [in] pCascadingSelect 级联选择控件
     */
    void UpdateSelectionText(ui::CascadingSelect* pCascadingSelect);

    /** 读取当前语言下的层级文本
     * @param [in] strId 语言 ID
     */
    DString GetLangText(const DString& strId) const;
};

#endif // EXAMPLES_CONTROLS_CASCADING_SELECT_FORM_H_
