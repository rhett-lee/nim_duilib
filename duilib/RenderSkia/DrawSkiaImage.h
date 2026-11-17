#ifndef UI_RENDER_SKIA_DRAW_SKIA_IMAGE_H_
#define UI_RENDER_SKIA_DRAW_SKIA_IMAGE_H_

#include "duilib/Core/UiRect.h"

#include <cstddef>

#include "SkiaHeaderBegin.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSamplingOptions.h"
#include "SkiaHeaderEnd.h"

namespace ui 
{

/** 调用Canvas绘制Skia的图片
*/
class DrawSkiaImage
{
public:

    /** 调用Canvas绘制Skia的图片，如果图片有缩放，会进行优化（因Skia自身的图片缩放速度较慢）
    * @param [in] pSkCanvas canvas接口
    * @param [in] rcDest 图片绘制的目标矩形区域
    * @param [in] skPointOrg 目标区域在canvas中的视图偏移坐标（左上角）
    * @param [in] skSrcImage 需要绘制的图片接口
    * @param [in] rcSrc 需要绘制的图片内容在源图片中的矩形区域
    * @param [in] skPaint paint属性
    * @param [in] skSampling 图片缩放时的采样参数
    */
    static void DrawImage(SkCanvas* pSkCanvas,
                          const UiRect& rcDest,
                          const SkPoint& skPointOrg,
                          const sk_sp<SkImage>& skSrcImage,
                          const UiRect& rcSrc,
                          const SkPaint& skPaint,
                          const SkSamplingOptions& skSampling);

public:
    /** 清空图片缩放结果缓存（同时清零命中计数，用于测试与内存回收）
    */
    static void ClearImageScaleCache();

    /** 获取图片缩放结果缓存的条目数
    */
    static size_t GetImageScaleCacheCount();

    /** 获取图片缩放结果缓存的命中次数（累计值）
    */
    static size_t GetImageScaleCacheHitCount();

    /** 启用/禁用图片缩放结果缓存（禁用时会清空已有缓存，用于A/B性能对比与问题定位）
    */
    static void EnableImageScaleCache(bool bEnable);

};

} // namespace ui

#endif // UI_RENDER_SKIA_DRAW_SKIA_IMAGE_H_
