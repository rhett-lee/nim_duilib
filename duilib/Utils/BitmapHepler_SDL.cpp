#include "BitmapHelper_SDL.h"
#include "duilib/Core/GlobalManager.h"

#if defined (DUILIB_BUILD_FOR_SDL) && !defined (DUILIB_BUILD_FOR_WIN)

namespace ui {

namespace BitmapHelper {

    IRender* CreateRenderObject(IBitmap* pBitmap)
    {
        ASSERT(pBitmap != nullptr);
        if (pBitmap == nullptr) {
            return nullptr;
        }
        IRender* pRender = nullptr;
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            pRender = pRenderFactory->CreateRender(nullptr);
        }
        ASSERT(pRender != nullptr);
        if (pRender == nullptr) {
            return nullptr;
        }
        if (pRender->Resize((int)pBitmap->GetWidth(), (int)pBitmap->GetHeight())) {
            UiRect rect = { 0, 0, (int)pBitmap->GetWidth(), (int)pBitmap->GetHeight() };
            pRender->DrawImage(rect, pBitmap, rect, UiRect(), rect, UiRect());
        }        
        return pRender;
    }
    
} //namespace BitmapHelper

} //namespace ui 

#endif //DUILIB_BUILD_FOR_SDL
