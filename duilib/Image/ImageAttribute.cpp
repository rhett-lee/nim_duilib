#include "ImageAttribute.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui 
{
ImageAttribute::ImageAttribute():
    rcDest(nullptr),
    rcPadding(nullptr),
    rcSource(nullptr),
    rcCorner(nullptr)
{
    Init();
}

ImageAttribute::ImageAttribute(const ImageAttribute& r) :
    rcDest(nullptr),
    rcPadding(nullptr),
    rcSource(nullptr),
    rcCorner(nullptr)
{
    Init();
    *this = r;
}

ImageAttribute& ImageAttribute::operator=(const ImageAttribute& r)
{
    if (&r == this) {
        return *this;
    }

    sImageString = r.sImageString;
    sImagePath = r.sImagePath;
    srcWidth = r.srcWidth;
    srcHeight = r.srcHeight;

    srcDpiScale = r.srcDpiScale;
    bHasSrcDpiScale = r.bHasSrcDpiScale;
    destDpiScale = r.destDpiScale;
    bHasDestDpiScale = r.bHasDestDpiScale;
    rcPaddingScale = r.rcPaddingScale;

    hAlign = r.hAlign;
    vAlign = r.vAlign;

    bFade = r.bFade;
    bTiledX = r.bTiledX;
    bFullTiledX = r.bFullTiledX;
    bTiledY = r.bTiledY;
    bFullTiledY = r.bFullTiledY;
    nTiledMargin = r.nTiledMargin;
    nPlayCount = r.nPlayCount;
    iconSize = r.iconSize;
    bPaintEnabled = r.bPaintEnabled;

    if (r.rcDest != nullptr) {
        if (rcDest == nullptr) {
            rcDest = new UiRect;
        }
        *rcDest = *r.rcDest;
    }
    else {
        if (rcDest != nullptr) {
            delete rcDest;
            rcDest = nullptr;
        }
    }

    if (r.rcPadding != nullptr) {
        if (rcPadding == nullptr) {
            rcPadding = new UiPadding16;
        }
        *rcPadding = *r.rcPadding;
    }
    else {
        if (rcPadding != nullptr) {
            delete rcPadding;
            rcPadding = nullptr;
        }
    }

    if (r.rcSource != nullptr) {
        if (rcSource == nullptr) {
            rcSource = new UiRect;
        }
        *rcSource = *r.rcSource;
    }
    else {
        if (rcSource != nullptr) {
            delete rcSource;
            rcSource = nullptr;
        }
    }

    if (r.rcCorner != nullptr) {
        if (rcCorner == nullptr) {
            rcCorner = new UiRect;
        }
        *rcCorner = *r.rcCorner;
    }
    else {
        if (rcCorner != nullptr) {
            delete rcCorner;
            rcCorner = nullptr;
        }
    }

    return *this;
}

void ImageAttribute::Init()
{
    sImageString.clear();
    sImagePath.clear(); 
    srcWidth.clear();
    srcHeight.clear();

    srcDpiScale = false;
    bHasSrcDpiScale = false;
    destDpiScale = false;
    bHasDestDpiScale = false;
    rcPaddingScale = 0;

    hAlign.clear();
    vAlign.clear();

    bFade = 0xFF;
    bTiledX = false;
    bFullTiledX = true;
    bTiledY = false;
    bFullTiledY = true;
    nTiledMargin = 0;
    nPlayCount = -1;
    iconSize = 0;
    bPaintEnabled = true;

    if (rcDest != nullptr) {
        delete rcDest;
        rcDest = nullptr;
    }
    if (rcSource != nullptr) {
        delete rcSource;
        rcSource = nullptr;
    }
    if (rcPadding != nullptr) {
        delete rcPadding;
        rcPadding = nullptr;
    }
    if (rcCorner != nullptr) {
        delete rcCorner;
        rcCorner = nullptr;
    }
    rcPaddingScale = 0;
}

ImageAttribute::~ImageAttribute()
{
    if (rcDest != nullptr) {
        delete rcDest;
        rcDest = nullptr;
    }
    if (rcSource != nullptr) {
        delete rcSource;
        rcSource = nullptr;
    }
    if (rcPadding != nullptr) {
        delete rcPadding;
        rcPadding = nullptr;
    }
    if (rcCorner != nullptr) {
        delete rcCorner;
        rcCorner = nullptr;
    }
}

void ImageAttribute::InitByImageString(const DString& strImageString, const DpiManager& dpi)
{
    Init();
    sImageString = strImageString;
    sImagePath = strImageString;
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
    imageAttribute.bHasSrcDpiScale = false;
    imageAttribute.bHasDestDpiScale = false;
    for (const auto& attribute : attributeList) {
        const DString& name = attribute.first;
        const DString& value = attribute.second;
        if (name.empty() || value.empty()) {
            continue;
        }
        if (name == _T("file") || name == _T("res")) {
            //图片资源文件名，根据此设置去加载图片资源
            imageAttribute.sImagePath = value;
        }
        else if (name == _T("width")) {
            //设置图片宽度，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
            imageAttribute.srcWidth = value;
        }
        else if (name == _T("height")) {
            //设置图片高度，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
            imageAttribute.srcHeight = value;
        }
        else if (name == _T("source")) {
            //图片源区域设置：可以用于仅包含源图片的部分图片内容（比如通过此机制，将按钮的各个状态图片整合到一张大图片上，方便管理图片资源）
            if (imageAttribute.rcSource == nullptr) {
                imageAttribute.rcSource = new UiRect;
            }
            AttributeUtil::ParseRectValue(value.c_str(), *imageAttribute.rcSource);
        }
        else if (name == _T("corner")) {
            //图片的圆角属性，如果设置此属性，绘制图片的时候，采用九宫格绘制方式绘制图片：
            //    四个角不拉伸图片，四个边部分拉伸，中间部分可以拉伸或者根据xtiled、ytiled属性来平铺绘制
            if (imageAttribute.rcCorner == nullptr) {
                imageAttribute.rcCorner = new UiRect;
            }
            AttributeUtil::ParseRectValue(value.c_str(), *imageAttribute.rcCorner);
        }
        else if ((name == _T("dpi_scale")) || (name == _T("dpiscale"))) {
            //加载图片时，按照DPI缩放图片大小（会影响width属性、height属性、sources属性、corner属性）
            imageAttribute.srcDpiScale = (value == _T("true"));
            imageAttribute.bHasSrcDpiScale = true;
        }
        else if (name == _T("dest")) {
            //设置目标区域，该区域是指相对于所属控件的Rect区域
            if (imageAttribute.rcDest == nullptr) {
                imageAttribute.rcDest = new UiRect;
            }
            AttributeUtil::ParseRectValue(value.c_str(), *imageAttribute.rcDest);
        }
        else if ((name == _T("dest_scale")) || (name == _T("destscale"))) {
            //加载时，对dest属性按照DPI缩放图片，仅当设置了dest属性时有效（会影响dest属性）
            //绘制时（内部使用），控制是否对dest属性进行DPI缩放
            imageAttribute.destDpiScale = (value == _T("true"));
            imageAttribute.bHasDestDpiScale = true;
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
                imageAttribute.hAlign = value;
            }
        }
        else if (name == _T("valign")) {
            //在目标区域中设置纵向对齐方式
            ASSERT((value == _T("top")) || (value == _T("center")) || (value == _T("bottom")));
            if ((value == _T("top")) || (value == _T("center")) || (value == _T("bottom"))) {
                imageAttribute.vAlign = value;
            }
        }
        else if (name == _T("fade")) {
            //图片的透明度
            imageAttribute.bFade = (uint8_t)StringUtil::StringToInt32(value);
        }
        else if (name == _T("xtiled")) {
            //横向平铺
            imageAttribute.bTiledX = (value == _T("true"));
        }
        else if ((name == _T("full_xtiled")) || (name == _T("fullxtiled"))) {
            //横向平铺时，保证整张图片绘制
            imageAttribute.bFullTiledX = (value == _T("true"));
        }
        else if (name == _T("ytiled")) {
            //纵向平铺
            imageAttribute.bTiledY = (value == _T("true"));
        }
        else if ((name == _T("full_ytiled")) || (name == _T("fullytiled"))) {
            //纵向平铺时，保证整张图片绘制
            imageAttribute.bFullTiledY = (value == _T("true"));
        }
        else if ((name == _T("tiled_margin")) || (name == _T("tiledmargin"))) {
            //平铺绘制时，各平铺图片之间的间隔，包括横向平铺和纵向平铺
            imageAttribute.nTiledMargin = StringUtil::StringToInt32(value);
        }
        else if ((name == _T("icon_size")) || (name == _T("iconsize"))) {
            //指定加载ICO文件的图片大小(仅当图片文件是ICO文件时有效)
            imageAttribute.iconSize = (uint32_t)StringUtil::StringToInt32(value);
        }
        else if ((name == _T("play_count")) || (name == _T("playcount"))) {
            //如果是GIF、APNG、WEBP等动画图片，可以指定播放次数 -1 ：一直播放，缺省值。
            imageAttribute.nPlayCount = StringUtil::StringToInt32(value);
            if (imageAttribute.nPlayCount < 0) {
                imageAttribute.nPlayCount = -1;
            }
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
    if ((rcDest.left >= 0) && (rcDest.top >= 0) && 
        (rcDest.Width() > 0) && (rcDest.Height() > 0)){
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
    if (rcSource != nullptr) {
        rc = *rcSource;
    }
    return rc;
}

UiRect ImageAttribute::GetImageDestRect(const DpiManager& dpi) const
{
    UiRect rc;
    if (rcDest != nullptr) {
        rc = *rcDest;
        if (bHasDestDpiScale && !destDpiScale) {
            //禁止DPI缩放
        }
        else {
            //应进行DPI缩放
            dpi.ScaleRect(rc);
        }
    }
    return rc;
}

UiPadding ImageAttribute::GetImagePadding(const DpiManager& dpi) const
{
    UiPadding rc;
    if (rcPadding != nullptr) {
        rc = UiPadding(rcPadding->left, rcPadding->top, rcPadding->right, rcPadding->bottom);
        if (rcPaddingScale != dpi.GetScale()) {
            rc = dpi.GetScalePadding(rc, rcPaddingScale);
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
    if (rcPadding == nullptr) {
        rcPadding = new UiPadding16;
    }
    rcPadding->left = TruncateToUInt16(newPadding.left);
    rcPadding->top = TruncateToUInt16(newPadding.top);
    rcPadding->right = TruncateToUInt16(newPadding.right);
    rcPadding->bottom = TruncateToUInt16(newPadding.bottom);
    rcPaddingScale = TruncateToUInt16(dpi.GetScale());
}

UiRect ImageAttribute::GetImageCorner() const
{
    UiRect rc;
    if (rcCorner != nullptr) {
        rc = *rcCorner;
    }
    return rc;
}

}
