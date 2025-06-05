#ifndef UI_CORE_SDL_MACOS_H_
#define UI_CORE_SDL_MACOS_H_

#if defined(__APPLE__) && defined(__MACH__)

struct SDL_Window;

namespace ui
{
/** MAC OS下，获取SDL窗口对应的NSView*接口
*/
void* GetSDLWindowContentView(SDL_Window* sdlWindow);

}

#endif

#endif //UI_CORE_SDL_MACOS_H_
