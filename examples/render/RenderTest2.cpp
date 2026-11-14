#include "RenderTest2.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"

namespace ui {

namespace {

/** 绘制项索引（需要与 GetDrawItems 的顺序保持一致）
*/
enum DrawItemIndex
{
    kDrawLine = 0,              //画直线（指定线宽）
    kDrawLinePen,               //画直线（使用画笔）
    kLineSolid,                 //实线
    kLineDash,                  //虚线（Dash）
    kLineDot,                   //虚线（Dot）
    kLineDashDot,               //虚线（DashDot）
    kDrawRect,                  //画矩形（指定线宽）
    kDrawRectPen,               //画矩形（画笔虚线）
    kDrawRectLineIn,            //画矩形（线宽绘制在矩形内部）
    kFillRect,                  //填充矩形
    kFillRectAlpha,             //填充矩形（半透明）
    kFillRectGradient,          //填充矩形（渐变颜色）
    kDrawRoundRect,             //画圆角矩形（指定线宽）
    kDrawRoundRectPen,          //画圆角矩形（画笔虚线）
    kFillRoundRect,             //填充圆角矩形
    kFillRoundRectAlpha,        //填充圆角矩形（半透明）
    kFillRoundRectGradient,     //填充圆角矩形（渐变颜色）
    kFillRoundRectGradientAlpha,//填充圆角矩形（渐变颜色+半透明）
    kDrawCircle,                //画圆
    kDrawCirclePen,             //画圆（画笔点线）
    kFillCircle,                //填充圆
    kFillCircleAlpha,           //填充圆（半透明）
    kDrawEllipse,               //画椭圆
    kFillEllipse,               //填充椭圆
    kDrawArc,                   //画圆弧
    kDrawArcCenter,             //画圆弧（连接中心点，扇形轮廓）
    kDrawArcPen,                //画圆弧（画笔虚线）
    kFillArc,                   //填充圆弧（扇形）
    kDrawPolygon,               //画多边形
    kFillPolygon,               //填充多边形
    kDrawPath,                  //画路径（圆角四边形）
    kFillPath,                  //填充路径（使用画刷）
    kFillPathGradient,          //填充路径（渐变颜色）
    kDrawBoxShadow,             //绘制阴影（圆角）
    kDrawBoxShadowRect,         //绘制阴影（直角）
    kDrawBoxShadowOffset,       //绘制阴影（带偏移）
    kDrawRegularHexagon,        //绘制正六边形（多边形接口）
    kDrawRegularHexagon3,       //绘制正六边形（路径接口，按区块填充）
    kDrawColorMap,              //用正六边形拼接复杂图形
    kDrawItemCount
};

/** 单元格标题（使用接口名称，不涉及多语言）
*/
const char* const kDrawItemCaptions[kDrawItemCount] = {
    "DrawLine (width)",
    "DrawLine (pen)",
    "Solid",
    "Dash",
    "Dot",
    "DashDot",
    "DrawRect (width)",
    "DrawRect (pen)",
    "DrawRect (lineInRect)",
    "FillRect",
    "FillRect (alpha 128)",
    "FillRect (gradient)",
    "DrawRoundRect",
    "DrawRoundRect (pen)",
    "FillRoundRect",
    "FillRoundRect (alpha)",
    "FillRoundRect (gradient)",
    "FillRoundRect (grad+alpha)",
    "DrawCircle",
    "DrawCircle (pen)",
    "FillCircle",
    "FillCircle (alpha 96)",
    "DrawEllipse",
    "FillEllipse",
    "DrawArc",
    "DrawArc (useCenter)",
    "DrawArc (pen)",
    "FillArc (useCenter)",
    "DrawPolygon",
    "FillPolygon",
    "DrawPath",
    "FillPath (brush)",
    "FillPath (gradient)",
    "DrawBoxShadow (round)",
    "DrawBoxShadow (rect)",
    "DrawBoxShadow (offset)",
    "DrawRegularHexagon",
    "DrawRegularHexagon3",
    "DrawColorMap",
};

/** 各绘制项占用的列数和行数
*/
const int32_t kDrawItemColumns[kDrawItemCount] = {
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    2, 2, 2,
};

const int32_t kDrawItemRows[kDrawItemCount] = {
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    1, 1, 2,
};

/** 总列数
*/
constexpr int32_t kColumnCount = 6;

/** 单个单元格的宽度（逻辑像素）
*/
constexpr int32_t kCellWidth = 110;

/** 单个单元格的高度（逻辑像素）
*/
constexpr int32_t kCellHeight = 110;

/** 单元格标题的高度（逻辑像素，足够显示两行标题）
*/
constexpr int32_t kCaptionHeight = 34;

/** 单元格之间的间距（逻辑像素）
*/
constexpr int32_t kCellGap = 10;

/** 内容区域的外边距（逻辑像素）
*/
constexpr int32_t kContentMargin = 6;

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

/** 获取画笔：按线型样式创建
*/
std::unique_ptr<IPen> CreatePen(IRenderFactory* pRenderFactory, const UiColor& color, float fWidth, int32_t nDashStyle)
{
    if (pRenderFactory == nullptr) {
        return nullptr;
    }
    std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(color, fWidth));
    if (pen == nullptr) {
        return nullptr;
    }
    switch (nDashStyle) {
    case 1: pen->SetDashStyle(IPen::DashStyle::kDashStyleDash); break;
    case 2: pen->SetDashStyle(IPen::DashStyle::kDashStyleDot); break;
    case 3: pen->SetDashStyle(IPen::DashStyle::kDashStyleDashDot); break;
    case 4: pen->SetDashStyle(IPen::DashStyle::kDashStyleDashDotDot); break;
    default: pen->SetDashStyle(IPen::DashStyle::kDashStyleSolid); break;
    }
    return pen;
}

} //end of namespace

