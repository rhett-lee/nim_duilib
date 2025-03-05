/**@brief 封装Cef浏览器对象为duilib控件
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2016/7/19
 */
#ifndef UI_CEF_CONTROL_CEF_CONTROL_H_
#define UI_CEF_CONTROL_CEF_CONTROL_H_

#include "duilib/Core/Window.h"
#include "duilib/CEFControl/CefControlBase.h"

namespace ui {

class CefMemoryBlock;

/** duilib的CEF控件，离屏渲染模式
*/
class CefControlOffScreen :public CefControlBase
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    , public ui::IUIMessageFilter
#endif
{
    typedef CefControlBase BaseClass;
public:
    explicit CefControlOffScreen(ui::Window* pWindow);
    virtual ~CefControlOffScreen(void) override;

    /// 重写父类接口，提供个性化功能
    virtual void Init() override;
    virtual void SetPos(UiRect rc) override;
    virtual void HandleEvent(const ui::EventArgs& msg) override;
    virtual void SetVisible(bool bVisible) override;
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;
    virtual void SetWindow(Window* pWindow) override;

    /** 打开开发者工具
    * @param [in] view 一个 CefControlOffScreen 控件实例(仅在CefControlOffScreen类里需要传入)
    * @return 成功返回 true，失败返回 false
    */
    virtual bool AttachDevTools(Control* view) override;

    /** 关闭开发者工具
    */
    virtual void DettachDevTools() override;

protected:
    /** 重新创建Browser控件
    */
    virtual void ReCreateBrowser() override;

    // 在非UI线程中被调用
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;

    //鼠标消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool MouseLeave(const EventArgs& msg) override;
    virtual bool MouseWheel(const EventArgs& msg) override;

    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool ButtonDoubleClick(const EventArgs& msg) override;
    virtual bool RButtonDown(const EventArgs& msg) override;
    virtual bool RButtonUp(const EventArgs& msg) override;
    virtual bool RButtonDoubleClick(const EventArgs& msg) override;
    virtual bool MButtonDown(const EventArgs& msg) override;
    virtual bool MButtonUp(const EventArgs& msg) override;
    virtual bool MButtonDoubleClick(const EventArgs& msg) override;

    //键盘消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
    virtual bool OnChar(const EventArgs& msg) override;
    virtual bool OnKeyDown(const EventArgs& msg) override;
    virtual bool OnKeyUp(const EventArgs& msg) override;

private:
    void SendButtonDownEvent(const EventArgs& msg);
    void SendButtonUpEvent(const EventArgs& msg);
    void SendButtonDoubleClickEvent(const EventArgs& msg);

    /** @brief 转换普通鼠标消息到 CEF 可识别的鼠标消息
     * @param [in] msg 消息
     * @return 返回转换后的结果
     */
    int32_t GetCefMouseModifiers(const EventArgs& msg) const;

#if defined (DUILIB_BUILD_FOR_SDL)
    /** 转发键盘相关消息到 BrowserHost
    */
    void SendKeyEvent(const EventArgs& msg, cef_key_event_type_t type);
#endif

protected:

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    /** 对消息进行过滤，处理部分业务(处理窗体消息，转发到Cef浏览器对象)
    */
    virtual LRESULT FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    /** 转发捕获焦点消息到 BrowserHost
     * @param[in] uMsg 消息
     * @param[in] wParam 消息附加参数
     * @param[in] lParam 消息附加参数
     * @param[out] bHandled 是否继续传递消息
     * @return 返回消息处理结果
     */
    LRESULT SendCaptureLostEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 转发键盘相关消息到 BrowserHost
     * @param[in] uMsg 消息
     * @param[in] wParam 消息附加参数
     * @param[in] lParam 消息附加参数
     * @param[out] bHandled 是否继续传递消息
     * @return 返回消息处理结果
     */
    LRESULT SendKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

#endif

private:
    /** CefRenderHandler接口, 在非UI线程中被调用
    *   当浏览器渲染数据变化时，会触发此接口，此时把渲染数据保存到内存dc
        并且通知窗体刷新控件，在控件的Paint函数里把内存dc的位图画到窗体上
        由此实现离屏渲染数据画到窗体上
    */
    virtual void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type,
                         const CefRenderHandler::RectList& dirtyRects, const void* buffer,
                         int width, int height) override;//CefRenderHandler接口
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;//CefRenderHandler接口
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;//CefRenderHandler接口

    /** 客户区坐标转换为控件坐标
    */
    virtual void ClientToControl(UiPoint& pt) override;

    /** 处理DPI自适应（离屏渲染模式与正常模式不同）
    */
    void AdaptDpiScale(CefMouseEvent& mouse_event);

private:
    // 页面绘制的内存数据,把cef离屏渲染的数据保存到缓存中
    std::unique_ptr<CefMemoryBlock> m_pCefMemData;

    // 页面弹出窗口的绘制的内存数据，把cef的popup窗口的离屏渲染数据保存到缓存中
    std::unique_ptr<CefMemoryBlock> m_pCefPopupMemData;

    // 当网页的组合框一类的控件弹出时，记录弹出的位置
    CefRect m_rectPopup;

    //开发者工具对应的控件
    CefControlOffScreen* m_pDevToolView;
};

}

#endif //UI_CEF_CONTROL_CEF_CONTROL_H_
