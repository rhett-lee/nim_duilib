#ifndef UI_UTILS_BITMAP_HELPER_WINDOWS_H_
#define UI_UTILS_BITMAP_HELPER_WINDOWS_H_

#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_WIN
#include "duilib/duilib_config_windows.h"

namespace ui 
{
    /** 位图与渲染对象的辅助类（Windows 平台实现）
    *   提供 Render 对象、Bitmap 对象与 GDI 位图（HBITMAP）之间的转换。
    */
    class DUILIB_API BitmapHelper
    {
    public:
        /** 创建一个Render对象，并将HDC上的内容复制并缩放绘制到Render
        * @param [in] srcRenderWidth 目标Render的宽度（像素）
        * @param [in] srcRenderHeight 目标Render的高度（像素）
        * @param [in] hSrcDc 源HDC，不能为nullptr
        * @param [in] srcDcWidth 源HDC的内容宽度（像素），必须大于0
        * @param [in] srcDcHeight 源HDC的内容高度（像素），必须大于0
        * @return 返回的Render对象，由调用者负责释放资源；参数非法或创建失败时返回nullptr
        */
        static IRender* CreateRenderObject(int srcRenderWidth, int srcRenderHeight, HDC hSrcDc, int srcDcWidth, int srcDcHeight);

        /** 创建一个Render对象，并复制Bitmap上的内容到Render
        * @param [in] pBitmap 源Bitmap，不能为nullptr
        * @return 返回的Render对象，由调用者负责释放资源；pBitmap为nullptr或创建失败时返回nullptr
        */
        static IRender* CreateRenderObject(IBitmap* pBitmap);

        /** 创建一个Bitmap对象，并复制HDC上的内容到位图
        * @param [in] srcRenderWidth 目标Bitmap的宽度（像素）
        * @param [in] srcRenderHeight 目标Bitmap的高度（像素）
        * @param [in] hSrcDc 源HDC，不能为nullptr
        * @param [in] srcDcWidth 源HDC的内容宽度（像素），必须大于0
        * @param [in] srcDcHeight 源HDC的内容高度（像素），必须大于0
        * @return 返回的IBitmap对象，由调用者负责释放资源；参数非法或创建失败时返回nullptr
        */
        static IBitmap* CreateBitmapObject(int srcRenderWidth, int srcRenderHeight, HDC hSrcDc, int srcDcWidth, int srcDcHeight);

        /** 从一个GDI Bitmap对象，获取一个HBITMAP句柄
        * @param [in] pBitmap 源Bitmap，不能为nullptr
        * @return 返回HBITMAP句柄，外部调用者需要释放资源，以避免资源泄露；pBitmap为nullptr或创建失败时返回nullptr
        */
        static HBITMAP CreateGDIBitmap(IBitmap* pBitmap);

        /** 创建一个设备无关的位图（32位像素格式）
        * @param [in] nWidth 位图宽度（像素），必须大于0
        * @param [in] nHeight 位图高度（像素），必须大于0
        * @param [in] flipHeight true表示位图从上到下存储（左上角为原点）；false表示从下到上存储（左下角为原点）
        * @param [out] pBits 返回位图像素数据的指针，可为nullptr（此时不返回像素地址）
        * @return 返回位图句柄，由调用方释放位图资源；参数非法或创建失败时返回nullptr
        */
        static HBITMAP CreateGDIBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits);
    };
}

#endif //DUILIB_BUILD_FOR_WIN

#endif //UI_UTILS_BITMAP_HELPER_WINDOWS_H_

