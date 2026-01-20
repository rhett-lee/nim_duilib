#include "Render_Skia_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "SkRasterWindowContext_Windows.h"
#include "SkGLWindowContext_Windows.h"

#include "SkiaHeaderBegin.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRegion.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathMeasure.h"

#include "SkiaHeaderEnd.h"

namespace ui {

/** 创建Raster实现的WindowContext
* @param [in] hWnd 关联的窗口句柄，可以为nullptr
* @param [in] params 显示相关的参数
*/
std::unique_ptr<skwindow::WindowContext> MakeRasterForWin(HWND hWnd, std::unique_ptr<const skwindow::DisplayParams> params)
{
    std::unique_ptr<skwindow::WindowContext> ctx(new SkRasterWindowContext_Windows(hWnd, std::move(params)));
    return ctx;
}

/** 创建GPU实现的WindowContext
* @param [in] hWnd 关联的窗口句柄，可以为nullptr
* @param [in] params 显示相关的参数
*/
std::unique_ptr<skwindow::WindowContext> MakeGLForWin(HWND hWnd, std::unique_ptr<const skwindow::DisplayParams> params)
{
    std::unique_ptr<skwindow::WindowContext> ctx(new SkGLWindowContext_Windows(hWnd, std::move(params)));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

Render_Skia_Windows::Render_Skia_Windows(HWND hWnd, RenderBackendType backendType):
    m_hWnd(hWnd),
    m_backendType(backendType),
    m_hDC(nullptr),
    m_hOldObj(nullptr)
{
    if (backendType == RenderBackendType::kNativeGL_BackendType) {
        //GPU的绘制，必须绑定窗口
        ASSERT(::IsWindow(hWnd));
        if (!::IsWindow(hWnd)) {
            backendType = RenderBackendType::kRaster_BackendType;
        }
    }
    //创建WindowContext
    if (backendType == RenderBackendType::kNativeGL_BackendType) {
        //GPU绘制
        m_pWindowContext = MakeGLForWin(hWnd, std::make_unique<skwindow::DisplayParams>());
        ASSERT(m_pWindowContext != nullptr);
        if (m_pWindowContext != nullptr) {
            m_backendType = RenderBackendType::kNativeGL_BackendType;
        }        
    }
    //如果GL不成功，则创建CPU绘制的上下文
    if (m_pWindowContext == nullptr) {
        //CPU绘制
        m_pWindowContext = MakeRasterForWin(hWnd, std::make_unique <skwindow::DisplayParams>());
        ASSERT(m_pWindowContext != nullptr);
        if (m_pWindowContext != nullptr) {
            m_backendType = RenderBackendType::kRaster_BackendType;
        }        
    }
}

Render_Skia_Windows::~Render_Skia_Windows()
{
    DeleteDC();
}

RenderBackendType Render_Skia_Windows::GetRenderBackendType() const
{
    return m_backendType;
}

bool Render_Skia_Windows::Resize(int32_t width, int32_t height)
{
    ASSERT((width > 0) && (height > 0));
    if ((width <= 0) || (height <= 0)) {
        return false;
    }
    if ((GetWidth() == width) && (GetHeight() == height)) {
        return true;
    }

    ASSERT(m_pWindowContext != nullptr);
    if (m_pWindowContext == nullptr) {
        return false;
    }
    m_pWindowContext->resize(width, height);
    ASSERT(GetSkCanvas() != nullptr);

    DeleteDC();
    return true;
}

int32_t Render_Skia_Windows::GetWidth() const
{
    if (m_pWindowContext != nullptr) {
        return m_pWindowContext->width();
    }
    return 0;
}

int32_t Render_Skia_Windows::GetHeight() const
{
    if (m_pWindowContext != nullptr) {
        return m_pWindowContext->height();
    }
    return 0;
}

std::unique_ptr<ui::IRender> Render_Skia_Windows::Clone()
{
    std::unique_ptr<ui::IRender> pClone = std::make_unique<ui::Render_Skia_Windows>(m_hWnd, m_backendType);
    pClone->Resize(GetWidth(), GetHeight());
    pClone->SetRenderDpi(GetRenderDpi());
    pClone->BitBlt(0, 0, GetWidth(), GetHeight(), this, 0, 0, RopMode::kSrcCopy);
    return pClone;
}

bool Render_Skia_Windows::PaintAndSwapBuffers(IRenderPaint* pRenderPaint)
{
    ASSERT(pRenderPaint != nullptr);
    ASSERT(m_pWindowContext != nullptr);
    if ((m_pWindowContext != nullptr) && (pRenderPaint != nullptr)) {
        if (m_backendType == RenderBackendType::kNativeGL_BackendType) {
            SkGLWindowContext_Windows* pWindowContext = dynamic_cast<SkGLWindowContext_Windows*>(m_pWindowContext.get());
            ASSERT(pWindowContext != nullptr);
            if (pWindowContext != nullptr) {
                return pWindowContext->PaintAndSwapBuffers(this, pRenderPaint);
            }
        }
        else if (m_backendType == RenderBackendType::kRaster_BackendType) {
            SkRasterWindowContext_Windows* pWindowContext = dynamic_cast<SkRasterWindowContext_Windows*>(m_pWindowContext.get());
            ASSERT(pWindowContext != nullptr);
            if (pWindowContext != nullptr) {
                return pWindowContext->PaintAndSwapBuffers(this, pRenderPaint);
            }
        }
        else {
            ASSERT(false);
        }
    }
    return false;
}

SkSurface* Render_Skia_Windows::GetSkSurface() const
{
    ASSERT(m_pWindowContext != nullptr);
    if (m_pWindowContext == nullptr) {
        return nullptr;
    }
    //由于m_pWindowContext内部保存了成员变量，返回SkSurface的裸指针是安全的
    sk_sp<SkSurface> backbuffer = m_pWindowContext->getBackbufferSurface();
    ASSERT(backbuffer != nullptr);
    return backbuffer.get();
}

SkCanvas* Render_Skia_Windows::GetSkCanvas() const
{
    ASSERT(m_pWindowContext != nullptr);
    if (m_pWindowContext == nullptr) {
        return nullptr;
    }
    sk_sp<SkSurface> backbuffer = m_pWindowContext->getBackbufferSurface();
    ASSERT(backbuffer != nullptr);
    if (backbuffer == nullptr) {
        return nullptr;
    }
    ASSERT(backbuffer->getCanvas() != nullptr);
    return backbuffer->getCanvas();
}

HDC Render_Skia_Windows::GetRenderDC(HWND hWnd)
{
    if (m_hDC != nullptr) {
        return m_hDC;
    }
    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return nullptr;
    }
    ASSERT(m_pWindowContext != nullptr);
    if (m_pWindowContext == nullptr) {
        return nullptr;
    }
    SkRasterWindowContext_Windows* pWindowContext = dynamic_cast<SkRasterWindowContext_Windows*>(m_pWindowContext.get());
    if (pWindowContext == nullptr) {
        //如果不是CPU渲染的，不提供DC，该GetRenderDC函数，目前只有RichEdit在用，这里返回nullptr让RichEdit采取另外一种绘制方法
        ASSERT(GetRenderBackendType() != RenderBackendType::kRaster_BackendType);
        return nullptr;
    }

    HBITMAP hBitmap = pWindowContext->GetHBitmap();
    ASSERT(hBitmap != nullptr);
    if (hBitmap == nullptr) {
        return nullptr;
    }
    HDC hDeskDC = ::GetDC(hWnd);
    HDC hGetDC = ::CreateCompatibleDC(hDeskDC);
    ::ReleaseDC(hWnd, hDeskDC);
    hDeskDC = nullptr;

    m_hOldObj = ::SelectObject(hGetDC, hBitmap);

    if (skCanvas->isClipEmpty()) {
        ::IntersectClipRect(hGetDC, 0, 0, 0, 0);
    }
    else if (skCanvas->isClipRect()) {
        SkRect rcClip;
        if (skCanvas->getLocalClipBounds(&rcClip)) {
            RECT rc = { (int)rcClip.left(),(int)rcClip.top(),(int)rcClip.right(),(int)rcClip.bottom() };
            ::InflateRect(&rc, -1, -1); //注意需要向内缩小一个象素
            ::IntersectClipRect(hGetDC, rc.left, rc.top, rc.right, rc.bottom);
        }
    }
    else
    {
        SkRegion rgn;
        skCanvas->temporary_internal_getRgnClip(&rgn);
        SkRegion::Iterator it(rgn);
        int nCount = 0;
        for (; !it.done(); it.next()) {
            ++nCount;
        }
        it.rewind();

        int nSize = sizeof(RGNDATAHEADER) + nCount * sizeof(RECT);
        RGNDATA* rgnData = (RGNDATA*)::malloc(nSize);
        ASSERT(rgnData != nullptr);
        if (rgnData != nullptr) {
            memset(rgnData, 0, nSize);
            rgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
            rgnData->rdh.iType = RDH_RECTANGLES;
            rgnData->rdh.nCount = nCount;
            rgnData->rdh.rcBound.right = GetWidth();
            rgnData->rdh.rcBound.bottom = GetHeight();

            nCount = 0;
            LPRECT pRc = (LPRECT)rgnData->Buffer;
            for (; !it.done(); it.next()) {
                SkIRect skrc = it.rect();
                RECT rc = { skrc.fLeft,skrc.fTop,skrc.fRight,skrc.fBottom };
                pRc[nCount++] = rc;
            }

            HRGN hRgn = ::ExtCreateRegion(nullptr, nSize, rgnData);
            ::free(rgnData);
            ::SelectClipRgn(hGetDC, hRgn);
            ::DeleteObject(hRgn);
        }
    }

    ::SetGraphicsMode(hGetDC, GM_ADVANCED);
    const SkPoint& ptOrg = GetPointOrg();
    ::SetViewportOrgEx(hGetDC, (int)ptOrg.x(), (int)ptOrg.y(), nullptr);

    struct IxForm
    {
        enum Index {
            kMScaleX = 0,
            kMSkewX,
            kMTransX,
            kMSkewY,
            kMScaleY,
            kMTransY,
            kMPersp0,
            kMPersp1,
            kMPersp2
        };
    };
    SkMatrix mtx = skCanvas->getTotalMatrix();
    XFORM xForm = { mtx.get(IxForm::kMScaleX),mtx.get(IxForm::kMSkewY),
                    mtx.get(IxForm::kMSkewX),mtx.get(IxForm::kMScaleY),
                    mtx.get(IxForm::kMTransX),mtx.get(IxForm::kMTransY) };
    ::SetWorldTransform(hGetDC, &xForm);
    m_hDC = hGetDC;
    return hGetDC;
}

void Render_Skia_Windows::ReleaseRenderDC(HDC hdc)
{
    if (hdc == m_hDC) {
        DeleteDC();
    }
}

void Render_Skia_Windows::DeleteDC()
{
    if (m_hDC != nullptr) {
        if (m_hOldObj != nullptr) {
            ::SelectObject(m_hDC, m_hOldObj);
            m_hOldObj = nullptr;
        }
        ::DeleteDC(m_hDC);
        m_hDC = nullptr;
    }
}

// 从Skia的圆角路径生成HRGN，保证与drawRoundRect完全一致
static HRGN CreateHRGNFromSkiaRoundRect(const SkRect& skRect, SkScalar rx, SkScalar ry)
{
    // 创建Skia圆角路径（确保路径闭合）
    SkPath path;
    path.addRoundRect(skRect, rx, ry);
    path.close(); // 显式闭合路径，避免端点缺失

    // 使用SkPathMeasure离散化路径为密集点集
    SkPathMeasure pathMeasure(path, true); // true表示闭合路径
    std::vector<POINT> points;

    // 路径总长度
    SkScalar pathLength = pathMeasure.getLength();
    if (pathLength <= 0) {
        return nullptr;
    }

    // 离散化步长（越小越精细，建议1.0f，兼顾精度和性能）
    const SkScalar step = 1.0f;
    SkScalar distance = 0.0f;

    // 遍历路径，按步长提取点
    while (distance <= pathLength) {
        SkPoint pos;
        SkVector tan;
        // 获取指定距离处的点坐标和切线
        if (pathMeasure.getPosTan(distance, &pos, &tan)) {
            // 转换为Windows POINT（浮点转整数，四舍五入更精准）
            POINT pt = {
                static_cast<LONG>(std::round(pos.x())),
                static_cast<LONG>(std::round(pos.y()))
            };
            // 避免重复添加相同点
            if (points.empty() || !(points.back().x == pt.x && points.back().y == pt.y)) {
                points.push_back(pt);
            }
        }
        distance += step;
    }

    // 移除最后一个重复的起点（路径闭合时会重复）
    if ((points.size() >= 2) &&
        (points.front().x == points.back().x) &&
        (points.front().y == points.back().y)) {
        points.pop_back();
    }

    // 验证点集有效性（至少3个点才能形成多边形）
    if (points.size() < 3) {
        return nullptr;
    }

    // 创建多边形HRGN（ALTERNATE填充模式适配圆角）
    HRGN hRgn = ::CreatePolygonRgn(points.data(), static_cast<int>(points.size()), ALTERNATE);
    return hRgn;
}

bool Render_Skia_Windows::SetWindowRoundRectRgn(const UiRect& rcWnd, float rx, float ry, bool bRedraw)
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    SkRect skRect;
    skRect.fLeft = 0;
    skRect.fTop = 0;
    skRect.fRight = skRect.fLeft + rcWnd.Width();
    skRect.fBottom = skRect.fTop + rcWnd.Height();

    HRGN hRgn = CreateHRGNFromSkiaRoundRect(skRect, rx, ry);
    int nRet = ::SetWindowRgn(m_hWnd, hRgn, bRedraw ? TRUE : FALSE);
    return nRet != 0;
}

bool Render_Skia_Windows::SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw)
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    HRGN hRgn = ::CreateRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);
    int nRet = ::SetWindowRgn(m_hWnd, hRgn, bRedraw ? TRUE : FALSE);
    return nRet != 0;
}

void Render_Skia_Windows::ClearWindowRgn(bool bRedraw)
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return;
    }
    ::SetWindowRgn(m_hWnd, nullptr, bRedraw ? TRUE : FALSE);
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
