#ifndef EXAMPLES_CONTROLS_POPOVER_FORM_H_
#define EXAMPLES_CONTROLS_POPOVER_FORM_H_

#include "duilib/duilib.h"

/** Popover 气泡卡片演示窗口
 */
class PopoverForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    PopoverForm();
    virtual ~PopoverForm() override;

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
    /** 读取当前语言下的文本
     * @param [in] strId 语言 ID
     */
    DString GetLangText(const DString& strId) const;

    /** 绑定基础用法锚点（悬停弹出，卡片正文由 content_xml 拼装成两行）
     */
    void BindBasicPopover();

    /** 绑定"三种触发方式"中需要 API 参与的部分（手动触发的打开按钮）
     */
    void BindTriggerPopovers();

    /** 绑定受控模式的锚点（手动触发 + 按钮控制 + 状态回显）
     */
    void BindControlledPopover();

    /** 给"复杂内容"锚点设置 content_xml，并在卡片打开后绑定卡片内的"从浮层内关闭"按钮
     */
    void BindRichPopover();

    /** 把卡片的打开状态回显到标签
     * @param [in] bOpen 是否已打开
     * @param [in] nReason 关闭原因（打开时忽略）
     */
    void UpdateStateText(bool bOpen, int32_t nReason);

private:
    /** 受控模式的气泡卡片控件（生命周期与窗口一致，不需要额外持有）
     */
    ui::Popover* m_pControlled = nullptr;
};

#endif // EXAMPLES_CONTROLS_POPOVER_FORM_H_
