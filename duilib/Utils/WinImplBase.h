#ifndef UI_UTILS_WINIMPLBASE_H_
#define UI_UTILS_WINIMPLBASE_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "duilib/Core/Window.h"

namespace ui
{

#define UI_WNDSTYLE_FRAME		(WS_VISIBLE | WS_OVERLAPPEDWINDOW)
#define UI_WNDSTYLE_DIALOG		(WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_DLGFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)

#define UI_CLASSSTYLE_FRAME		(CS_VREDRAW | CS_HREDRAW)
#define UI_CLASSSTYLE_DIALOG	(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_SAVEBITS)

/** 实现一个带有标题栏的窗体，带有最大化、最小化、还原按钮的支持
* 该实现支持窗口阴影，单仅提供带有 WS_EX_LAYERED 属性窗口阴影。
* 该类实现，封装了2种窗口模式：
* （1）正常模式，不附加阴影的窗口：可以通过XML配置文件<Window>如下配置（举例）：
*              shadow_attached="false" layered_window="false" ，不需要配置alpha属性，不支持alpha属性;
*              上述配置等同于：layered_window="false"，不设置shadow_attached属性
* （2）附加阴影窗口模式，需要使用层窗口(带有WS_EX_LAYERED属性)，可以通过XML配置文件<Window>如下配置（举例）：
*              shadow_attached="true" layered_window="true" ，alpha为可选，设置窗口透明度，如果不设置默认为255；
*              上述配置等同于：layered_window="true"，不设置shadow_attached属性
*/
class UILIB_API WindowImplBase : public Window
{
public:
    WindowImplBase();

protected:
    //析构函数不允许外部调用，因在OnFinalMessage函数中，有delete this操作
    virtual ~WindowImplBase();

public:
    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录
    */
    virtual std::wstring GetSkinFolder() = 0;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual std::wstring GetSkinFile() = 0;

    /** 创建窗口时被调用，由子类实现用以获取窗口唯一的类名称
    * @return 子类需实现并返回窗口唯一的类名称
    */
    virtual std::wstring GetWindowClassName() const override = 0;

    /** 获取窗口样式
    * @return 默认返回当前窗口的样式去掉WS_CAPTION属性
    *         如果子类重写该函数后，返回值为0，则不改变当前窗口的样式
    */
    virtual UINT GetStyle() const;

public:

    /** 当要创建的控件不是标准的控件名称时会调用该函数
    * @param [in] strClass 控件名称
    * @return 返回一个自定义控件指针，一般情况下根据 strClass 参数创建自定义的控件
    */
    virtual Control* CreateControl(const std::wstring& strClass);

protected:
    /** 当接收到窗口创建消息时被调用，供子类中做一些初始化的工作
     */
    virtual void OnInitWindow() override;

    /** 窗口消息的派发函数, 子类继承该函数，重写函数中，需要调用基类的函数
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 消息是否已经处理，
                返回 true  表明已经成功处理消息，不需要再传递给窗口过程；
                返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果
    */
    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    /** 收到窗口创建消息（WM_CREATE）时被调用，请使用 OnInitWindow 接口来实现自定义需求
    * @param [in] uMsg 消息ID
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 消息是否已经被处理
    * @return 返回消息处理结果
    */
    virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 在窗口收到 WM_NCDESTROY 消息时会被调用
    * @param [in] hWnd 要销毁的窗口句柄
    */
    virtual void OnFinalMessage(HWND hWnd) override;

    /** 接收窗口控制命令消息时（WM_SYSCOMMAND）被调用
    *  @param [in] uMsg 消息内容
    *  @param [in] wParam 消息附加参数
    *  @param [in] lParam 消息附加参数
    *  @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    *  @return 返回消息处理结果
    */
    virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

protected:
    /** 当收到窗口关闭消息(WM_CLOSE)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 当收到窗口被销毁消息(WM_DESTROY)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收到鼠标移动消息(WM_MOUSEMOVE)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 当接收到(WM_MOUSEWHEEL)消息时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 当接收到(WM_MOUSEHOVER)消息时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收到鼠标左键按下消息(WM_LBUTTONDOWN)时被调用
    *  @param [in] uMsg 消息内容
    *  @param [in] wParam 消息附加参数
    *  @param [in] lParam 消息附加参数
    *  @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    *  @return 返回消息处理结果
    */
    virtual LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收到鼠标左键弹起消息(WM_LBUTTONUP)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收到鼠标左键双击消息(WM_LBUTTONDBLCLK)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnLButtonDbClk(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收到鼠标左键按下消息(WM_RBUTTONDOWN)时被调用
    *  @param [in] uMsg 消息内容
    *  @param [in] wParam 消息附加参数
    *  @param [in] lParam 消息附加参数
    *  @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    *  @return 返回消息处理结果
    */
    virtual LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收到鼠标左键弹起消息(WM_RBUTTONUP)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收到鼠标左键双击消息(WM_RBUTTONDBLCLK)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnRButtonDbClk(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 当接收到字符按键消息时（WM_CHAR）被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收键盘按键按下消息(WM_KEYDOWN)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收键盘按键按下消息(WM_KEYUP)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收键盘按键按下消息(WM_SYSKEYDOWN)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnSysKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收键盘按键按下消息(WM_SYSKEYUP)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param [out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnSysKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 接收键盘按键按下消息(WM_HOTKEY)时被调用
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param [out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT OnHotKey(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

protected:
    /** 进入全屏状态
    */
    virtual void OnWindowEnterFullScreen() override;

    /** 退出全屏状态
    */
    virtual void OnWindowExitFullScreen() override;

    /** 进入最大化状态
    */
    virtual void OnWindowMaximized() override;

    /** 从最大化还原
    */
    virtual void OnWindowRestored() override;

    /** 进入最小化状态
    */
    virtual void OnWindowMinimized() override;

    /** 切换系统标题栏与自绘标题栏
    */
    virtual void OnUseSystemCaptionBarChanged() override;

private:
    /** 窗口功能按钮被点击时调用
    * @param [in] param 携带的参数
    * @return 始终返回 true
    */
    bool OnButtonClick(const EventArgs& param);

    /** 处理最大化/还原按钮的状态
    */
    void ProcessMaxRestoreStatus();
};
}

#endif // UI_UTILS_WINIMPLBASE_H_