RenderTest2::RenderTest2(ui::Window* pWindow):
    ui::Control(pWindow)
{
    SetBkColor(ui::UiColor(0xFFF7F7F7));
}

RenderTest2::~RenderTest2()
{
}

const std::vector<RenderTest2::DrawItem>& RenderTest2::GetDrawItems()
{
    static std::vector<DrawItem> s_items;
    if (s_items.empty()) {
        for (int32_t nIndex = 0; nIndex < kDrawItemCount; ++nIndex) {
            DrawItem item;
            item.pCaption = kDrawItemCaptions[nIndex];
            item.nColumns = kDrawItemColumns[nIndex];
            item.nRows = kDrawItemRows[nIndex];
            s_items.push_back(item);
        }
    }
    return s_items;
}

RenderTest2::LayoutInfo RenderTest2::CalcLayout(std::vector<ItemLayout>& itemLayouts) const
{
    LayoutInfo layout;
    layout.nColumns = kColumnCount;
    layout.nCellWidth = Dpi().GetScaleInt(kCellWidth);
    layout.nCellHeight = Dpi().GetScaleInt(kCellHeight);
    layout.nCaptionHeight = Dpi().GetScaleInt(kCaptionHeight);
    layout.nGap = Dpi().GetScaleInt(kCellGap);
    layout.nMargin = Dpi().GetScaleInt(kContentMargin);

    const int32_t nRowStride = layout.nCellHeight + layout.nCaptionHeight + layout.nGap;

    const std::vector<DrawItem>& items = GetDrawItems();
    itemLayouts.clear();
    itemLayouts.resize(items.size());

    //按顺序摆放各个绘制项，记录已占用的单元格
    std::vector<std::vector<bool>> occupied;
    auto ensureRows = [&occupied, this](int32_t nRows) {
        while ((int32_t)occupied.size() < nRows) {
            occupied.push_back(std::vector<bool>(kColumnCount, false));
        }
    };
    auto isFree = [&occupied](int32_t nRow, int32_t nColumn, int32_t nRows, int32_t nColumns) {
        for (int32_t r = 0; r < nRows; ++r) {
            for (int32_t c = 0; c < nColumns; ++c) {
                if (occupied[(size_t)(nRow + r)][(size_t)(nColumn + c)]) {
                    return false;
                }
            }
        }
        return true;
    };

    int32_t nCursorRow = 0;
    int32_t nCursorColumn = 0;
    for (size_t nItem = 0; nItem < items.size(); ++nItem) {
        const DrawItem& item = items[nItem];
        int32_t nItemColumns = std::max(1, item.nColumns);
        int32_t nItemRows = std::max(1, item.nRows);
        nItemColumns = std::min(nItemColumns, kColumnCount);
        while (true) {
            ensureRows(nCursorRow + nItemRows);
            if (nCursorColumn + nItemColumns > kColumnCount) {
                ++nCursorRow;
                nCursorColumn = 0;
                continue;
            }
            if (isFree(nCursorRow, nCursorColumn, nItemRows, nItemColumns)) {
                break;
            }
            ++nCursorColumn;
        }
        for (int32_t r = 0; r < nItemRows; ++r) {
            for (int32_t c = 0; c < nItemColumns; ++c) {
                occupied[(size_t)(nCursorRow + r)][(size_t)(nCursorColumn + c)] = true;
            }
        }

        const int32_t nDrawWidth = nItemColumns * layout.nCellWidth + (nItemColumns - 1) * layout.nGap;
        const int32_t nDrawHeight = nItemRows * layout.nCellHeight + (nItemRows - 1) * (layout.nCaptionHeight + layout.nGap);

        ItemLayout& itemLayout = itemLayouts[nItem];
        itemLayout.rcDraw.left = layout.nMargin + nCursorColumn * (layout.nCellWidth + layout.nGap);
        itemLayout.rcDraw.top = layout.nMargin + nCursorRow * nRowStride;
        itemLayout.rcDraw.right = itemLayout.rcDraw.left + nDrawWidth;
        itemLayout.rcDraw.bottom = itemLayout.rcDraw.top + nDrawHeight;

        itemLayout.rcCaption.left = itemLayout.rcDraw.left;
        itemLayout.rcCaption.right = itemLayout.rcDraw.right;
        itemLayout.rcCaption.top = itemLayout.rcDraw.bottom;
        itemLayout.rcCaption.bottom = itemLayout.rcCaption.top + layout.nCaptionHeight;

        nCursorColumn += nItemColumns;
    }

    layout.nRowCount = (int32_t)occupied.size();
    layout.nContentWidth = layout.nMargin * 2 + layout.nColumns * layout.nCellWidth + (layout.nColumns - 1) * layout.nGap;
    layout.nContentHeight = layout.nMargin * 2 + (layout.nRowCount - 1) * nRowStride + layout.nCellHeight + layout.nCaptionHeight;
    return layout;
}

