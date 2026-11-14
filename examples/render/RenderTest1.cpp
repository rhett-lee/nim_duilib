#include "RenderTest1.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Utils/StringConvert.h"

#if defined (DUILIB_BUILD_FOR_WIN)
    #include "duilib/Utils/BitmapHelper_Windows.h"
#elif defined(DUILIB_BUILD_FOR_SDL)
    #include "duilib/Utils/BitmapHelper_SDL.h"
#endif

namespace ui {

namespace {

/** 单元格总数（需要与 GetCellType 的实现保持一致）
*/
constexpr int32_t kCellCount = 14;

/** 单元格列数
*/
constexpr int32_t kColumns = 4;

/** 单元格标题的高度（逻辑像素）
*/
constexpr int32_t kCaptionHeight = 34;

/** 将标题文本拆分为两行显示（按可用宽度自动选择拆分点）
* @param [in] text 标题文本
* @param [in] fMaxWidth 单行可用的最大宽度
* @param [in] pFont 字体接口
* @param [out] line1 第一行文本
* @param [out] line2 第二行文本，无法拆分时为空
*/
void SplitCaption(const DString& text, float fMaxWidth, IFont* pFont, DString& line1, DString& line2)
{
    line1 = text;
    line2.clear();
    if ((pFont == nullptr) || (fMaxWidth <= 0) || (text.size() < 2)) {
        return;
    }
    if (pFont->MeasureTextWidth(text) <= fMaxWidth) {
        //一行可以完整显示，不需要拆分
        return;
    }
    //收集可用的拆分点：空格和左括号
    std::vector<size_t> candidates;
    for (size_t i = 1; i < text.size(); ++i) {
        if ((DString::value_type)' ' == text[i]) {
            candidates.push_back(i);
        }
        else if ((DString::value_type)'(' == text[i]) {
            candidates.push_back(i);
        }
    }
    if (candidates.empty()) {
        return;
    }
    //选择使最长行宽度最小的拆分点
    size_t nBestPos = 0;
    float fBestScore = 0;
    DString bestLine1;
    DString bestLine2;
    for (size_t nPos : candidates) {
        DString s1 = text.substr(0, nPos);
        DString s2 = text.substr(nPos);
        while (!s2.empty() && ((DString::value_type)' ' == s2[0])) {
            s2.erase(0, 1);
        }
        if (s1.empty() || s2.empty()) {
            continue;
        }
        const float fWidth1 = pFont->MeasureTextWidth(s1);
        const float fWidth2 = pFont->MeasureTextWidth(s2);
        //超出可用宽度时增加惩罚值，优先选择两行都能完整显示的位置
        float fScore = std::max(fWidth1, fWidth2);
        if (fWidth1 > fMaxWidth) {
            fScore += (fWidth1 - fMaxWidth) * 2;
        }
        if (fWidth2 > fMaxWidth) {
            fScore += (fWidth2 - fMaxWidth) * 2;
        }
        if ((nBestPos == 0) || (fScore < fBestScore)) {
            nBestPos = nPos;
            fBestScore = fScore;
            bestLine1 = s1;
            bestLine2 = s2;
        }
    }
    if (nBestPos > 0) {
        line1 = bestLine1;
        line2 = bestLine2;
    }
}

}

RenderTest1::RenderTest1(ui::Window* pWindow):
    ui::Control(pWindow)
{
    SetBkColor(ui::UiColor(0xFFF7F7F7));
}

RenderTest1::~RenderTest1()
{
}

Image* RenderTest1::GetImage()
{
    if (m_pImage == nullptr) {
        m_pImage = std::make_unique<Image>();
        // 目前支持的图片格式：png, jpg, bmp, gif, svg, webp, ico
        // svg格式支持矢量缩放，其他格式的图片不支持矢量缩放
        m_pImage->SetImageString(_T("file='autumn.png' async_load='false'"), Dpi());
    }
    return m_pImage.get();
}

bool RenderTest1::GetImageSize(int32_t& nImageWidth, int32_t& nImageHeight)
{
    Image* pImage = GetImage();
    if (pImage == nullptr) {
        return false;
    }
    LoadImageInfo(*pImage, true);
    std::shared_ptr<ImageInfo> pImageInfo = pImage->GetImageInfo();
    if (pImageInfo == nullptr) {
        return false;
    }
    nImageWidth = (int32_t)pImageInfo->GetWidth();
    nImageHeight = (int32_t)pImageInfo->GetHeight();
    return (nImageWidth > 0) && (nImageHeight > 0);
}

RenderTest1::LayoutInfo RenderTest1::CalcLayout(int32_t nImageWidth, int32_t nImageHeight) const
{
    LayoutInfo layout;
    layout.nColumns = kColumns;
    layout.nCellWidth = nImageWidth;
    layout.nCellHeight = nImageHeight;
    layout.nCaptionHeight = Dpi().GetScaleInt(kCaptionHeight);
    layout.nGap = Dpi().GetScaleInt(12);
    layout.nMargin = Dpi().GetScaleInt(4);
    layout.nRows = (kCellCount + layout.nColumns - 1) / layout.nColumns;
    return layout;
}

int32_t RenderTest1::GetCellCount()
{
    return kCellCount;
}

RenderTest1::CellType RenderTest1::GetCellType(int32_t nIndex)
{
    switch (nIndex) {
    case 0:  return CellType::kNormal;
    case 1:  return CellType::kFade;
    case 2:  return CellType::kScaleNearest;
    case 3:  return CellType::kScaleLinear;
    case 4:  return CellType::kRotate;
    case 5:  return CellType::kClip;
    case 6:  return CellType::kBitBlt;
    case 7:  return CellType::kStretchBlt;
    case 8:  return CellType::kAlphaBlend;
    case 9:  return CellType::kTiled;
    case 10: return CellType::kTiledMargin;
    case 11: return CellType::kNinePatch;
    case 12: return CellType::kNinePatchTiled;
    case 13: return CellType::kWindowShadow;
    default: return CellType::kNormal;
    }
}

const char* RenderTest1::GetCellCaption(CellType cellType)
{
    switch (cellType) {
    case CellType::kNormal:         return "DrawImage (normal)";
    case CellType::kFade:           return "DrawImage (fade 100)";
    case CellType::kScaleNearest:   return "DrawImage (scale, nearest)";
    case CellType::kScaleLinear:    return "DrawImage (scale, linear)";
    case CellType::kRotate:         return "DrawImage (matrix rotate 30)";
    case CellType::kClip:           return "DrawImage (clip path)";
    case CellType::kBitBlt:         return "BitBlt (kSrcCopy)";
    case CellType::kStretchBlt:     return "StretchBlt";
    case CellType::kAlphaBlend:     return "AlphaBlend (alpha 96)";
    case CellType::kTiled:          return "DrawImage (tiled)";
    case CellType::kTiledMargin:    return "DrawImage (tiled + margin)";
    case CellType::kNinePatch:      return "DrawImage (9-slice)";
    case CellType::kNinePatchTiled: return "DrawImage (9-slice + tiled)";
    case CellType::kWindowShadow:   return "DrawImage (window shadow)";
    default:                        return "";
    }
}

UiEstSize RenderTest1::EstimateSize(UiSize szAvailable)
{
    UiFixedSize fixedSize;
    UiEstSize returnEstSize;
    if (!PreEstimateSize(szAvailable, fixedSize, returnEstSize)) {
        return returnEstSize;
    }

    int32_t nImageWidth = 0;
    int32_t nImageHeight = 0;
    if (!GetImageSize(nImageWidth, nImageHeight)) {
        return BaseClass::EstimateSize(szAvailable);
    }
    const LayoutInfo layout = CalcLayout(nImageWidth, nImageHeight);
    const UiPadding rcPadding = GetControlPadding();

    const int32_t nWidth = rcPadding.left + rcPadding.right +
                           layout.nMargin * 2 + layout.nColumns * layout.nCellWidth + (layout.nColumns - 1) * layout.nGap;
    const int32_t nHeight = rcPadding.top + rcPadding.bottom +
                            layout.nMargin * 2 + layout.nRows * (layout.nCellHeight + layout.nCaptionHeight) + (layout.nRows - 1) * layout.nGap;

    if (fixedSize.cx.IsAuto()) {
        fixedSize.cx.SetInt32(nWidth);
    }
    if (fixedSize.cy.IsAuto()) {
        fixedSize.cy.SetInt32(nHeight);
    }
    UiEstSize estSize = MakeEstSize(fixedSize);
    SetEstimateSize(estSize, szAvailable);
    return estSize;
}

void RenderTest1::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }
    int32_t nImageWidth = 0;
    int32_t nImageHeight = 0;
    if (!GetImageSize(nImageWidth, nImageHeight)) {
        return;
    }
    Image* pImage = GetImage();
    if (pImage == nullptr) {
        return;
    }

    //获取原图位图
    UiRect rcSource(0, 0, nImageWidth, nImageHeight);
    UiRect rcDest(0, 0, nImageWidth, nImageHeight);
    UiRect rcSourceCorner;
    std::shared_ptr<IBitmap> pBitmap = pImage->GetCurrentBitmap(false, rcDest, rcSource, rcSourceCorner, nullptr);
    if ((pBitmap == nullptr) || (pBitmap->GetWidth() == 0) || (pBitmap->GetHeight() == 0)) {
        return;
    }

    //创建原图对应的Render对象（BitBlt、StretchBlt、AlphaBlend使用）
    std::unique_ptr<IRender> pSrcRender(BitmapHelper::CreateRenderObject(pBitmap.get()));

    //创建小图位图，用于平铺绘制
    const int32_t nTileSize = Dpi().GetScaleInt(32);
    std::unique_ptr<IBitmap> pTiledBitmap = ImageUtil::ResizeImageBitmap(pBitmap.get(), nTileSize, nTileSize);

    PaintContext context;
    context.pRender = pRender;
    context.pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    context.pRcPaint = &rcPaint;
    context.pBitmap = pBitmap.get();
    context.pTiledBitmap = pTiledBitmap.get();
    context.pSrcRender = pSrcRender.get();
    context.rcImageSource = rcSource;
    if (pTiledBitmap != nullptr) {
        context.rcTiledSource = UiRect(0, 0, pTiledBitmap->GetWidth(), pTiledBitmap->GetHeight());
    }

    const LayoutInfo layout = CalcLayout(nImageWidth, nImageHeight);
    const UiRect rcCtrl = GetRect();

    for (int32_t nIndex = 0; nIndex < GetCellCount(); ++nIndex) {
        const int32_t nRow = nIndex / layout.nColumns;
        const int32_t nColumn = nIndex % layout.nColumns;

        UiRect rcCell;
        rcCell.left = rcCtrl.left + layout.nMargin + nColumn * (layout.nCellWidth + layout.nGap);
        rcCell.top = rcCtrl.top + layout.nMargin + nRow * (layout.nCellHeight + layout.nCaptionHeight + layout.nGap);
        rcCell.right = rcCell.left + layout.nCellWidth;
        rcCell.bottom = rcCell.top + layout.nCellHeight;

        const CellType cellType = GetCellType(nIndex);
        PaintCell(context, cellType, rcCell);

        //绘制单元格标题（放在单元格下方，用固定的字体显示）
        UiRect rcCaption(rcCell.left, rcCell.bottom, rcCell.right, rcCell.bottom + layout.nCaptionHeight);
        PaintCellCaption(pRender, GetCellCaption(cellType), rcCaption);
    }
}

