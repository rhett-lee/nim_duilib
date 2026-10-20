#ifndef DUILIB_CONFIG_H_
#define DUILIB_CONFIG_H_

/** 平台检测：Windows、Linux、macOS、FreeBSD
*/
#if defined (_WIN32) || defined (_WIN64)
    //Windows平台
    #define DUILIB_BUILD_FOR_WIN    1
#elif defined(linux) || defined(__linux) || defined(__linux__)
    //Linux平台
    #define DUILIB_BUILD_FOR_LINUX  1
#elif defined(__APPLE__) && defined(__MACH__)
    //macOS平台
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #define DUILIB_BUILD_FOR_MACOS  1
    #else
        #error "Unknown Platform!"
    #endif
#elif defined(__FreeBSD__)
    //FreeBSD平台
    #define DUILIB_BUILD_FOR_FREEBSD    1
#else
    //不支持的系统
    #error "Unknown Platform!"
#endif


//不同平台的宏定义
#if defined DUILIB_BUILD_FOR_WIN
    //是否使用SDL的窗口和鼠标键盘事件（目前只支持SDL3）
    #if (DUILIB_SDL)
        //可以将msvc\PropertySheets\SDLSettings.props文件中的SDLEnabled改为1，以开启SDL功能
        #define DUILIB_BUILD_FOR_SDL    1
    #endif

    //WebView2控件功能开关
    #if (DUILIB_WEBVIEW2)
        //可以将msvc\PropertySheets\WebView2Settings.props文件中的WebView2Enabled改为1，以开启WebView2功能
        #define DUILIB_BUILD_FOR_WEBVIEW2   1
    #endif

    //CEF控件功能开关
    #ifndef DUILIB_CEF
        //默认开启
        #define DUILIB_BUILD_FOR_CEF        1
    #else
        #if (DUILIB_CEF)
            //可以将msvc\PropertySheets\CEFSettings.props文件中的LibCefEnabled改为1，以开启CEF功能
            #define DUILIB_BUILD_FOR_CEF    1
        #endif
    #endif

    /** RichEdit控件绘制优化选项是否开启（Windows版本的RichEdit控件）
    */
    #define DUILIB_RICH_EDIT_DRAW_OPT 1

    /** 是否支持libjpeg-turbo库来解码JPEG格式
    */
    #if (DUILIB_JPEG_TURBO)
        #define DUILIB_IMAGE_SUPPORT_JPEG_TURBO 1
    #endif

    /** 是否支持libpag库来解码PAG格式
    */
    #if (DUILIB_LIB_PAG)
        #define DUILIB_IMAGE_SUPPORT_LIB_PAG 1
    #endif
#else
    //非Windows平台
    //是否使用SDL的窗口和鼠标键盘事件（目前只支持SDL3）
    #define DUILIB_BUILD_FOR_SDL    1
    //定义是否支持CEF
    //注意：Chromium 官方不提供 FreeBSD 构建，FreeBSD 上不支持 CEF 控件，
    //      故此处不定义 DUILIB_BUILD_FOR_CEF。CMakeLists.txt 已同步排除
    //      CEFControl 全部源码与 include 路径，build/freebsd_build.sh 亦已
    //      注释掉 cef / CefBrowser 示例程序。
    #if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_MACOS)
        #define DUILIB_BUILD_FOR_CEF    1
    #endif
#endif

#if defined (__MINGW32__) || defined (__MINGW64__)
    //MinGW-w64 编译器
    #define DUILIB_COMPILER_MINGW 1
#endif

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
#else
    #error "Unknown Platform!"
#endif

//字符串类的定义
#include "duilib_string.h"

#endif //DUILIB_CONFIG_H_
