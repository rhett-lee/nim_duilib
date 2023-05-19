#ifndef UI_RENDER_BITMAP_HELPER_H_
#define UI_RENDER_BITMAP_HELPER_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace ui 
{
    namespace BitmapHelper 
    {

        /** 将图像以中心为圆点，旋转角度dAngle(0 - 360度)
        * @param [in] pBitmap 需要旋转的位图接口
        * @param [in] dAngle 需要旋转的角度(0 - 360度)
        * @return 返回新生成的位图接口，资源由调用方负责释放
        */
        IBitmap* RotateBitmapAroundCenter(const IBitmap* pBitmap, double dAngle);
    }
}

#endif //UI_RENDER_BITMAP_HELPER_H_

