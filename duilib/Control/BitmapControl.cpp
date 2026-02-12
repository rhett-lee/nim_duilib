#include "BitmapControl.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Image/ImageAttribute.h"

namespace ui
{

BitmapControl::BitmapControl(Window* pWindow):
    Box(pWindow),
    m_hAlignType(HorAlignType::kAlignLeft),
    m_vAlignType(VerAlignType::kAlignTop),
    m_nBitmapAlpha(255),
    m_bAdaptiveDestRect(false),
    m_bStretchedDrawing(false),
    m_bSupportMultiThread(true)
{
}

BitmapControl::~BitmapControl()
{
    m_pBitmap.reset();
}

DString BitmapControl::GetType() const { return DUI_CTR_BITMAP_CONTROL; }

void BitmapControl::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("bitmap_halign")) {
        ASSERT((strValue == _T("left")) || (strValue == _T("center")) || (strValue == _T("right")));
        if (strValue == _T("center")) {
            SetBitmapHAlignType(HorAlignType::kAlignCenter);
        }
        else if (strValue == _T("right")) {
            SetBitmapHAlignType(HorAlignType::kAlignRight);
        }
        else {
            SetBitmapHAlignType(HorAlignType::kAlignLeft);
        }
    }
    else if (strName == _T("bitmap_valign")) {
        ASSERT((strValue == _T("top")) || (strValue == _T("center")) || (strValue == _T("bottom")));
        if (strValue == _T("center")) {
            SetBitmapVAlignType(VerAlignType::kAlignCenter);
        }
        else if (strValue == _T("bottom")) {
            SetBitmapVAlignType(VerAlignType::kAlignBottom);
        }
        else {
            SetBitmapVAlignType(VerAlignType::kAlignTop);
        }
    }
    else if (strName == _T("bitmap_alpha")) {
        SetBitmapAlpha((uint8_t)StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("bitmap_dest")) {
        UiRect rcDest;
        DString::value_type* pstr = nullptr;
        rcDest.left = StringUtil::StringToInt32(strValue.c_str(), &pstr, 10); ASSERT(pstr);
        AttributeUtil::SkipSepChar(pstr);
        if (*pstr != _T('\0')) {
            rcDest.top = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
            AttributeUtil::SkipSepChar(pstr);
        }
        if (*pstr != _T('\0')) {
            rcDest.right = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
            AttributeUtil::SkipSepChar(pstr);
        }
        if (*pstr != _T('\0')) {
            rcDest.bottom = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
        }
        SetBitmapDest(rcDest, true);
    }
    else if (strName == _T("bitmap_src")) {
        UiRect rcSource;
        AttributeUtil::ParseRectValue(strValue.c_str(), rcSource);
        rcSource.left = std::max(rcSource.left, 0);
        rcSource.top = std::max(rcSource.top, 0);
        SetBitmapSource(rcSource, true);
    }
    else if (strName == _T("bitmap_margin")) {
        UiMargin rcMargin;
        AttributeUtil::ParseMarginValue(strValue.c_str(), rcMargin);
        SetBitmapMargin(rcMargin, true);
    }
    else if (strName == _T("bitmap_adaptive_dest_rect")) {
        SetAdaptiveDestRect(strValue == _T("true"));
    }
    else if (strName == _T("bitmap_stretch")) {
        SetStretchedDrawing(strValue == _T("true"));
    }
    else if (strName == _T("bitmap_multi_thread")) {
        SetSupportMultiThread(strValue == _T("true"));
    }
    else if (strName == _T("bitmap_file")) {
        //设置关联的图片文件：主要用于测试
        m_bitmapFile = strValue;
        if (m_pBitmap != nullptr) {
            m_pBitmap.reset();
            CheckLoadBitmapFile();
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

UiSize BitmapControl::EstimateImage(UiSize szAvailable, EstimateImageType estImageType)
{
    UiSize estSize = BaseClass::EstimateImage(szAvailable, estImageType);

    //按需加载指定的图片
    CheckLoadBitmapFile();

    int32_t nImageInfoWidth = 0;
    int32_t nImageInfoHeight = 0;
    GetBitmapSize(nImageInfoWidth, nImageInfoHeight);
    if ((nImageInfoWidth <= 0) || (nImageInfoHeight <= 0)) {
        //没有图像数据
        return estSize;
    }

    //控件自身的内边距
    const UiPadding rcControlPadding = GetControlPadding();
    UiRect rcDest;
    bool hasDestAttr = false;
    if (m_rcDest != nullptr) {
        //使用配置中指定的目标区域（已按配置做好DPI自适应）：优先作为图片大小的依据
        rcDest = *m_rcDest;
        if (rcDest.left < 0) {
            rcDest.left = 0;
        }
        if (rcDest.top < 0) {
            rcDest.top = 0;
        }
        if (rcDest.right <= rcDest.left) {
            rcDest.right = rcDest.left + nImageInfoWidth;
        }
        if (rcDest.bottom <= rcDest.top) {
            rcDest.bottom = rcDest.top + nImageInfoHeight;
        }
        hasDestAttr = true;
    }
    UiRect rcSource;
    if (m_rcSource != nullptr) {
        rcSource = *m_rcSource;
    }
    if (rcSource.right > (int32_t)nImageInfoWidth) {
        rcSource.right = (int32_t)nImageInfoWidth;
    }
    if (rcSource.bottom > (int32_t)nImageInfoHeight) {
        rcSource.bottom = (int32_t)nImageInfoHeight;
    }

    UiSize imageSize;
    if (rcDest.Width() > 0) {
        //以0为基点，right为边界
        imageSize.cx = rcDest.right;
    }
    else if (rcSource.Width() > 0) {
        imageSize.cx = rcSource.Width();
    }
    else {
        imageSize.cx = nImageInfoWidth;
    }

    if (rcDest.Height() > 0) {
        //以0为基点，bottom为边界
        imageSize.cy = rcDest.bottom;
    }
    else if (rcSource.Height() > 0) {
        imageSize.cy = rcSource.Height();
    }
    else {
        imageSize.cy = nImageInfoHeight;
    }

    if (!hasDestAttr) {
        //如果没有rcDest属性，则需要增加图片的外边距（图片自身的外边距属性）
        UiMargin rcImageMargin;
        if (m_rcMargin != nullptr) {
            rcImageMargin = *m_rcMargin;
        }
        imageSize.cx += (rcImageMargin.left + rcImageMargin.right);
        imageSize.cy += (rcImageMargin.top + rcImageMargin.bottom);
    }
    if (m_bAdaptiveDestRect) {
        //自动适应目标区域（等比例缩放图片）：根据图片大小，调整绘制区域
        const int32_t nImageWidth = rcSource.Width();
        const int32_t nImageHeight = rcSource.Height();
        UiRect rcControlDest = UiRect(0, 0,
                                      szAvailable.cx - rcControlPadding.left - rcControlPadding.right,
                                      szAvailable.cy - rcControlPadding.top - rcControlPadding.bottom);
        rcControlDest.Validate();
        if (rcControlDest.Width() > 0 && rcControlDest.Height() > 0) {
            DString hAlign = _T("left");
            if (m_hAlignType == HorAlignType::kAlignCenter) {
                hAlign = _T("center");
            }
            else if (m_hAlignType == HorAlignType::kAlignRight) {
                hAlign = _T("right");
            }
            DString vAlign = _T("top");
            if (m_vAlignType == VerAlignType::kAlignCenter) {
                vAlign = _T("center");
            }
            else if (m_vAlignType == VerAlignType::kAlignBottom) {
                vAlign = _T("bottom");
            }
            rcControlDest = ImageAttribute::CalculateAdaptiveRect(nImageWidth, nImageHeight, rcControlDest, hAlign, vAlign);
            imageSize.cx = rcControlDest.Width();
            imageSize.cy = rcControlDest.Height();
        }
    }

    //图片大小，需要附加控件的内边距
    if (imageSize.cx > 0) {
        imageSize.cx += (rcControlPadding.left + rcControlPadding.right);
    }
    if (imageSize.cy > 0) {
        imageSize.cy += (rcControlPadding.top + rcControlPadding.bottom);
    }
    if ((estImageType == EstimateImageType::kBoth) || (estImageType == EstimateImageType::kWidthOnly)) {
        //宽度为自动计算
        estSize.cx = std::max(estSize.cx, imageSize.cx);
    }
    if ((estImageType == EstimateImageType::kBoth) || (estImageType == EstimateImageType::kHeightOnly)) {
        //高度为自动计算
        estSize.cy = std::max(estSize.cy, imageSize.cy);
    }
    return estSize;
}

void BitmapControl::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    if (HasBitmapDest()) {
        UiRect rcDest = GetBitmapDest();
        rcDest = Dpi().GetScaleRect(rcDest, nOldDpiScale);
        SetBitmapDest(rcDest, false);
    }
    if (HasBitmapSource()) {
        UiRect rcSource = GetBitmapSource();
        rcSource = Dpi().GetScaleRect(rcSource, nOldDpiScale);
        SetBitmapSource(rcSource, false);
    }
    if (HasBitmapMargin()) {
        UiMargin rcMargin = GetBitmapMargin();
        rcMargin = Dpi().GetScaleMargin(rcMargin, nOldDpiScale);
        SetBitmapMargin(rcMargin, false);
    }
}

void BitmapControl::SetBitmapHAlignType(HorAlignType hAlignType)
{
    if (m_hAlignType != hAlignType) {
        m_hAlignType = hAlignType;
        //重绘图片
        Invalidate();
    }
}

HorAlignType BitmapControl::GetBitmapHAlignType() const
{
    return m_hAlignType;
}

void BitmapControl::SetBitmapVAlignType(VerAlignType vAlignType)
{
    if (m_vAlignType != vAlignType) {
        m_vAlignType = vAlignType;
        //重绘图片
        Invalidate();
    }
}

VerAlignType BitmapControl::GetBitmapVAlignType() const
{
    return m_vAlignType;
}

void BitmapControl::SetBitmapAlpha(uint8_t nBitmapAlpha)
{
    ASSERT(nBitmapAlpha <= 255);
    if (nBitmapAlpha > 255) {
        return;
    }
    if (m_nBitmapAlpha != nBitmapAlpha) {
        m_nBitmapAlpha = nBitmapAlpha;
        //重绘图片
        Invalidate();
    }
}

uint8_t BitmapControl::GetBitmapAlpha() const
{
    return m_nBitmapAlpha;
}

void BitmapControl::SetBitmapDest(UiRect rcDest, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScaleRect(rcDest);
    }
    rcDest.Validate();
    if (m_rcDest == nullptr) {
        m_rcDest = std::make_unique<UiRect>(rcDest);
        //重绘图片
        Invalidate();
    }
    else {
        if (*m_rcDest != rcDest) {
            *m_rcDest = rcDest;
            //重绘图片
            Invalidate();
        }
    }
}

UiRect BitmapControl::GetBitmapDest() const
{
    if (m_rcDest != nullptr) {
        return *m_rcDest;
    }
    return UiRect();
}

bool BitmapControl::HasBitmapDest() const
{
    return (m_rcDest != nullptr);
}

void BitmapControl::SetBitmapSource(UiRect rcSource, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScaleRect(rcSource);
    }
    rcSource.Validate();
    if (m_rcSource == nullptr) {
        m_rcSource = std::make_unique<UiRect>(rcSource);
        //重绘图片
        Invalidate();
    }
    else {
        if (*m_rcSource != rcSource) {
            *m_rcSource = rcSource;
            //重绘图片
            Invalidate();
        }
    }
}

