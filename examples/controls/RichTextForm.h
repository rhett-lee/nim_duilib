#ifndef EXAMPLES_CONTROLS_RICH_TEXT_FORM_H_
#define EXAMPLES_CONTROLS_RICH_TEXT_FORM_H_

#include "duilib/duilib.h"

/** RichText 富文本控件测试窗口：覆盖内联格式、链接、段落、排版控制与交互 API
*/
class RichTextForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    RichTextForm();
    virtual ~RichTextForm() override;

    /** 加载资源文件的目录，位于 bin/resources/ 目录下
    */
    virtual DString GetSkinFolder() override;

    /** 窗口使用的 XML 布局文件
    */
    virtual DString GetSkinFile() override;

    /** 窗口初始化
    */
    virtual void OnInitWindow() override;

    /** 首次布局完成（第一次绘制前）：此时富文本已完成解析，可准确回显链接数量等状态
    */
    virtual void OnInitLayout() override;

private:
    /** 绑定运行时 API 测试按钮
    */
    void BindApiButtons();

    /** 应用当前的文本对齐方式（left / center / right / justify 轮换）
    */
    void ApplyTextAlign();

    /** 刷新状态回显（对齐方式、链接数量、选中字符数、自动换行）
    */
    void UpdateStatus();

    /** 显示链接事件（点击/悬停/离开）
    * @param [in] strTextId 事件文案的语言资源ID
    * @param [in] args 事件参数，wParam 为链接的URL
    */
    void ShowLinkEvent(const DString& strTextId, const ui::EventArgs& args);

    /** 获取当前对齐方式的显示名称
    */
    DString GetAlignName() const;

private:
    /** 可交互的富文本控件（用于运行时 API 测试）
    */
    ui::RichText* m_pInteractive = nullptr;

    /** 状态回显标签
    */
    ui::Label* m_pStatus = nullptr;

    /** 当前的对齐方式索引：0左对齐，1居中，2右对齐，3两端对齐
    */
    int32_t m_nAlignIndex = 0;

    /** 链接颜色是否已切换为自定义配色
    */
    bool m_bCustomLinkColor = false;
};

#endif // EXAMPLES_CONTROLS_RICH_TEXT_FORM_H_
