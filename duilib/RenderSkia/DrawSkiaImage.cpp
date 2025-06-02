#include "DrawSkiaImage.h"

#include "duilib/Utils/PerformanceUtil.h"

#pragma warning (push)
#pragma warning (disable: 4505)
    #define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_TRIANGLE
    #include "duilib/third_party/stb_image/stb_image_resize2.h"
#pragma warning (pop)

#include "SkiaHeaderBegin.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkData.h"
#include "SkiaHeaderEnd.h"

//定义DUILIB_HAVE_OPENCV这个宏，表示启用OpenCV，使用OpenCV对图片进行缩放，速度是最快的
//如果启用，需要做如下处理：
//1. 在duilib的工程里，添加opencv库的安装路径根目录到VC工程的include目录中
//2. 在可执行程序的工程里，需要在链接时，添加如下lib：opencv_core4110.lib;opencv_imgproc4110.lib（以上是opencv 4.11版本的库，其他版本库的lib名称会不一样）

//#define DUILIB_HAVE_OPENCV 1

#ifdef DUILIB_HAVE_OPENCV
    #pragma warning (push)
    #pragma warning (disable: 4127)
        #include "opencv2/opencv.hpp"
    #pragma warning (pop)
#endif

//#ifdef DUILIB_HAVE_OPENCV
//    #ifdef _DEBUG
//        #pragma comment (lib, "opencv_core4110d.lib")
//        #pragma comment (lib, "opencv_imgproc4110d.lib")
//    #else
//        #pragma comment (lib, "opencv_core4110.lib")
//        #pragma comment (lib, "opencv_imgproc4110.lib")
//    #endif
//#endif

