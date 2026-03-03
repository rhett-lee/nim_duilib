#ifndef UI_RENDER_SKIA_WINDOW_RGN_WINDOWS_H_
#define UI_RENDER_SKIA_WINDOW_RGN_WINDOWS_H_

#include "duilib/Core/UiTypes.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui 
{
class UILIB_API WindowRgn
{
public:
    /** 设置窗口的形状为圆角矩形
    * @param [in] hWnd 关联的窗口句柄
    * @param [in] rcWnd 需要设置RGN的区域，坐标为屏幕坐标
    * @param [in] rx 圆角的宽度，其值不能为0
    * @param [in] ry 圆角的高度，其值不能为0
    * @param [in] bRedraw 是否重绘
    */
    static bool SetWindowRoundRectRgn(HWND hWnd, const UiRect& rcWnd, float rx, float ry, bool bRedraw);

    /** 设置窗口的形状为直角矩形
    * @param [in] hWnd 关联的窗口句柄
    * @param [in] rcWnd 需要设置RGN的区域，坐标为屏幕坐标
    * @param [in] bRedraw 是否重绘
    */
    static bool SetWindowRectRgn(HWND hWnd, const UiRect& rcWnd, bool bRedraw);

    /** 清除窗口的形状设置, 恢复为系统默认形状
    * @param [in] hWnd 关联的窗口句柄
    * @param [in] bRedraw 是否重绘
    */
    static void ClearWindowRgn(HWND hWnd, bool bRedraw);
};

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN

#endif //UI_RENDER_SKIA_WINDOW_RGN_WINDOWS_H_
