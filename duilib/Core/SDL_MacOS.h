#ifndef UI_CORE_SDL_MACOS_H_
#define UI_CORE_SDL_MACOS_H_

/** MAC OS下，获取SDL窗口对应的NSView*接口
*/
struct SDL_Window;
void* GetSDLWindowContentView(SDL_Window* sdlWindow);

#endif //UI_CORE_SDL_MACOS_H_