UiEstSize RenderTest2::EstimateSize(UiSize szAvailable)
{
    UiFixedSize fixedSize;
    UiEstSize returnEstSize;
    if (!PreEstimateSize(szAvailable, fixedSize, returnEstSize)) {
        return returnEstSize;
    }
    std::vector<ItemLayout> itemLayouts;
    const LayoutInfo layout = CalcLayout(itemLayouts);
    const UiPadding rcPadding = GetControlPadding();

    if (fixedSize.cx.IsAuto()) {
        fixedSize.cx.SetInt32(layout.nContentWidth + rcPadding.left + rcPadding.right);
    }
    if (fixedSize.cy.IsAuto()) {
        fixedSize.cy.SetInt32(layout.nContentHeight + rcPadding.top + rcPadding.bottom);
    }
    UiEstSize estSize = MakeEstSize(fixedSize);
    SetEstimateSize(estSize, szAvailable);
    return estSize;
}

void RenderTest2::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    std::vector<ItemLayout> itemLayouts;
    CalcLayout(itemLayouts);
    const std::vector<DrawItem>& items = GetDrawItems();
    if (items.size() != itemLayouts.size()) {
        return;
    }

    const UiPoint ptOffset(GetRect().left, GetRect().top);
    for (size_t nItem = 0; nItem < items.size(); ++nItem) {
        UiRect rcDraw = itemLayouts[nItem].rcDraw;
        rcDraw.Offset(ptOffset);
        //跳过不需要绘制的区域（提升绘制效率）
        if (!UiRect::Intersect(rcDraw, rcDraw, rcPaint)) {
            continue;
        }
        //注意：上面裁剪后的rcDraw仅用于判断是否需要绘制，绘制时需要使用完整区域
        UiRect rcFullDraw = itemLayouts[nItem].rcDraw;
        rcFullDraw.Offset(ptOffset);
        PaintItem(pRender, pRenderFactory, (int32_t)nItem, rcFullDraw);

        UiRect rcCaption = itemLayouts[nItem].rcCaption;
        rcCaption.Offset(ptOffset);
        PaintCaption(pRender, items[nItem].pCaption, rcCaption);
    }
}

