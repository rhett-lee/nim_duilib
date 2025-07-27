#ifndef UI_CORE_SDL_MACOS_H_
#define UI_CORE_SDL_MACOS_H_

#if defined(__APPLE__) && defined(__MACH__)

struct SDL_Window;

namespace ui
{
/** MAC OS下，获取SDL窗口对应的NSView*接口
*/
void* GetSDLWindowContentView(SDL_Window* sdlWindow);

// 封装MacOS的SetFocus函数，功能类似于Windows的SetFocus(HWND)
// 参数: window - 要设置焦点的窗口指针
// 返回值: 成功返回YES，失败返回NO
bool SetFocus_MacOS(void* pNSWindow);

}

#endif

#endif //UI_CORE_SDL_MACOS_H_
