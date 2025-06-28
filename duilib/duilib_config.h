#ifndef DUILIB_CONFIG_H_
#define DUILIB_CONFIG_H_

/** 平台检测：Windows、Linux 或 macOS
*/
#if defined (_WIN32) || defined (_WIN64)
    #define DUILIB_BUILD_FOR_WIN    1
    //是否使用SDL的窗口和鼠标键盘事件（目前只支持SDL3）
#if (DUILIB_SDL)
    //可以将msvc\PropertySheets\SDLSettings.props文件中的SDLEnabled改为1，以开启SDL功能
    #define DUILIB_BUILD_FOR_SDL    1
#endif

#if (DUILIB_WEBVIEW2)
    //可以将msvc\PropertySheets\WebView2Settings.props文件中的WebView2Enabled改为1，以开启WebView2功能
    #define DUILIB_BUILD_FOR_WEBVIEW2   1
#endif
    
#elif defined(linux) || defined(__linux) || defined(__linux__)
    #define DUILIB_BUILD_FOR_LINUX  1
    //是否使用SDL的窗口和鼠标键盘事件（目前只支持SDL3）
    #define DUILIB_BUILD_FOR_SDL    1
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #define DUILIB_BUILD_FOR_MACOS  1
        //是否使用SDL的窗口和鼠标键盘事件（目前只支持SDL3）
        #define DUILIB_BUILD_FOR_SDL    1
    #endif
#elif defined(__FreeBSD__)
    #define DUILIB_BUILD_FOR_FREEBSD  1
    //是否使用SDL的窗口和鼠标键盘事件（目前只支持SDL3）
    #define DUILIB_BUILD_FOR_SDL  1
#else
    //不支持的系统
    #pragma message("Unknown Platform!")
#endif

#if defined (__MINGW32__) || defined (__MINGW64__)
    //MinGW-w64 编译器
    #define DUILIB_COMPILER_MINGW 1
#endif

/** RichEdit控件绘制优化选项是否开启
*/
#define DUILIB_RICH_EDIT_DRAW_OPT 1

/** 64位操作系统标识
*/
#if defined(_M_X64) || defined(_M_AMD64) || defined(_WIN64) || defined(__x86_64__) 
    #define DUILIB_BIT_64   1
#endif

/** Unicode or Ansi 版本(Ansi版本，文件的编码是UTF-8的，所以字符串编码也是UTF-8的)
*/
#if defined(UNICODE) || defined(_UNICODE)
    #define DUILIB_UNICODE 1
#endif

//未使用的变量宏，避免编译器报警报
#ifndef UNUSED_VARIABLE
    #define UNUSED_VARIABLE(x) ((void)(x))
#endif

#ifndef ASSERT_UNUSED_VARIABLE
    #ifdef _DEBUG
        #define ASSERT_UNUSED_VARIABLE(expr)  ASSERT(expr)
    #else
        #define ASSERT_UNUSED_VARIABLE(expr)  UNUSED_VARIABLE(expr)
    #endif
#endif

#if defined DUILIB_BUILD_FOR_WIN
    #include "duilib_config_windows.h"

    #ifndef ASSERT
        #define ASSERT(expr)  _ASSERTE(expr)
    #endif

#elif defined DUILIB_BUILD_FOR_LINUX
    #include "duilib_config_linux.h"
    #include <cassert>

    #ifdef _DEBUG
        #define ASSERT(expr)  assert(expr)
    #else
        #define ASSERT(expr)  ((void)(0))
    #endif

#elif defined DUILIB_BUILD_FOR_MACOS
    #include "duilib_config_macos.h"
    #include <cassert>

    #ifdef _DEBUG
        #define ASSERT(expr)  assert(expr)
    #else
        #define ASSERT(expr)  ((void)(0))
    #endif

#elif defined DUILIB_BUILD_FOR_FREEBSD
    #include "duilib_config_freebsd.h"
    #include <cassert>

    #ifdef _DEBUG
        #define ASSERT(expr)  assert(expr)
    #else
        #define ASSERT(expr)  ((void)(0))
    #endif

#endif

//字符串类的定义
#include "duilib_string.h"

#endif //DUILIB_CONFIG_H_
