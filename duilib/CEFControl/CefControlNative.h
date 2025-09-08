/**@brief 封装Cef浏览器对象为duilib控件
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2016/7/19
 */
#ifndef UI_CEF_CONTROL_CEF_NATIVE_CONTROL_H_
#define UI_CEF_CONTROL_CEF_NATIVE_CONTROL_H_

#include "CefControl.h"

namespace ui {

/** duilib的CEF控件，窗口模式
*/
class CefControlNative : public CefControl
{
    typedef CefControl BaseClass;
public:
    explicit CefControlNative(ui::Window* pWindow);
    virtual ~CefControlNative(void) override;

    virtual void Init() override;
    virtual void SetPos(ui::UiRect rc) override;
    virtual bool OnSetFocus(const EventArgs& msg) override;
    virtual bool OnKillFocus(const EventArgs& msg) override;
    virtual void SetWindow(ui::Window* pWindow) override;

protected:
    /** 重新创建Browser对象
    */
    virtual void ReCreateBrowser() override;

    /** 更新CEF控件窗口的位置（子窗口模式）
    */
    virtual void UpdateCefWindowPos() override;

    /** 关闭所有的Browser对象
    */
    virtual void CloseAllBrowsers() override;

    /** 将网页保存为一张图片, 图片大小与控件大小相同
    */
    virtual std::shared_ptr<IBitmap> MakeImageSnapshot() override;

    /** 是否为CEF的子窗口模式控件
    */
    virtual bool IsCefNative() const override;

    /** 页面获得了焦点
    */
    virtual void OnGotFocus() override;

    /** 设置可见状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetVisible(bool bChanged) override;

private:
    /** 关闭所有的Browser对象
    */
    void DoCloseAllNativeBrowsers(bool bForceClose);

private:
    /** 首次绘制的事件是否关联
    */
    bool m_bWindowFirstShown;

    /** CEF窗口的父子关系是否已经解除
    */
    bool m_bSetCefWindowParentNull;
};
}

#endif //UI_CEF_CONTROL_CEF_NATIVE_CONTROL_H_
