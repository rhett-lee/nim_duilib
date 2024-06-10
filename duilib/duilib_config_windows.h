#ifndef DUILIB_CONFIG_WINDOWS_H_
#define DUILIB_CONFIG_WINDOWS_H_

/** Windows SDK 实现(内部采用Windows API实现部分功能)
*   最低支持的系统平台：Win7
*/
#if defined(UILIB_DLL)
    #if defined(UILIB_EXPORTS)
        #define UILIB_API __declspec(dllexport)
    #else
        #define UILIB_API __declspec(dllimport)
    #endif
#else
    #define UILIB_API
#endif

#ifndef VC_EXTRALEAN
    #define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN            // remove rarely used header files, including 'winsock.h'
    #define WIN32_LEAN_AND_MEAN            // which will conflict with 'winsock2.h'
#endif

#ifndef WINVER
    #define WINVER _WIN32_WINNT_WIN7
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT _WIN32_WINNT_WIN7
#endif

#ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS _WIN32_WINNT_WIN7
#endif

#ifndef _WIN32_IE
    #define _WIN32_IE _WIN32_IE_WIN7
#endif

#ifndef NOMINMAX
    #define NOMINMAX 1
#endif

// 包括 SDKDDKVer.h 将定义可用的最高版本的 Windows 平台。

// 如果要为以前的 Windows 平台生成应用程序，请包括 WinSDKVer.h，并
// 将 _WIN32_WINNT 宏设置为要支持的平台，然后再包括 SDKDDKVer.h。
#include <SDKDDKVer.h>

// Windows 平台必要的头文件
#include <windows.h>
#include <crtdbg.h>
#include <tchar.h>

///////////////////////////////////////////////////////
// windowsx.h
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))

/* Value for rolling one detent */
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#define GET_KEYSTATE_WPARAM(wParam)     (LOWORD(wParam))

#endif //DUILIB_CONFIG_WINDOWS_H_
