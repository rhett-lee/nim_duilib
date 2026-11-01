#include "Avatar.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/UiColors.h"
#include "duilib/Image/Image.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>

namespace ui
{

namespace
{
/** 默认头像尺寸（原始像素）
*/
const int32_t kDefaultAvatarSize = 32;

/** 默认方形圆角（原始像素）
*/
const int32_t kDefaultAvatarCorner = 8;

/** 默认图标边长占控件短边的百分比
*/
const int32_t kDefaultIconScale = 50;

/** 默认字号占控件短边的百分比
*/
const int32_t kDefaultTextScale = 50;

/** 字号自动缩小的下限（占初始字号的百分比）
*/
const int32_t kMinFontZoomPercent = 30;

/** 自动调色板的颜色数量（bg_avatar_1 ~ bg_avatar_6）
*/
const size_t kPaletteColorCount = 6;

/** 解析头像形状（忽略大小写与 '_' / '-'）
*/
AvatarShape ParseAvatarShape(const DString& strValue)
{
    DString strShape = StringUtil::MakeLowerString(strValue);
    StringUtil::ReplaceAll(_T("_"), _T(""), strShape);
    StringUtil::ReplaceAll(_T("-"), _T(""), strShape);
    StringUtil::Trim(strShape);
    if ((strShape == _T("square")) || (strShape == _T("rect")) || (strShape == _T("rounded"))) {
        return AvatarShape::kSquare;
    }
    return AvatarShape::kCircle;
}

/** 解析头像尺寸：支持 small/default/medium/large 预设与数值
*/
int32_t ParseAvatarSize(const DString& strValue)
{
    DString strSize = StringUtil::MakeLowerString(strValue);
    StringUtil::Trim(strSize);
    if (strSize == _T("small")) {
        return 24;
    }
    if ((strSize == _T("default")) || (strSize == _T("medium"))) {
        return kDefaultAvatarSize;
    }
    if (strSize == _T("large")) {
        return 40;
    }
    return StringUtil::StringToInt32(strSize);
}
} // namespace

Avatar::Avatar(Window* pWindow) :
    BaseClass(pWindow),
    m_nSize(kDefaultAvatarSize),
    m_shape(AvatarShape::kCircle),
    m_nCorner(kDefaultAvatarCorner),
    m_strImageString(),
    m_pImage(),
    m_strIconString(),
    m_pIcon(),
    m_nIconScale(kDefaultIconScale),
    m_strText(),
    m_strTextId(),
    m_strFontId(_T("system_regular_14")),
    m_strTextColor(_T("text_avatar")),
    m_nTextScale(kDefaultTextScale),
    m_strColor(),
    m_bAutoPalette(true)
{
    //默认使用固定尺寸，避免父容器按拉伸布局把头像拉成非正方形
    UpdateFixedSize(false);
    SetBorderRound(UiSize(m_nCorner, m_nCorner));
}

Avatar::~Avatar() = default;

DString Avatar::GetType() const
{
    return DUI_CTR_AVATAR;
}

void Avatar::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("size")) {
        SetSize(ParseAvatarSize(strValue));
    }
    else if (strName == _T("shape")) {
        SetShape(ParseAvatarShape(strValue));
    }
    else if (strName == _T("corner")) {
        SetCorner(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("image")) {
        SetImage(strValue);
    }
    else if (strName == _T("icon")) {
        SetIcon(strValue);
    }
    else if ((strName == _T("icon_scale")) || (strName == _T("iconscale"))) {
        SetIconScale(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("text")) {
        SetText(strValue);
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
        SetTextId(strValue);
    }
    else if ((strName == _T("font")) || (strName == _T("font_id")) || (strName == _T("fontid"))) {
        SetFontId(strValue);
    }
    else if ((strName == _T("text_color")) || (strName == _T("textcolor"))) {
        SetTextColor(strValue);
    }
    else if ((strName == _T("text_scale")) || (strName == _T("textscale"))) {
        SetTextScale(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("color")) || (strName == _T("avatar_color"))) {
        SetColor(strValue);
    }
    else if (strName == _T("palette")) {
        SetAutoPalette(StringUtil::IsValueTrue(strValue));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

Image* Avatar::GetEstimateImage()
{
    if ((m_pImage != nullptr) && !m_pImage->GetImagePath().empty()) {
        return m_pImage.get();
    }
    if ((m_pIcon != nullptr) && !m_pIcon->GetImagePath().empty()) {
        return m_pIcon.get();
    }
    return BaseClass::GetEstimateImage();
}

void Avatar::ClearImageCache()
{
    BaseClass::ClearImageCache();
    if (m_pImage != nullptr) {
        m_pImage->ClearImageCache();
    }
    if (m_pIcon != nullptr) {
        m_pIcon->ClearImageCache();
    }
}

void Avatar::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    if (!m_strTextId.empty() && bRedraw) {
        //文字内容随语言变化，需要重绘
        Invalidate();
    }
}

void Avatar::SetSize(int32_t nSize)
{
    if (nSize <= 0) {
        return;
    }
    if (m_nSize == nSize) {
        return;
    }
    m_nSize = nSize;
    UpdateFixedSize(true);
}

void Avatar::SetShape(AvatarShape shape)
{
    if (m_shape == shape) {
        return;
    }
    m_shape = shape;
    Invalidate();
}

void Avatar::SetCorner(int32_t nCorner)
{
    if (nCorner < 0) {
        nCorner = 0;
    }
    if (m_nCorner == nCorner) {
        return;
    }
    m_nCorner = nCorner;
    SetBorderRound(UiSize(m_nCorner, m_nCorner));
    Invalidate();
}

void Avatar::SetImage(const DString& strImage)
{
    if (m_strImageString == strImage) {
        return;
    }
    m_strImageString = strImage;
    if (m_pImage == nullptr) {
        m_pImage = std::make_unique<Image>();
    }
    m_pImage->ClearImageCache();
    m_pImage->SetImageString(strImage, Dpi());
    Invalidate();
}

void Avatar::SetIcon(const DString& strIcon)
{
    if (m_strIconString == strIcon) {
        return;
    }
    m_strIconString = strIcon;
    if (m_pIcon == nullptr) {
        m_pIcon = std::make_unique<Image>();
    }
    m_pIcon->ClearImageCache();
    m_pIcon->SetImageString(strIcon, Dpi());
    Invalidate();
}

void Avatar::SetIconScale(int32_t nIconScale)
{
    if (nIconScale < 1) {
        nIconScale = 1;
    }
    if (nIconScale > 100) {
        nIconScale = 100;
    }
    if (m_nIconScale == nIconScale) {
        return;
    }
    m_nIconScale = nIconScale;
    Invalidate();
}

void Avatar::SetText(const DString& strText)
{
    if (m_strText == strText) {
        return;
    }
    m_strText = strText;
    Invalidate();
}

DString Avatar::GetText() const
{
    if (!m_strText.empty()) {
        return m_strText;
    }
    if (!m_strTextId.empty()) {
        return GlobalManager::Instance().Lang().GetStringByID(m_strTextId);
    }
    return DString();
}

void Avatar::SetTextId(const DString& strTextId)
{
    if (m_strTextId == strTextId) {
        return;
    }
    m_strTextId = strTextId;
    Invalidate();
}

void Avatar::SetFontId(const DString& strFontId)
{
    if (m_strFontId == strFontId) {
        return;
    }
    m_strFontId = strFontId;
    Invalidate();
}

void Avatar::SetTextColor(const DString& strTextColor)
{
    if (m_strTextColor == strTextColor) {
        return;
    }
    m_strTextColor = strTextColor;
    Invalidate();
}

void Avatar::SetTextScale(int32_t nTextScale)
{
    if (nTextScale < 1) {
        nTextScale = 1;
    }
    if (nTextScale > 100) {
        nTextScale = 100;
    }
    if (m_nTextScale == nTextScale) {
        return;
    }
    m_nTextScale = nTextScale;
    Invalidate();
}

void Avatar::SetColor(const DString& strColor)
{
    if (m_strColor == strColor) {
        return;
    }
    m_strColor = strColor;
    Invalidate();
}

void Avatar::SetAutoPalette(bool bAutoPalette)
{
    if (m_bAutoPalette == bAutoPalette) {
        return;
    }
    m_bAutoPalette = bAutoPalette;
    Invalidate();
}

void Avatar::UpdateFixedSize(bool bArrange)
{
    SetFixedWidth(UiFixedInt(m_nSize), bArrange, true);
    SetFixedHeight(UiFixedInt(m_nSize), bArrange, true);
}

void Avatar::GetShapeRound(const UiRect& rc, float& fRoundWidth, float& fRoundHeight) const
{
    fRoundWidth = 0.0f;
    fRoundHeight = 0.0f;
    if (rc.IsEmpty()) {
        return;
    }
    if (m_shape == AvatarShape::kCircle) {
        //圆形：半径随控件矩形实时计算（非正方形区域时自然成为椭圆），不受 DPI 缩放时机影响
        fRoundWidth = (float)rc.Width() / 2.0f;
        fRoundHeight = (float)rc.Height() / 2.0f;
    }
    else {
        float fRound = 0.0f;
        float fRoundY = 0.0f;
        GetBorderRound(fRound, fRoundY);
        fRoundWidth = std::min(fRound, (float)rc.Width() / 2.0f);
        fRoundHeight = std::min(fRoundY, (float)rc.Height() / 2.0f);
        if ((fRoundWidth < 0.5f) || (fRoundHeight < 0.5f)) {
            fRoundWidth = 0.0f;
            fRoundHeight = 0.0f;
        }
    }
}

DString Avatar::GetPaletteColorName() const
{
    if (!m_bAutoPalette) {
        return DString();
    }
    DString strText = GetText();
    if (strText.empty()) {
        return DString();
    }
    size_t nIndex = std::hash<DString>{}(strText) % kPaletteColorCount;
    return StringUtil::Printf(_T("bg_avatar_%d"), (int32_t)(nIndex + 1));
}

UiColor Avatar::GetAvatarBkColor() const
{
    if (!m_strColor.empty()) {
        UiColor color = GetUiColor(m_strColor);
        if (color.GetARGB() != 0) {
            return color;
        }
    }
    DString strBkColor = GetBkColor();
    if (!strBkColor.empty()) {
        UiColor color = GetUiColor(strBkColor);
        if (color.GetARGB() != 0) {
            return color;
        }
    }
    DString strPalette = GetPaletteColorName();
    if (!strPalette.empty()) {
        UiColor color = GetUiColor(strPalette);
        if (color.GetARGB() != 0) {
            return color;
        }
    }
    return GetUiColor(_T("bg_avatar"));
}

DString Avatar::GetStateColorName(ControlStateType stateType) const
{
    //回退顺序与 StateColorMap::PaintStateColor 保持一致
    if ((stateType == kControlStatePressed) && GetStateColor(kControlStatePressed).empty()) {
        stateType = kControlStateHovered;
    }
    if ((stateType == kControlStateHovered) && GetStateColor(kControlStateHovered).empty()) {
        stateType = kControlStateNormal;
    }
    if ((stateType == kControlStateDisabled) && GetStateColor(kControlStateDisabled).empty()) {
        stateType = kControlStateNormal;
    }
    return GetStateColor(stateType);
}

void Avatar::PaintBkColor(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    UiColor bkColor = GetAvatarBkColor();
    if (bkColor.GetARGB() == 0) {
        return;
    }
    float fRoundWidth = 0.0f;
    float fRoundHeight = 0.0f;
    GetShapeRound(rc, fRoundWidth, fRoundHeight);
    //bkcolor2 设置时按渐变填充，否则纯色填充
    UiColor bkColor2;
    DString strBkColor2 = GetBkColor2();
    if (!strBkColor2.empty()) {
        bkColor2 = GetUiColor(strBkColor2);
    }
    if (!bkColor2.IsEmpty()) {
        int8_t nColor2Direction = (int8_t)StringUtil::StringToInt32(GetBkColor2Direction());
        if ((nColor2Direction < 1) || (nColor2Direction > 4)) {
            nColor2Direction = 1;
        }
        pRender->FillRoundRect(UiRectF::MakeFromRect(rc), fRoundWidth, fRoundHeight,
                               bkColor, bkColor2, nColor2Direction, GetAlpha());
    }
    else {
        pRender->FillRoundRect(UiRectF::MakeFromRect(rc), fRoundWidth, fRoundHeight, bkColor, GetAlpha());
    }
}

void Avatar::PaintStateColors(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    if (GetStateColor(kControlStateNormal).empty() &&
        GetStateColor(kControlStateHovered).empty() &&
        GetStateColor(kControlStatePressed).empty() &&
        GetStateColor(kControlStateDisabled).empty()) {
        return;
    }
    DString strColor = GetStateColorName(GetState());
    if (strColor.empty()) {
        return;
    }
    UiColor stateColor = GetUiColor(strColor);
    if (stateColor.GetARGB() == 0) {
        return;
    }
    float fRoundWidth = 0.0f;
    float fRoundHeight = 0.0f;
    GetShapeRound(rc, fRoundWidth, fRoundHeight);
    pRender->FillRoundRect(UiRectF::MakeFromRect(rc), fRoundWidth, fRoundHeight, stateColor, GetAlpha());
}

void Avatar::PaintText(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if (rc.IsEmpty()) {
        return;
    }
    //按头像形状裁剪内容，避免图片的方角溢出
    float fRoundWidth = 0.0f;
    float fRoundHeight = 0.0f;
    GetShapeRound(GetRect(), fRoundWidth, fRoundHeight);
    AutoClip shapeClip(pRender, GetRect(), fRoundWidth, fRoundHeight, true);

    if ((m_pImage != nullptr) && !m_pImage->GetImagePath().empty()) {
        PaintCoverImage(pRender, m_pImage.get(), rc);
    }
    else if ((m_pIcon != nullptr) && !m_pIcon->GetImagePath().empty()) {
        PaintContainImage(pRender, m_pIcon.get(), rc);
    }
    else {
        PaintAvatarText(pRender, rc);
    }
}

void Avatar::PaintCoverImage(IRender* pRender, Image* pImage, const UiRect& rcContent)
{
    if ((pRender == nullptr) || (pImage == nullptr) || rcContent.IsEmpty()) {
        return;
    }
    LoadImageInfo(*pImage);
    const std::shared_ptr<ImageInfo>& imageInfo = pImage->GetImageInfo();
    if ((imageInfo == nullptr) || (imageInfo->GetWidth() <= 0) || (imageInfo->GetHeight() <= 0)) {
        //图片未加载成功（异步加载/解码失败）：交给 PaintImage 处理错误标记与延迟绘制
        PaintImage(pRender, pImage, _T(""), DUI_NOSET_VALUE, nullptr, &rcContent, nullptr);
        return;
    }
    const float fImageWidth = (float)imageInfo->GetWidth();
    const float fImageHeight = (float)imageInfo->GetHeight();
    //cover：等比放大到覆盖整个内容矩形，居中裁切溢出部分
    const float fScale = std::max((float)rcContent.Width() / fImageWidth,
                                  (float)rcContent.Height() / fImageHeight);
    const int32_t nDestWidth = std::max<int32_t>(1, static_cast<int32_t>(fImageWidth * fScale + 0.5f));
    const int32_t nDestHeight = std::max<int32_t>(1, static_cast<int32_t>(fImageHeight * fScale + 0.5f));
    UiRect rcDest(0, 0, nDestWidth, nDestHeight);
    rcDest.Offset(rcContent.left + (rcContent.Width() - nDestWidth) / 2,
                  rcContent.top + (rcContent.Height() - nDestHeight) / 2);
    PaintImage(pRender, pImage, _T(""), DUI_NOSET_VALUE, nullptr, &rcDest, nullptr);
}

void Avatar::PaintContainImage(IRender* pRender, Image* pImage, const UiRect& rcContent)
{
    if ((pRender == nullptr) || (pImage == nullptr) || rcContent.IsEmpty()) {
        return;
    }
    LoadImageInfo(*pImage);
    const std::shared_ptr<ImageInfo>& imageInfo = pImage->GetImageInfo();
    if ((imageInfo == nullptr) || (imageInfo->GetWidth() <= 0) || (imageInfo->GetHeight() <= 0)) {
        PaintImage(pRender, pImage, _T(""), DUI_NOSET_VALUE, nullptr, &rcContent, nullptr);
        return;
    }
    const float fImageWidth = (float)imageInfo->GetWidth();
    const float fImageHeight = (float)imageInfo->GetHeight();
    const int32_t nMaxSide = std::max<int32_t>(1, std::min(rcContent.Width(), rcContent.Height()) * m_nIconScale / 100);
    //contain：等比缩小到完整落在图标方框内
    const float fScale = std::min((float)nMaxSide / fImageWidth, (float)nMaxSide / fImageHeight);
    const int32_t nDestWidth = std::max<int32_t>(1, static_cast<int32_t>(fImageWidth * fScale + 0.5f));
    const int32_t nDestHeight = std::max<int32_t>(1, static_cast<int32_t>(fImageHeight * fScale + 0.5f));
    UiRect rcDest(0, 0, nDestWidth, nDestHeight);
    rcDest.Offset(rcContent.left + (rcContent.Width() - nDestWidth) / 2,
                  rcContent.top + (rcContent.Height() - nDestHeight) / 2);
    PaintImage(pRender, pImage, _T(""), DUI_NOSET_VALUE, nullptr, &rcDest, nullptr);
}

void Avatar::PaintAvatarText(IRender* pRender, const UiRect& rcContent)
{
    if ((pRender == nullptr) || rcContent.IsEmpty()) {
        return;
    }
    DString strText = GetText();
    if (strText.empty()) {
        return;
    }
    const int32_t nShortSide = std::min(rcContent.Width(), rcContent.Height());
    const int32_t nTargetFontSize = std::max<int32_t>(1, nShortSide * m_nTextScale / 100);

    FontManager& fontManager = GlobalManager::Instance().Font();
    IFont* pBaseFont = fontManager.GetIFont(m_strFontId, 100);
    if (pBaseFont == nullptr) {
        return;
    }
    const int32_t nBaseFontSize = std::max<int32_t>(1, pBaseFont->FontSize());
    //目标字号换算为字体缩放百分比，复用同一个 fontId（不新增全局字体 ID，也不受主题字号上限限制）
    uint32_t nZoom = static_cast<uint32_t>(std::max<int32_t>(1, static_cast<int32_t>(
        static_cast<int64_t>(nTargetFontSize) * 100 / nBaseFontSize)));
    const uint32_t nInitialZoom = nZoom;
    IFont* pFont = fontManager.GetIFont(m_strFontId, nZoom);
    if (pFont == nullptr) {
        return;
    }

    //测量后按可用区域等比缩小字号（下限为初始字号的 30%），最后仍超出则由省略号收尾
    MeasureStringParam measureParam;
    measureParam.pFont = pFont;
    measureParam.uFormat = TEXT_SINGLELINE | TEXT_HCENTER | TEXT_VCENTER;
    UiRect rcMeasure = pRender->MeasureString(strText, measureParam);
    if ((rcMeasure.Width() > rcContent.Width()) || (rcMeasure.Height() > rcContent.Height())) {
        double fScale = 1.0;
        if ((rcMeasure.Width() > rcContent.Width()) && (rcMeasure.Width() > 0)) {
            fScale = (double)rcContent.Width() / (double)rcMeasure.Width();
        }
        if ((rcMeasure.Height() > rcContent.Height()) && (rcMeasure.Height() > 0)) {
            fScale = std::min(fScale, (double)rcContent.Height() / (double)rcMeasure.Height());
        }
        uint32_t nNewZoom = static_cast<uint32_t>(std::max<int32_t>(1, static_cast<int32_t>(nZoom * fScale)));
        const uint32_t nMinZoom = std::max<uint32_t>(1, static_cast<uint32_t>(
            static_cast<uint64_t>(nInitialZoom) * kMinFontZoomPercent / 100));
        if (nNewZoom < nMinZoom) {
            nNewZoom = nMinZoom;
        }
        if (nNewZoom < nZoom) {
            nZoom = nNewZoom;
            pFont = fontManager.GetIFont(m_strFontId, nZoom);
            if (pFont == nullptr) {
                return;
            }
        }
    }

    UiColor textColor = GetUiColor(m_strTextColor);
    if (textColor.GetARGB() == 0) {
        //语义色缺失时的兜底色，避免文字完全不可见
        textColor = UiColor(UiColors::Black);
    }
    DrawStringParam drawParam;
    drawParam.textRect = rcContent;
    drawParam.dwTextColor = textColor;
    drawParam.pFont = pFont;
    drawParam.uFormat = TEXT_SINGLELINE | TEXT_HCENTER | TEXT_VCENTER | TEXT_END_ELLIPSIS;
    pRender->DrawString(strText, drawParam);
}

void Avatar::PaintBorder(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    if (m_shape != AvatarShape::kCircle) {
        //方形：交给基类处理 border_round 圆角描边与 border_dash_style
        BaseClass::PaintBorder(pRender);
        return;
    }

    UiColor borderColor;
    if (IsFocused()) {
        DString strFocusedColor = GetFocusedBorderColor();
        if (!strFocusedColor.empty()) {
            borderColor = GetUiColor(strFocusedColor);
        }
    }
    if (borderColor.GetARGB() == 0) {
        DString strBorderColor = GetBorderColor(GetState());
        if (!strBorderColor.empty()) {
            borderColor = GetUiColor(strBorderColor);
        }
    }
    if (borderColor.GetARGB() == 0) {
        return;
    }

    UiRectF borderSize = GetBorderSize();
    const float fWidth = borderSize.left;
    if ((fWidth < 0.1f) ||
        !IsFloatEqual(borderSize.left, borderSize.right) ||
        !IsFloatEqual(borderSize.left, borderSize.top) ||
        !IsFloatEqual(borderSize.left, borderSize.bottom)) {
        //圆形头像只支持四边等宽的边框；非等宽时不做绘制，避免出现方形边线
        return;
    }

    UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    const float fHalf = fWidth / 2.0f;
    UiRectF rcDraw((float)rc.left + fHalf, (float)rc.top + fHalf,
                   (float)rc.right - fHalf, (float)rc.bottom - fHalf);
    if (rcDraw.IsEmpty()) {
        return;
    }
    const float fRoundWidth = rcDraw.Width() / 2.0f;
    const float fRoundHeight = rcDraw.Height() / 2.0f;
    const int8_t nDashStyle = GetBorderDashStyle();
    if (nDashStyle == IPen::DashStyle::kDashStyleSolid) {
        pRender->DrawRoundRect(rcDraw, fRoundWidth, fRoundHeight, borderColor, fWidth);
    }
    else {
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        if (pRenderFactory != nullptr) {
            std::unique_ptr<IPen> pPen(pRenderFactory->CreatePen(borderColor, fWidth));
            pPen->SetDashStyle((IPen::DashStyle)nDashStyle);
            pRender->DrawRoundRect(rcDraw, fRoundWidth, fRoundHeight, pPen.get());
        }
        else {
            pRender->DrawRoundRect(rcDraw, fRoundWidth, fRoundHeight, borderColor, fWidth);
        }
    }
}

} // namespace ui
