#include "SkRasterWindowContext_Windows.h"
#include "duilib/Render/IRender.h"

namespace ui {

SkRasterWindowContext_Windows::SkRasterWindowContext_Windows(HWND hWnd, const skwindow::DisplayParams& params):
    RenderWindowContext(params),
    m_hWnd(hWnd),
    m_hBitmap(nullptr)
{
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
    else {
        fWidth = 0;
        fHeight = 0;
    }
}

SkRasterWindowContext_Windows::~SkRasterWindowContext_Windows()
{
    if (m_hBitmap != nullptr) {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
}

void SkRasterWindowContext_Windows::setDisplayParams(const skwindow::DisplayParams& params)
{
    fDisplayParams = params;
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
        return;
    }
    ASSERT(pixels != nullptr);
    if (pixels == nullptr) {
        ::DeleteObject(hBitmap);
        return;
    }
    m_hBitmap = hBitmap;
    SkImageInfo info = SkImageInfo::Make(nWidth, nHeight, fDisplayParams.fColorType, SkAlphaType::kPremul_SkAlphaType, fDisplayParams.fColorSpace);
    m_fBackbufferSurface = SkSurfaces::WrapPixels(info, pixels, sizeof(uint32_t) * nWidth);
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
    if (!::GetUpdateRect(m_hWnd, &rectUpdate, FALSE)) {
        //无需绘制
        return false;
    }

    //窗口透明度
    uint8_t nWindowAlpha = pRenderPaint->GetWindowAlpha();

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
        SwapPaintBuffers(hPaintDC, rcPaint, pRender, nWindowAlpha);

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
        SwapPaintBuffers(hPaintDC, rcUpdate, pRender, nWindowAlpha);
        ::ReleaseDC(hWnd, hPaintDC);

        //标记绘制区域为有效区域
        ::ValidateRect(hWnd, &rectUpdate);
    }
    return false;
}

bool SkRasterWindowContext_Windows::SwapPaintBuffers(HDC hPaintDC, const UiRect& rcPaint, IRender* pRender, uint8_t nWindowAlpha) const
{
    ASSERT(hPaintDC != nullptr);
    if (hPaintDC == nullptr) {
        return false;
    }
    ASSERT(!rcPaint.IsEmpty());
    if (rcPaint.IsEmpty()) {
        return false;
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }

    // 渲染到窗口
    bool bRet = false;
    if (::GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) {
        //分层窗口
        UiRect rcWindow;
        GetWindowRect(rcWindow);
        UiRect rcClient;
        GetClientRect(rcClient);
        POINT pt = { rcWindow.left, rcWindow.top };
        SIZE szWindow = { rcClient.Width(), rcClient.Height()};
        POINT ptSrc = { 0, 0 };
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, nWindowAlpha, AC_SRC_ALPHA };
        HDC hdc = pRender->GetRenderDC(m_hWnd);
        ASSERT(hdc != nullptr);
        if (hdc != nullptr) {
            bRet = ::UpdateLayeredWindow(m_hWnd, NULL, &pt, &szWindow, hdc, &ptSrc, 0, &bf, ULW_ALPHA) != FALSE;
            ASSERT(bRet);
            pRender->ReleaseRenderDC(hdc);
        }
    }
    else {
        //普通窗口
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

    BITMAPINFO bmi = { 0 };
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
        hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, pBits, NULL, 0);
    }
    ::ReleaseDC(m_hWnd, hdc);
    return hBitmap;
}

} // namespace ui