UiRect BitmapControl::GetBitmapSource() const
{
    if (m_rcSource != nullptr) {
        return *m_rcSource;
    }
    return UiRect();
}

bool BitmapControl::HasBitmapSource() const
{
    return (m_rcSource != nullptr);
}

void BitmapControl::SetBitmapMargin(UiMargin rcMargin, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScaleMargin(rcMargin);
    }
    if (m_rcMargin == nullptr) {
        m_rcMargin = std::make_unique<UiMargin>(rcMargin);
        //重绘图片
        Invalidate();
    }
    else {
        if (*m_rcMargin != rcMargin) {
            *m_rcMargin = rcMargin;
            //重绘图片
            Invalidate();
        }
    }
}

UiMargin BitmapControl::GetBitmapMargin() const
{
    if (m_rcMargin != nullptr) {
        return *m_rcMargin;
    }
    return UiMargin();
}

bool BitmapControl::HasBitmapMargin() const
{
    return (m_rcMargin != nullptr);
}

void BitmapControl::SetAdaptiveDestRect(bool bAdaptiveDestRect)
{
    if (m_bAdaptiveDestRect != bAdaptiveDestRect) {
        m_bAdaptiveDestRect = bAdaptiveDestRect;
        //重绘图片
        Invalidate();
    }
}