void RenderTest2::PaintCaption(IRender* pRender, const char* pCaption, const UiRect& rcCaption)
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

void RenderTest2::PaintItem(IRender* pRender, IRenderFactory* pRenderFactory, int32_t nIndex, const UiRect& rcDraw)
{
    const int32_t nPadding = Dpi().GetScaleInt(10);
    UiRect rc = rcDraw;
    rc.Deflate(nPadding, nPadding);
    const UiRectF rcF = UiRectF::MakeFromRect(rc);
    const float fPenWidth = DpiScaledFloat(2);
    const int32_t nRoundSize = Dpi().GetScaleInt(12);

    switch (nIndex) {
    case kDrawLine:
        {
            //等间距画多条直线，交替使用两种线宽参数
            const int32_t nSep = std::max(1, rc.Height() / 6);
            int32_t nLineIndex = 0;
            for (int32_t nTop = rc.top; nTop <= rc.bottom; nTop += nSep) {
                const float fWidth = ((nLineIndex % 2) == 0) ? DpiScaledFloat(3) : DpiScaledFloat(1);
                pRender->DrawLine(UiPointF((float)rc.left, (float)nTop), UiPointF((float)rc.right, (float)nTop),
                                  UiColor(UiColors::DarkCyan), fWidth);
                ++nLineIndex;
            }
        }
        break;

    case kDrawLinePen:
        {
            std::unique_ptr<IPen> pen = CreatePen(pRenderFactory, UiColor(UiColors::CornflowerBlue), fPenWidth, 0);
            if (pen != nullptr) {
                const int32_t nSep = std::max(1, rc.Height() / 6);
                for (int32_t nTop = rc.top; nTop <= rc.bottom; nTop += nSep) {
                    pRender->DrawLine(UiPointF((float)rc.left, (float)nTop), UiPointF((float)rc.right, (float)nTop), pen.get());
                }
            }
        }
        break;

    case kLineSolid:
    case kLineDash:
    case kLineDot:
    case kLineDashDot:
        {
            const int32_t nDashStyle = (nIndex == kLineSolid) ? 0 : (nIndex - kLineSolid);
            std::unique_ptr<IPen> pen = CreatePen(pRenderFactory, UiColor(UiColors::CornflowerBlue), fPenWidth, nDashStyle);
            if (pen != nullptr) {
                const int32_t nSep = std::max(1, rc.Height() / 6);
                for (int32_t nTop = rc.top; nTop <= rc.bottom; nTop += nSep) {
                    pRender->DrawLine(UiPointF((float)rc.left, (float)nTop), UiPointF((float)rc.right, (float)nTop), pen.get());
                }
            }
        }
        break;

    case kDrawRect:
        pRender->DrawRect(rcF, UiColor(UiColors::Fuchsia), fPenWidth);
        break;

    case kDrawRectPen:
        {
            std::unique_ptr<IPen> pen = CreatePen(pRenderFactory, UiColor(UiColors::CornflowerBlue), fPenWidth, 1);
            if (pen != nullptr) {
                pRender->DrawRect(rcF, pen.get());
            }
        }
        break;

    case kDrawRectLineIn:
        pRender->DrawRect(rcF, UiColor(UiColors::DarkCyan), DpiScaledFloat(6), true);
        break;

    case kFillRect:
        pRender->FillRect(rcF, UiColor(UiColors::Brown));
        break;

    case kFillRectAlpha:
        pRender->FillRect(rcF, UiColor(UiColors::Brown), 128);
        break;

    case kFillRectGradient:
        pRender->FillRect(rcF, UiColor(UiColors::Gold), UiColor(UiColors::Crimson), 1);
        break;

    case kDrawRoundRect:
        pRender->DrawRoundRect(rcF, (float)nRoundSize, (float)nRoundSize, UiColor(0xffC63535), fPenWidth);
        break;

    case kDrawRoundRectPen:
        {
            std::unique_ptr<IPen> pen = CreatePen(pRenderFactory, UiColor(UiColors::CornflowerBlue), fPenWidth, 1);
            if (pen != nullptr) {
                pRender->DrawRoundRect(rcF, (float)nRoundSize, (float)nRoundSize, pen.get());
            }
        }
        break;

    case kFillRoundRect:
        pRender->FillRoundRect(rcF, (float)nRoundSize, (float)nRoundSize, UiColor(UiColors::Blue));
        break;

    case kFillRoundRectAlpha:
        pRender->FillRoundRect(rcF, (float)nRoundSize, (float)nRoundSize, UiColor(UiColors::Blue), 128);
        break;

    case kFillRoundRectGradient:
        pRender->FillRoundRect(rcF, (float)nRoundSize, (float)nRoundSize, UiColor(UiColors::Gold), UiColor(UiColors::Crimson), 2);
        break;

    case kFillRoundRectGradientAlpha:
        pRender->FillRoundRect(rcF, (float)nRoundSize, (float)nRoundSize, UiColor(UiColors::Gold), UiColor(UiColors::Crimson), 3, 128);
        break;

    case kDrawCircle:
        pRender->DrawCircle(UiPointF::MakeFromPoint(rc.Center()), std::min(rc.Width(), rc.Height()) / 2.0f,
                            UiColor(UiColors::Blue), fPenWidth);
        break;

    case kDrawCirclePen:
        {
            std::unique_ptr<IPen> pen = CreatePen(pRenderFactory, UiColor(UiColors::CornflowerBlue), fPenWidth, 2);
            if (pen != nullptr) {
                pRender->DrawCircle(UiPointF::MakeFromPoint(rc.Center()), std::min(rc.Width(), rc.Height()) / 2.0f, pen.get());
            }
        }
        break;

    case kFillCircle:
        pRender->FillCircle(UiPointF::MakeFromPoint(rc.Center()), std::min(rc.Width(), rc.Height()) / 2.0f,
                            UiColor(UiColors::CadetBlue), 255);
        break;

    case kFillCircleAlpha:
        pRender->FillCircle(UiPointF::MakeFromPoint(rc.Center()), std::min(rc.Width(), rc.Height()) / 2.0f,
                            UiColor(UiColors::CadetBlue), 96);
        break;

    case kDrawEllipse:
        pRender->DrawEllipse(rcF, UiColor(UiColors::Blue), fPenWidth);
        break;

    case kFillEllipse:
        pRender->FillEllipse(rcF, UiColor(UiColors::CadetBlue), 255);
        break;

    case kDrawArc:
    case kDrawArcCenter:
        {
            std::unique_ptr<IPen> pen = CreatePen(pRenderFactory, UiColor(0xff006DD9), fPenWidth, 0);
            if (pen != nullptr) {
                pRender->DrawArc(rcF, 30.0f, 270.0f, (nIndex == kDrawArcCenter), pen.get());
            }
        }
        break;

    case kDrawArcPen:
        {
            std::unique_ptr<IPen> pen = CreatePen(pRenderFactory, UiColor(0xff006DD9), fPenWidth, 1);
            if (pen != nullptr) {
                pRender->DrawArc(rcF, 30.0f, 270.0f, false, pen.get());
            }
        }
        break;

    case kFillArc:
        pRender->FillArc(rcF, 30.0f, 270.0f, true, UiColor(UiColors::MediumPurple), 255);
        break;

    case kDrawPolygon:
        {
            //正五边形
            const float fRadius = std::min(rc.Width(), rc.Height()) / 2.0f;
            const float fCenterX = (float)rc.CenterX();
            const float fCenterY = (float)rc.CenterY();
            std::vector<UiPointF> points;
            for (int32_t i = 0; i < 5; ++i) {
                const float fRadian = (i * 72 - 90) / 57.2957795f;
                points.push_back(UiPointF(fCenterX + fRadius * std::cos(fRadian), fCenterY + fRadius * std::sin(fRadian)));
            }
            pRender->DrawPolygon(points.data(), (int32_t)points.size(), UiColor(0xff006DD9), fPenWidth);
        }
        break;

    case kFillPolygon:
        {
            //正五边形
            const float fRadius = std::min(rc.Width(), rc.Height()) / 2.0f;
            const float fCenterX = (float)rc.CenterX();
            const float fCenterY = (float)rc.CenterY();
            std::vector<UiPointF> points;
            for (int32_t i = 0; i < 5; ++i) {
                const float fRadian = (i * 72 - 90) / 57.2957795f;
                points.push_back(UiPointF(fCenterX + fRadius * std::cos(fRadian), fCenterY + fRadius * std::sin(fRadian)));
            }
            pRender->FillPolygon(points.data(), (int32_t)points.size(), UiColor(UiColors::MediumPurple), 255);
        }
        break;

    case kDrawPath:
        {
            std::unique_ptr<IPath> path = CreateRoundRectPath(pRenderFactory, rc, nRoundSize);
            std::unique_ptr<IPen> pen = CreatePen(pRenderFactory, UiColor(0xff006DD9), fPenWidth, 0);
            if ((path != nullptr) && (pen != nullptr)) {
                pRender->DrawPath(path.get(), pen.get());
            }
        }
        break;

    case kFillPath:
        {
            std::unique_ptr<IPath> path = CreateRoundRectPath(pRenderFactory, rc, nRoundSize);
            std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(UiColor(UiColors::Red)));
            if ((path != nullptr) && (brush != nullptr)) {
                pRender->FillPath(path.get(), brush.get());
            }
        }
        break;

    case kFillPathGradient:
        {
            std::unique_ptr<IPath> path = CreateRoundRectPath(pRenderFactory, rc, nRoundSize);
            if (path != nullptr) {
                pRender->FillPath(path.get(), rcF, UiColor(UiColors::Gold), UiColor(UiColors::Crimson), 3);
            }
        }
        break;

    case kDrawBoxShadow:
        {
            UiRect rcShadow = rc;
            rcShadow.Deflate(Dpi().GetScaleInt(20), Dpi().GetScaleInt(20));
            pRender->DrawBoxShadow(rcShadow, UiSize(nRoundSize, nRoundSize), UiPoint(0, 0),
                                   DpiScaledInt(6), DpiScaledInt(2), UiColor(0xffC63535), 255);
        }
        break;

    case kDrawBoxShadowRect:
        {
            UiRect rcShadow = rc;
            rcShadow.Deflate(Dpi().GetScaleInt(20), Dpi().GetScaleInt(20));
            pRender->DrawBoxShadow(rcShadow, UiSize(0, 0), UiPoint(0, 0),
                                   DpiScaledInt(6), DpiScaledInt(4), UiColor(0xffC63535), 255);
        }
        break;

    case kDrawBoxShadowOffset:
        {
            UiRect rcShadow = rc;
            rcShadow.Deflate(Dpi().GetScaleInt(20), Dpi().GetScaleInt(20));
            pRender->DrawBoxShadow(rcShadow, UiSize(0, 0), UiPoint(DpiScaledInt(2), DpiScaledInt(3)),
                                   DpiScaledInt(4), DpiScaledInt(4), UiColor(0xffC63535), 255);
        }
        break;

    case kDrawRegularHexagon:
        {
            //半径按宽高共同约束，避免图形超出单元格范围而覆盖标题
            const int32_t nRadius = std::min(rc.Width(), rc.Height()) / 2;
            DrawRegularHexagon(pRender, UiPointF((float)rc.CenterX(), (float)rc.CenterY()), nRadius,
                               UiColor(UiColors::Olive), DpiScaledInt(2), UiColor(UiColors::White));
        }
        break;

    case kDrawRegularHexagon3:
        {
            //半径按宽高共同约束，避免图形超出单元格范围而覆盖标题
            const int32_t nRadius = std::min(rc.Width(), rc.Height()) / 2;
            DrawRegularHexagon3(pRender, rc.Center(), nRadius,
                                UiColor(UiColors::Olive), DpiScaledInt(2), UiColor(UiColors::White));
        }
        break;

    case kDrawColorMap:
        DrawColorMap(pRender, rc);
        break;

    default:
        break;
    }
}

