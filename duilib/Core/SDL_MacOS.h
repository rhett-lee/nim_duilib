#ifndef UI_CORE_SDL_MACOS_H_
#define UI_CORE_SDL_MACOS_H_

#if defined(__APPLE__) && defined(__MACH__)

#include "NativeWindowShadow.h"

struct SDL_Window;

namespace ui
{
/** MAC OS下，获取SDL窗口对应的NSView*接口
*/
void* GetSDLWindowContentView(SDL_Window* sdlWindow);

/** 封装MacOS的SetFocus函数，功能类似于Windows的SetFocus(HWND)
@param [in] pNSWindow 窗口指针
*/
bool SetFocus_MacOS(void* pNSWindow);

/** 设置系统的窗口阴影属性，以支持系统阴影
@param [in] pNSWindow 窗口指针
@param [in] nativeShadowType 系统阴影类型
@param [in] bUseSystemCaption 是否使用系统标题栏（使用系统标题栏时，不能隐藏标题栏）
*/
bool ModifyNsWindowShadowType(void* pNSWindow, NativeWindowShadowType nativeShadowType, bool bUseSystemCaption);

/** 全屏退出后延迟恢复窗口阴影（确保 SDL 完成全屏退出动作后再设置圆角）
@param [in] pNSWindow 窗口指针
@param [in] nativeShadowType 系统阴影类型
@param [in] bUseSystemCaption 是否使用系统标题栏（使用系统标题栏时，不能隐藏标题栏）
*/
void RestoreWindowShadowAfterFullscreen(void* pNSWindow, NativeWindowShadowType nativeShadowType, bool bUseSystemCaption);

}

#endif

#endif //UI_CORE_SDL_MACOS_H_
