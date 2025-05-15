#include "ImageAttribute.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui 
{
ImageAttribute::ImageAttribute():
    m_rcDest(nullptr),
    m_rcPadding(nullptr),
    m_rcSource(nullptr),
    m_rcCorner(nullptr)
{
    Init();
}

ImageAttribute::ImageAttribute(const ImageAttribute& r) :
    m_rcDest(nullptr),
    m_rcPadding(nullptr),
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
    m_srcWidth = r.m_srcWidth;
    m_srcHeight = r.m_srcHeight;

    m_srcDpiScale = r.m_srcDpiScale;
    m_bHasSrcDpiScale = r.m_bHasSrcDpiScale;
    m_destDpiScale = r.m_destDpiScale;
    m_bHasDestDpiScale = r.m_bHasDestDpiScale;
    m_rcPaddingScale = r.m_rcPaddingScale;

    m_hAlign = r.m_hAlign;
    m_vAlign = r.m_vAlign;

    m_bFade = r.m_bFade;
    m_bTiledX = r.m_bTiledX;
    m_bFullTiledX = r.m_bFullTiledX;
    m_bTiledY = r.m_bTiledY;
    m_bFullTiledY = r.m_bFullTiledY;
    m_bWindowShadowMode = r.m_bWindowShadowMode;
    m_nTiledMargin = r.m_nTiledMargin;
    m_nPlayCount = r.m_nPlayCount;
    m_iconSize = r.m_iconSize;
    m_bPaintEnabled = r.m_bPaintEnabled;
    m_bAdaptiveDestRect = r.m_bAdaptiveDestRect;

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

    if (r.m_rcPadding != nullptr) {
        if (m_rcPadding == nullptr) {
            m_rcPadding = new UiPadding16;
        }
        *m_rcPadding = *r.m_rcPadding;
    }
    else {
        if (m_rcPadding != nullptr) {
            delete m_rcPadding;
            m_rcPadding = nullptr;
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
    m_srcWidth.clear();
    m_srcHeight.clear();

    m_srcDpiScale = false;
    m_bHasSrcDpiScale = false;
    m_destDpiScale = false;
    m_bHasDestDpiScale = false;
    m_rcPaddingScale = 0;

    m_hAlign.clear();
    m_vAlign.clear();

    m_bFade = 0xFF;
    m_bTiledX = false;
    m_bFullTiledX = true;
    m_bTiledY = false;
    m_bFullTiledY = true;
    m_bWindowShadowMode = false;
    m_nTiledMargin = 0;
    m_nPlayCount = -1;
    m_iconSize = 0;
    m_bPaintEnabled = true;
    m_bAdaptiveDestRect = false;

    if (m_rcDest != nullptr) {
        delete m_rcDest;
        m_rcDest = nullptr;
    }
    if (m_rcSource != nullptr) {
        delete m_rcSource;
        m_rcSource = nullptr;
    }
    if (m_rcPadding != nullptr) {
        delete m_rcPadding;
        m_rcPadding = nullptr;
    }
    if (m_rcCorner != nullptr) {
        delete m_rcCorner;
        m_rcCorner = nullptr;
    }
    m_rcPaddingScale = 0;
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
    if (m_rcPadding != nullptr) {
        delete m_rcPadding;
        m_rcPadding = nullptr;
    }
    if (m_rcCorner != nullptr) {
        delete m_rcCorner;
        m_rcCorner = nullptr;
    }
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
    if (strImageString.find(_T('=')) == DString::npos) {
        //不含有等号，说明没有属性，直接返回
        return;
    }
    std::vector<std::pair<DString, DString>> attributeList;
    AttributeUtil::ParseAttributeList(strImageString, _T('\''), attributeList);

    ImageAttribute& imageAttribute = *this;
    imageAttribute.m_bHasSrcDpiScale = false;
    imageAttribute.m_bHasDestDpiScale = false;
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
        }
        else if (name == _T("corner")) {
            //图片的圆角属性，如果设置此属性，绘制图片的时候，采用九宫格绘制方式绘制图片：
            //    四个角不拉伸图片，四个边部分拉伸，中间部分可以拉伸或者根据xtiled、ytiled属性来平铺绘制
            if (imageAttribute.m_rcCorner == nullptr) {
                imageAttribute.m_rcCorner = new UiRect;
            }
            AttributeUtil::ParseRectValue(value.c_str(), *imageAttribute.m_rcCorner);
        }
        else if ((name == _T("dpi_scale")) || (name == _T("dpiscale"))) {
            //加载图片时，按照DPI缩放图片大小（会影响width属性、height属性、sources属性、corner属性）
            imageAttribute.m_srcDpiScale = (value == _T("true"));
            imageAttribute.m_bHasSrcDpiScale = true;
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
        else if ((name == _T("dest_scale")) || (name == _T("destscale"))) {
            //加载时，对dest属性按照DPI缩放图片，仅当设置了dest属性时有效（会影响dest属性）
            //绘制时（内部使用），控制是否对dest属性进行DPI缩放
            imageAttribute.m_destDpiScale = (value == _T("true"));
            imageAttribute.m_bHasDestDpiScale = true;
        }
        else if (name == _T("padding")) {
            //在目标区域中设置内边距
            UiPadding padding;
            AttributeUtil::ParsePaddingValue(value.c_str(), padding);
            imageAttribute.SetImagePadding(padding, true, dpi);
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
            imageAttribute.m_bTiledX = (value == _T("true"));
        }
        else if ((name == _T("full_xtiled")) || (name == _T("fullxtiled"))) {
            //横向平铺时，保证整张图片绘制
            imageAttribute.m_bFullTiledX = (value == _T("true"));
        }
        else if (name == _T("ytiled")) {
            //纵向平铺
            imageAttribute.m_bTiledY = (value == _T("true"));
        }
        else if ((name == _T("full_ytiled")) || (name == _T("fullytiled"))) {
            //纵向平铺时，保证整张图片绘制
            imageAttribute.m_bFullTiledY = (value == _T("true"));
        }
        else if (name == _T("window_shadow_mode")) {
            //九宫格绘制时，不绘制中间部分（比如窗口阴影，只需要绘制边框，不需要绘制中间部分）
            imageAttribute.m_bWindowShadowMode = (value == _T("true"));
        }
        else if ((name == _T("tiled_margin")) || (name == _T("tiledmargin"))) {
            //平铺绘制时，各平铺图片之间的间隔，包括横向平铺和纵向平铺
            imageAttribute.m_nTiledMargin = StringUtil::StringToInt32(value);
        }
        else if ((name == _T("icon_size")) || (name == _T("iconsize"))) {
            //指定加载ICO文件的图片大小(仅当图片文件是ICO文件时有效)
            imageAttribute.m_iconSize = (uint32_t)StringUtil::StringToInt32(value);
        }
        else if ((name == _T("play_count")) || (name == _T("playcount"))) {
            //如果是GIF、APNG、WEBP等动画图片，可以指定播放次数 -1 ：一直播放，缺省值。
            imageAttribute.m_nPlayCount = StringUtil::StringToInt32(value);
            if (imageAttribute.m_nPlayCount < 0) {
                imageAttribute.m_nPlayCount = -1;
            }
        }
        else if (name == _T("adaptive_dest_rect")) {
            //自动适应目标区域（等比例缩放图片）
            imageAttribute.m_bAdaptiveDestRect = (value == _T("true"));
        }
        else {
            ASSERT(!"ImageAttribute::ModifyAttribute: fount unknown attribute!");
        }
    }
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

void ImageAttribute::ScaleImageRect(uint32_t imageWidth, uint32_t imageHeight, 
                                    const DpiManager& dpi, bool bImageDpiScaled,
                                    UiRect& rcDestCorners,
                                    UiRect& rcSource, UiRect& rcSourceCorners)
{
    ASSERT((imageWidth > 0) && (imageHeight > 0));
    if ((imageWidth == 0) || (imageHeight == 0)) {
        return;
    }
    //对rcImageSourceCorners进行处理：对边角值进行容错处理（四个边代表边距，不代表矩形区域）
    //在XML解析加载的时候，未做DPI自适应；
    //在绘制的时候，如果图片做过DPI自适应，也要做DPI自适应，如果图片未做DPI自适应，也不需要做。    
    if ((rcSourceCorners.left < 0) || (rcSourceCorners.top < 0) ||
        (rcSourceCorners.right < 0)|| (rcSourceCorners.bottom < 0)) {
        rcSourceCorners.Clear();
    }
    else if (bImageDpiScaled) {
        dpi.ScaleRect(rcSourceCorners);
    }

    //对rcDestCorners进行处理：由rcSourceCorners赋值，边角保持一致，避免绘制图片的时候四个角有变形；
    //采用九宫格绘制的时候，四个角的存在，是为了避免绘制的时候四个角出现变形
    rcDestCorners = rcSourceCorners;
    if (!bImageDpiScaled) {
        //rcDestCorners必须做DPI自适应，rcSourceCorners可能不做DPI自适应（根据配置指定，跟随图片）
        dpi.ScaleRect(rcDestCorners);
    }

    // 如果源位图已经按照DPI缩放过，那么对应的rcImageSource也需要缩放
    if ((rcSource.left < 0) || (rcSource.top < 0) ||
        (rcSource.right < 0) || (rcSource.bottom < 0) ||
        (rcSource.Width() <= 0) || (rcSource.Height() <= 0)) {
        //如果是无效值，则重置为整个图片大小
        rcSource.left = 0;
        rcSource.top = 0;
        rcSource.right = (int32_t)imageWidth;
        rcSource.bottom = (int32_t)imageHeight;
    }
    else if (bImageDpiScaled) {
        //如果外部设置此值，做DPI自适应处理
        dpi.ScaleRect(rcSource);
    }

    //图片源容错处理
    if (rcSource.right > (int32_t)imageWidth) {
        rcSource.right = (int32_t)imageWidth;
    }
    if (rcSource.bottom > (int32_t)imageHeight) {
        rcSource.bottom = (int32_t)imageHeight;
    }
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
        if (m_bHasDestDpiScale && !m_destDpiScale) {
            //禁止DPI缩放
        }
        else {
            //应进行DPI缩放
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

UiPadding ImageAttribute::GetImagePadding(const DpiManager& dpi) const
{
    UiPadding rc;
    if (m_rcPadding != nullptr) {
        rc = UiPadding(m_rcPadding->left, m_rcPadding->top, m_rcPadding->right, m_rcPadding->bottom);
        if (m_rcPaddingScale != dpi.GetScale()) {
            rc = dpi.GetScalePadding(rc, m_rcPaddingScale);
        }
    }
    return rc;
}

void ImageAttribute::SetImagePadding(const UiPadding& newPadding, bool bNeedDpiScale, const DpiManager& dpi)
{
    UiPadding rcPaddingDpi = newPadding;
    if (bNeedDpiScale) {
        dpi.ScalePadding(rcPaddingDpi);
    }
    if (m_rcPadding == nullptr) {
        m_rcPadding = new UiPadding16;
    }
    m_rcPadding->left = TruncateToUInt16(newPadding.left);
    m_rcPadding->top = TruncateToUInt16(newPadding.top);
    m_rcPadding->right = TruncateToUInt16(newPadding.right);
    m_rcPadding->bottom = TruncateToUInt16(newPadding.bottom);
    m_rcPaddingScale = TruncateToUInt16(dpi.GetScale());
}

UiRect ImageAttribute::GetImageCorner() const
{
    UiRect rc;
    if (m_rcCorner != nullptr) {
        rc = *m_rcCorner;
    }
    return rc;
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
