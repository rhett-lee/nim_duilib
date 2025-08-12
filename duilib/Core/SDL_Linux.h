#ifndef UI_CORE_SDL_LINUX_H_
#define UI_CORE_SDL_LINUX_H_

#include "duilib/duilib_defs.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)

namespace ui
{
/** 封装Linux的SetFocus函数，功能类似于Windows的SetFocus(HWND)
*/
bool SetFocus_Linux(uint64_t x11WindowNumber);

}

#endif //DUILIB_BUILD_FOR_LINUX

#endif //UI_CORE_SDL_LINUX_H_