bool BitmapControl::IsAdaptiveDestRect() const
{
    return m_bAdaptiveDestRect;
}

void BitmapControl::SetStretchedDrawing(bool bStretchedDrawing)
{
    if (m_bStretchedDrawing != bStretchedDrawing) {
        m_bStretchedDrawing = bStretchedDrawing;
        //重绘图片
        Invalidate();
    }
}

bool BitmapControl::IsStretchedDrawing() const
{
    return m_bStretchedDrawing;
}

void BitmapControl::SetSupportMultiThread(bool bSupportMultiThread)
{
    m_bSupportMultiThread = bSupportMultiThread;
}

bool BitmapControl::IsSupportMultiThread() const
{
    return m_bSupportMultiThread;
}

void BitmapControl::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);

    //绘制图片
    PaintBitmap(pRender, rcPaint);
}

void BitmapControl::CheckLoadBitmapFile()
{
    if (!m_bitmapFile.empty() && (m_pBitmap == nullptr)) {
        //加载指定的图片: 加载图片数据时无优化，仅供测试功能时使用
        ImageDecodeParam decodeParam;
        FilePath resPath = ui::GlobalManager::Instance().GetResourcePath();
        resPath += m_bitmapFile.c_str();
        decodeParam.m_imageFilePath = resPath;
        decodeParam.m_pFileData = std::make_shared<std::vector<uint8_t>>();
        decodeParam.m_fImageSizeScale = Dpi().GetDisplayScale();
        FileUtil::ReadFileData(decodeParam.m_imageFilePath, *decodeParam.m_pFileData);
        std::shared_ptr<IBitmap> pBitmap = GlobalManager::Instance().ImageDecoders().DecodeImageData(decodeParam);
        ASSERT(pBitmap != nullptr);
        if (pBitmap != nullptr) {
            SetBitmapDataWithCopy(pBitmap.get());
        }
    }
}