void RenderTest1::PaintCellCaption(IRender* pRender, const char* pCaption, const UiRect& rcCaption)
{
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(_T("system_12"), Dpi());
    if ((pFont == nullptr) || (pCaption == nullptr)) {
        return;
    }
    DString text = StringConvert::UTF8ToT(pCaption);
    DString line1;
    DString line2;
    SplitCaption(text, (float)rcCaption.Width(), pFont, line1, line2);

    DrawStringParam strParam;
    strParam.dwTextColor = ui::UiColor(0xFF333333);
    strParam.pFont = pFont;
    strParam.uFormat = (uint32_t)DrawStringFormat::TEXT_HCENTER | (uint32_t)DrawStringFormat::TEXT_VCENTER;

    //裁剪到标题区域，避免文本溢出到相邻单元格
    int32_t nState = 0;
    pRender->SaveClip(nState);
    pRender->ClipRect(rcCaption);

    const int32_t nLineHeight = rcCaption.Height() / 2;
    strParam.textRect = UiRect(rcCaption.left, rcCaption.top, rcCaption.right, rcCaption.top + nLineHeight);
    pRender->DrawString(line1, strParam);
    if (!line2.empty()) {
        strParam.textRect = UiRect(rcCaption.left, rcCaption.top + nLineHeight, rcCaption.right, rcCaption.bottom);
        pRender->DrawString(line2, strParam);
    }
    pRender->RestoreClip(nState);
}

