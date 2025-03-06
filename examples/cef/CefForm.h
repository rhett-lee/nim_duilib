#ifndef EXAMPLES_CEF_FORM_H_
#define EXAMPLES_CEF_FORM_H_

// duilib
#include "duilib/duilib.h"

// CEF
#include "duilib/duilib_cef.h"

// 控制是否使用离屏渲染：
// 当为 true  时将使用默认窗口阴影方案，离屏渲染模式下窗口有 WS_EX_LAYERED 属性；
// 当为 false 时因使用了真窗口模式不支持带有 WS_EX_LAYERED 属性窗口，所以使用外置窗口阴影方案；
const bool kEnableOffScreenRendering = false;

/** CEF控件的窗口
*/
class CefForm : public std::conditional<kEnableOffScreenRendering, ui::WindowImplBase, ui::ShadowWnd>::type
{
public:
    CefForm();
    virtual ~CefForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

private:
    bool OnClicked(const ui::EventArgs& msg);
    bool OnNavigate(const ui::EventArgs& msg);
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode);

private:
    ui::CefControlBase* m_pCefControl;
    ui::CefControlBase* m_pCefControlDev;
    ui::Button* m_pDevToolBtn;
    ui::RichEdit* m_pEditUrl;
};

#endif //EXAMPLES_CEF_FORM_H_
