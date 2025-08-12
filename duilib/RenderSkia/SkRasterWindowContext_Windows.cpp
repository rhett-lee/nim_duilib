#include "SkRasterWindowContext_Windows.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/PerformanceUtil.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui {

SkRasterWindowContext_Windows::SkRasterWindowContext_Windows(HWND hWnd, std::unique_ptr<const skwindow::DisplayParams> params):
    skwindow::internal::RasterWindowContext(std::move(params)),
    m_hWnd(hWnd),
    m_hBitmap(nullptr)
{
    fWidth = 0;
    fHeight = 0;
    if (m_hWnd != nullptr) {
        SkASSERT(::IsWindow(m_hWnd));
        if (!::IsWindow(m_hWnd)) {
            m_hWnd = nullptr;
        }
    }
    if (m_hWnd != nullptr) {
        RECT rect{ 0, };
        ::GetClientRect(m_hWnd, &rect);
        this->resize(rect.right - rect.left, rect.bottom - rect.top);
    }
}

SkRasterWindowContext_Windows::~SkRasterWindowContext_Windows()
{
    if (m_hBitmap != nullptr) {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
}

void SkRasterWindowContext_Windows::setDisplayParams(std::unique_ptr<const skwindow::DisplayParams> params)
{
    fDisplayParams = std::move(params);
    RECT rect;
    ::GetClientRect(m_hWnd, &rect);
    this->resize(rect.right - rect.left, rect.bottom - rect.top);
}

void SkRasterWindowContext_Windows::resize(int nWidth, int nHeight)
{
    if (nWidth < 0) {
        nWidth = 0;
    }
    if (nHeight < 0) {
        nHeight = 0;
    }
    if ((fWidth == nWidth) && (fHeight == nHeight)) {
        if ((fWidth > 0) && (fHeight > 0)){
            ASSERT(m_fBackbufferSurface != nullptr);
            ASSERT(m_hBitmap != nullptr);
        }
        return;
    }
    const skwindow::DisplayParams* pDisplayParams = getDisplayParams();
    ASSERT(pDisplayParams != nullptr);
    if (pDisplayParams == nullptr) {
        return;
    }

    fWidth = nWidth;
    fHeight = nHeight;

    m_fBackbufferSurface.reset();
    if (m_hBitmap != nullptr) {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }

    if ((nWidth == 0) || (nHeight == 0)) {
        return;
    }

    LPVOID pixels = nullptr;
    HBITMAP hBitmap = CreateHBitmap(nWidth, nHeight, true, &pixels);
    ASSERT(hBitmap != nullptr);
    if (hBitmap == nullptr) {
        fWidth = 0;
        fHeight = 0;
        return;
    }
    ASSERT(pixels != nullptr);
    if (pixels == nullptr) {
        ::DeleteObject(hBitmap);
        fWidth = 0;
        fHeight = 0;
        return;
    }
    m_hBitmap = hBitmap;
    SkImageInfo info = SkImageInfo::Make(nWidth, nHeight, pDisplayParams->colorType(), SkAlphaType::kPremul_SkAlphaType, pDisplayParams->colorSpace());
    m_fBackbufferSurface = SkSurfaces::WrapPixels(info, pixels, sizeof(uint32_t) * nWidth);
    ASSERT(m_fBackbufferSurface != nullptr);
    if (m_fBackbufferSurface == nullptr) {
        if (m_hBitmap != nullptr) {
            ::DeleteObject(m_hBitmap);
            m_hBitmap = nullptr;
        }
        fWidth = 0;
        fHeight = 0;
        return;
    }
}

sk_sp<SkSurface> SkRasterWindowContext_Windows::getBackbufferSurface()
{
    return m_fBackbufferSurface;
}

void SkRasterWindowContext_Windows::onSwapBuffers()
{
}

bool SkRasterWindowContext_Windows::PaintAndSwapBuffers(IRender* pRender, IRenderPaint* pRenderPaint)
{
    HWND hWnd = m_hWnd;
    SkASSERT(::IsWindow(hWnd));
    if ((hWnd == nullptr) || !::IsWindow(hWnd)) {
        return false;
    }
    SkASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    SkASSERT(pRenderPaint != nullptr);
    if (pRenderPaint == nullptr) {
        return false;
    }

    //获取需要绘制的区域
    RECT rectUpdate = { 0, };
    if (!::GetUpdateRect(hWnd, &rectUpdate, FALSE)) {
        //无需绘制
        return false;
    }

    //窗口透明度
    uint8_t nLayeredWindowAlpha = pRenderPaint->GetLayeredWindowAlpha();

    //开始绘制
    bool bRet = false;
    PAINTSTRUCT ps = { 0, };
    HDC hPaintDC = ::BeginPaint(hWnd, &ps);
    UiRect rcPaint;
    rcPaint.left = ps.rcPaint.left;
    rcPaint.top = ps.rcPaint.top;
    rcPaint.right = ps.rcPaint.right;
    rcPaint.bottom = ps.rcPaint.bottom;
    if (!rcPaint.IsEmpty() && (hPaintDC != nullptr)) {
        //执行绘制
        bRet = pRenderPaint->DoPaint(rcPaint);

        //绘制完成后，更新到窗口
        SwapPaintBuffers(hPaintDC, rcPaint, pRender, nLayeredWindowAlpha);

        //结束本次绘制
        ::EndPaint(hWnd, &ps);
        hPaintDC = nullptr;
    }
    else {
        //开始绘制返回值无效，结束绘制，使用另外一种绘制方法
        ::EndPaint(hWnd, &ps);
        hPaintDC = nullptr;

        //设置更新区域
        UiRect rcUpdate;
        rcUpdate.left = rectUpdate.left;
        rcUpdate.top = rectUpdate.top;
        rcUpdate.right = rectUpdate.right;
        rcUpdate.bottom = rectUpdate.bottom;

        //执行绘制
        bRet = pRenderPaint->DoPaint(rcUpdate);

        //绘制完成后，更新到窗口
        hPaintDC = ::GetDC(hWnd);
        SwapPaintBuffers(hPaintDC, rcUpdate, pRender, nLayeredWindowAlpha);
        ::ReleaseDC(hWnd, hPaintDC);

        //标记绘制区域为有效区域
        ::ValidateRect(hWnd, &rectUpdate);
    }
    return bRet;
}

bool SkRasterWindowContext_Windows::SwapPaintBuffers(HDC hPaintDC, const UiRect& rcPaint, IRender* pRender, uint8_t nLayeredWindowAlpha) const
{
    PerformanceStat statPerformance(_T("SkRasterWindowContext_Windows::SwapPaintBuffers"));
    ASSERT(hPaintDC != nullptr);
    if (hPaintDC == nullptr) {
        return false;
    }
    //ASSERT(!rcPaint.IsEmpty()); //当窗口部分在屏幕外时，会出现为空的情况
    if (rcPaint.IsEmpty()) {
        return false;
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }

    // 渲染到窗口
    bool bRet = false;
    bool bPainted = false;
    if (::GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) {
        //分层窗口
        COLORREF crKey = 0;
        BYTE bAlpha = 0;
        DWORD dwFlags = LWA_ALPHA | LWA_COLORKEY;
        //当返回true的时候，不能按分层窗口绘制，必须按普通的窗口模式绘制
        bool bAttributes = ::GetLayeredWindowAttributes(m_hWnd, &crKey, &bAlpha, &dwFlags) != FALSE;
        if (!bAttributes) {
            UiRect rcWindow;
            GetWindowRect(rcWindow);
            UiRect rcClient;
            GetClientRect(rcClient);
            POINT pt = { rcWindow.left, rcWindow.top };
            SIZE szWindow = { rcClient.Width(), rcClient.Height() };
            POINT ptSrc = { 0, 0 };
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, nLayeredWindowAlpha, AC_SRC_ALPHA };
            HDC hdc = pRender->GetRenderDC(m_hWnd);
            ASSERT(hdc != nullptr);
            if (hdc != nullptr) {
                //按分层窗口模式绘制
                bRet = ::UpdateLayeredWindow(m_hWnd, nullptr, &pt, &szWindow, hdc, &ptSrc, 0, &bf, ULW_ALPHA) != FALSE;
                bPainted = true;
                ASSERT(bRet);
                pRender->ReleaseRenderDC(hdc);
            }
        }        
    }
    if (!bPainted) {
        //按普通窗口模式绘制
        ASSERT(hPaintDC != nullptr);
        HDC hdc = pRender->GetRenderDC(m_hWnd);
        ASSERT(hdc != nullptr);
        if (hdc != nullptr) {
            bRet = ::BitBlt(hPaintDC, rcPaint.left, rcPaint.top, rcPaint.Width(), rcPaint.Height(),
                            hdc, rcPaint.left, rcPaint.top, SRCCOPY) != FALSE;
            pRender->ReleaseRenderDC(hdc);
        }
    }
    return bRet;
}