std::unique_ptr<IPath> RenderTest2::CreateRoundRectPath(IRenderFactory* pRenderFactory, const UiRect& rc, int32_t nRoundSize)
{
    if (pRenderFactory == nullptr) {
        return nullptr;
    }
    std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
    if (path == nullptr) {
        return nullptr;
    }
    //注意：圆弧端点坐标为浮点值，连线起点必须使用相同的浮点值，否则路径会被断开，导致填充异常
    const float fHalfRound = nRoundSize / 2.0f;
    path->AddArc(UiRect(rc.left, rc.top, rc.left + nRoundSize, rc.top + nRoundSize), 180, 90);
    path->AddLine(rc.left + fHalfRound, (float)rc.top, rc.right - fHalfRound, (float)rc.top);
    path->AddArc(UiRect(rc.right - nRoundSize, rc.top, rc.right, rc.top + nRoundSize), 270, 90);
    path->AddLine((float)rc.right, rc.top + fHalfRound, (float)rc.right, rc.bottom - fHalfRound);
    path->AddArc(UiRect(rc.right - nRoundSize, rc.bottom - nRoundSize, rc.right, rc.bottom), 0, 90);
    path->AddLine(rc.right - fHalfRound, (float)rc.bottom, rc.left + fHalfRound, (float)rc.bottom);
    path->AddArc(UiRect(rc.left, rc.bottom - nRoundSize, rc.left + nRoundSize, rc.bottom), 90, 90);
    path->AddLine((float)rc.left, rc.bottom - fHalfRound, (float)rc.left, rc.top + fHalfRound);
    path->Close();
    return path;
}

