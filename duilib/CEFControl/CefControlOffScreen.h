/**@brief 封装Cef浏览器对象为duilib控件
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2016/7/19
 */
#ifndef UI_CEF_CONTROL_CEF_OSR_CONTROL_H_
#define UI_CEF_CONTROL_CEF_OSR_CONTROL_H_

#include "duilib/Core/Window.h"
#include "duilib/CEFControl/CefControl.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
namespace client
{
    class OsrImeHandlerWin;
}
#endif

namespace ui {

class CefMemoryBlock;

/** duilib的CEF控件，离屏渲染模式
*/
class CefControlOffScreen :public CefControl
{
    typedef CefControl BaseClass;
public:
    explicit CefControlOffScreen(ui::Window* pWindow);
    virtual ~CefControlOffScreen(void) override;

    /// 重写父类接口，提供个性化功能
    virtual void Init() override;
    virtual void SetPos(UiRect rc) override;
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;
    virtual void SetWindow(Window* pWindow) override;

    /** 是否为CEF的离屏渲染控件
    */
    virtual bool IsCefOSR() const override;

    /** 是否为CEF的离屏渲染控件，并自己处理输入法消息的模式
    */
    virtual bool IsCefOsrImeMode() const override;

    /** 将网页保存为一张图片, 图片大小与控件大小相同
    */
    virtual std::shared_ptr<IBitmap> MakeImageSnapshot() override;

protected:
    /** 设置可见状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetVisible(bool bChanged) override;

    /** 重新创建Browser控件
    */
    virtual void ReCreateBrowser() override;
    virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser, const CefRange& selected_range, const std::vector<CefRect>& character_bounds) override;

    /** 焦点元素发生变化（在主线程中调用）
    */
    virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefDOMNode::Type type,
                                      bool bText,
                                      bool bEditable,
                                      const CefRect& nodeRect) override;

    /** 设置光标(仅离屏渲染模式有效)
    */
    virtual void OnCursorChange(cef_cursor_type_t type) override;

    //光标消息
    virtual bool OnSetCursor(const EventArgs& msg) override;

    //控件所属窗口的鼠标捕获丢失
    virtual bool OnCaptureChanged(const EventArgs& msg) override;

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

    //焦点相关消息处理
    virtual bool OnSetFocus(const EventArgs& msg) override;
    virtual bool OnKillFocus(const EventArgs& msg) override;

    //键盘消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
    virtual bool OnChar(const EventArgs& msg) override;
    virtual bool OnKeyDown(const EventArgs& msg) override;
    virtual bool OnKeyUp(const EventArgs& msg) override;

    //输入法相关消息处理
    virtual bool OnImeSetContext(const EventArgs& msg) override;
    virtual bool OnImeStartComposition(const EventArgs& msg) override;
    virtual bool OnImeComposition(const EventArgs& msg) override;
    virtual bool OnImeEndComposition(const EventArgs& msg) override;

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
    /** 转发键盘相关消息到 BrowserHost
     * @param[in] uMsg 消息
     * @param[in] wParam 消息附加参数
     * @param[in] lParam 消息附加参数
     * @param[out] bHandled 是否继续传递消息
     * @return 返回消息处理结果
     */
    LRESULT SendKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 输入法相关的消息处理
    */
    void OnIMEStartComposition();
    void OnIMESetContext(UINT message, WPARAM wParam, LPARAM lParam);
    void OnIMEComposition(UINT message, WPARAM wParam, LPARAM lParam);
    void OnIMECancelCompositionEvent();

private:
    /** IME消息处理，以支持输入法的输入操作
    */
    std::unique_ptr<client::OsrImeHandlerWin> m_imeHandler;

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

    /** 处理焦点元素变化事件
    */
    void OnFocusedNodeChanged(bool bEditable, const CefRect& nodeRect);

private:
    // 页面绘制的内存数据,把cef离屏渲染的数据保存到缓存中
    std::unique_ptr<CefMemoryBlock> m_pCefMemData;

    // 页面弹出窗口的绘制的内存数据，把cef的popup窗口的离屏渲染数据保存到缓存中
    std::unique_ptr<CefMemoryBlock> m_pCefPopupMemData;

    // 当网页的组合框一类的控件弹出时，记录弹出的位置
    CefRect m_rectPopup;

private:
    //焦点元素的属性
    bool m_bHasFocusNode;
    bool m_bFocusNodeEditable;
    CefRect m_focusNodeRect;
};

}

#endif //UI_CEF_CONTROL_CEF_OSR_CONTROL_H_
