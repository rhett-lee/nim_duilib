#ifndef EXAMPLES_CONTROLS_POPCONFIRM_FORM_H_
#define EXAMPLES_CONTROLS_POPCONFIRM_FORM_H_

#include "duilib/duilib.h"

/** Popconfirm 确认气泡卡片演示窗口
 */
class PopconfirmForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    PopconfirmForm();
    virtual ~PopconfirmForm() override;

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

    /** "基本用法"的锚点：回显卡片的打开状态与关闭原因（确定/取消也走该事件）
     */
    void BindStateEcho();

    /** 为各演示锚点绑定"确定"/"取消"事件，回显最近一次的操作结果
     */
    void BindResultEcho();

    /** "保持打开"（close_on_ok="false"）锚点：由右侧按钮调用 Confirm() / Cancel()
     */
    void BindKeepOpen();

    /** 把卡片的打开状态与关闭原因回显到标签
     * @param [in] bOpen 是否已打开
     * @param [in] nReason 关闭原因（打开时忽略）
     */
    void UpdateStateText(bool bOpen, int32_t nReason);

    /** 把最近一次"确定"/"取消"的结果回显到标签
     * @param [in] bConfirm true 为"确定"，false 为"取消"
     */
    void UpdateResultText(bool bConfirm);

private:
    /** "保持打开"的气泡卡片控件（生命周期与窗口一致，不需要额外持有）
     */
    ui::Popconfirm* m_pKeepOpen = nullptr;
};

#endif // EXAMPLES_CONTROLS_POPCONFIRM_FORM_H_
