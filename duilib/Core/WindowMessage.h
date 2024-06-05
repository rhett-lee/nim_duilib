#ifndef UI_CORE_WINDOW_MESSAGE_H_
#define UI_CORE_WINDOW_MESSAGE_H_

#include "duilib/duilib_defs.h"

namespace ui {
    
#ifndef DUILIB_PLATFORM_WIN
    #define MAKEWORD(a, b)      ((uint16_t)(((uint8_t)(((size_t)(a)) & 0xff)) | ((uint16_t)((uint8_t)(((size_t)(b)) & 0xff))) << 8))
    #define MAKELONG(a, b)      ((uint32_t)(((uint16_t)(((size_t)(a)) & 0xffff)) | ((uint32_t)((uint16_t)(((size_t)(b)) & 0xffff))) << 16))
    #define LOWORD(l)           ((uint16_t)(((size_t)(l)) & 0xffff))
    #define HIWORD(l)           ((uint16_t)((((size_t)(l)) >> 16) & 0xffff))
    #define LOBYTE(w)           ((uint8_t)(((size_t)(w)) & 0xff))
    #define HIBYTE(w)           ((uint8_t)((((size_t)(w)) >> 8) & 0xff))
#endif

///////////////////////////////////////////////////////
// windowsx.h
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

/* Value for rolling one detent */
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#define GET_KEYSTATE_WPARAM(wParam)     (LOWORD(wParam))

/** 窗口消息定义，只定义部分使用到的消息(和Windows系统定义一致，WinUser.h)
*/
enum WindowMessage{
    //仅供窗口内部使用的消息

    
    //仅供内部控件使用的消息


    //给应用层使用的消息
};


} // namespace ui

#endif // UI_CORE_WINDOW_MESSAGE_H_