void RenderTest2::DrawColorMap(IRender* pRender, const UiRect& rect)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    //根据矩形区域计算六边形的半径，使其完整显示在矩形区域内
    const float fWidthRadius = rect.Width() / 22.5f;    //横向：13个六边形（2 * cos(30°) ≈ 1.732，合计约 22.5 倍半径）
    const float fHeightRadius = rect.Height() / 19.0f;  //纵向：13行（行间距为 1.5 倍半径）
    const int32_t radius = static_cast<int32_t>(std::min(fWidthRadius, fHeightRadius));
    if (radius <= 0) {
        return;
    }
    const float distance = radius * std::cos(30 / 57.2957795f); //中心点到边的垂直距离

    UiPointF firstCenterPt = UiPointF((float)rect.CenterX(), (float)rect.CenterY()); //矩形中心点坐标
    firstCenterPt.x = firstCenterPt.x - distance * 2 * 6 * std::sin(30 / 57.2957795f); //第一个六边形中心点X坐标
    firstCenterPt.y = firstCenterPt.y - distance * 2 * 6 * std::cos(30 / 57.2957795f); //第一个六边形中心点Y坐标

    for (int32_t y = 0; y < 13; ++y) { //共计13行
        int32_t count = 0;
        if (y < 7) {
            count = 7 + y;
        }
        else {
            count = 7 + (13 - y - 1);
        }
        for (int32_t x = 0; x < count; ++x) {
            UiPointF centerPt = firstCenterPt;
            if (y < 7) {
                centerPt.x += distance * 2 * x - distance * y;
            }
            else {
                centerPt.x += distance * 2 * x - distance * (13 - y - 1);
            }
            centerPt.y += radius * 1.5f * y;
            DrawRegularHexagon(pRender, centerPt, radius, UiColor(UiColors::Blue), 1, UiColor(UiColors::Salmon));
        }
    }
}

