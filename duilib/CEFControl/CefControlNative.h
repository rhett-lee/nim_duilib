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
    virtual void SetVisible(bool bVisible) override;
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

    /** 焦点元素发生变化（在主线程中调用）
    */
    virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefDOMNode::Type type,
                                      bool bText,
                                      bool bEditable,
                                      const CefRect& nodeRect) override;

    /** 将网页保存为一张图片, 图片大小与控件大小相同
    */
    virtual std::shared_ptr<IBitmap> MakeImageSnapshot() override;

private:
    /** 将页面保存为成一张位图数据
    */
#if defined (DUILIB_BUILD_FOR_WIN)
    bool CaptureWindowBitmap_Win32(std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height);
#elif defined (DUILIB_BUILD_FOR_MACOS)
    bool CaptureWindowBitmap_Mac(std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height);
#elif defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
    bool CaptureWindowBitmap_X11(std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height);
#endif

private:
    /** 首次绘制的事件是否关联
    */
    bool m_bWindowFirstShown;
};
}

#endif //UI_CEF_CONTROL_CEF_NATIVE_CONTROL_H_
