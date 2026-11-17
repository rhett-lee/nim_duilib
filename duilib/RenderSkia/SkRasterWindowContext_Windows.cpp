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
        ASSERT(::IsWindow(m_hWnd));
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

bool SkRasterWindowContext_Windows::PaintAndSwapBuffers(IRenderWindow* pRender, IRenderPaint* pRenderPaint)
{
    HWND hWnd = m_hWnd;
    ASSERT(::IsWindow(hWnd));
    if ((hWnd == nullptr) || !::IsWindow(hWnd)) {
        return false;
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    ASSERT(pRenderPaint != nullptr);
    if (pRenderPaint == nullptr) {
        return false;
    }

    //获取需要绘制的区域
    RECT rectUpdate = { 0, };
    if (!::GetUpdateRect(hWnd, &rectUpdate, FALSE)) {
        //无需绘制
        return false;
    }

    //获取需要绘制的区域列表（可能是多个不连续的脏矩形）
    std::vector<UiRect> rcPaintList;
    UiRect rcPaint; //所有脏矩形的并集边界
    UiRect rcClient;
    GetClientRect(rcClient);
    if (pRenderPaint->GetUpdateRects(rcPaintList)) {
        std::vector<UiRect> rcValidList;
        for (UiRect& rcItem : rcPaintList) {
            rcItem.Intersect(rcClient);
            if (!rcItem.IsEmpty()) {
                rcValidList.push_back(rcItem);
                rcPaint.Union(rcItem);
            }
        }
        rcPaintList.swap(rcValidList);
    }
    if (rcPaint.IsEmpty()) {
        rcPaint.left = rectUpdate.left;
        rcPaint.top = rectUpdate.top;
        rcPaint.right = rectUpdate.right;
        rcPaint.bottom = rectUpdate.bottom;
        rcPaintList.clear();
        rcPaintList.push_back(rcPaint);
    }

    //窗口透明度
    uint8_t nLayeredWindowAlpha = pRenderPaint->GetLayeredWindowAlpha();

    //开始绘制
    bool bRet = false;
    PAINTSTRUCT ps = { 0, };
    HDC hPaintDC = ::BeginPaint(hWnd, &ps);
    if (!rcPaint.IsEmpty() && (hPaintDC != nullptr)) {
        //完全绘制时，使用单个矩形，走快速路径
        if ((rcPaint.Width() == rcClient.Width()) && (rcPaint.Height() == rcClient.Height())) {
            rcPaintList.clear();
            rcPaintList.push_back(rcClient);
        }

        //设置本次绘制的脏区列表及画布的裁剪区域（多个脏矩形的并集，脏区之外不会被重新光栅化）
        int32_t nClipState = static_cast<IRender*>(pRender)->BeginPaintRects(rcPaintList);

        //执行绘制（使用并集边界作为绘制范围，控件级使用脏区列表精确剔除）
        bRet = pRenderPaint->DoPaint(rcPaint);

        //恢复裁剪状态
        static_cast<IRender*>(pRender)->EndPaintRects(nClipState);

        //绘制完成后，更新到窗口
        SwapPaintBuffers(hPaintDC, rcPaintList, pRender, nLayeredWindowAlpha);

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
        rcPaintList.clear();
        rcPaintList.push_back(rcUpdate);

        //设置本次绘制的脏区列表及画布的裁剪区域
        int32_t nClipState = static_cast<IRender*>(pRender)->BeginPaintRects(rcPaintList);

        //执行绘制
        bRet = pRenderPaint->DoPaint(rcUpdate);

        //恢复裁剪状态
        static_cast<IRender*>(pRender)->EndPaintRects(nClipState);

        //绘制完成后，更新到窗口
        hPaintDC = ::GetDC(hWnd);
        SwapPaintBuffers(hPaintDC, rcPaintList, pRender, nLayeredWindowAlpha);
        ::ReleaseDC(hWnd, hPaintDC);

        //标记绘制区域为有效区域
        ::ValidateRect(hWnd, &rectUpdate);
    }
    return bRet;
}

bool SkRasterWindowContext_Windows::SwapPaintBuffers(HDC hPaintDC, const std::vector<UiRect>& rcPaintList, IRenderWindow* pRender, uint8_t nLayeredWindowAlpha) const
{
#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("PaintWindow 9, SkRasterWindowContext_Windows::SwapPaintBuffers");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    ASSERT(hPaintDC != nullptr);
    if (hPaintDC == nullptr) {
        return false;
    }
    //ASSERT(!rcPaintList.empty()); //当窗口部分在屏幕外时，会出现为空的情况
    if (rcPaintList.empty()) {
        return false;
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    //渲染器的DC
    HDC hRenderDC = pRender->GetRenderDC(m_hWnd);
    ASSERT(hRenderDC != nullptr);
    if (hRenderDC == nullptr) {
        return false;
    }

    // 渲染到窗口
    bool bPainted = false;
    if (::GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) {
        //分层窗口
        COLORREF crKey = 0;
        BYTE bAlpha = 255;
        DWORD dwFlags = 0;
        //当返回true的时候，不能按分层窗口绘制，必须按普通的窗口模式绘制
        bool bLayeredWindowAttributes = ::GetLayeredWindowAttributes(m_hWnd, &crKey, &bAlpha, &dwFlags) != FALSE;
        if (bLayeredWindowAttributes) {
            if ((bAlpha == 255) || (crKey == 0)) {
                //这种情况下，需要按照分层窗口绘制（当切换分层窗口后，会出现这个现象）
                bLayeredWindowAttributes = false;
            }
        }
        if (!bLayeredWindowAttributes) {
            UiRect rcWindow;
            GetWindowRect(rcWindow);
            POINT pt = { rcWindow.left, rcWindow.top };
            //UpdateLayeredWindow的psize参数表示窗口的整体尺寸（而不是客户区尺寸），
            //如果传入客户区尺寸，窗口会被系统重新设置为该尺寸（使用系统标题栏等有非客户区时，窗口会逐帧变小）
            SIZE szWindow = { rcWindow.Width(), rcWindow.Height() };
            POINT ptSrc = { 0, 0 };
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, nLayeredWindowAlpha, AC_SRC_ALPHA };
            //按分层窗口模式绘制
            bPainted = ::UpdateLayeredWindow(m_hWnd, nullptr, &pt, &szWindow, hRenderDC, &ptSrc, 0, &bf, ULW_ALPHA) != FALSE;
        }        
    }
    if (!bPainted) {
        //按普通窗口模式绘制
        bPainted = true;
        //决定像素拷贝方式：脏矩形比较分散时按矩形逐个拷贝（减少拷贝的像素量），
        //否则按并集边界一次拷贝（减少 BitBlt 的调用次数，每次调用都有固定的驱动开销）
        UiRect rcUnion;
        int64_t nRectsArea = 0;
        for (const UiRect& rcPaint : rcPaintList) {
            if (rcPaint.IsEmpty()) {
                continue;
            }
            rcUnion.Union(rcPaint);
            nRectsArea += (int64_t)rcPaint.Width() * rcPaint.Height();
        }
        const int64_t nUnionArea = (int64_t)rcUnion.Width() * rcUnion.Height();
        const bool bCopyByRects = (rcPaintList.size() > 1) && (rcPaintList.size() <= 6) &&
                                  (nRectsArea * 2 <= nUnionArea);
        if (bCopyByRects) {
            for (const UiRect& rcPaint : rcPaintList) {
                if (rcPaint.IsEmpty()) {
                    continue;
                }
                if (!::BitBlt(hPaintDC, rcPaint.left, rcPaint.top, rcPaint.Width(), rcPaint.Height(),
                              hRenderDC, rcPaint.left, rcPaint.top, SRCCOPY)) {
                    bPainted = false;
                }
            }
        }
        else if (!rcUnion.IsEmpty()) {
            bPainted = ::BitBlt(hPaintDC, rcUnion.left, rcUnion.top, rcUnion.Width(), rcUnion.Height(),
                                hRenderDC, rcUnion.left, rcUnion.top, SRCCOPY) != FALSE;
        }
    }
    pRender->ReleaseRenderDC(hRenderDC);
    ASSERT(bPainted);
    return bPainted;
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