bool RenderTest2::DrawRegularHexagon(IRender* pRender, const UiPointF& centerPt, int32_t radius,
                                     const UiColor& penColor, int32_t penWidth, const UiColor& brushColor)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return false;
    }
    ASSERT(radius > 0); //多边形的半径
    if (radius <= 0) {
        return false;
    }

    const int32_t count = 6; //多边形的边数
    //正多边形上任意一个顶点的坐标为： x = r * cos(θ) y = r * sin(θ)
    std::vector<UiPointF> polygonPoints;
    for (int32_t i = 0; i < count; ++i) {
        int32_t degree = i * 60 + 30;// +30是为了使顶点在中心点的最上方
        float radian = degree / 57.2957795f;
        float x = radius * std::cos(radian);
        float y = radius * std::sin(radian);
        polygonPoints.push_back(UiPointF(centerPt.x + x, centerPt.y + y));
    }

    std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
    if (path == nullptr) {
        return false;
    }
    path->AddPolygon(polygonPoints.data(), (int32_t)polygonPoints.size());
    path->Close();

    bool bRet = false;
    if (brushColor.GetARGB() != 0) {
        std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(brushColor));
        if (brush != nullptr) {
            pRender->FillPath(path.get(), brush.get());
            bRet = true;
        }
    }
    if ((penColor.GetARGB() != 0) && (penWidth > 0)) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(penColor, (float)penWidth));
        if (pen != nullptr) {
            pRender->DrawPath(path.get(), pen.get());
            bRet = true;
        }
    }
    return bRet;
}

