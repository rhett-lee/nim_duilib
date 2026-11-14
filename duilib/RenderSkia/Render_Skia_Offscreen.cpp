#include "Render_Skia_Offscreen.h"
#include "SkOffscreenGLContext.h"

#include "SkiaHeaderBegin.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/GpuTypes.h"

#include "SkiaHeaderEnd.h"

namespace ui {

namespace {

/** 将AlphaType转换为Skia的Alpha类型
*/
SkAlphaType ToSkAlphaType(AlphaType alphaType)
{
    switch (alphaType) {
    case AlphaType::kUnknown:   return SkAlphaType::kUnknown_SkAlphaType;
    case AlphaType::kOpaque:    return SkAlphaType::kOpaque_SkAlphaType;
    case AlphaType::kPremul:    return SkAlphaType::kPremul_SkAlphaType;
    case AlphaType::kUnpremul:  return SkAlphaType::kUnpremul_SkAlphaType;
    default:
        break;
    }
    return SkAlphaType::kPremul_SkAlphaType;
}

} // namespace

Render_Skia_Offscreen::Render_Skia_Offscreen(uint32_t nWidth, uint32_t nHeight,
                                             RenderBackendType backendType,
                                             AlphaType alphaType):
    m_backendType(backendType),
    m_alphaType(alphaType)
{
    if (m_backendType != RenderBackendType::kRaster_BackendType) {
        //GPU后端：获取当前线程共享的图形上下文（增加引用计数，防止上下文提前释放）
        m_pGrContext = sk_ref_sp(OffscreenGLContext::GetDirectContext());
    }
    if ((nWidth > 0) || (nHeight > 0)) {
        Resize((int32_t)nWidth, (int32_t)nHeight);
    }
}

Render_Skia_Offscreen::~Render_Skia_Offscreen() = default;

RenderBackendType Render_Skia_Offscreen::GetRenderBackendType() const
{
    return m_backendType;
}

bool Render_Skia_Offscreen::CreateSurface(int32_t width, int32_t height)
{
    ASSERT((width >= 0) && (height >= 0));
    if ((width < 0) || (height < 0)) {
        return false;
    }
    if ((width > kMaxOffscreenSize) || (height > kMaxOffscreenSize)) {
        return false;
    }
    if (((width == 0) && (height != 0)) || ((width != 0) && (height == 0))) {
        //宽度和高度必须同时为0（表示未创建画布）或同时大于0
        return false;
    }
    m_skSurface.reset();
    if ((width == 0) || (height == 0)) {
        //宽高为0时，不创建画布（后续可通过Resize创建）
        return true;
    }

    SkImageInfo imageInfo = SkImageInfo::Make(width, height, kN32_SkColorType, ToSkAlphaType(m_alphaType));
    if (m_backendType == RenderBackendType::kRaster_BackendType) {
        m_skSurface = SkSurfaces::Raster(imageInfo);
    }
    else {
        //GPU后端：创建独立的RenderTarget画布（不使用共享上下文的后备缓冲）
        if (!m_pGrContext) {
            return false;
        }
        m_skSurface = SkSurfaces::RenderTarget(m_pGrContext.get(), skgpu::Budgeted::kNo, imageInfo);
    }
    return m_skSurface != nullptr;
}

bool Render_Skia_Offscreen::Resize(int32_t width, int32_t height)
{
    if ((width < 0) || (height < 0)) {
        return false;
    }
    if ((width > kMaxOffscreenSize) || (height > kMaxOffscreenSize)) {
        return false;
    }
    if (((width == 0) && (height != 0)) || ((width != 0) && (height == 0))) {
        //宽度和高度必须同时为0（表示不创建画布）或同时大于0
        return false;
    }
    if ((width == GetWidth()) && (height == GetHeight())) {
        //尺寸未发生变化，保留原有内容
        return true;
    }
    return CreateSurface(width, height);
}

int32_t Render_Skia_Offscreen::GetWidth() const
{
    SkSurface* pSkSurface = GetSkSurface();
    return (pSkSurface != nullptr) ? pSkSurface->width() : 0;
}

int32_t Render_Skia_Offscreen::GetHeight() const
{
    SkSurface* pSkSurface = GetSkSurface();
    return (pSkSurface != nullptr) ? pSkSurface->height() : 0;
}

std::unique_ptr<IRender> Render_Skia_Offscreen::Clone() const
{
    const int32_t nWidth = GetWidth();
    const int32_t nHeight = GetHeight();
    auto pClone = std::make_unique<Render_Skia_Offscreen>(0, 0, m_backendType, m_alphaType);
    pClone->Resize(nWidth, nHeight);
    pClone->SetRenderDpi(GetRenderDpi());
    pClone->SetRenderFactory(GetRenderFactory());
    pClone->SetImageSamplingQuality(GetImageSamplingQuality());
    if (IsValid() && pClone->IsValid()) {
        pClone->BitBlt(0, 0, nWidth, nHeight, this, 0, 0, RopMode::kSrcCopy);
    }
    return pClone;
}

SkSurface* Render_Skia_Offscreen::GetSkSurface() const
{
    return m_skSurface.get();
}

SkCanvas* Render_Skia_Offscreen::GetSkCanvas() const
{
    SkSurface* pSkSurface = GetSkSurface();
    return (pSkSurface != nullptr) ? pSkSurface->getCanvas() : nullptr;
}

bool Render_Skia_Offscreen::IsValid() const
{
    return m_skSurface != nullptr;
}

} // namespace ui