HBITMAP SkRasterWindowContext_Windows::GetHBitmap() const
{
    ASSERT(m_hBitmap != nullptr);
    return m_hBitmap;
}

void SkRasterWindowContext_Windows::GetWindowRect(UiRect& rcWindow) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetWindowRect(m_hWnd, &rc);
    rcWindow = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

void SkRasterWindowContext_Windows::GetClientRect(UiRect& rcClient) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetClientRect(m_hWnd, &rc);
    rcClient = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

HBITMAP SkRasterWindowContext_Windows::CreateHBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits) const
{
    ASSERT((nWidth > 0) && (nHeight > 0));
    if (nWidth <= 0 || nHeight <= 0) {
        return nullptr;
    }

    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = nWidth;
    if (flipHeight) {
        bmi.bmiHeader.biHeight = -nHeight;//负数表示位图方向：从上到下，左上角为圆点
    }
    else {
        bmi.bmiHeader.biHeight = nHeight; //正数表示位图方向：从下到上，左下角为圆点
    }
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = nWidth * nHeight * sizeof(DWORD);

    HBITMAP hBitmap = nullptr;
    HDC hdc = ::GetDC(m_hWnd);
    ASSERT(hdc != nullptr);
    if (hdc != nullptr) {
        hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, pBits, nullptr, 0);
    }
    ::ReleaseDC(m_hWnd, hdc);
    return hBitmap;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