bool BitmapControl::SetBitmapData(int32_t nWidth, int32_t nHeight, const uint8_t* pPixelBits, int32_t nPixelBitsSize)
{
    ASSERT((pPixelBits != nullptr) && (nPixelBitsSize > 0) && (nWidth > 0) && (nHeight > 0));
    if ((pPixelBits == nullptr) || (nPixelBitsSize <= 0) || (nWidth <= 0) || (nHeight <= 0)) {
        return false;
    }
    ASSERT(nPixelBitsSize == nHeight * nWidth * (int32_t)sizeof(uint32_t));
    if (nPixelBitsSize != nHeight * nWidth * (int32_t)sizeof(uint32_t)) {
        return false;
    }

    //支持多线程时，对m_pBitmap操作前先加锁
    std::unique_ptr<std::unique_lock<std::mutex>> spMutexLock;
    if (IsSupportMultiThread()) {
        spMutexLock = std::make_unique<std::unique_lock<std::mutex>>(m_bitmapMutex);
    }

    if (m_pBitmap == nullptr) {
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            m_pBitmap.reset(pRenderFactory->CreateBitmap());
        }
    }
    ASSERT(m_pBitmap != nullptr);
    if (m_pBitmap == nullptr) {
        return false;
    }
    bool bRet = false;
    if (((int32_t)m_pBitmap->GetWidth() == nWidth) && ((int32_t)m_pBitmap->GetHeight() == nHeight)) {
        void* pBits = m_pBitmap->LockPixelBits();
        if (pBits != nullptr) {
            //复制图片数据到位图
            ::memcpy(pBits, pPixelBits, nWidth * nHeight * sizeof(uint32_t));
            m_pBitmap->UnLockPixelBits();
            bRet = true;
        }
    }
    else {
        bRet = m_pBitmap->Init(nWidth, nHeight, pPixelBits);
    }
    if (bRet) {
        //重绘图片
        Invalidate();
    }
    return bRet;
}

