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

        //将图像以中心为圆点，旋转角度dAngle(0 - 360度)
        HBITMAP RotateBitmapAroundCenter(HBITMAP hBitmap, double dAngle);

        //获取Bitmap的宽和高
        bool GetBitmapWidthHeight(HBITMAP hBitmap, int& imageWidth, int& imageHeight);
    }

}
