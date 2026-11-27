#ifndef UI_UTILS_BITMAP_HELPER_SDL_H_
#define UI_UTILS_BITMAP_HELPER_SDL_H_

#include "duilib/Render/IRender.h"

#if defined (DUILIB_BUILD_FOR_SDL) && !defined (DUILIB_BUILD_FOR_WIN)

namespace ui 
{
    /** 位图与渲染对象的辅助功能（SDL 平台实现）
    */
    namespace BitmapHelper 
    {
        /** 创建一个Render对象，并复制Bitmap上的内容到Render
        * @param [in] pBitmap 源Bitmap，不能为nullptr
        * @return 返回的Render对象，由调用者负责释放资源；创建失败时返回nullptr
        */
        IRender* CreateRenderObject(IBitmap* pBitmap);
    }
}

#endif //DUILIB_BUILD_FOR_SDL

#endif //UI_UTILS_BITMAP_HELPER_SDL_H_

