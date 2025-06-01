#ifndef DUILIB_CONFIG_MACOS_H_
#define DUILIB_CONFIG_MACOS_H_

#include <cstddef>
#include <cstdint>
#include <climits>
#include <TargetConditionals.h>

// macOS 平台导出宏定义
#define UILIB_API

// 基础类型定义（保持与Linux一致）
typedef unsigned int        UINT;
typedef unsigned long long  WPARAM;
typedef long long           LPARAM;
typedef long long           LRESULT;

// 字节操作宏（与Windows/Linux兼容）
#define MAKEWORD(a, b)      ((uint16_t)(((uint8_t)(((size_t)(a)) & 0xff)) | ((uint16_t)((uint8_t)(((size_t)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((uint32_t)(((uint16_t)(((size_t)(a)) & 0xffff)) | ((uint32_t)((uint16_t)(((size_t)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((uint16_t)(((size_t)(l)) & 0xffff))
#define HIWORD(l)           ((uint16_t)((((size_t)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((uint8_t)(((size_t)(w)) & 0xff))
#define HIBYTE(w)           ((uint8_t)((((size_t)(w)) >> 8) & 0xff))

// 成功状态码（与Windows兼容）
#define S_OK            ((long)0L)
#define S_FALSE         ((long)1L)

// 回调函数修饰符
#define CALLBACK

// macOS特有的参数解析宏（模拟Windows消息参数解析）
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#define GET_KEYSTATE_WPARAM(wParam)     (LOWORD(wParam))

// macOS系统头文件
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>  // 用于键盘事件处理

// 资源路径定义（macOS使用bundle结构）
#ifdef __OBJC__
    #import <Foundation/Foundation.h>
    #define DUILIB_RESOURCE_PATH [[NSBundle mainBundle] resourcePath].UTF8String
#else
    #define DUILIB_RESOURCE_PATH ""
#endif

#endif // DUILIB_CONFIG_MACOS_H_
