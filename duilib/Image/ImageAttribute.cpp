#include "ImageAttribute.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui 
{
ImageAttribute::ImageAttribute():
    m_rcDest(nullptr),
    m_rcMargin(nullptr),
    m_rcSource(nullptr),
    m_rcCorner(nullptr)
{
    Init();
}

ImageAttribute::ImageAttribute(const ImageAttribute& r) :
    m_rcDest(nullptr),
    m_rcMargin(nullptr),
    m_rcSource(nullptr),
    m_rcCorner(nullptr)
{
    Init();
    *this = r;
}

ImageAttribute& ImageAttribute::operator=(const ImageAttribute& r)
{
    if (&r == this) {
        return *this;
    }

    m_sImageString = r.m_sImageString;
    m_sImagePath = r.m_sImagePath;
    m_sImageName = r.m_sImageName;
    m_srcWidth = r.m_srcWidth;
    m_srcHeight = r.m_srcHeight;

    m_bImageDpiScaleEnabled = r.m_bImageDpiScaleEnabled;
    m_bDestDpiScaleEnabled = r.m_bDestDpiScaleEnabled;
    m_rcMarginScale = r.m_rcMarginScale;

    m_hAlign = r.m_hAlign;
    m_vAlign = r.m_vAlign;

    m_bFade = r.m_bFade;
    m_bWindowShadowMode = r.m_bWindowShadowMode;
    m_bAutoPlay = r.m_bAutoPlay;
    m_bAsyncLoad = r.m_bAsyncLoad;
    m_nPlayCount = r.m_nPlayCount;
    m_nIconSize = r.m_nIconSize;
    m_bIconAsAnimation = r.m_bIconAsAnimation;
    m_nIconFrameDelayMs = r.m_nIconFrameDelayMs;
    m_fPagMaxFrameRate = r.m_fPagMaxFrameRate;
    m_bPaintEnabled = r.m_bPaintEnabled;
    m_bAssertEnabled = r.m_bAssertEnabled;
    m_bAdaptiveDestRect = r.m_bAdaptiveDestRect;

    if (r.m_pTiledDrawParam != nullptr) {
        if (m_pTiledDrawParam == nullptr) {
            m_pTiledDrawParam = std::make_unique<TiledDrawParam>();
        }
        *m_pTiledDrawParam = *r.m_pTiledDrawParam;
    }
    else {
        m_pTiledDrawParam.reset();
    }

    if (r.m_rcDest != nullptr) {
        if (m_rcDest == nullptr) {
            m_rcDest = new UiRect;
        }
        *m_rcDest = *r.m_rcDest;
    }
    else {
        if (m_rcDest != nullptr) {
            delete m_rcDest;
            m_rcDest = nullptr;
        }
    }

    if (r.m_rcMargin != nullptr) {
        if (m_rcMargin == nullptr) {
            m_rcMargin = new UiMargin16;
        }
        *m_rcMargin = *r.m_rcMargin;
    }
    else {
        if (m_rcMargin != nullptr) {
            delete m_rcMargin;
            m_rcMargin = nullptr;
        }
    }

    if (r.m_rcSource != nullptr) {
        if (m_rcSource == nullptr) {
            m_rcSource = new UiRect;
        }
        *m_rcSource = *r.m_rcSource;
    }
    else {
        if (m_rcSource != nullptr) {
            delete m_rcSource;
            m_rcSource = nullptr;
        }
    }

    if (r.m_rcCorner != nullptr) {
        if (m_rcCorner == nullptr) {
            m_rcCorner = new UiRect;
        }
        *m_rcCorner = *r.m_rcCorner;
    }
    else {
        if (m_rcCorner != nullptr) {
            delete m_rcCorner;
            m_rcCorner = nullptr;
        }
    }

    return *this;
}

void ImageAttribute::Init()
{
    m_sImageString.clear();
    m_sImagePath.clear();
    m_sImageName.clear();
    m_srcWidth.clear();
    m_srcHeight.clear();

    m_bImageDpiScaleEnabled = true;
    m_bDestDpiScaleEnabled = true;
    m_rcMarginScale = 0;

    m_hAlign.clear();
    m_vAlign.clear();

    m_bFade = 0xFF;
    m_bWindowShadowMode = false;
    m_bAutoPlay = true;
    m_bAsyncLoad = GlobalManager::Instance().Image().IsImageAsyncLoad();
    m_nPlayCount = -1;    
    m_nIconSize = 0;
    m_bIconAsAnimation = false;
    m_nIconFrameDelayMs = 1000;
    m_fPagMaxFrameRate = 30.0f;
    m_bPaintEnabled = true;
    m_bAssertEnabled = true;
    m_bAdaptiveDestRect = false;
    m_pTiledDrawParam.reset();

    if (m_rcDest != nullptr) {
        delete m_rcDest;
        m_rcDest = nullptr;
    }
    if (m_rcSource != nullptr) {
        delete m_rcSource;
        m_rcSource = nullptr;
    }
    if (m_rcMargin != nullptr) {
        delete m_rcMargin;
        m_rcMargin = nullptr;
    }
    if (m_rcCorner != nullptr) {
        delete m_rcCorner;
        m_rcCorner = nullptr;
    }
    m_rcMarginScale = 0;
}

ImageAttribute::~ImageAttribute()
{
    if (m_rcDest != nullptr) {
        delete m_rcDest;
        m_rcDest = nullptr;
    }
    if (m_rcSource != nullptr) {
        delete m_rcSource;
        m_rcSource = nullptr;
    }
    if (m_rcMargin != nullptr) {
        delete m_rcMargin;
        m_rcMargin = nullptr;
    }
    if (m_rcCorner != nullptr) {
        delete m_rcCorner;
        m_rcCorner = nullptr;
    }
    m_pTiledDrawParam.reset();
}

void ImageAttribute::InitByImageString(const DString& strImageString, const DpiManager& dpi)
{
    Init();
    m_sImageString = strImageString;
    m_sImagePath = strImageString;
    ModifyAttribute(strImageString, dpi);
}

void ImageAttribute::ModifyAttribute(const DString& strImageString, const DpiManager& dpi)
{
    // 说明：图片的属性说明文档(docs/Global.md)中有对各个属性的详细描述
    if (strImageString.find(_T('=')) == DString::npos) {
        //不含有等号，说明没有属性，直接返回
        return;
    }
    std::vector<std::pair<DString, DString>> attributeList;
    AttributeUtil::ParseAttributeList(strImageString, _T('\''), attributeList);

    ImageAttribute& imageAttribute = *this;
    imageAttribute.m_bImageDpiScaleEnabled = true;
    imageAttribute.m_bDestDpiScaleEnabled = true;
    for (const auto& attribute : attributeList) {
        const DString& name = attribute.first;
        const DString& value = attribute.second;
        if (name.empty() || value.empty()) {
            continue;
        }
        if (name == _T("file") || name == _T("res")) {
            //图片资源文件名，根据此设置去加载图片资源
            imageAttribute.m_sImagePath = value;
        }
        else if (name == _T("name")) {
            //图片资源名称
            imageAttribute.m_sImageName = value;
        }
        else if (name == _T("width")) {
            //设置图片宽度，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
            imageAttribute.m_srcWidth = value;
        }
        else if (name == _T("height")) {
            //设置图片高度，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
            imageAttribute.m_srcHeight = value;
        }
        else if ((name == _T("src")) || (name == _T("source"))) {
            //图片源区域设置：可以用于仅包含源图片的部分图片内容（比如通过此机制，将按钮的各个状态图片整合到一张大图片上，方便管理图片资源）
            if (imageAttribute.m_rcSource == nullptr) {
                imageAttribute.m_rcSource = new UiRect;
            }
            AttributeUtil::ParseRectValue(value.c_str(), *imageAttribute.m_rcSource);
            imageAttribute.m_rcSource->left = std::max(imageAttribute.m_rcSource->left, 0);
            imageAttribute.m_rcSource->top = std::max(imageAttribute.m_rcSource->top, 0);
        }
        else if (name == _T("corner")) {
            //图片的圆角属性，如果设置此属性，绘制图片的时候，采用九宫格绘制方式绘制图片：
            //    四个角不拉伸图片，四个边部分拉伸，中间部分可以拉伸或者根据xtiled、ytiled属性来平铺绘制
            if (imageAttribute.m_rcCorner == nullptr) {
                imageAttribute.m_rcCorner = new UiRect;
            }
            AttributeUtil::ParseRectValue(value.c_str(), *imageAttribute.m_rcCorner);
            imageAttribute.m_rcCorner->left = std::max(imageAttribute.m_rcCorner->left, 0);
            imageAttribute.m_rcCorner->top = std::max(imageAttribute.m_rcCorner->top, 0);
            imageAttribute.m_rcCorner->right = std::max(imageAttribute.m_rcCorner->right, 0);
            imageAttribute.m_rcCorner->bottom = std::max(imageAttribute.m_rcCorner->bottom, 0);
        }
        else if (name == _T("window_shadow_mode")) {
            //九宫格绘制时，不绘制中间部分（比如窗口阴影，只需要绘制边框，不需要绘制中间部分）
            imageAttribute.m_bWindowShadowMode = (value == _T("true"));
        }
        else if ((name == _T("dpi_scale")) || (name == _T("dpiscale"))) {
            //加载图片时，按照DPI缩放图片大小
            imageAttribute.m_bImageDpiScaleEnabled = (value == _T("true"));
        }
        else if ((name == _T("dest_scale")) || (name == _T("destscale"))) {
            //加载时，对dest属性按照DPI缩放图片，仅当设置了dest属性时有效（会影响dest属性）
            //绘制时（内部使用），控制是否对dest属性进行DPI缩放
            imageAttribute.m_bDestDpiScaleEnabled = (value == _T("true"));
        }
        else if (name == _T("dest")) {
            //设置目标区域，该区域是指相对于所属控件的Rect区域
            if (!value.empty()) {
                if (imageAttribute.m_rcDest == nullptr) {
                    imageAttribute.m_rcDest = new UiRect;
                }
                UiRect& rect = *imageAttribute.m_rcDest;
                DString::value_type* pstr = nullptr;
                rect.left = StringUtil::StringToInt32(value.c_str(), &pstr, 10); ASSERT(pstr);
                AttributeUtil::SkipSepChar(pstr);
                if (*pstr != _T('\0')) {
                    rect.top = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
                    AttributeUtil::SkipSepChar(pstr);
                }
                if (*pstr != _T('\0')) {
                    rect.right = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
                    AttributeUtil::SkipSepChar(pstr);
                }
                if (*pstr != _T('\0')) {
                    rect.bottom = StringUtil::StringToInt32(pstr, &pstr, 10); ASSERT(pstr);
                }
            }
        }
        else if ((name == _T("margin") || (name == _T("padding")))) {
            //在目标区域中设置图片的外边距(旧的名字"padding"，保留兼容性)
            UiMargin margin;
            AttributeUtil::ParseMarginValue(value.c_str(), margin);
            imageAttribute.SetImageMargin(margin, true, dpi);
        }
        else if (name == _T("halign")) {
            //在目标区域中设置横向对齐方式            
            ASSERT((value == _T("left")) || (value == _T("center")) || (value == _T("right")));
            if ((value == _T("left")) || (value == _T("center")) || (value == _T("right"))) {
                imageAttribute.m_hAlign = value;
            }
        }
        else if (name == _T("valign")) {
            //在目标区域中设置纵向对齐方式
            ASSERT((value == _T("top")) || (value == _T("center")) || (value == _T("bottom")));
            if ((value == _T("top")) || (value == _T("center")) || (value == _T("bottom"))) {
                imageAttribute.m_vAlign = value;
            }
        }
        else if (name == _T("fade")) {
            //图片的透明度
            imageAttribute.m_bFade = (uint8_t)StringUtil::StringToInt32(value);
        }
        else if (name == _T("xtiled")) {
            //横向平铺
            if (m_pTiledDrawParam == nullptr) {
                m_pTiledDrawParam = std::make_unique<TiledDrawParam>();
            }
            imageAttribute.m_pTiledDrawParam->m_bTiledX = (value == _T("true"));
        }
        else if ((name == _T("full_xtiled")) || (name == _T("fullxtiled"))) {
            //横向平铺时，保证整张图片绘制
            if (m_pTiledDrawParam == nullptr) {
                m_pTiledDrawParam = std::make_unique<TiledDrawParam>();
            }
            imageAttribute.m_pTiledDrawParam->m_bFullTiledX = (value == _T("true"));
        }
        else if (name == _T("ytiled")) {
            //纵向平铺
            if (m_pTiledDrawParam == nullptr) {
                m_pTiledDrawParam = std::make_unique<TiledDrawParam>();
            }
            imageAttribute.m_pTiledDrawParam->m_bTiledY = (value == _T("true"));
        }
        else if ((name == _T("full_ytiled")) || (name == _T("fullytiled"))) {
            //纵向平铺时，保证整张图片绘制
            if (m_pTiledDrawParam == nullptr) {
                m_pTiledDrawParam = std::make_unique<TiledDrawParam>();
            }
            imageAttribute.m_pTiledDrawParam->m_bFullTiledY = (value == _T("true"));
        }
        else if ((name == _T("tiled_margin")) || (name == _T("tiledmargin"))) {
            //平铺绘制时，各平铺图片之间的间隔，包括横向平铺和纵向平铺
            if (m_pTiledDrawParam == nullptr) {
                m_pTiledDrawParam = std::make_unique<TiledDrawParam>();
            }
            imageAttribute.m_pTiledDrawParam->m_nTiledMarginX = StringUtil::StringToInt32(value);
            imageAttribute.m_pTiledDrawParam->m_nTiledMarginY = imageAttribute.m_pTiledDrawParam->m_nTiledMarginX;
        }
        else if (name == _T("tiled_margin_x")) {
            //平铺绘制时，各平铺图片之间的间隔，横向平铺
            if (m_pTiledDrawParam == nullptr) {
                m_pTiledDrawParam = std::make_unique<TiledDrawParam>();
            }
            imageAttribute.m_pTiledDrawParam->m_nTiledMarginX = StringUtil::StringToInt32(value);
        }
        else if (name == _T("tiled_margin_y")) {
            //平铺绘制时，各平铺图片之间的间隔，纵向平铺
            if (m_pTiledDrawParam == nullptr) {
                m_pTiledDrawParam = std::make_unique<TiledDrawParam>();
            }
            imageAttribute.m_pTiledDrawParam->m_nTiledMarginY = StringUtil::StringToInt32(value);
        }
        else if (name == _T("tiled_padding")) {
            if (m_pTiledDrawParam == nullptr) {
                m_pTiledDrawParam = std::make_unique<TiledDrawParam>();
            }
            UiPadding rcPadding;
            AttributeUtil::ParsePaddingValue(value.c_str(), rcPadding);
            m_pTiledDrawParam->m_rcTiledPadding = rcPadding;
        }
        else if ((name == _T("icon_size")) || (name == _T("iconsize"))) {
            //指定加载ICO文件的图片大小(仅当图片文件是ICO文件时有效)
            imageAttribute.m_nIconSize = (uint32_t)StringUtil::StringToInt32(value);
        }
        else if (name == _T("icon_as_animation")) {
            //如果是ICO文件，指定是否按多帧图片加载（按动画图片显示）
            imageAttribute.m_bIconAsAnimation = (value == _T("true"));
        }
        else if (name == _T("icon_frame_delay")) {
            //如果是ICO文件，当按多帧图片显示时，每帧播放的时间间隔，毫秒
            imageAttribute.m_nIconFrameDelayMs = StringUtil::StringToInt32(value);
            if (imageAttribute.m_nIconFrameDelayMs <= 0) {
                imageAttribute.m_nIconFrameDelayMs = 1000;
            }
        }
        else if (name == _T("pag_max_frame_rate")) {
            //如果是PAG文件，用于指定动画的帧率，默认为30.0f
            imageAttribute.m_fPagMaxFrameRate = (float)StringUtil::StringToInt32(value);
        }
        else if ((name == _T("play_count")) || (name == _T("playcount"))) {
            //如果是动画图片，取值代表的含义
            //  -1: 表示一直播放
            //  0 : 表示无有效的播放次数，使用图片的默认值
            // > 0: 具体的播放次数，达到播放次数后，停止播放
            imageAttribute.m_nPlayCount = StringUtil::StringToInt32(value);
            if (imageAttribute.m_nPlayCount < 0) {
                imageAttribute.m_nPlayCount = -1;
            }
        }
        else if (name == _T("auto_play")) {
            //如果是动画图片，是否自动播放
            imageAttribute.m_bAutoPlay = (value == _T("true"));
        }
        else if (name == _T("async_load")) {
            //该图片是否支持异步加载（即放在子线程中加载图片数据，避免主界面卡顿）
            imageAttribute.m_bAsyncLoad = (value == _T("true"));
        }
        else if (name == _T("adaptive_dest_rect")) {
            //自动适应目标区域（等比例缩放图片）
            imageAttribute.m_bAdaptiveDestRect = (value == _T("true"));
        }
        else if (name == _T("assert")) {
            //图片加载失败时，代码断言的设置（debug编译时启用，用于排查图片加载过程中的错误，尤其时图片数据错误导致加载失败的问题）
            imageAttribute.m_bAssertEnabled = (value == _T("true"));
        }
        else {
            ASSERT(!"ImageAttribute::ModifyAttribute: fount unknown attribute!");
        }
    }
}

bool ImageAttribute::IsAssertEnabled() const
{
    return m_bAssertEnabled;
}

DString ImageAttribute::GetImageName() const
{
    return m_sImageName.c_str();
}

bool ImageAttribute::HasValidImageRect(const UiRect& rcDest)
{
    if (rcDest.IsZero() || rcDest.IsEmpty()) {
        return false;
    }
    if ((rcDest.Width() > 0) && (rcDest.Height() > 0)) {
        return true;
    }
    return false;
}

UiRect ImageAttribute::GetImageSourceRect() const
{
    UiRect rc;
    if (m_rcSource != nullptr) {
        rc = *m_rcSource;
    }
    return rc;
}

UiRect ImageAttribute::GetImageDestRect(int32_t imageWidth, int32_t imageHeight, const DpiManager& dpi) const
{
    UiRect rc;
    if (m_rcDest != nullptr) {
        rc = *m_rcDest;
        if (m_bDestDpiScaleEnabled) {
            //rcDest应进行DPI缩放
            dpi.ScaleRect(rc);
        }
        //如果未指定完整的区域，则自动计算该区域(允许只设置left,top，不设置right和bottom)
        if (rc.right <= rc.left) {
            if (imageWidth > 0) {
                rc.right = rc.left + imageWidth;
            }
        }
        if (rc.bottom <= rc.top) {
            if (imageHeight > 0) {
                rc.bottom = rc.top + imageHeight;
            }
        }
    }
    return rc;
}

bool ImageAttribute::HasDestRect() const
{
    return m_rcDest != nullptr;
}

UiMargin ImageAttribute::GetImageMargin(const DpiManager& dpi) const
{
    UiMargin rc;
    if (m_rcMargin != nullptr) {
        rc = UiMargin(m_rcMargin->left, m_rcMargin->top, m_rcMargin->right, m_rcMargin->bottom);
        if (m_rcMarginScale != dpi.GetDisplayScaleFactor()) {
            rc = dpi.GetScaleMargin(rc, m_rcMarginScale);
        }
    }
    return rc;
}

void ImageAttribute::SetImageMargin(const UiMargin& newMargin, bool bNeedDpiScale, const DpiManager& dpi)
{
    UiMargin rcMarginDpi = newMargin;
    if (bNeedDpiScale) {
        dpi.ScaleMargin(rcMarginDpi);
    }
    if (m_rcMargin == nullptr) {
        m_rcMargin = new UiMargin16;
    }
    m_rcMargin->left = TruncateToUInt16(rcMarginDpi.left);
    m_rcMargin->top = TruncateToUInt16(rcMarginDpi.top);
    m_rcMargin->right = TruncateToUInt16(rcMarginDpi.right);
    m_rcMargin->bottom = TruncateToUInt16(rcMarginDpi.bottom);
    m_rcMarginScale = TruncateToUInt16(dpi.GetDisplayScaleFactor());
}

bool ImageAttribute::IsTiledDraw() const
{
    if (m_pTiledDrawParam != nullptr) {
        return m_pTiledDrawParam->m_bTiledX || m_pTiledDrawParam->m_bTiledY;
    }
    return false;
}

TiledDrawParam ImageAttribute::GetTiledDrawParam(const DpiManager& dpi) const
{
    TiledDrawParam tiledDrawParam;
    if (m_pTiledDrawParam != nullptr) {
        tiledDrawParam = *m_pTiledDrawParam;
        //对部分值做DPI缩放
        dpi.ScaleInt(tiledDrawParam.m_nTiledMarginX);
        dpi.ScaleInt(tiledDrawParam.m_nTiledMarginY);
        dpi.ScalePadding(tiledDrawParam.m_rcTiledPadding);
    }
    return tiledDrawParam;
}

UiRect ImageAttribute::GetImageCorner() const
{
    UiRect rc;
    if (m_rcCorner != nullptr) {
        rc = *m_rcCorner;
    }
    return rc;
}

bool ImageAttribute::HasImageCorner() const
{
    UiRect rcCorner = GetImageCorner();
    return (rcCorner.left > 0) || (rcCorner.top > 0) || (rcCorner.right > 0) || (rcCorner.bottom > 0);
}

/** 计算保持比例的自适应目标区域大小
 * @param nImageWidth 原始图片宽度
 * @param nImageHeight 原始图片高度
 * @param targetSize 目标区域大小(width, height)
 * @return 自适应后的新大小(width, height)
 */
static UiSize CalculateAdaptiveSize(int32_t nImageWidth, int32_t nImageHeight, const UiSize& targetSize)
{
    if ((nImageWidth <= 0) || (nImageHeight <= 0) ||
        (targetSize.cx <= 0) || (targetSize.cy <= 0)) {
        return UiSize();
    }

    float imageRatio = static_cast<float>(nImageWidth) / nImageHeight;
    float targetRatio = static_cast<float>(targetSize.cx) / targetSize.cy;

    int32_t newWidth = targetSize.cx;
    int32_t newHeight = targetSize.cy;

    if (imageRatio > targetRatio) {
        // 以宽度为准，高度按比例缩放
        newHeight = static_cast<int32_t>(targetSize.cx / imageRatio);
    }
    else {
        // 以高度为准，宽度按比例缩放
        newWidth = static_cast<int32_t>(targetSize.cy * imageRatio);
    }

    return UiSize(newWidth, newHeight);
}

UiRect ImageAttribute::CalculateAdaptiveRect(int32_t nImageWidth, int32_t nImageHeight,
                                             const UiRect& targetRect,
                                             const DString& hAlign,
                                             const DString& vAlign)
{
    int32_t targetWidth = targetRect.Width();
    int32_t targetHeight = targetRect.Height();

    UiSize newSize = CalculateAdaptiveSize(nImageWidth, nImageHeight, UiSize(targetWidth, targetHeight));

    // 计算横向位置
    int32_t newLeft = targetRect.left;
    if (hAlign == _T("center")) {
        newLeft = targetRect.left + (targetWidth - newSize.cx) / 2;
    }
    else if (hAlign == _T("right")) {
        newLeft = targetRect.left + targetWidth - newSize.cx;
    }
    // LEFT对齐不需要调整

    // 计算纵向位置
    int32_t newTop = targetRect.top;
    if (vAlign == _T("center")) {
        newTop = targetRect.top + (targetHeight - newSize.cy) / 2;
    }
    else if (vAlign == _T("bottom")) {
        newTop = targetRect.top + targetHeight - newSize.cy;
    }
    // TOP对齐不需要调整

    int32_t newRight = newLeft + newSize.cx;
    int32_t newBottom = newTop + newSize.cy;
    return UiRect(newLeft, newTop, newRight, newBottom);
}

}