bool BitmapControl::SetBitmapDataWithCopy(IBitmap* pBitmap)
{
    if (pBitmap == nullptr) {
        return false;
    }
    int32_t nWidth = pBitmap->GetWidth();
    int32_t nHeight = pBitmap->GetHeight();
    const uint8_t* pPixelBits = (const uint8_t*)pBitmap->LockPixelBits();
    int32_t nPixelBitsSize = nHeight * nWidth * sizeof(uint32_t);
    return SetBitmapData(nWidth, nHeight, pPixelBits, nPixelBitsSize);
}

void BitmapControl::ClearBitmapData()
{
    //支持多线程时，对m_pBitmap操作前先加锁
    std::unique_ptr<std::unique_lock<std::mutex>> spMutexLock;
    if (IsSupportMultiThread()) {
        spMutexLock = std::make_unique<std::unique_lock<std::mutex>>(m_bitmapMutex);
    }
    if (m_pBitmap != nullptr) {
        m_pBitmap.reset();
        //重绘图片
        Invalidate();
    }    
}

bool BitmapControl::HasBitmapData()
{
    //支持多线程时，对m_pBitmap操作前先加锁
    std::unique_ptr<std::unique_lock<std::mutex>> spMutexLock;
    if (IsSupportMultiThread()) {
        spMutexLock = std::make_unique<std::unique_lock<std::mutex>>(m_bitmapMutex);
    }
    return (m_pBitmap != nullptr) && (m_pBitmap->GetWidth() > 0) && (m_pBitmap->GetHeight() > 0);
}

void BitmapControl::GetBitmapSize(int32_t& nImageWidth, int32_t& nImageHeight)
{
    //支持多线程时，对m_pBitmap操作前先加锁
    std::unique_ptr<std::unique_lock<std::mutex>> spMutexLock;
    if (IsSupportMultiThread()) {
        spMutexLock = std::make_unique<std::unique_lock<std::mutex>>(m_bitmapMutex);
    }
    nImageWidth = 0;
    nImageHeight = 0;
    IBitmap* pBitmap = m_pBitmap.get();
    if (pBitmap != nullptr) {
        nImageWidth = pBitmap->GetWidth();
        nImageHeight = pBitmap->GetHeight();
    }
}

