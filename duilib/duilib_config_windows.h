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

#include <SDKDDKVer.h>
#include <windows.h>
#include <crtdbg.h>
#include <tchar.h>

#endif //DUILIB_CONFIG_WINDOWS_H_
