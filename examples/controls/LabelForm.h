#ifndef EXAMPLES_CONTROLS_LABEL_FORM_H_
#define EXAMPLES_CONTROLS_LABEL_FORM_H_

#include "duilib/duilib.h"

/** Label 文本控件测试窗口：覆盖文本、对齐、多行换行、状态颜色、纵向文本、
*  省略号、内边距、富文本模式与运行时 API
*/
class LabelForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    LabelForm();
    virtual ~LabelForm() override;

    /** 加载资源文件的目录，位于 bin/resources/ 目录下
    */
    virtual DString GetSkinFolder() override;

    /** 窗口使用的 XML 布局文件
    */
    virtual DString GetSkinFile() override;

    /** 窗口初始化
    */
    virtual void OnInitWindow() override;

    /** 首次布局完成（第一次绘制前）：刷新状态回显
    */
    virtual void OnInitLayout() override;

private:
    /** 绑定运行时 API 测试按钮
    */
    void BindApiButtons();

    /** 切换文本的水平对齐方式（left / center / right / justify 轮换）
    */
    void CycleTextAlign();

    /** 刷新状态回显（对齐方式、单行、纵向文本、换行替换、RichText）
    */
    void UpdateStatus();

    /** 获取当前水平对齐方式的显示名称
    */
    DString GetAlignName() const;

private:
    /** 可交互的标签控件（用于运行时 API 测试）
    */
    ui::Label* m_pTarget = nullptr;

    /** 状态回显标签
    */
    ui::Label* m_pStatus = nullptr;

    /** 当前的水平对齐方式索引：0左对齐，1居中，2右对齐，3两端对齐
    */
    int32_t m_nAlignIndex = 0;
};

#endif // EXAMPLES_CONTROLS_LABEL_FORM_H_