void RenderTest1::PaintCell(const PaintContext& context, CellType cellType, const UiRect& rcCell)
{
    IRender* pRender = context.pRender;
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    //采样质量默认使用最近邻采样，每个单元格按需要修改
    pRender->SetImageSamplingQuality(ImageSamplingQuality::kNearest);

    ImageDrawParam drawParam;
    if (context.pRcPaint != nullptr) {
        drawParam.rcPaint = *context.pRcPaint;
    }

    switch (cellType) {
    case CellType::kNormal:
        pRender->DrawImage(context.pBitmap, rcCell, context.rcImageSource, drawParam);
        break;

    case CellType::kFade:
        drawParam.uFade = 100;
        pRender->DrawImage(context.pBitmap, rcCell, context.rcImageSource, drawParam);
        break;

    case CellType::kScaleNearest:
    case CellType::kScaleLinear:
        {
            //缩小到一半绘制，对比不同采样质量的显示效果
            UiRect rcScale = rcCell;
            rcScale.left = rcCell.left + rcCell.Width() / 4;
            rcScale.top = rcCell.top + rcCell.Height() / 4;
            rcScale.right = rcScale.left + rcCell.Width() / 2;
            rcScale.bottom = rcScale.top + rcCell.Height() / 2;
            if (cellType == CellType::kScaleNearest) {
                pRender->SetImageSamplingQuality(ImageSamplingQuality::kNearest);
            }
            else {
                pRender->SetImageSamplingQuality(ImageSamplingQuality::kLinear);
            }
            pRender->DrawImage(context.pBitmap, rcScale, context.rcImageSource, drawParam);
        }
        break;

    case CellType::kRotate:
        {
            std::unique_ptr<IMatrix> pMatrix(context.pRenderFactory->CreateMatrix());
            std::unique_ptr<IMatrix> pScaleMatrix(context.pRenderFactory->CreateMatrix());
            if ((pMatrix != nullptr) && (pScaleMatrix != nullptr)) {
                //旋转后图片会超出单元格范围：先把图片按旋转后的包围盒缩小，再旋转，保证图片完整显示；
                //同时裁剪到单元格内，避免覆盖相邻的标题
                int32_t nClipState = 0;
                pRender->SaveClip(nClipState);
                pRender->ClipRect(rcCell);
                const float fCenterX = (float)(rcCell.left + rcCell.Width() / 2);
                const float fCenterY = (float)(rcCell.top + rcCell.Height() / 2);
                //旋转 30 度后包围盒的放大系数为 cos(30) + sin(30)，取倒数并留出少量边距
                constexpr float kRotateScale = 0.68f;
                //注意：Scale/RotateAt 是设置矩阵（会覆盖已有变换），组合变换需要使用 Concat 叠加
                pMatrix->RotateAt(30.0f, fCenterX, fCenterY);
                pScaleMatrix->Scale(kRotateScale, kRotateScale, fCenterX, fCenterY);
                pMatrix->Concat(pScaleMatrix.get());
                drawParam.pMatrix = pMatrix.get();
                pRender->DrawImage(context.pBitmap, rcCell, context.rcImageSource, drawParam);
                pRender->RestoreClip(nClipState);
            }
        }
        break;

    case CellType::kClip:
        {
            //使用圆角矩形裁剪区域，只绘制裁剪区域内的部分
            UiRect rcClip = rcCell;
            rcClip.Deflate(Dpi().GetScaleInt(16), Dpi().GetScaleInt(16));
            const int32_t nClipState = pRender->ClipRoundRect(rcClip, (float)rcClip.Width() / 2, (float)rcClip.Height() / 2);
            if (nClipState >= 0) {
                pRender->DrawImage(context.pBitmap, rcCell, context.rcImageSource, drawParam);
                pRender->RestoreClip(nClipState);
            }
        }
        break;

    case CellType::kBitBlt:
        pRender->BitBlt(rcCell.left, rcCell.top, rcCell.Width(), rcCell.Height(),
                        context.pSrcRender, 0, 0, ui::RopMode::kSrcCopy);
        break;

    case CellType::kStretchBlt:
        {
            //缩小到一半绘制，展示拉伸绘制的效果
            const int32_t nDestWidth = rcCell.Width() / 2;
            const int32_t nDestHeight = rcCell.Height() / 2;
            const int32_t nDestLeft = rcCell.left + rcCell.Width() / 4;
            const int32_t nDestTop = rcCell.top + rcCell.Height() / 4;
            pRender->StretchBlt(nDestLeft, nDestTop, nDestWidth, nDestHeight,
                                context.pSrcRender, 0, 0, context.rcImageSource.Width(), context.rcImageSource.Height(),
                                ui::RopMode::kSrcCopy);
        }
        break;

    case CellType::kAlphaBlend:
        pRender->AlphaBlend(rcCell.left, rcCell.top, rcCell.Width(), rcCell.Height(),
                            context.pSrcRender, 0, 0, context.rcImageSource.Width(), context.rcImageSource.Height(), 96);
        break;

    case CellType::kTiled:
    case CellType::kTiledMargin:
    case CellType::kNinePatchTiled:
        {
            if ((context.pTiledBitmap == nullptr) || context.rcTiledSource.IsEmpty()) {
                break;
            }
            TiledDrawParam tiledParam;
            tiledParam.m_bTiledX = true;
            tiledParam.m_bTiledY = true;
            tiledParam.m_bFullTiledX = true;
            tiledParam.m_bFullTiledY = true;
            if (cellType == CellType::kTiledMargin) {
                //平铺时图片之间留出间隔
                tiledParam.m_nTiledMarginX = Dpi().GetScaleInt(2);
                tiledParam.m_nTiledMarginY = Dpi().GetScaleInt(2);
            }
            if (cellType == CellType::kNinePatchTiled) {
                //九宫格 + 平铺 + 内边距
                drawParam.rcImageBorder = UiMargin(Dpi().GetScaleInt(6), Dpi().GetScaleInt(6), Dpi().GetScaleInt(6), Dpi().GetScaleInt(6));
                tiledParam.m_rcTiledPadding = UiPadding(Dpi().GetScaleInt(2), Dpi().GetScaleInt(2), Dpi().GetScaleInt(2), Dpi().GetScaleInt(2));
            }
            drawParam.pTiledDrawParam = &tiledParam;
            pRender->DrawImage(context.pTiledBitmap, rcCell, context.rcTiledSource, drawParam);
        }
        break;

    case CellType::kNinePatch:
        drawParam.rcImageBorder = UiMargin(Dpi().GetScaleInt(6), Dpi().GetScaleInt(6), Dpi().GetScaleInt(6), Dpi().GetScaleInt(6));
        pRender->DrawImage(context.pBitmap, rcCell, context.rcImageSource, drawParam);
        break;

    case CellType::kWindowShadow:
        //窗口阴影模式：只绘制边框，不绘制中间部分
        drawParam.rcImageBorder = UiMargin(Dpi().GetScaleInt(20), Dpi().GetScaleInt(20), Dpi().GetScaleInt(20), Dpi().GetScaleInt(20));
        drawParam.bWindowShadowMode = true;
        pRender->DrawImage(context.pBitmap, rcCell, context.rcImageSource, drawParam);
        break;

    default:
        break;
    }

    //恢复默认的采样质量
    pRender->SetImageSamplingQuality(ImageSamplingQuality::kNearest);
}

} //end of namespace ui
