#ifndef DUILIB_CONFIG_FREEBSD_H_
#define DUILIB_CONFIG_FREEBSD_H_

#include <cstddef>
#include <cstdint>
#include <climits>

#define UILIB_API

typedef unsigned int        UINT;
typedef unsigned long long  WPARAM;
typedef long long           LPARAM;
typedef long long           LRESULT;

#define MAKEWORD(a, b)      ((uint16_t)(((uint8_t)(((size_t)(a)) & 0xff)) | ((uint16_t)((uint8_t)(((size_t)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((uint32_t)(((uint16_t)(((size_t)(a)) & 0xffff)) | ((uint32_t)((uint16_t)(((size_t)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((uint16_t)(((size_t)(l)) & 0xffff))
#define HIWORD(l)           ((uint16_t)((((size_t)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((uint8_t)(((size_t)(w)) & 0xff))
#define HIBYTE(w)           ((uint8_t)((((size_t)(w)) >> 8) & 0xff))
//
//#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
//#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
//
//#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
//#define GET_KEYSTATE_WPARAM(wParam)     (LOWORD(wParam))

//
// Success codes
//
#define S_OK            ((long)0L)
#define S_FALSE         ((long)1L)

//
// C函数的调用方式
//
#define CALLBACK 

#endif //DUILIB_CONFIG_FREEBSD_H_