bool RenderTest2::DrawRegularHexagon3(IRender* pRender, const UiPoint& centerPt, int32_t radius,
                                      const UiColor& penColor, int32_t penWidth, const UiColor& brushColor)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return false;
    }
    if (radius <= 0) {
        return false;
    }

    bool bRet = false;
    const int count = 6;
    //开始绘制多边形，并为每个区块上色（每个区块是以中心点为顶点的一个三角形）
    for (int i = 0; i < count; ++i) {
        //设中心点到边的垂线与半径的夹角为degree=(360/count)/2即：
        float degree = 180.0f / count;

        float radian = degree / 57.2957795f;
        int32_t width = static_cast<int32_t>(radius * std::sin(radian));
        int32_t height = static_cast<int32_t>(radius * std::cos(radian));

        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
        if (path == nullptr) {
            continue;
        }
        //绘制该三角区块（顶点在中心点，坐标为相对坐标）
        path->AddLine(0, 0, -width, -height);
        path->AddLine(-width, -height, width, -height);
        path->AddLine(width, -height, 0, 0);

        std::unique_ptr<IMatrix> spMatrix(pRenderFactory->CreateMatrix());
        if (spMatrix != nullptr) {
            float angle = 2 * degree * i;
            spMatrix->RotateAt(angle, 0.0f, 0.0f);
            path->Transform(spMatrix.get());
        }
        //将相对坐标转换为客户区坐标（直接平移路径，不使用视区原点，避免依赖全局状态）
        std::unique_ptr<IMatrix> translateMatrix(pRenderFactory->CreateMatrix());
        if (translateMatrix != nullptr) {
            translateMatrix->Translate((float)centerPt.x, (float)centerPt.y);
            path->Transform(translateMatrix.get());
        }

        if (brushColor.GetARGB() != 0) {
            std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(brushColor));
            if (brush != nullptr) {
                pRender->FillPath(path.get(), brush.get());
                bRet = true;
            }
        }
        if ((penColor.GetARGB() != 0) && (penWidth > 0)) {
            std::unique_ptr<IPen> pen = CreatePen(pRenderFactory, penColor, (float)penWidth, 0);
            if (pen != nullptr) {
                pRender->DrawPath(path.get(), pen.get());
                bRet = true;
            }
        }
    }
    return bRet;
}

int32_t RenderTest2::DpiScaledInt(int32_t nValue) const
{
    return Dpi().GetScaleInt(nValue);
}

float RenderTest2::DpiScaledFloat(int32_t nValue) const
{
    return Dpi().GetScaleFloat(nValue);
}

} //end of namespace ui
