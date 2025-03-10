#ifndef UI_UTILS_BITMAP_HELPER_WINDOWS_H_
#define UI_UTILS_BITMAP_HELPER_WINDOWS_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN
#include "duilib/duilib_config_windows.h"

namespace ui 
{
    namespace BitmapHelper 
    {
        /** 创建一个Render对象，并复制HDC上的内容到Render
        * @return 返回的Render对象，由调用者负责释放资源
        */
        IRender* CreateRenderObject(int srcRenderWidth, int srcRenderHeight, HDC hSrcDc, int srcDcWidth, int srcDcHeight);

        /** 创建一个Render对象，并复制Bitmap上的内容到Render
        * @return 返回的Render对象，由调用者负责释放资源
        */
        IRender* CreateRenderObject(IBitmap* pBitmap);

        /** 创建一个Bitmap对象，并复制HDC上的内容到位图
        * @return 返回的IBitmap对象，由调用者负责释放资源
        */
        IBitmap* CreateBitmapObject(int srcRenderWidth, int srcRenderHeight, HDC hSrcDc, int srcDcWidth, int srcDcHeight);

        /** 从一个GDI Bitmap对象，获取一个HBITMAP句柄
        * @return 返回HBITMAP句柄，外部调用者需要释放资源，以避免资源泄露
        */
        HBITMAP CreateGDIBitmap(IBitmap* pBitmap);

        /** 创建一个设备无关的位图
        *@return 返回位图句柄，由调用方释放位图资源
        */
        HBITMAP CreateGDIBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits);
    }
}

#endif //DUILIB_BUILD_FOR_WIN

#endif //UI_UTILS_BITMAP_HELPER_WINDOWS_H_