namespace ui 
{

#ifdef DUILIB_HAVE_OPENCV
//OpenCV

// 直接内存映射（零拷贝）
static cv::Mat SkImageToCvMat_Opt(const sk_sp<SkImage>& skImage)
{
    // 获取像素信息
    SkPixmap pixmap;
    if (!skImage->peekPixels(&pixmap)) {
        return cv::Mat();
    }

    // 创建共享内存的Mat
    cv::Mat viewMat(pixmap.height(), pixmap.width(), CV_8UC4, pixmap.writable_addr(), pixmap.rowBytes());

    // 通道重排（RGBA -> BGRA）
    if (skImage->colorType() == SkColorType::kRGBA_8888_SkColorType) {
        cv::mixChannels(viewMat, viewMat, { 2,0, 1,1, 0,2, 3,3 }); // 原地交换R与B通道(OpenCV默认处理的是BGRA格式)
    }    
    return viewMat; // 注意：Mat的生命周期必须短于SkImage
}

// 优化版缩放函数
static sk_sp<SkImage> SkiaResizeWithOpenCV_Opt(const sk_sp<SkImage>& srcImage, int newWidth, int newHeight)
{
    // 获取源图像内存视图
    cv::Mat srcMat = SkImageToCvMat_Opt(srcImage);
    if (srcMat.empty()) {
        return nullptr;
    }

    // 使用预分配内存执行缩放
    SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(newWidth, newHeight);
    sk_sp<SkData> dstData = SkData::MakeUninitialized(dstInfo.computeMinByteSize());
    cv::Mat dstMat(newHeight, newWidth, CV_8UC4, dstData->writable_data(), dstInfo.minRowBytes());

    // 执行缩放并转换颜色空间
    cv::resize(srcMat, dstMat, dstMat.size(), 0, 0, cv::INTER_LINEAR);

    // 通道重排（BGRA->RGBA）
    if (dstInfo.colorType() == SkColorType::kRGBA_8888_SkColorType) {
        cv::mixChannels(dstMat, dstMat, { 2,0, 1,1, 0,2, 3,3 });
    }

    // 创建共享内存的SkImage
    return SkImages::RasterFromData(dstInfo, dstData, dstInfo.minRowBytes());
}

/** 对Skia的图片进行resize操作，以适配绘制目标大小，避免绘制时缩放导致速度降低(OpenCV实现)
*/
static bool ResizeSkiaImageByOpenCV(const sk_sp<SkImage>& skImage, const UiRect& rcSrc, const UiRect& rcDest, sk_sp<SkImage>& skNewImage)
{
    if ((skImage == nullptr) || (rcSrc.left != 0) || (rcSrc.top != 0) || (rcSrc.Width() < 10) || (rcSrc.Height() < 10) || (rcDest.Width() < 10) || (rcDest.Height() < 10)) {
        return false;
    }
    if ((rcSrc.Width() == rcDest.Width()) && (rcSrc.Height() == rcDest.Height())) {
        //区域相同，无缩放
        return false;
    }
    if ((skImage->width() != rcSrc.Width()) || (skImage->height() != rcSrc.Height())) {
        //不是整个图片绘制
        return false;
    }
    SkPixmap srcPixmap;
    if (!skImage->peekPixels(&srcPixmap)) {
        return false;
    }
    if ((srcPixmap.width() != rcSrc.Width()) || (srcPixmap.height() != rcSrc.Height())) {
        //有错误
        return false;
    }

    PerformanceStat statPerformance(_T("Render_Skia::DrawSkiaImage::ResizeSkiaImageByOpenCV"));
    skNewImage = SkiaResizeWithOpenCV_Opt(skImage, rcDest.Width(), rcDest.Height());
    return skNewImage != nullptr;
}
#else
/** 对Skia的图片进行resize操作，以适配绘制目标大小，避免绘制时缩放导致速度降低(stb_image实现)
*/
static bool ResizeSkiaImageByStbImage(const sk_sp<SkImage>& skImage, const UiRect& rcSrc, const UiRect& rcDest, sk_sp<SkImage>& skNewImage)
{
    if ((skImage == nullptr) || (rcSrc.left != 0) || (rcSrc.top != 0) || (rcSrc.Width() < 10) || (rcSrc.Height() < 10) || (rcDest.Width() < 10) || (rcDest.Height() < 10)) {
        return false;
    }
    if ((rcSrc.Width() == rcDest.Width()) && (rcSrc.Height() == rcDest.Height())) {
        //区域相同，无缩放
        return false;
    }
    if ((skImage->width() != rcSrc.Width()) || (skImage->height() != rcSrc.Height())) {
        //不是整个图片绘制
        return false;
    }
    SkPixmap srcPixmap;
    if (!skImage->peekPixels(&srcPixmap)) {
        return false;
    }
    if ((srcPixmap.width() != rcSrc.Width()) || (srcPixmap.height() != rcSrc.Height())) {
        //有错误
        return false;
    }

    PerformanceStat statPerformance(_T("Render_Skia::DrawSkiaImage::ResizeSkiaImageByStbImage"));

    sk_sp<SkData> skData = SkData::MakeUninitialized(rcDest.Height() * rcDest.Width() * sizeof(uint32_t));
    const unsigned char* input_pixels = (const unsigned char*)srcPixmap.addr();
    int input_w = srcPixmap.width();
    int input_h = srcPixmap.height();
    int input_stride_in_bytes = 0;
    unsigned char* output_pixels = (unsigned char*)skData->writable_data();
    int output_w = rcDest.Width();
    int output_h = rcDest.Height();
    int output_stride_in_bytes = 0;
    stbir_pixel_layout num_channels = STBIR_RGBA;
    unsigned char* result = stbir_resize_uint8_linear(input_pixels, input_w, input_h, input_stride_in_bytes,
                                                      output_pixels, output_w, output_h, output_stride_in_bytes,
                                                      num_channels);
    if (result != nullptr) {
        skNewImage = SkImages::RasterFromData(SkImageInfo::Make(SkISize::Make(output_w, output_h), skImage->imageInfo().colorInfo()), skData, rcDest.Width() * sizeof(uint32_t));
        if ((skNewImage->height() == rcDest.Height()) && (skNewImage->width() == rcDest.Width())) {
            return (skNewImage != nullptr);
        }
    }
    return false;
}
#endif //end of OpenCV


void DrawSkiaImage::DrawImage(SkCanvas* pSkCanvas,
                              const UiRect& rcDest,
                              const SkPoint& skPointOrg,
                              const sk_sp<SkImage>& skSrcImage,
                              const UiRect& rcSrc,
                              const SkPaint& skPaint)
{
    if ((pSkCanvas == nullptr) || (skSrcImage == nullptr)) {
        return;
    }
    SkIRect rcSkDestI = { rcDest.left, rcDest.top, rcDest.right, rcDest.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(skPointOrg);

    SkIRect rcSkSrcI = { rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom };
    SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

#ifdef DUILIB_HAVE_OPENCV
    sk_sp<SkImage> skNewImage;
    if (ResizeSkiaImageByOpenCV(skSrcImage, rcSrc, rcDest, skNewImage)) {
        PerformanceStat statPerformance(_T("Render_Skia::DrawSkiaImage::DrawImage drawImageRect(OpenCV)"));
        rcSkSrc.fRight = rcSkSrc.fLeft + skNewImage->width();
        rcSkSrc.fBottom = rcSkSrc.fTop + skNewImage->height();
        pSkCanvas->drawImageRect(skNewImage, rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kStrict_SrcRectConstraint);
    }
#else
    sk_sp<SkImage> skNewImage;
    if (ResizeSkiaImageByStbImage(skSrcImage, rcSrc, rcDest, skNewImage)) {
        PerformanceStat statPerformance(_T("Render_Skia::DrawSkiaImage::DrawImage drawImageRect(StbImage)"));
        rcSkSrc.fRight = rcSkSrc.fLeft + skNewImage->width();
        rcSkSrc.fBottom = rcSkSrc.fTop + skNewImage->height();
        pSkCanvas->drawImageRect(skNewImage, rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kStrict_SrcRectConstraint);
    }
#endif
    else {
        PerformanceStat statPerformance(_T("Render_Skia::DrawSkiaImage::DrawImage drawImageRect(Skia Only)"));
        pSkCanvas->drawImageRect(skSrcImage, rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kStrict_SrcRectConstraint);
    }
}

} // namespace ui

