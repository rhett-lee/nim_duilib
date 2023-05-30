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

#ifdef UILIB_IMPL_WINSDK

        /** 创建一个Render对象，并复制HDC上的内容到Render
        * @return 返回的Render对象，由调用者负责释放资源
        */
        IRender* CreateRenderObject(int srcRenderWidth, int srcRenderHeight, HDC hSrcDc, int srcDcWidth, int srcDcHeight);

        /** 创建一个Render对象，并复制Bitmap上的内容到Render
        * @return 返回的Render对象，由调用者负责释放资源
        */
        IRender* CreateRenderObject(IBitmap* pBitmp);

        /** 创建一个Bitmap对象，并复制HDC上的内容到位图
        * @return 返回的IBitmap对象，由调用者负责释放资源
        */
        IBitmap* CreateBitmapObject(int srcRenderWidth, int srcRenderHeight, HDC hSrcDc, int srcDcWidth, int srcDcHeight);

#endif
    }
}

#endif //UI_RENDER_BITMAP_HELPER_H_

