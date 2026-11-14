#include "Render_Skia_Bitmap.h"
#include "Bitmap_Skia.h"
#include "Render_Skia_Offscreen.h"

#include "SkiaHeaderBegin.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"

#include "SkiaHeaderEnd.h"

namespace ui {

Render_Skia_Bitmap::Render_Skia_Bitmap(IBitmap* pBitmap):
    m_pBitmap(pBitmap),
    m_alphaType(AlphaType::kPremul)
{
    if ((m_pBitmap == nullptr) || !m_pBitmap->IsValid()) {
        return;
    }
    m_alphaType = m_pBitmap->GetAlphaType();

    Bitmap_Skia* pBitmapSkia = dynamic_cast<Bitmap_Skia*>(m_pBitmap);
    if (pBitmapSkia == nullptr) {
        return;
    }
    SkPixmap pixmap;
    if (!pBitmapSkia->GetSkBitmap().peekPixels(&pixmap)) {
        return;
    }
    //零拷贝：直接包装位图的像素内存，绘制结果写入位图
    m_skSurface = SkSurfaces::WrapPixels(pixmap);
}

Render_Skia_Bitmap::~Render_Skia_Bitmap() = default;

RenderBackendType Render_Skia_Bitmap::GetRenderBackendType() const
{
    return RenderBackendType::kRaster_BackendType;
}

bool Render_Skia_Bitmap::Resize(int32_t width, int32_t height)
{
    //目标位图在创建时已确定尺寸，不支持调整；尺寸一致时视为成功
    return (width == GetWidth()) && (height == GetHeight());
}

int32_t Render_Skia_Bitmap::GetWidth() const
{
    return (m_skSurface != nullptr) ? m_skSurface->width() : 0;
}

int32_t Render_Skia_Bitmap::GetHeight() const
{
    return (m_skSurface != nullptr) ? m_skSurface->height() : 0;
}

std::unique_ptr<IRender> Render_Skia_Bitmap::Clone() const
{
    const int32_t nWidth = GetWidth();
    const int32_t nHeight = GetHeight();
    auto pClone = std::make_unique<Render_Skia_Offscreen>((uint32_t)nWidth, (uint32_t)nHeight,
                                                          RenderBackendType::kRaster_BackendType,
                                                          m_alphaType);
    pClone->SetRenderDpi(GetRenderDpi());
    pClone->SetRenderFactory(GetRenderFactory());
    pClone->SetImageSamplingQuality(GetImageSamplingQuality());
    if (IsValid() && pClone->IsValid()) {
        pClone->BitBlt(0, 0, nWidth, nHeight, this, 0, 0, RopMode::kSrcCopy);
    }
    return pClone;
}

SkSurface* Render_Skia_Bitmap::GetSkSurface() const
{
    return m_skSurface.get();
}

SkCanvas* Render_Skia_Bitmap::GetSkCanvas() const
{
    return (m_skSurface != nullptr) ? m_skSurface->getCanvas() : nullptr;
}

bool Render_Skia_Bitmap::IsValid() const
{
    return m_skSurface != nullptr;
}

} // namespace ui
