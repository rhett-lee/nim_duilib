#include "Render_Skia_Windows.h"
#include "SkRasterWindowContext_Windows.h"
#include "RenderWindowContext.h"

#pragma warning (push)
#pragma warning (disable: 4267 4244 4201 4100)

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRegion.h"

#include "tools/window/WindowContext.h"

//暂未支持OpenGL渲染
//#define SK_GL
//#define GR_TEST_UTILS
//#include "include/gpu/GrDirectContext.h"
//#include "include/gpu/GrRecordingContext.h"

#pragma warning (pop)

namespace ui {

/** 创建Raster实现的WindowContext
* @param [in] hWnd 关联的窗口句柄，可以为nullptr
* @param [in] params 显示相关的参数
*/
std::unique_ptr<RenderWindowContext> MakeRasterForWin(HWND hWnd, const skwindow::DisplayParams& params)
{
    std::unique_ptr<RenderWindowContext> ctx(new SkRasterWindowContext_Windows(hWnd, params));
    return ctx;
}

Render_Skia_Windows::Render_Skia_Windows(HWND hWnd, RenderBackendType backendType):
    m_hWnd(hWnd),
    m_backendType(backendType),
    m_hDC(nullptr),
    m_hOldObj(nullptr)
{
    //创建WindowContext
    m_pWindowContext = MakeRasterForWin(hWnd, skwindow::DisplayParams());
    ASSERT(m_pWindowContext != nullptr);
}

Render_Skia_Windows::~Render_Skia_Windows()
{
    DeleteDC();
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
        return m_pWindowContext->PaintAndSwapBuffers(this, pRenderPaint);
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
    ASSERT(pWindowContext != nullptr);
    if (pWindowContext == nullptr) {
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

            HRGN hRgn = ::ExtCreateRegion(NULL, nSize, rgnData);
            ::free(rgnData);
            ::SelectClipRgn(hGetDC, hRgn);
            ::DeleteObject(hRgn);
        }
    }

    ::SetGraphicsMode(hGetDC, GM_ADVANCED);
    const SkPoint& ptOrg = GetPointOrg();
    ::SetViewportOrgEx(hGetDC, (int)ptOrg.x(), (int)ptOrg.y(), NULL);

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

} // namespace ui
