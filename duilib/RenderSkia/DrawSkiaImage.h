#ifndef UI_RENDER_SKIA_DRAW_SKIA_IMAGE_H_
#define UI_RENDER_SKIA_DRAW_SKIA_IMAGE_H_

#include "duilib/Core/UiRect.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
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
    */
    static void DrawImage(SkCanvas* pSkCanvas,
                          const UiRect& rcDest,
                          const SkPoint& skPointOrg,
                          const sk_sp<SkImage>& skSrcImage,
                          const UiRect& rcSrc,
                          const SkPaint& skPaint);

};

} // namespace ui

#endif // UI_RENDER_SKIA_DRAW_SKIA_IMAGE_H_