void BitmapControl::PaintBitmap(IRender* pRender, const UiRect& rcPaint)
{
    GlobalManager::Instance().AssertUIThread();

    //按需加载指定的图片
    CheckLoadBitmapFile();

    //统计绘制图片的性能
    PerformanceStat statPerformance(_T("BitmapControl::Paint"));

    //支持多线程时，对m_pBitmap操作前先加锁
    std::unique_ptr<std::unique_lock<std::mutex>> spMutexLock;
    if (IsSupportMultiThread()) {
        spMutexLock = std::make_unique<std::unique_lock<std::mutex>>(m_bitmapMutex);
    }

    int32_t nImageInfoWidth = 0;
    int32_t nImageInfoHeight = 0;
    IBitmap* pBitmap = m_pBitmap.get();
    if (pBitmap != nullptr) {
        nImageInfoWidth = pBitmap->GetWidth();
        nImageInfoHeight = pBitmap->GetHeight();
    }
    if ((nImageInfoWidth <= 0) || (nImageInfoHeight <= 0)) {
        //没有图像数据
        return;
    }

    UiRect rcDest;
    if (m_rcDest != nullptr) {
        //使用配置中指定的目标区域
        rcDest = *m_rcDest;
        if (rcDest.left < 0) {
            rcDest.left = 0;
        }
        if (rcDest.top < 0) {
            rcDest.top = 0;
        }
        if (rcDest.right <= rcDest.left) {
            rcDest.right = rcDest.left + nImageInfoWidth;
        }
        if (rcDest.bottom <= rcDest.top) {
            rcDest.bottom = rcDest.top + nImageInfoHeight;
        }
        rcDest.Offset(GetRect().left, GetRect().top);
    }
    else {
        rcDest = GetRect();
        rcDest.Deflate(GetControlPadding());
    }

    //图片的外边距(剪去)
    if (m_rcMargin != nullptr) {
        rcDest.Deflate(*m_rcMargin);
    }

    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcDest, GetRect())) {
        return;//rcDest与目标区域无交集，无法绘制
    }

    UiRect rcSource;
    if (m_rcSource != nullptr) {
        rcSource = *m_rcSource;        
        if (rcSource.right > (int32_t)m_pBitmap->GetWidth()) {
            rcSource.right = (int32_t)m_pBitmap->GetWidth();
        }
        if (rcSource.bottom > (int32_t)m_pBitmap->GetHeight()) {
            rcSource.bottom = (int32_t)m_pBitmap->GetHeight();
        }
    }
    else {
        rcSource.left = 0;
        rcSource.top = 0;
        rcSource.right = rcSource.left + m_pBitmap->GetWidth();
        rcSource.bottom = rcSource.top + m_pBitmap->GetHeight();
    }

    rcSource.Validate();
    if (rcSource.IsEmpty()) {
        return;//无有效数据区域
    }

    const int32_t nImageWidth = rcSource.Width();
    const int32_t nImageHeight = rcSource.Height();

    bool bAdaptiveDestRect = m_bAdaptiveDestRect; //自动适应目标区域（等比例缩放后，按指定对齐方式绘制）
    bool bStretchedDrawing = m_bStretchedDrawing; //拉伸绘制，其优先级低于bAdaptiveDestRect这个选项
    if (!bAdaptiveDestRect && !bStretchedDrawing) {
        //当图片的大小，大于目标区域大小时，固定设置为自动适应目标区域
        if ((nImageWidth > rcDest.Width()) || (nImageHeight > rcDest.Height())) {
            bAdaptiveDestRect = true;
        }
    }
    if (bAdaptiveDestRect) {
        //自动适应目标区域（等比例缩放图片）：根据图片大小，调整绘制区域
        DString hAlign = _T("left");
        if (m_hAlignType == HorAlignType::kAlignCenter) {
            hAlign = _T("center");
        }
        else if (m_hAlignType == HorAlignType::kAlignRight) {
            hAlign = _T("right");
        }
        DString vAlign = _T("top");
        if (m_vAlignType == VerAlignType::kAlignCenter) {
            vAlign = _T("center");
        }
        else if (m_vAlignType == VerAlignType::kAlignBottom) {
            vAlign = _T("bottom");
        }
        rcDest = ImageAttribute::CalculateAdaptiveRect(nImageWidth, nImageHeight, rcDest, hAlign, vAlign);
    }
    else if (!bStretchedDrawing) {
        //不是拉伸绘制，处理对齐方式
        if (m_hAlignType == HorAlignType::kAlignLeft) {
            rcDest.right = rcDest.left + nImageWidth;
        }
        else if (m_hAlignType == HorAlignType::kAlignCenter) {
            rcDest.left = rcDest.CenterX() - nImageWidth / 2;
            rcDest.right = rcDest.left + nImageWidth;
        }
        else if (m_hAlignType == HorAlignType::kAlignRight) {
            rcDest.left = rcDest.right - nImageWidth;
        }

        if (m_vAlignType == VerAlignType::kAlignTop) {
            rcDest.bottom = rcDest.top + nImageHeight;
        }
        else if (m_vAlignType == VerAlignType::kAlignCenter) {
            rcDest.top = rcDest.CenterY() - nImageHeight / 2;
            rcDest.bottom = rcDest.top + nImageHeight;
        }
        else if (m_vAlignType == VerAlignType::kAlignBottom) {
            rcDest.top = rcDest.bottom - nImageHeight;
        }
    }
    pRender->DrawImage(rcPaint, m_pBitmap.get(), rcDest, rcSource, m_nBitmapAlpha);
}

}//namespace ui
