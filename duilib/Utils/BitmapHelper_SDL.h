#ifndef UI_UTILS_BITMAP_HELPER_SDL_H_
#define UI_UTILS_BITMAP_HELPER_SDL_H_

#include "duilib/Render/IRender.h"

#if defined (DUILIB_BUILD_FOR_SDL) && !defined (DUILIB_BUILD_FOR_WIN)

namespace ui 
{
    namespace BitmapHelper 
    {
        /** 创建一个Render对象，并复制Bitmap上的内容到Render
        * @return 返回的Render对象，由调用者负责释放资源
        */
        IRender* CreateRenderObject(IBitmap* pBitmap);
    }
}

#endif //DUILIB_BUILD_FOR_SDL

#endif //UI_UTILS_BITMAP_HELPER_SDL_H_

