#pragma once

#include "duilib/duilib_defs.h"

namespace Gdiplus {
    class Bitmap;
}

namespace ui 
{
    namespace GdiHelper 
    {

        Gdiplus::Bitmap* CreateBitmapFromHBITMAP(HBITMAP hBitmap);

    }

}
