#ifndef _UI_CONTROL_DATETIME_WND_WINDOWS_H_
#define _UI_CONTROL_DATETIME_WND_WINDOWS_H_

#include "duilib/Core/UiTypes.h"
#include <ctime>

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

namespace ui
{
class DateTime;
class WindowBase;

/** 日期时间选择控件窗口的实现（Windows平台）
*/
class DateTimeWnd
{
public:
    explicit DateTimeWnd(DateTime* pOwner);
    ~DateTimeWnd();

public:
    //初始化
    bool Init(DateTime* pOwner);

    //更新窗口的位置
    void UpdateWndPos();

    //显示窗口
    void ShowWindow();

private:
    //注册控件窗口类
    bool RegisterSuperClass();

    //窗口类名
    DString GetWindowClassName() const;

    //窗口过程函数
    static LRESULT CALLBACK __ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    //在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是清理资源，并调用OnDeleteSelf函数销毁该窗口对象）
    void OnFinalMessage();

    //窗口消息的处理函数, 从系统接收到消息后，进入的第一个处理函数
    LRESULT WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    UiRect CalPos();
    HFONT CreateHFont() const;
    struct tm SystemTimeToStdTime(const SYSTEMTIME& sysTime) const;
    SYSTEMTIME StdTimeToSystemTime(const struct tm& tmTime) const;

private:
    HWND m_hDateTimeWnd;
    WNDPROC m_OldWndProc; //原来的窗口过程函数
    DateTime* m_pOwner;
    bool m_bInit;
    bool m_bDropOpen;
    SYSTEMTIME m_oldSysTime;
    HFONT m_hFont;
};

} //namespace ui

#endif // (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#endif // _UI_CONTROL_DATETIME_WND_WINDOWS_H_
