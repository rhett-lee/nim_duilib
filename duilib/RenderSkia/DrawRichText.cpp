#include "DrawRichText.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/SkTextBox.h"
#include "duilib/RenderSkia/DrawSkiaText.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/PerformanceUtil.h"

#include "SkiaHeaderBegin.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"

#include "SkiaHeaderEnd.h"

#include <unordered_set>
#include <unordered_map>

namespace ui {

//待绘制的文本
struct TPendingDrawRichText
{
    //在richTextData中的索引号
    uint32_t m_nDataIndex = 0;

    //物理行号
    uint32_t m_nLineNumber = 0;

    //行号(是绘制后的逻辑行号，当自动换行的时候，一个物理行显示为多个逻辑行，物理行号是指按文本中的换行符划分的行号）
    uint32_t m_nRowIndex = 0;

    //待绘制文本
    std::wstring_view m_textView;

    //绘制目标区域
    UiRect m_destRect;

    //Font对象
    std::shared_ptr<IFont> m_spFont;

    /** 文字颜色
    */
    UiColor m_textColor;

    /** 背景颜色
    */
    UiColor m_bgColor;

    /** 绘制文字的属性(包含文本对齐方式等属性，参见 enum DrawStringFormat)
    */
    uint16_t m_textStyle = 0;

    /** 字间距（像素） */
    SkScalar m_fLetterSpacing = 0;

    /** 两端对齐时，额外分配给每个字形的间距（像素） */
    SkScalar m_fJustifySpacing = 0;

    /** 本段文本包含的字形数量（用于两端对齐） */
    uint32_t m_nGlyphCount = 0;

    /** 本段文本的绘制用字形运行结果（布局阶段产出，绘制阶段直接批量提交，不再重新断行/解析字形） */
    std::shared_ptr<TextGlyphRunList> m_spGlyphRuns;

    /** m_spGlyphRuns 中所有字形的宽度之和（用于计算绘制时的水平对齐） */
    SkScalar m_fGlyphRunsWidth = 0;

    /** 是否绘制水平分割线（用于 <hr> 标签） */
    bool m_bHorizontalRule = false;

    /** 内联图片的位图（用于 <img> 标签），非空时绘制图片，忽略文字内容 */
    std::shared_ptr<IBitmap> m_spImage;
};

/** 省略号的文本内容（静态存储，绘制缓存中以 string_view 引用）
*/
static const DStringW::value_type g_ellipsisText[] = { (DStringW::value_type)0x2026, (DStringW::value_type)0 };

/** 获取省略号的文本视图
*/
static std::wstring_view GetEllipsisTextView()
{
    return std::wstring_view(g_ellipsisText, 1);
}

/** 文本中是否包含可绘制的字符（排除回车和换行符）
*/
static bool HasDrawableText(const std::wstring_view& textView)
{
    for (DStringW::value_type ch : textView) {
        if ((ch != L'\r') && (ch != L'\n')) {
            return true;
        }
    }
    return false;
}

/** 绘制缓存
*/
class DrawRichTextCache
{
public:
    /** 原始参数
    */
    UiRect m_textRect;
    std::vector<RichTextData> m_richTextData;

    SkTextEncoding m_textEncoding = SkTextEncoding::kUTF16;
    size_t m_textCharSize = sizeof(DStringW::value_type);

    /** 生成好的待绘制的数据
    */
    std::vector<TPendingDrawRichText> m_pendingTextData;

    /** 每行文本的宽度（用于逐行水平对齐）
    */
    std::vector<int32_t> m_rowWidthVector;
};

DrawRichText::DrawRichText(IRender* pRender, SkCanvas* pSkCanvas, SkPaint* pSkPaint, SkPoint* pSkPointOrg) :
    m_pRender(pRender),
    m_pSkCanvas(pSkCanvas),
    m_pSkPaint(pSkPaint),
    m_pSkPointOrg(pSkPointOrg)
{
}

//设置行高数据
inline static void SaveRowHeight(FastVector<int32_t>& rowHeightVector, uint32_t nRowIndex, int32_t nRowHeight)
{
    if (nRowIndex >= rowHeightVector.size()) {
        rowHeightVector.resize(nRowIndex + 1);
    }
    rowHeightVector[nRowIndex] = nRowHeight;
}

//设置行宽数据
inline static void SaveRowWidth(FastVector<int32_t>& rowWidthVector, uint32_t nRowIndex, int32_t nRowWidth)
{
    if (nRowIndex >= rowWidthVector.size()) {
        rowWidthVector.resize(nRowIndex + 1);
    }
    rowWidthVector[nRowIndex] = nRowWidth;
}

//设置行内容的向下偏移量
inline static void SaveRowTopOffset(FastVector<int32_t>& rowTopOffsetVector, uint32_t nRowIndex, int32_t nRowTopOffset)
{
    if (nRowIndex >= rowTopOffsetVector.size()) {
        rowTopOffsetVector.resize(nRowIndex + 1);
    }
    rowTopOffsetVector[nRowIndex] = nRowTopOffset;
}

UiRect DrawRichText::InternalDrawRichText(const UiRect& rcTextRect,
                                          const UiSize& szScrollOffset,
                                          IRenderFactory* pRenderFactory,
                                          const std::vector<RichTextData>& richTextData,
                                          uint8_t uFade,
                                          bool bMeasureOnly,
                                          RichTextLineInfoParam* pLineInfoParam,
                                          std::shared_ptr<DrawRichTextCache>* pDrawRichTextCache,
                                          std::vector<std::vector<UiRect>>* pRichTextRects)
{
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return UiRect();
    }

    //内部使用string_view实现，避免字符串复制影响性能
    if (rcTextRect.IsEmpty()) {
        return UiRect();
    }
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return UiRect();
    }

#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("DrawRichText::InternalDrawRichText");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    //绘制区域：绘制区域的坐标以 (rcTextRect.left,rcTextRect.top)作为(0,0)点
    UiRect rcDrawRect = rcTextRect;
    rcDrawRect.Offset(-szScrollOffset.cx, -szScrollOffset.cy);

    if ((pLineInfoParam != nullptr) || (pDrawRichTextCache != nullptr)) {
        ASSERT(bMeasureOnly);
        if (!bMeasureOnly) {
            return UiRect();
        }
        //使用(0,0)坐标点，作为估算的(0,0)点
        rcDrawRect.Offset(-rcDrawRect.left, -rcDrawRect.top);
        ASSERT(rcDrawRect.left == 0);
        ASSERT(rcDrawRect.top == 0);
    }

    //文本编码：固定为UTF16 或者 UTF32
    constexpr const SkTextEncoding textEncoding = (sizeof(DStringW::value_type) == sizeof(uint32_t)) ? SkTextEncoding::kUTF32 : SkTextEncoding::kUTF16;
    constexpr const size_t textCharSize = sizeof(DStringW::value_type);

    //当绘制超过目标矩形边界时，是否继续绘制
    const bool bBreakWhenOutOfRect = !bMeasureOnly && (pDrawRichTextCache == nullptr);

    //是否在布局阶段构建字形运行结果：只在需要绘制时构建（纯测量不会用到，避免增加测量开销）
    const bool bBuildGlyphRuns = !bMeasureOnly || (pDrawRichTextCache != nullptr);

    std::vector<TPendingDrawRichText> pendingTextData;
    pendingTextData.reserve(richTextData.size());

    const int32_t nTextRectRightMax = (int32_t)rcTextRect.right;   //绘制区域的最右侧
    const int32_t nTextRectBottomMax = (int32_t)rcTextRect.bottom; //绘制区域的最底端

    SkScalar xPos = (SkScalar)rcDrawRect.left;  //水平坐标：字符绘制的时候，是按浮点型坐标，每个字符所占的宽度是浮点型的，不能对齐到像素
    int32_t yPos = rcDrawRect.top;              //垂直坐标，对齐到像素，所以用整型
    int32_t nRowHeight = 0;   //行高（本行中，所有字符绘制高度的最大值，对齐到像素）
    int32_t nRowTopOffset = 0;//本行内容的向下偏移量（行内有高于文本的内联图片时，文本和图片整体下移）
    uint32_t nLineNumber = 0; //物理行号
    uint32_t nRowIndex = 0;   //逻辑行号
    uint32_t nLineTextRowIndex = 0; //物理行内的逻辑行号(每个物理行中，从0开始编号，跨文本片段连续编号)

    FastVector<int32_t> rowHeightVector; //每行的实际行高表
    rowHeightVector.reserve(richTextData.size() * 2);

    FastVector<int32_t> rowWidthVector; //每行的实际文本宽度表（用于逐行水平对齐）
    rowWidthVector.reserve(richTextData.size() * 2);

    FastVector<int32_t> rowTopOffsetVector; //每行内容的向下偏移量表
    rowTopOffsetVector.reserve(richTextData.size() * 2);
    SkScalar rowRightX = (SkScalar)rcDrawRect.left; //本行文本的最右侧坐标

    int32_t nMaxRowWidth = 0;                //内容的最大行宽（作为测量结果的宽度）
    int32_t nContentBottom = rcDrawRect.top; //内容的最底端（作为测量结果的高度）

    //保存行宽数据，并记录内容的最大行宽
    auto saveRowWidth = [&rowWidthVector, &nMaxRowWidth](uint32_t nRow, int32_t nWidth) {
        SaveRowWidth(rowWidthVector, nRow, nWidth);
        if (nWidth > nMaxRowWidth) {
            nMaxRowWidth = nWidth;
        }
    };

    //字体缓存(由于创建字体比较耗时，所以尽量复用相同的对象)
    SharePtr<UiFontEx> lastFont;
    std::shared_ptr<IFont> spLastSkiaFont;

    if (pLineInfoParam != nullptr) {
        //设置起始行号
        nLineNumber = (uint32_t)pLineInfoParam->m_nStartLineIndex;
        ASSERT(pLineInfoParam->m_pLineInfoList != nullptr);
        if (pLineInfoParam->m_pLineInfoList == nullptr) {
            return UiRect();
        }
        ASSERT(nLineNumber < pLineInfoParam->m_pLineInfoList->size());
        if (nLineNumber >= pLineInfoParam->m_pLineInfoList->size()) {
            return UiRect();
        }
        //起始的逻辑行号
        nRowIndex = pLineInfoParam->m_nStartRowIndex;
    }

    //绘制属性
    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        //透明度
        skPaint.setAlpha(uFade);
    }
    UiColor textColor;

    BreakTextTempData breakTextData;            //评估可绘制字符数量的临时变量（外部管理，以减少内存分配，提高性能）
    breakTextData.m_bNeedGlyphFonts = bBuildGlyphRuns; //只有需要构建绘制用run时，才产出字形命中的字体
    MeasureTextTempData measureTempData;        //内部临时变量，为提升执行速度，在外部声明变量

    //按换行符进行文本切分
    std::vector<std::wstring_view> lineTextViewList;

    //分行时文本切分的内部临时变量，为提升执行速度，在外部声明变量
    std::vector<uint32_t> lineSeprators;

    //是否正在绘制TAB键（按4个字符对齐）
    bool bDrawTabChar = false;

    //当前是否为段落的起始位置（首行缩进只对段落的第一行生效）
    bool bParagraphStart = true;

    //当前行是否已经绘制了文本（用于避免块级标签产生多余的空行）
    bool bRowHasText = false;

    //段落结束的行号（这些行不做两端对齐）
    std::vector<uint8_t> rowParagraphEnd;
    auto markRowParagraphEnd = [&rowParagraphEnd](uint32_t nRow) {
        if (rowParagraphEnd.size() <= nRow) {
            rowParagraphEnd.resize((size_t)nRow + 1, 0);
        }
        rowParagraphEnd[(size_t)nRow] = 1;
    };

    //是否因超出最大行数而截断
    bool bTruncated = false;

    //截断时最后一行的top坐标与是否显示省略号
    int32_t nTruncatedRowTop = 0;
    bool bTruncatedEllipsis = true;

    //发生截断时所在的文本片段索引（用于省略号的绘制区域归属）
    uint32_t nTruncatedDataIndex = 0;

    //是否因不换行文本超出可视宽度而在当前行（尚未换行的行）被截断
    bool bTruncatedAtCurrentRow = false;

    // const DString statHashBreakName = _T("DrawRichText::InternalDrawRichText Break");
    // PerformanceUtil::Instance().AddStat(statHashBreakName);
    // static const size_t statHashBreak = std::hash<DString>{}(statHashBreakName); //2710 ms

    for (size_t index = 0; index < richTextData.size(); ++index) {
        const RichTextData& textData = richTextData[index];
        if (textData.m_bHorizontalRule) {
            //水平分割线：独占一行
            if (xPos > (SkScalar)rcDrawRect.left) {
                ASSERT(((int64_t)yPos + (int64_t)nRowHeight) < INT32_MAX);
                yPos += nRowHeight;
                nContentBottom = yPos;
                SaveRowHeight(rowHeightVector, nRowIndex, nRowHeight);
                saveRowWidth(nRowIndex, SkScalarCeilToInt(rowRightX - rcDrawRect.left));
                SaveRowTopOffset(rowTopOffsetVector, nRowIndex, nRowTopOffset);
                markRowParagraphEnd(nRowIndex);
                xPos = (SkScalar)rcDrawRect.left;
                rowRightX = xPos;
                nRowHeight = 0;
                nRowTopOffset = 0;
                bRowHasText = false;
                ++nRowIndex;
                ++nLineNumber;
            }
            const int32_t nHrHeight = 1;
            TPendingDrawRichText& hrData = pendingTextData.emplace_back();
            hrData.m_nDataIndex = (uint32_t)index;
            hrData.m_nLineNumber = nLineNumber;
            hrData.m_nRowIndex = nRowIndex;
            hrData.m_textStyle = textData.m_textStyle;
            hrData.m_textColor = textData.m_textColor;
            hrData.m_bHorizontalRule = true;
            hrData.m_destRect.left = rcDrawRect.left;
            hrData.m_destRect.right = rcDrawRect.right;
            hrData.m_destRect.top = yPos;
            hrData.m_destRect.bottom = yPos + nHrHeight;
            SaveRowHeight(rowHeightVector, nRowIndex, nHrHeight);
            saveRowWidth(nRowIndex, rcDrawRect.Width());
            SaveRowTopOffset(rowTopOffsetVector, nRowIndex, 0);
            yPos += nHrHeight;
            nContentBottom = yPos;
            xPos = (SkScalar)rcDrawRect.left;
            rowRightX = xPos;
            nRowHeight = 0;
            nRowTopOffset = 0;
            bRowHasText = false;
            ++nRowIndex;
            ++nLineNumber;
            bParagraphStart = true;
            if (bBreakWhenOutOfRect && (yPos >= nTextRectBottomMax)) {
                break;
            }
            if ((textData.m_nMaxLines > 0) && (nRowIndex >= (uint32_t)textData.m_nMaxLines)) {
                bTruncated = true;
                nTruncatedRowTop = (int32_t)(yPos - nHrHeight);
                bTruncatedEllipsis = textData.m_bTextOverflowEllipsis;
                nTruncatedDataIndex = (uint32_t)index;
                break;
            }
            continue;
        }
        if (textData.m_textView.empty() && (textData.m_pImage == nullptr)) {
            continue;
        }

        //首行缩进：仅对段落的第一行生效（纯换行符不缩进，避免产生空行占位）
        if (bParagraphStart && (textData.m_nFirstLineIndent > 0) && (xPos <= (SkScalar)rcDrawRect.left) &&
            (HasDrawableText(textData.m_textView) || (textData.m_pImage != nullptr))) {
            xPos += (SkScalar)textData.m_nFirstLineIndent;
            rowRightX = std::max(rowRightX, xPos);
        }

        //设置文本颜色
        if (textColor != textData.m_textColor) {
            const UiColor& color = textData.m_textColor;
            //颜色自身的Alpha与叠加透明度相乘（uFade 不再覆盖颜色的Alpha）
            skPaint.setARGB((uint8_t)(((uint32_t)color.GetA() * (uint32_t)uFade + 127) / 255),
                            color.GetR(), color.GetG(), color.GetB());
            textColor = textData.m_textColor;
        }
        
        std::shared_ptr<IFont> spSkiaFont;
        if ((spLastSkiaFont != nullptr) &&
            (textData.m_pFontInfo != nullptr) && (lastFont != nullptr) &&
            ((textData.m_pFontInfo == lastFont) || (*textData.m_pFontInfo == *lastFont))) {
            //复用缓存中的字体对象
            spSkiaFont = spLastSkiaFont;
        }
        else {
            spSkiaFont.reset(pRenderFactory->CreateIFont());
            ASSERT(spSkiaFont != nullptr);
            if (spSkiaFont == nullptr) {
                continue;
            }
            ASSERT(textData.m_pFontInfo != nullptr);
            if (textData.m_pFontInfo == nullptr) {
                continue;
            }
            if (!spSkiaFont->InitFont(*textData.m_pFontInfo)) {
                spSkiaFont.reset();
                continue;
            }
        }

        Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(spSkiaFont.get());
        ASSERT(pSkiaFont != nullptr);
        if (pSkiaFont == nullptr) {
            continue;
        }
        const SkFont* pSkFont = pSkiaFont->GetFontHandle();
        ASSERT(pSkFont != nullptr);
        if (pSkFont == nullptr) {
            continue;
        }

        if (spLastSkiaFont != spSkiaFont) {
            spLastSkiaFont = spSkiaFont;
            lastFont = textData.m_pFontInfo;
        }

        FallbackFontCreator fallbackFontCreator = [this, spSkiaFont](SkUnichar unicodeChar, SkGlyphID* glyphId) {
            return DrawSkiaText::CreateFallbackFont(spSkiaFont.get(), unicodeChar, glyphId);
            };

        const SkFont& skFont = *pSkFont;
        SkFontMetrics fontMetrics;
        SkScalar fFontHeight = skFont.getMetrics(&fontMetrics);     //字体高度，换行时使用
        const SkScalar textMeasuredHeight = fontMetrics.fDescent - fontMetrics.fAscent; //当前要绘制的文本所需高度(使用字体的高度)
        fFontHeight = textData.m_fRowSpacingMul * fFontHeight + textData.m_fRowSpacingAdd; //运用行间距倍数和行间距附加量
        const int32_t nFontHeight = SkScalarCeilToInt(fFontHeight);   //行高对齐到像素
        nRowHeight = std::max(nRowHeight, nFontHeight);
        if (nRowHeight <= 0) {
            continue;
        }
        const uint32_t uTextStyle = textData.m_textStyle;
        const bool bSingleLineMode = (uTextStyle & DrawStringFormat::TEXT_SINGLELINE) ? true : false; //是否为单行模式，单行模式下，不换行
        const bool bWordWrap = bSingleLineMode ? false : ((uTextStyle & DrawStringFormat::TEXT_WORD_WRAP) ? true : false);
        //不换行/单行模式下：测量时按内容完整宽度布局；生成绘制缓存（实际绘制）时按可视宽度裁剪，避免越界绘制
        //注意：带行信息的测量（pLineInfoParam != nullptr，如RichEdit的排版与光标定位）必须按完整内容测量，
        //因此只对纯绘制缓存（pLineInfoParam == nullptr）启用裁剪
        const bool bClipNoWrapText = (!bWordWrap || bSingleLineMode) && (pDrawRichTextCache != nullptr) && (pLineInfoParam == nullptr);
        bool bBreakAll = false;//标记是否终止

        if (textData.m_pImage != nullptr) {
            //内联图片：图片底部与当前文本的基线对齐，图片独占一个绘制片段
            int32_t nImageWidth = textData.m_nImageWidth;
            int32_t nImageHeight = textData.m_nImageHeight;
            if (nImageWidth <= 0) {
                nImageWidth = (int32_t)textData.m_pImage->GetWidth();
            }
            if (nImageHeight <= 0) {
                nImageHeight = (int32_t)textData.m_pImage->GetHeight();
            }
            if ((nImageWidth <= 0) || (nImageHeight <= 0)) {
                //无法获取图片尺寸，忽略该图片
                continue;
            }

            //自动换行：当前行放不下图片时，先换行
            if (bWordWrap && (xPos > (SkScalar)rcDrawRect.left) &&
                ((xPos + (SkScalar)nImageWidth) > (SkScalar)rcDrawRect.right)) {
                xPos = (SkScalar)rcDrawRect.left;
                ASSERT(((int64_t)yPos + (int64_t)nRowHeight) < INT32_MAX);
                yPos += nRowHeight;
                nContentBottom = yPos;
                SaveRowHeight(rowHeightVector, nRowIndex, nRowHeight);
                saveRowWidth(nRowIndex, SkScalarCeilToInt(rowRightX - rcDrawRect.left));
                SaveRowTopOffset(rowTopOffsetVector, nRowIndex, nRowTopOffset);
                rowRightX = (SkScalar)rcDrawRect.left;
                nRowHeight = nFontHeight;
                nRowTopOffset = 0;
                bRowHasText = false;
                ++nRowIndex;

                if ((textData.m_nMaxLines > 0) && (nRowIndex >= (uint32_t)textData.m_nMaxLines)) {
                    //超过最大行数：终止绘制
                    bTruncated = true;
                    nTruncatedRowTop = (int32_t)(yPos - rowHeightVector[nRowIndex - 1]);
                    bTruncatedEllipsis = textData.m_bTextOverflowEllipsis;
                    nTruncatedDataIndex = (uint32_t)index;
                    bBreakAll = true;
                    break;
                }
                if (bBreakWhenOutOfRect && (yPos >= nTextRectBottomMax)) {
                    //Y坐标底部已经超出目标矩形的范围，终止绘制
                    bBreakAll = true;
                    break;
                }
            }

            //计算图片的绘制区域：图片底部对齐到文本基线
            const int32_t nAscent = SkScalarCeilToInt(-fontMetrics.fAscent);
            int32_t nImageBottom = yPos + nRowTopOffset + nAscent;
            int32_t nImageTop = nImageBottom - nImageHeight;
            if (nImageTop < yPos) {
                //图片高于本行的文本：本行向下扩展，图片顶部对齐到行顶，文本基线同步下移
                const int32_t nShift = yPos - nImageTop;
                for (TPendingDrawRichText& pendingData : pendingTextData) {
                    if (pendingData.m_nRowIndex == nRowIndex) {
                        pendingData.m_destRect.Offset(0, nShift);
                    }
                }
                nRowTopOffset += nShift;
                nRowHeight += nShift;
                nImageTop = yPos;
                nImageBottom = yPos + nImageHeight;
            }
            nRowHeight = std::max(nRowHeight, nImageBottom - yPos);

            TPendingDrawRichText& imgData = pendingTextData.emplace_back();
            imgData.m_nDataIndex = (uint32_t)index;
            imgData.m_nLineNumber = nLineNumber;
            imgData.m_nRowIndex = nRowIndex;
            imgData.m_spImage = textData.m_pImage;
            imgData.m_textStyle = textData.m_textStyle;
            imgData.m_textColor = textData.m_textColor;
            imgData.m_bgColor = textData.m_bgColor;
            imgData.m_destRect.left = SkScalarTruncToInt(xPos);
            imgData.m_destRect.right = SkScalarTruncToInt(xPos) + nImageWidth;
            imgData.m_destRect.top = nImageTop;
            imgData.m_destRect.bottom = nImageBottom;
            xPos += (SkScalar)nImageWidth;
            rowRightX = std::max(rowRightX, xPos);
            bParagraphStart = false;
            bRowHasText = true;
            continue;
        }

        //按换行符进行文本切分
        lineTextViewList.clear();
        SplitLines(textData.m_textView, lineSeprators, lineTextViewList);

        for (const std::wstring_view& lineTextView : lineTextViewList) {
            bDrawTabChar = false;
            if (lineTextView.size() == 1) {              
                if (lineTextView[0] == L'\r') {
                    //处理回车
                    if (pLineInfoParam != nullptr) {
                        OnDrawUnicodeChar(pLineInfoParam, lineTextView[0], 1, 2, nLineNumber, nLineTextRowIndex, xPos, yPos, 0, nRowHeight);
                    }
                    continue; //忽略回车
                }
                if (lineTextView[0] == L'\t') {
                    //处理TAB键
                    bDrawTabChar = true;
                }
                else if (lineTextView[0] == L'\n') {
                    //处理换行符
                    if (pLineInfoParam != nullptr) {
                        OnDrawUnicodeChar(pLineInfoParam, lineTextView[0], 1, 2, nLineNumber, nLineTextRowIndex, xPos, yPos, 0, nRowHeight);
                    }

                    //换行：执行换行操作(物理换行)
                    if (!bSingleLineMode) {
                        if (!bRowHasText && (nRowHeight == 0) && (nRowIndex > 0)) {
                            //当前行是空行且行高为0（例如块级标签紧跟在<hr>之后），忽略这个多余的换行，避免产生空行
                            bParagraphStart = true;
                            continue;
                        }
                        xPos = (SkScalar)rcDrawRect.left;
                        ASSERT(((int64_t)yPos + (int64_t)nRowHeight) < INT32_MAX);
                        yPos += nRowHeight;
                        nContentBottom = yPos;
                        SaveRowHeight(rowHeightVector, nRowIndex, nRowHeight);
                        saveRowWidth(nRowIndex, SkScalarCeilToInt(rowRightX - rcDrawRect.left));
                        SaveRowTopOffset(rowTopOffsetVector, nRowIndex, nRowTopOffset);
                        markRowParagraphEnd(nRowIndex);
                        rowRightX = (SkScalar)rcDrawRect.left;
                        nRowHeight = nFontHeight;
                        nRowTopOffset = 0;
                        bRowHasText = false;
                        ++nRowIndex;
                        nLineTextRowIndex = 0; //新物理行内的逻辑行号从0开始
                        ++nLineNumber;
                        bParagraphStart = true;
                        if ((textData.m_nMaxLines > 0) && (nRowIndex >= (uint32_t)textData.m_nMaxLines)) {
                            //超过最大行数：终止绘制
                            bTruncated = true;
                            nTruncatedRowTop = (int32_t)(yPos - rowHeightVector[nRowIndex - 1]);
                            bTruncatedEllipsis = textData.m_bTextOverflowEllipsis;
                            nTruncatedDataIndex = (uint32_t)index;
                            bBreakAll = true;
                            break;
                        }
                    }
                    continue; //处理下一行
                }
            }

            //绘制的文本下标开始值
            const size_t textCount = lineTextView.size();
            size_t textStartIndex = 0;
            while (textStartIndex < textCount) {
                //估算文本绘制区域                
                size_t byteLength = (textCount - textStartIndex) * textCharSize;                
                SkScalar maxWidth = SkIntToScalar(rcDrawRect.right) - xPos;//可用宽度
                if (!bWordWrap || bSingleLineMode) {
                    //不自动换行 或者 单行模式：测量时使用完整内容宽度；
                    //生成绘制缓存（实际绘制）时按可视宽度裁剪，避免文本越界绘制
                    if (!bClipNoWrapText) {
                        maxWidth = SK_FloatInfinity;
                    }
                }
                //if (maxWidth <= 0.0f) {
                //    ASSERT(maxWidth > 0);
                //}
                maxWidth = std::max(maxWidth, 0.0f);
                SkScalar textMeasuredWidth = 0;  //当前要绘制的文本，估算的所需宽度                

                breakTextData.glyphIDs.clear();
                breakTextData.glyphChars.clear();
                breakTextData.glyphWidths.clear();

                size_t nDrawLength = 0;
                if (bDrawTabChar) {
                    ASSERT(textCount == 1);
                    // 获取当前字体下一个空格的宽度
                    const DStringW blank = L"0";
                    SkScalar standardCharWidth = skFont.measureText(blank.c_str(), sizeof(DStringW::value_type), textEncoding, nullptr, &skPaint);
                    // 每个制表位 = 4个标准字符宽度
                    const int TAB_COUNT = 4;
                    SkScalar tabStopWidth = standardCharWidth * TAB_COUNT;

                    // 计算当前位置需要补齐多少宽度才到下一个制表位
                    // 制表位按行内偏移计算（相对于文本区域左边界），避免使用绝对坐标
                    SkScalar currentX = xPos - SkIntToScalar(rcDrawRect.left);
                    SkScalar remainder = fmod(currentX, tabStopWidth);
                    SkScalar tabWidth = tabStopWidth - remainder; // 最终制表符宽度

                    // 最终制表符宽度
                    textMeasuredWidth = tabWidth;
                    nDrawLength = textCount * sizeof(DStringW::value_type);
                    if (nDrawLength > 0) {
                        //TAB键按一个字符处理
                        breakTextData.glyphChars.clear();
                        breakTextData.glyphWidths.clear();
                        breakTextData.glyphIDs.resize(1, 0);
                        breakTextData.glyphChars.resize(1, sizeof(DStringW::value_type));
                        breakTextData.glyphWidths.resize(1, tabWidth);
                    }
                }
                else {
                    //breakText函数执行时间占比约40%
                    //PerformanceStatFast ssStatHashBreak(statHashBreak);
                    nDrawLength = DrawSkiaText::BreakText(lineTextView.data() + textStartIndex,
                                                          byteLength, textEncoding,
                                                          skFont, fallbackFontCreator, skPaint,
                                                          maxWidth, &textMeasuredWidth, 
                                                          measureTempData, &breakTextData);
                }

                bool bEllipsisTruncated = false;    //是否为省略号预留宽度而截断了绘制内容
                if (bClipNoWrapText && (nDrawLength < byteLength) && textData.m_bTextOverflowEllipsis && !bDrawTabChar) {
                    //不换行文本：剩余内容超出可视宽度，需要为末尾的省略号预留宽度。
                    //直接从已断行的字形数据中截取能放下的前缀，避免对同一段文本再次全量断行
                    const SkScalar fEllipsisWidth = skFont.measureText(g_ellipsisText, sizeof(DStringW::value_type), textEncoding, nullptr, &skPaint);
                    const SkScalar fPrefixWidth = std::max(maxWidth - fEllipsisWidth, 0.0f);
                    const size_t nGlyphCount = breakTextData.glyphWidths.size();
                    size_t nKeepCount = 0;      //保留下来的字形个数
                    size_t nKeepBytes = 0;      //保留下来的文本字节数
                    SkScalar fKeepWidth = 0;    //保留下来的文本宽度
                    for (size_t nGlyph = 0; nGlyph < nGlyphCount; ++nGlyph) {
                        const SkScalar fGlyphWidth = breakTextData.glyphWidths[nGlyph];
                        if ((fKeepWidth > 0.01) && ((fKeepWidth + fGlyphWidth) > fPrefixWidth)) {
                            //已经达到宽度限制（至少保留一个字符，与BreakText的行为保持一致）
                            break;
                        }
                        fKeepWidth += fGlyphWidth;
                        nKeepBytes += breakTextData.glyphChars[nGlyph];
                        ++nKeepCount;
                    }
                    if (nKeepCount < nGlyphCount) {
                        breakTextData.glyphIDs.resize(nKeepCount);
                        breakTextData.glyphChars.resize(nKeepCount);
                        breakTextData.glyphWidths.resize(nKeepCount);
                        if (!breakTextData.glyphFonts.empty()) {
                            breakTextData.glyphFonts.resize(nKeepCount);
                        }
                        nDrawLength = nKeepBytes;
                        textMeasuredWidth = fKeepWidth;
                        maxWidth = fPrefixWidth;
                        bEllipsisTruncated = true;
                    }
                }

                //字间距：BreakText按字体默认间距计算，这里按字间距修正可绘制的字形数量
                if (!bDrawTabChar && (nDrawLength > 0) && (textData.m_fLetterSpacing > 0.0f)) {
                    while (!breakTextData.glyphWidths.empty()) {
                        SkScalar fTotalWidth = 0;
                        const size_t nGlyphCount = breakTextData.glyphWidths.size();
                        for (size_t nGlyph = 0; nGlyph < nGlyphCount; ++nGlyph) {
                            fTotalWidth += breakTextData.glyphWidths[nGlyph];
                            if ((nGlyph + 1) < nGlyphCount) {
                                fTotalWidth += textData.m_fLetterSpacing;
                            }
                        }
                        if ((nGlyphCount <= 1) || (fTotalWidth <= maxWidth)) {
                            //当前字形可以放下
                            textMeasuredWidth = fTotalWidth;
                            nDrawLength = 0;
                            for (size_t nGlyph = 0; nGlyph < nGlyphCount; ++nGlyph) {
                                nDrawLength += breakTextData.glyphChars[nGlyph];
                            }
                            break;
                        }
                        //放不下时，去掉最后一个字形后重新计算
                        const size_t nLastGlyph = nGlyphCount - 1;
                        nDrawLength -= breakTextData.glyphChars[nLastGlyph];
                        breakTextData.glyphIDs.resize(nLastGlyph);
                        breakTextData.glyphChars.resize(nLastGlyph);
                        breakTextData.glyphWidths.resize(nLastGlyph);
                    }
                }

                //校验
                bool bBreakTextError = false;
                if (nDrawLength == 0) {
                    ASSERT(breakTextData.glyphIDs.empty() && breakTextData.glyphChars.empty() && breakTextData.glyphWidths.empty());
                    if (!breakTextData.glyphIDs.empty() || !breakTextData.glyphChars.empty() || !breakTextData.glyphWidths.empty()) {
                        bBreakTextError = true;
                    }
                }
                else {
                    ASSERT(!breakTextData.glyphIDs.empty() && !breakTextData.glyphChars.empty() && !breakTextData.glyphWidths.empty());
                    if ((breakTextData.glyphIDs.size() != breakTextData.glyphChars.size()) ||
                        (breakTextData.glyphIDs.size() != breakTextData.glyphWidths.size())) {
                        bBreakTextError = true;
                    }
                }
                if (bBreakTextError) {
                    //出错了(BreakText函数返回了不正常的值)
                    bBreakAll = true;
                    break;
                }
                
                if (nDrawLength == 0) {
                    //位于行首时可用宽度最大，此时仍无法绘制任何字符，说明无法继续绘制
                    const bool bAtRowStart = (xPos <= SkIntToScalar(rcDrawRect.left));
                    if (!bWordWrap || bSingleLineMode || bAtRowStart) {
                        //出错了(不能换行，或者换行后依然不够)
                        if (bClipNoWrapText) {
                            //不可换行文本超出可视宽度：按截断处理（必要时在行末追加省略号）
                            bTruncated = true;
                            bTruncatedAtCurrentRow = true;
                            bTruncatedEllipsis = textData.m_bTextOverflowEllipsis;
                            nTruncatedRowTop = yPos;
                            nTruncatedDataIndex = (uint32_t)index;
                        }
                        bBreakAll = true;
                        break;
                    }
                }
                else {
                    TPendingDrawRichText& newPendingTextData = pendingTextData.emplace_back();
                    newPendingTextData.m_nDataIndex = (uint32_t)index;
                    newPendingTextData.m_nLineNumber = nLineNumber;
                    newPendingTextData.m_nRowIndex = nRowIndex;
                    newPendingTextData.m_textView = std::wstring_view(lineTextView.data() + textStartIndex, nDrawLength / textCharSize);
                    newPendingTextData.m_spFont = spSkiaFont;

                    newPendingTextData.m_bgColor = textData.m_bgColor;
                    newPendingTextData.m_textColor = textData.m_textColor;
                    newPendingTextData.m_textStyle = textData.m_textStyle;
                    newPendingTextData.m_fLetterSpacing = textData.m_fLetterSpacing;
                    newPendingTextData.m_nGlyphCount = (uint32_t)breakTextData.glyphIDs.size();

                    //构建绘制用的字形运行结果：绘制阶段直接批量提交，不再重新断行与解析字形。
                    //字形ID与宽度直接复用断行结果，保证绘制位置与布局阶段的排版一致。
                    if (!bDrawTabChar && bBuildGlyphRuns && !bEllipsisTruncated) {
                        size_t nGlyphCount = breakTextData.glyphIDs.size();
                        if (breakTextData.glyphFonts.size() < nGlyphCount) {
                            //字间距修正会裁掉末尾的字形，此时字形ID/宽度/字体数组取交集
                            nGlyphCount = breakTextData.glyphFonts.size();
                        }
                        if (nGlyphCount > 0) {
                            size_t nTextBytes = 0;
                            for (size_t nIndex = 0; nIndex < nGlyphCount; ++nIndex) {
                                nTextBytes += breakTextData.glyphChars[nIndex];
                            }
                            //行尾空白不参与绘制（与SkTextBox的行为保持一致，避免影响水平对齐）
                            while ((nGlyphCount > 0) && (nTextBytes >= textCharSize) &&
                                   (newPendingTextData.m_textView[(nTextBytes / textCharSize) - 1] == L' ')) {
                                --nGlyphCount;
                                nTextBytes -= textCharSize;
                            }
                            //字形数量需要与参与绘制的字形保持一致（两端对齐时按实际绘制的字形数分配额外间距）
                            newPendingTextData.m_nGlyphCount = (uint32_t)nGlyphCount;
                            if (nGlyphCount > 0) {
                                std::shared_ptr<TextGlyphRunList> spGlyphRuns = std::make_shared<TextGlyphRunList>();
                                DrawSkiaText::BuildTextRuns(breakTextData.glyphIDs.data(),
                                                            breakTextData.glyphWidths.data(),
                                                            breakTextData.glyphFonts.data(),
                                                            nGlyphCount, *spGlyphRuns,
                                                            newPendingTextData.m_fGlyphRunsWidth);
                                newPendingTextData.m_spGlyphRuns = std::move(spGlyphRuns);
                            }
                        }
                    }

                    bParagraphStart = false;
                    bRowHasText = true;

                    //绘制文字所需的矩形区域
                    newPendingTextData.m_destRect.left = SkScalarTruncToInt(xPos); //左值：直接截断，如果有小数部分，直接去掉小数即可

                    SkScalar fRight = xPos + textMeasuredWidth;             //右值：如果有小数，则需要增加1个像素
                    newPendingTextData.m_destRect.right = SkScalarCeilToInt(fRight);
                    newPendingTextData.m_destRect.top = yPos + nRowTopOffset + textData.m_nBaselineOffset;
                    newPendingTextData.m_destRect.bottom = newPendingTextData.m_destRect.top + SkScalarCeilToInt(textMeasuredHeight); //记录字符的真实高度
                    rowRightX = std::max(rowRightX, fRight); //更新本行的最右侧坐标

                    if (pLineInfoParam != nullptr) {
                        //评估每个字符的矩形范围
                        const size_t glyphCount = breakTextData.glyphChars.size();
                        SkScalar glyphWidth = 0;
                        uint8_t glyphCharCount = 0;
                        SkScalar glyphLeft = (SkScalar)SkScalarTruncToInt(xPos);
                        for (size_t glyphIndex = 0; glyphIndex < glyphCount; ++glyphIndex) {
                            glyphWidth = breakTextData.glyphWidths[glyphIndex];     //字符宽度
                            glyphCharCount = breakTextData.glyphChars[glyphIndex];  //该字占几个字节，2或者4
                            ASSERT((glyphCharCount == 2) || (glyphCharCount == 4));
                            ASSERT((glyphCharCount % sizeof(DStringW::value_type)) == 0);
                            glyphCharCount /= sizeof(DStringW::value_type); //转换为StringW编码字符数，内部使用
                            ASSERT((glyphCharCount == 1) || (glyphCharCount == 2)); //该字占几个字符（UTF16编码，可能是1或者2）
                            OnDrawUnicodeChar(pLineInfoParam, 0, glyphCharCount, glyphCount, nLineNumber, nLineTextRowIndex, glyphLeft, yPos, glyphWidth, nRowHeight);
                            glyphLeft += glyphWidth;
                        }
                    }
                }

                if (bClipNoWrapText && (nDrawLength < byteLength)) {
                    //不可换行文本超出可视宽度：截断剩余内容，必要时在行末追加省略号
                    bTruncated = true;
                    bTruncatedAtCurrentRow = true;
                    bTruncatedEllipsis = textData.m_bTextOverflowEllipsis;
                    nTruncatedRowTop = yPos;
                    nTruncatedDataIndex = (uint32_t)index;
                    bBreakAll = true;
                    break;
                }

                bool bNextRow = false; //是否需要换行的标志
                if (nDrawLength < byteLength) {
                    //宽度不足，需要换行
                    if (!bSingleLineMode) {
                        bNextRow = true;
                    }                    
                    textStartIndex += nDrawLength / textCharSize;
                    ASSERT(((int64_t)xPos + (int64_t)textMeasuredWidth) < INT32_MAX);
                    xPos += textMeasuredWidth;
                }
                else {
                    //当前行可容纳文本绘制
                    textStartIndex = textCount;//标记，结束循环
                    ASSERT(((int64_t)xPos + (int64_t)textMeasuredWidth) < INT32_MAX);
                    xPos += textMeasuredWidth;
                    //文本恰好占满一行时，如果后面没有其他文本，则不需要换行（避免多算一行）
                    const bool bHasMoreData = (index + 1) < richTextData.size();
                    if ((xPos > nTextRectRightMax) || ((xPos >= nTextRectRightMax) && bHasMoreData)) {
                        //X坐标右侧已经超出目标矩形的范围
                        if (bBreakWhenOutOfRect && bSingleLineMode) {
                            //单行模式，终止绘制
                            bBreakAll = true;
                            break;
                        }
                        else if (bWordWrap && !bSingleLineMode) {
                            //在自动换行的情况下，换行
                            bNextRow = true;
                        }
                    }
                }

                if (bNextRow) {
                    //换行：执行换行操作（逻辑换行，对nLineNumber不增加）
                    xPos = (SkScalar)rcDrawRect.left;
                    ASSERT(((int64_t)yPos + (int64_t)nRowHeight) < INT32_MAX);
                    yPos += nRowHeight;
                    nContentBottom = yPos;
                    SaveRowHeight(rowHeightVector, nRowIndex, nRowHeight);
                    saveRowWidth(nRowIndex, SkScalarCeilToInt(rowRightX - rcDrawRect.left));
                    SaveRowTopOffset(rowTopOffsetVector, nRowIndex, nRowTopOffset);
                    rowRightX = (SkScalar)rcDrawRect.left;
                    nRowHeight = nFontHeight;
                    nRowTopOffset = 0;
                    bRowHasText = false;
                    ++nRowIndex;
                    ++nLineTextRowIndex;

                    if ((textData.m_nMaxLines > 0) && (nRowIndex >= (uint32_t)textData.m_nMaxLines)) {
                        //超过最大行数：终止绘制
                        bTruncated = true;
                        nTruncatedRowTop = (int32_t)(yPos - rowHeightVector[nRowIndex - 1]);
                        bTruncatedEllipsis = textData.m_bTextOverflowEllipsis;
                        nTruncatedDataIndex = (uint32_t)index;
                        bBreakAll = true;
                        break;
                    }

                    if (bBreakWhenOutOfRect && (yPos >= nTextRectBottomMax)) {
                        //Y坐标底部已经超出目标矩形的范围，终止绘制
                        bBreakAll = true;
                        break;
                    }
                }
            }
            if (bBreakAll) {
                break;
            }
        }
        if (bBreakAll) {
            break;
        }
    }

    //记录最后一行的行高：最后一行没有内容时不计入（如文本以换行结尾），避免多算一行；
    //不换行文本在当前行被截断时，即使该行还没有绘制任何文本，也需要记录行高（用于绘制省略号）
    if (bRowHasText || rowHeightVector.empty() || bTruncatedAtCurrentRow) {
        SaveRowHeight(rowHeightVector, nRowIndex, nRowHeight);
        saveRowWidth(nRowIndex, SkScalarCeilToInt(rowRightX - rcDrawRect.left));
        SaveRowTopOffset(rowTopOffsetVector, nRowIndex, nRowTopOffset);
        nContentBottom = yPos + nRowHeight;
    }

    //文本被截断：在最后一行的末尾追加省略号（text_overflow="clip"时不追加）
    if (bTruncated && bTruncatedEllipsis && (spLastSkiaFont != nullptr) &&
        ((nRowIndex > 0) || bTruncatedAtCurrentRow)) {
        const uint32_t nLastRow = bTruncatedAtCurrentRow ? nRowIndex : (nRowIndex - 1);
        Font_Skia* pLastFont = dynamic_cast<Font_Skia*>(spLastSkiaFont.get());
        const SkFont* pLastSkFont = (pLastFont != nullptr) ? pLastFont->GetFontHandle() : nullptr;
        if ((pLastSkFont != nullptr) && (nLastRow < rowHeightVector.size()) && (nLastRow < rowWidthVector.size())) {
            FallbackFontCreator fallbackFontCreator = [this, spLastSkiaFont](SkUnichar unicodeChar, SkGlyphID* glyphId) {
                return DrawSkiaText::CreateFallbackFont(spLastSkiaFont.get(), unicodeChar, glyphId);
                };
            const SkScalar fEllipsisWidth = DrawSkiaText::MeasureTextChar(*pLastSkFont, (DUTF32Char)0x2026, nullptr, m_pSkPaint, fallbackFontCreator, true);
            SkScalar fLeft = (SkScalar)rcDrawRect.left + (SkScalar)rowWidthVector[nLastRow];
            if ((fLeft + fEllipsisWidth) > (SkScalar)rcDrawRect.right) {
                fLeft = (SkScalar)rcDrawRect.right - fEllipsisWidth;
            }
            fLeft = std::max(fLeft, (SkScalar)rcDrawRect.left);

            TPendingDrawRichText& ellipsisData = pendingTextData.emplace_back();
            ellipsisData.m_nDataIndex = nTruncatedDataIndex;
            ellipsisData.m_nLineNumber = nLineNumber;
            ellipsisData.m_nRowIndex = nLastRow;
            ellipsisData.m_textView = GetEllipsisTextView();
            ellipsisData.m_spFont = spLastSkiaFont;
            ellipsisData.m_textColor = textColor;
            ellipsisData.m_destRect.left = SkScalarTruncToInt(fLeft);
            ellipsisData.m_destRect.right = SkScalarCeilToInt(fLeft + fEllipsisWidth + 1);
            int32_t nLastRowTopOffset = 0;
            if (nLastRow < rowTopOffsetVector.size()) {
                nLastRowTopOffset = rowTopOffsetVector[nLastRow];
            }
            ellipsisData.m_destRect.top = nTruncatedRowTop + nLastRowTopOffset;
            ellipsisData.m_destRect.bottom = ellipsisData.m_destRect.top + rowHeightVector[nLastRow] - nLastRowTopOffset;
        }
    }

    //逐行两端对齐：把行内的剩余宽度平均分配到该行的每个字形之间
    const int32_t nAvailableWidth = rcDrawRect.Width();
    if ((nAvailableWidth > 0) && !pendingTextData.empty()) {
        std::vector<uint32_t> rowGlyphCount(rowWidthVector.size(), 0);
        std::vector<uint8_t> rowHasImage(rowWidthVector.size(), 0);
        for (const TPendingDrawRichText& textData : pendingTextData) {
            if (textData.m_nRowIndex >= rowGlyphCount.size()) {
                continue;
            }
            if (textData.m_spImage != nullptr) {
                //含内联图片的行不两端对齐（图片宽度固定，无法分摊额外间距）
                rowHasImage[textData.m_nRowIndex] = 1;
            }
            else if (!textData.m_bHorizontalRule) {
                rowGlyphCount[textData.m_nRowIndex] += textData.m_nGlyphCount;
            }
        }
        const size_t nLastRowIndex = rowWidthVector.empty() ? 0 : (rowWidthVector.size() - 1);
        for (TPendingDrawRichText& textData : pendingTextData) {
            if (textData.m_bHorizontalRule || (textData.m_spImage != nullptr) ||
                ((textData.m_textStyle & DrawStringFormat::TEXT_HJUSTIFY) == 0)) {
                continue;
            }
            if ((textData.m_nRowIndex >= nLastRowIndex) || (textData.m_nRowIndex >= rowWidthVector.size())) {
                continue; //最后一行不两端对齐
            }
            if ((textData.m_nRowIndex < rowHasImage.size()) && (rowHasImage[textData.m_nRowIndex] != 0)) {
                continue; //含内联图片的行不两端对齐
            }
            if ((textData.m_nRowIndex < rowParagraphEnd.size()) && (rowParagraphEnd[textData.m_nRowIndex] != 0)) {
                continue; //段落的最后一行不两端对齐
            }
            const int32_t nRowWidth = rowWidthVector[textData.m_nRowIndex];
            const uint32_t nGlyphCount = rowGlyphCount[textData.m_nRowIndex];
            if ((nGlyphCount > 1) && (nRowWidth > 0) && (nRowWidth < nAvailableWidth)) {
                textData.m_fJustifySpacing = (SkScalar)(nAvailableWidth - nRowWidth) / (SkScalar)(nGlyphCount - 1);
            }
        }
    }

    //更新每行的行高(只有提前确定行高，才能正确绘制纵向对齐的文本)
    for (TPendingDrawRichText& textData : pendingTextData) {
        if (textData.m_spImage != nullptr) {
            //内联图片的高度由图片自身决定，不随行高变化
            continue;
        }
        if (textData.m_nRowIndex < rowHeightVector.size()) {
            int32_t nTextRowTopOffset = 0;
            if (textData.m_nRowIndex < rowTopOffsetVector.size()) {
                nTextRowTopOffset = rowTopOffsetVector[textData.m_nRowIndex];
            }
            //本行内容整体下移时，文本的矩形底部不应超出本行的底部
            textData.m_destRect.bottom = textData.m_destRect.top + rowHeightVector[textData.m_nRowIndex] - nTextRowTopOffset;
        }
    }

    //逐行水平对齐：按行计算起始偏移（水平居中或右对齐）
    if (!bMeasureOnly) {
        for (TPendingDrawRichText& textData : pendingTextData) {
            if (textData.m_nRowIndex >= rowWidthVector.size()) {
                continue;
            }
            const int32_t nRowWidth = rowWidthVector[textData.m_nRowIndex];
            if ((nRowWidth > 0) && (nRowWidth < nAvailableWidth)) {
                int32_t nXOffset = 0;
                if (textData.m_textStyle & DrawStringFormat::TEXT_HCENTER) {
                    nXOffset = (nAvailableWidth - nRowWidth) / 2;
                }
                else if (textData.m_textStyle & DrawStringFormat::TEXT_RIGHT) {
                    nXOffset = nAvailableWidth - nRowWidth;
                }
                if (nXOffset > 0) {
                    textData.m_destRect.Offset(nXOffset, 0);
                }
            }
        }
    }

    if (pRichTextRects != nullptr) {
        pRichTextRects->clear();
        pRichTextRects->resize(richTextData.size());
        for (const TPendingDrawRichText& textData : pendingTextData) {
            //保存绘制的目标区域，同一个文本，可能会有多个区域（换行时）
            ASSERT(textData.m_nDataIndex < pRichTextRects->size());
            std::vector<UiRect>& textRects = (*pRichTextRects)[textData.m_nDataIndex];
            textRects.push_back(textData.m_destRect); 
        }
    }

    if (pDrawRichTextCache != nullptr) {
        //生成绘制缓存，但不执行绘制
        std::shared_ptr<DrawRichTextCache> spDrawRichTextCache = std::make_shared<DrawRichTextCache>();
        *pDrawRichTextCache = spDrawRichTextCache;
        spDrawRichTextCache->m_richTextData = richTextData;
        spDrawRichTextCache->m_textRect = rcTextRect;

        spDrawRichTextCache->m_textEncoding = textEncoding;
        spDrawRichTextCache->m_textCharSize = textCharSize;

        spDrawRichTextCache->m_pendingTextData.swap(pendingTextData);
        spDrawRichTextCache->m_rowWidthVector.assign(rowWidthVector.begin(), rowWidthVector.end());
    }
    else if (!bMeasureOnly) {
        UiRect rcTemp;
        for (const TPendingDrawRichText& textData : pendingTextData) {
            //执行绘制            
            const UiRect& rcDestRect = textData.m_destRect;
            if (!UiRect::Intersect(rcTemp, rcDestRect, rcTextRect)) {
                continue;
            }

            //绘制水平分割线
            if (textData.m_bHorizontalRule) {
                m_pRender->FillRect(UiRectF::MakeFromRect(rcTemp), textData.m_textColor, uFade);
                continue;
            }

            //绘制内联图片
            if (textData.m_spImage != nullptr) {
                const int32_t nImageWidth = (int32_t)textData.m_spImage->GetWidth();
                const int32_t nImageHeight = (int32_t)textData.m_spImage->GetHeight();
                if ((nImageWidth > 0) && (nImageHeight > 0)) {
                    if (!textData.m_bgColor.IsEmpty()) {
                        m_pRender->FillRect(UiRectF::MakeFromRect(rcDestRect), textData.m_bgColor, uFade);
                    }
                    m_pRender->DrawImageRect(rcTemp, textData.m_spImage.get(), rcDestRect,
                                             UiRect(0, 0, nImageWidth, nImageHeight), uFade);
                }
                continue;
            }

            //绘制文字的背景色（无背景色时不绘制，与使用缓存时的逻辑保持一致）
            if (!textData.m_bgColor.IsEmpty()) {
                m_pRender->FillRect(UiRectF::MakeFromRect(rcDestRect), textData.m_bgColor, uFade);
            }

            if (textColor != textData.m_textColor) {
                const UiColor& color = textData.m_textColor;
                //颜色自身的Alpha与叠加透明度相乘（uFade 不再覆盖颜色的Alpha）
                skPaint.setARGB((uint8_t)(((uint32_t)color.GetA() * (uint32_t)uFade + 127) / 255),
                                color.GetR(), color.GetG(), color.GetB());
                textColor = textData.m_textColor;
            }

            //不自动换行时，超出的部分裁剪到控件区域内，并用省略号表示
            uint32_t uDrawFormat = textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE;
            UiRect rcDrawText = rcDestRect;
            if (!(textData.m_textStyle & DrawStringFormat::TEXT_WORD_WRAP) && (rcDrawText.right > rcTextRect.right)) {
                rcDrawText.right = rcTextRect.right;
                uDrawFormat |= DrawStringFormat::TEXT_END_ELLIPSIS;
            }
            if (rcDrawText.right <= rcDrawText.left) {
                continue;
            }

            //绘制文字
            if ((textData.m_textView.size() == 1) && (textData.m_textView[0] == L'\t')) {
                //绘制TAB键
                const char* text = (const char*)" ";
                const size_t len = 1; //字节数
                DrawTextString(rcDrawText, text, len, SkTextEncoding::kUTF8,
                               uDrawFormat,
                               skPaint, textData.m_spFont.get());
            }
            else {
                //字间距 + 两端对齐的额外间距
                const SkScalar fSpacing = textData.m_fLetterSpacing + textData.m_fJustifySpacing;
                DrawTextRun(rcDrawText, textData.m_textView, textCharSize, textEncoding,
                            uDrawFormat, skPaint, textData.m_spFont.get(), fSpacing,
                            textData.m_spGlyphRuns.get(), textData.m_fGlyphRunsWidth);
            }
        }
    }
    //返回内容的最大行宽和总高度（以(0,0)为起点，与入参的坐标系无关）
    return UiRect(0, 0, nMaxRowWidth, nContentBottom - rcDrawRect.top);
}

void DrawRichText::DrawTextRun(const UiRect& textRect, const std::wstring_view& textView, size_t textCharSize,
                               SkTextEncoding textEncoding, uint32_t uFormat,
                               const SkPaint& skPaint, IFont* pFont, float fSpacing,
                               const TextGlyphRunList* pGlyphRuns, float fGlyphRunsWidth) const
{
    if (textView.empty() || (pFont == nullptr)) {
        return;
    }

    //布局阶段已经产出了字形运行结果时，直接批量提交（不再重新断行、不再解析字符）。
    //启用省略号时绘制内容会被截断，下划线/删除线需要在绘制文字后另画线条，这两种情况仍走原实现。
    if ((pGlyphRuns != nullptr) && !pGlyphRuns->IsEmpty() &&
        (m_pSkCanvas != nullptr) && (m_pSkPointOrg != nullptr) &&
        ((uFormat & (DrawStringFormat::TEXT_END_ELLIPSIS | DrawStringFormat::TEXT_PATH_ELLIPSIS)) == 0)) {
        Font_Skia* pRunFont = dynamic_cast<Font_Skia*>(pFont);
        const SkFont* pRunSkFont = (pRunFont != nullptr) ? pRunFont->GetFontHandle() : nullptr;
        if ((pRunSkFont != nullptr) && !pRunFont->IsUnderline() && !pRunFont->IsStrikeOut()) {
            size_t nGlyphCount = 0;
            const size_t nRunCount = pGlyphRuns->GetRunCount();
            for (size_t nRunIndex = 0; nRunIndex < nRunCount; ++nRunIndex) {
                nGlyphCount += pGlyphRuns->GetRun(nRunIndex).m_nCount;
            }
            //整段文本的实际宽度（包含字间距）
            SkScalar fTotalWidth = fGlyphRunsWidth;
            if ((fSpacing > 0.0f) && (nGlyphCount > 1)) {
                fTotalWidth += (SkScalar)(nGlyphCount - 1) * fSpacing;
            }

            SkFontMetrics fontMetrics;
            pRunSkFont->getMetrics(&fontMetrics);
            const SkScalar fTextHeight = fontMetrics.fDescent - fontMetrics.fAscent;
            const SkScalar fBoxWidth = SkIntToScalar(textRect.Width());
            const SkScalar fBoxHeight = SkIntToScalar(textRect.Height());

            //水平对齐
            SkScalar fX = SkIntToScalar(textRect.left);
            if (fTotalWidth < fBoxWidth) {
                if (uFormat & DrawStringFormat::TEXT_HCENTER) {
                    fX += (fBoxWidth - fTotalWidth) / 2;
                }
                else if (uFormat & DrawStringFormat::TEXT_RIGHT) {
                    fX += fBoxWidth - fTotalWidth;
                }
            }

            //纵向对齐
            SkScalar fTop = SkIntToScalar(textRect.top);
            if (uFormat & DrawStringFormat::TEXT_VCENTER) {
                fTop += (fBoxHeight - fTextHeight) / 2;
            }
            else if (uFormat & DrawStringFormat::TEXT_BOTTOM) {
                fTop += fBoxHeight - fTextHeight;
            }
            const SkScalar fBaseline = fTop - fontMetrics.fAscent;

            //裁剪到绘制区域（两端对齐/字间距会使文本超出片段自身的宽度，这里按整段文本的实际宽度放宽右边界）
            SkCanvas* skCanvas = m_pSkCanvas;
            skCanvas->save();
            if (!(uFormat & DrawStringFormat::TEXT_NOCLIP)) {
                SkScalar fClipRight = SkIntToScalar(textRect.right);
                const SkScalar fRunRight = fX + fTotalWidth;
                if (fRunRight > fClipRight) {
                    fClipRight = fRunRight;
                }
                SkRect rcClip = SkRect::MakeLTRB(SkIntToScalar(textRect.left), SkIntToScalar(textRect.top),
                                                 fClipRight, SkIntToScalar(textRect.bottom));
                rcClip.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);
                skCanvas->clipRect(rcClip);
            }
            DrawSkiaText::DrawGlyphRuns(skCanvas, *pGlyphRuns, 0, nRunCount,
                                        fX + m_pSkPointOrg->fX, fBaseline + m_pSkPointOrg->fY,
                                        skPaint, fSpacing);
            skCanvas->restore();
            return;
        }
    }

    if (fSpacing <= 0.0f) {
        //按默认字间距绘制
        const char* text = (const char*)textView.data();
        const size_t len = textView.size() * textCharSize; //字节数
        DrawTextString(textRect, text, len, textEncoding, uFormat, skPaint, pFont);
        return;
    }

    //按字间距逐字绘制
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFont);
    if ((pSkiaFont == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPointOrg == nullptr)) {
        return;
    }
    const SkFont* pSkFont = pSkiaFont->GetFontHandle();
    if (pSkFont == nullptr) {
        return;
    }

    FallbackFontCreator fallbackFontCreator = [this, pFont](SkUnichar unicodeChar, SkGlyphID* glyphId) {
        return DrawSkiaText::CreateFallbackFont(pFont, unicodeChar, glyphId);
        };

    struct TGlyphItem
    {
        DUTF32Char m_ch = 0;
        SkScalar m_fWidth = 0;
    };
    std::vector<TGlyphItem> glyphItems;
    glyphItems.reserve(textView.size());

    SkScalar fTotalWidth = 0;
    for (size_t nIndex = 0; nIndex < textView.size();) {
        DUTF32Char ch = (DUTF32Char)textView[nIndex];
        size_t nCharCount = 1;
        if ((sizeof(DStringW::value_type) == sizeof(uint16_t)) &&
            (ch >= 0xD800) && (ch <= 0xDBFF) && ((nIndex + 1) < textView.size())) {
            const DUTF32Char chLow = (DUTF32Char)textView[nIndex + 1];
            if ((chLow >= 0xDC00) && (chLow <= 0xDFFF)) {
                //UTF16编码的代理对
                ch = 0x10000 + ((ch - 0xD800) << 10) + (chLow - 0xDC00);
                nCharCount = 2;
            }
        }
        TGlyphItem& glyphItem = glyphItems.emplace_back();
        glyphItem.m_ch = ch;
        glyphItem.m_fWidth = DrawSkiaText::MeasureTextChar(*pSkFont, ch, nullptr, &skPaint, fallbackFontCreator, true);
        fTotalWidth += glyphItem.m_fWidth;
        if ((nIndex + nCharCount) < textView.size()) {
            fTotalWidth += fSpacing;
        }
        nIndex += nCharCount;
    }
    if (glyphItems.empty()) {
        return;
    }

    SkFontMetrics fontMetrics;
    pSkFont->getMetrics(&fontMetrics);
    const SkScalar fTextHeight = fontMetrics.fDescent - fontMetrics.fAscent;
    const SkScalar fBoxWidth = SkIntToScalar(textRect.Width());
    const SkScalar fBoxHeight = SkIntToScalar(textRect.Height());

    //水平对齐
    SkScalar fX = SkIntToScalar(textRect.left);
    if (fTotalWidth < fBoxWidth) {
        if (uFormat & DrawStringFormat::TEXT_HCENTER) {
            fX += (fBoxWidth - fTotalWidth) / 2;
        }
        else if (uFormat & DrawStringFormat::TEXT_RIGHT) {
            fX += fBoxWidth - fTotalWidth;
        }
    }

    //纵向对齐
    SkScalar fTop = SkIntToScalar(textRect.top);
    if (uFormat & DrawStringFormat::TEXT_VCENTER) {
        fTop += (fBoxHeight - fTextHeight) / 2;
    }
    else if (uFormat & DrawStringFormat::TEXT_BOTTOM) {
        fTop += fBoxHeight - fTextHeight;
    }
    const SkScalar fBaseline = fTop - fontMetrics.fAscent;

    //裁剪到绘制区域（两端对齐/字间距会使文本超出片段自身的宽度，这里按整段文本的实际宽度放宽右边界）
    SkCanvas* skCanvas = m_pSkCanvas;
    skCanvas->save();
    SkScalar fClipRight = SkIntToScalar(textRect.right);
    const SkScalar fRunRight = fX + fTotalWidth;
    if (fRunRight > fClipRight) {
        fClipRight = fRunRight;
    }
    SkRect rcClip = SkRect::MakeLTRB(SkIntToScalar(textRect.left), SkIntToScalar(textRect.top),
                                     fClipRight, SkIntToScalar(textRect.bottom));
    rcClip.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);
    skCanvas->clipRect(rcClip);
    for (const TGlyphItem& glyphItem : glyphItems) {
        DrawSkiaText::DrawSimpleText(skCanvas, glyphItem.m_ch,
                                     fX + m_pSkPointOrg->fX, fBaseline + m_pSkPointOrg->fY,
                                     *pSkFont, skPaint, fallbackFontCreator);
        fX += glyphItem.m_fWidth + fSpacing;
    }
    skCanvas->restore();
}

void DrawRichText::SplitLines(const std::wstring_view& lineText, std::vector<uint32_t>& lineSeprators, std::vector<std::wstring_view>& lineTextViewList)
{
    if (lineText.empty()) {
        return;
    }
    lineSeprators.clear();
    const uint32_t nTextLen = (uint32_t)lineText.size();
    lineSeprators.reserve(nTextLen/100);
    for (uint32_t nTextIndex = 0; nTextIndex < nTextLen; ++nTextIndex) {
        const std::wstring_view::value_type& ch = lineText[nTextIndex];
        if ((ch == L'\t') || (ch == L'\r') || (ch == L'\n')) {
            lineSeprators.push_back(nTextIndex);
        }
    }
    if (lineSeprators.empty()) {
        //没有换行分隔符，单行
        lineTextViewList.emplace_back(std::wstring_view(lineText.data(), lineText.size()));
    }
    else {
        //有换行分隔符，切分为多行, 并保留换行符
        size_t nLastIndex = 0;
        size_t nCurrentIndex = 0;
        size_t nCharCount = 0;
        const size_t nLineSepCount = lineSeprators.size();
        lineTextViewList.reserve(nLineSepCount * 2 + 1);
        for (size_t nLine = 0; nLine < nLineSepCount; ++nLine) {
            if (nLine == 0) {
                //首行
                nLastIndex = 0;
                nCurrentIndex = lineSeprators[nLine];
                ASSERT(nCurrentIndex < lineText.size());
                nCharCount = nCurrentIndex - nLastIndex;
                if (nCharCount > 0) {
                    lineTextViewList.emplace_back(std::wstring_view(lineText.data(), nCharCount));
                }
                lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nCurrentIndex, 1));
            }
            else {
                //中间行
                nLastIndex = lineSeprators[nLine - 1];
                nCurrentIndex = lineSeprators[nLine];
                ASSERT(nCurrentIndex > nLastIndex);
                ASSERT(nCurrentIndex < lineText.size());
                nCharCount = nCurrentIndex - nLastIndex - 1;
                if (nCharCount > 0) {
                    lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nLastIndex + 1, nCharCount));
                }
                lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nCurrentIndex, 1));
            }

            if (nLine == (nLineSepCount - 1)) {
                //末行: 将最后一行数据添加进来
                nLastIndex = lineSeprators[nLine];
                nCurrentIndex = lineText.size();
                ASSERT(nCurrentIndex > nLastIndex);
                nCharCount = nCurrentIndex - nLastIndex - 1;
                if (nCharCount > 0) {
                    lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nLastIndex + 1, nCharCount));
                }
            }
        }
    }
}

void DrawRichText::OnDrawUnicodeChar(RichTextLineInfoParam* pLineInfoParam,
                                     DStringW::value_type ch, uint8_t glyphChars, size_t glyphCount,
                                     size_t nLineTextIndex, uint32_t nLineTextRowIndex,
                                     float xPos, int32_t nRowTop, float glyphWidth, int32_t nRowHeight)
{
    //该函数执行频率非常高，每个字符都会调用一次，性能敏感    
    if (pLineInfoParam == nullptr) {
        ASSERT(pLineInfoParam != nullptr);
        return;
    }    
    if (pLineInfoParam->m_pLineInfoList == nullptr) {
        ASSERT(pLineInfoParam->m_pLineInfoList != nullptr);
        return;
    }
    size_t nIndex = nLineTextIndex; //外部已经加上了pLineInfoParam->m_nStartLineIndex的值    
    if (nIndex >= pLineInfoParam->m_pLineInfoList->size()) {
        ASSERT(nIndex < pLineInfoParam->m_pLineInfoList->size());
        return;
    }
    if ((*pLineInfoParam->m_pLineInfoList)[nIndex] == nullptr) {
        ASSERT((*pLineInfoParam->m_pLineInfoList)[nIndex] != nullptr);
        return;
    }

    RichTextLineInfo& lineInfo = *(*pLineInfoParam->m_pLineInfoList)[nIndex];
    bool bFound = (nLineTextRowIndex < lineInfo.m_rowInfo.size()) ? true : false;
    if (!bFound) {        
        if (nLineTextRowIndex != lineInfo.m_rowInfo.size()) {
            ASSERT(nLineTextRowIndex == lineInfo.m_rowInfo.size());
            return;
        }
        lineInfo.m_rowInfo.push_back(RichTextRowInfoPtr(new RichTextRowInfo));        
    }    
    if (lineInfo.m_rowInfo[nLineTextRowIndex] == nullptr) {
        ASSERT(lineInfo.m_rowInfo[nLineTextRowIndex] != nullptr);
        return;
    }
    RichTextRowInfo& rowInfo = *lineInfo.m_rowInfo[nLineTextRowIndex];
    if (!bFound) {
        //该行的第一个字符：行矩形的高度取当前行高
        rowInfo.m_rowRect.left = xPos;
        rowInfo.m_rowRect.right = xPos + glyphWidth;
        rowInfo.m_rowRect.top = (SkScalar)nRowTop;
        rowInfo.m_rowRect.bottom = rowInfo.m_rowRect.top + nRowHeight;
        ASSERT(nRowHeight > 0);

        rowInfo.m_charInfo.reserve(glyphCount + 2);
    }
    else {
        rowInfo.m_rowRect.right += glyphWidth;
        //行内出现更高的内容（如高于文本的内联图片、更大字号的文字）时，行高会增大，
        //此时需要同步更新行矩形的高度（行的顶部不变），否则字符位置信息与实际行高不一致
        if (nRowHeight != (int32_t)rowInfo.m_rowRect.Height()) {
            rowInfo.m_rowRect.bottom = rowInfo.m_rowRect.top + nRowHeight;
        }
    }

    RichTextCharInfo charInfo;
    charInfo.SetCharWidth(glyphWidth);    
    if (ch == '\r') {
        //回车
        charInfo.SetCharFlag(0);
        charInfo.AddCharFlag(RichTextCharFlag::kIsIgnoredChar);
        charInfo.AddCharFlag(RichTextCharFlag::kIsReturn);
        charInfo.SetCharWidth(0);
    }
    else if (ch == '\n') {
        //换行
        charInfo.SetCharFlag(0);
        charInfo.AddCharFlag(RichTextCharFlag::kIsNewLine);
        charInfo.SetCharWidth(0);
    }

    rowInfo.m_charInfo.emplace_back(std::move(charInfo));

    if (glyphChars == 2) {
        RichTextCharInfo charInfo2;
        charInfo2.SetCharWidth(0);
        charInfo2.SetCharFlag(0);
        charInfo2.AddCharFlag(RichTextCharFlag::kIsIgnoredChar);
        charInfo2.AddCharFlag(RichTextCharFlag::kIsLowSurrogate);
        rowInfo.m_charInfo.emplace_back(std::move(charInfo2));
    }
    ASSERT((glyphChars == 1) || (glyphChars == 2));
}

bool DrawRichText::IsValidDrawRichTextCache(const UiRect& textRect,
                                            const std::vector<RichTextData>& richTextData,
                                            const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                            bool bCompareTextColor)
{
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return false;
    }
    if (spDrawRichTextCache == nullptr) {
        return false;
    }
    if ((spDrawRichTextCache->m_textRect.Width() != textRect.Width()) ||
        (spDrawRichTextCache->m_textRect.Height() != textRect.Height())) {
        //矩形大小发生变化，不能使用缓存(位置变化时，可以使用缓存)
        return false;
    }
    if (spDrawRichTextCache->m_richTextData.size() != richTextData.size()) {
        return false;
    }
    bool bValid = true;
    const size_t nCount = richTextData.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        const RichTextData& textData = richTextData[nIndex];
        const RichTextData& textDataCache = spDrawRichTextCache->m_richTextData[nIndex];
        if (textData.m_textView.data() != textDataCache.m_textView.data()) {
            bValid = false;
        }
        else if (textData.m_textView.size() != textDataCache.m_textView.size()) {
            bValid = false;
        }
        else if (bCompareTextColor && (textData.m_textColor != textDataCache.m_textColor)) {
            //文字颜色不影响排版：绘制时可通过pTextColors覆盖，避免悬停/按下时重新排版
            bValid = false;
        }
        else if (textData.m_bgColor != textDataCache.m_bgColor) {
            bValid = false;
        }
        else if (textData.m_pFontInfo != textDataCache.m_pFontInfo) {
            if ((textData.m_pFontInfo == nullptr) || (textDataCache.m_pFontInfo == nullptr)) {
                bValid = false;
            }
            else if (*textData.m_pFontInfo != *textDataCache.m_pFontInfo) {
                bValid = false;
            }
        }
        else if (textData.m_fRowSpacingMul != textDataCache.m_fRowSpacingMul) {
            bValid = false;
        }
        else if (textData.m_fRowSpacingAdd != textDataCache.m_fRowSpacingAdd) {
            bValid = false;
        }
        else if (textData.m_textStyle != textDataCache.m_textStyle) {
            bValid = false;
        }
        else if (textData.m_fLetterSpacing != textDataCache.m_fLetterSpacing) {
            bValid = false;
        }
        else if (textData.m_nBaselineOffset != textDataCache.m_nBaselineOffset) {
            bValid = false;
        }
        else if (textData.m_nFirstLineIndent != textDataCache.m_nFirstLineIndent) {
            bValid = false;
        }
        else if ((textData.m_nMaxLines != textDataCache.m_nMaxLines) ||
                 (textData.m_bTextOverflowEllipsis != textDataCache.m_bTextOverflowEllipsis)) {
            bValid = false;
        }
        else if (textData.m_bHorizontalRule != textDataCache.m_bHorizontalRule) {
            bValid = false;
        }
        else if ((textData.m_pImage != textDataCache.m_pImage) ||
                 (textData.m_nImageWidth != textDataCache.m_nImageWidth) ||
                 (textData.m_nImageHeight != textDataCache.m_nImageHeight)) {
            bValid = false;
        }

        if (!bValid) {
            break;
        }
    }
    return bValid;
}

bool DrawRichText::UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>& spOldDrawRichTextCache,
                                           const std::shared_ptr<DrawRichTextCache>& spUpdateDrawRichTextCache,
                                           std::vector<RichTextData>& richTextDataNew,
                                           size_t nStartLine,
                                           const std::vector<size_t>& modifiedLines,
                                           size_t nModifiedRows,
                                           const std::vector<size_t>& deletedLines,
                                           size_t nDeletedRows,
                                           const std::vector<int32_t>& rowRectTopList)
{
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return false;
    }
    ASSERT(spOldDrawRichTextCache != nullptr);
    if (spOldDrawRichTextCache == nullptr) {
        return false;
    }

    ASSERT(!modifiedLines.empty() || !deletedLines.empty());

#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("DrawRichText::UpdateDrawRichTextCache");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    if (!modifiedLines.empty()) {
        if (nStartLine != modifiedLines[0]) {
            ASSERT(modifiedLines.empty() || (nStartLine == modifiedLines[0]));
            return false;
        }        
    }
    else if (!deletedLines.empty()) {
        if (nStartLine != deletedLines[0]) {
            ASSERT(deletedLines.empty() || (nStartLine == deletedLines[0]));
            return false;
        }
    }

    //删除的行数据，对应移除
    DrawRichTextCache& oldData = *spOldDrawRichTextCache;
    oldData.m_richTextData.swap(richTextDataNew);

    if (!deletedLines.empty()) {
        std::unordered_set<uint32_t> deletedLineSet;
        for (size_t nLine : deletedLines) {
            deletedLineSet.insert((uint32_t)nLine);
        }
        const int32_t nCount = (int32_t)oldData.m_pendingTextData.size();
        for (int32_t nIndex = nCount - 1; nIndex >= 0; --nIndex) {
            const TPendingDrawRichText& pendingData = oldData.m_pendingTextData[nIndex];
            if (deletedLineSet.find(pendingData.m_nLineNumber) != deletedLineSet.end()) {
                oldData.m_pendingTextData.erase(oldData.m_pendingTextData.begin() + nIndex);
            }
        }
    }

    //更新缓存开始的元素下标值    
    size_t nUpdateCacheStartIndex = (size_t)-1;
    //是否将修改的内容追加到最后了
    bool bAppendUpdateAtEnd = false;
    if (spUpdateDrawRichTextCache != nullptr) {
        DrawRichTextCache& updateData = *spUpdateDrawRichTextCache;
        if (!updateData.m_pendingTextData.empty()) {//容器可能为空（当本行为空行时为空）
            //缓存的文本矩形，其位置可能发生变化（如控件移动、滚动后重新布局），
            //此时仍可复用缓存（绘制时按新的矩形位置计算目标区域），
            //但宽度和高度必须一致，否则行数据（相对坐标）不能合并
            ASSERT((updateData.m_textRect.Width() == oldData.m_textRect.Width()) &&
                   (updateData.m_textRect.Height() == oldData.m_textRect.Height()));
            if ((updateData.m_textRect.Width() != oldData.m_textRect.Width()) ||
                (updateData.m_textRect.Height() != oldData.m_textRect.Height())) {
                return false;
            }
            //同步缓存中记录的文本矩形，避免后续比较（IsDrawRichTextCacheEqual）时位置不一致
            oldData.m_textRect = updateData.m_textRect;
            ASSERT(updateData.m_textEncoding == oldData.m_textEncoding);
            if (updateData.m_textEncoding != oldData.m_textEncoding) {
                return false;
            }
            ASSERT(updateData.m_textCharSize == oldData.m_textCharSize);
            if (updateData.m_textCharSize != oldData.m_textCharSize) {
                return false;
            }
            //将新的绘制缓存，合并到原绘制缓存中
            const int32_t nCount = (int32_t)oldData.m_pendingTextData.size();
            for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
                const TPendingDrawRichText& pendingData = oldData.m_pendingTextData[nIndex];
                if (pendingData.m_nLineNumber > nStartLine) {
                    oldData.m_pendingTextData.insert(oldData.m_pendingTextData.begin() + nIndex, updateData.m_pendingTextData.begin(), updateData.m_pendingTextData.end());
                    nUpdateCacheStartIndex = nIndex + updateData.m_pendingTextData.size();
                    break;
                }
            }
            if (nUpdateCacheStartIndex == (size_t)-1) {
                //追加在最后, 行号（物理行号，逻辑行号）无需更新，因为增量绘制的时候已经设置了正确的值
                bAppendUpdateAtEnd = true;
                oldData.m_pendingTextData.insert(oldData.m_pendingTextData.end(), updateData.m_pendingTextData.begin(), updateData.m_pendingTextData.end());                
            }
        }
    }

    int32_t nLineNumberDiff = (int32_t)modifiedLines.size() - (int32_t)deletedLines.size();
    int32_t nRowIndexDiff = (int32_t)nModifiedRows - (int32_t)nDeletedRows;
    bool bUpdateIndex = (nLineNumberDiff != 0) || (nRowIndexDiff != 0);

    //修正物理行号，逻辑行号，本行的绘制目标区域值
    const size_t nRowRectCount = rowRectTopList.size();
    bool bUpdateLineRows = false;
    const int32_t nCount = (int32_t)oldData.m_pendingTextData.size();
    for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        TPendingDrawRichText& pendingData = oldData.m_pendingTextData[nIndex];
        if (!bUpdateLineRows && bUpdateIndex) {
            if ((nUpdateCacheStartIndex != (size_t)-1)) {
                //更新行号(有修改，并且修改点不再最后)
                if (nIndex >= (int32_t)nUpdateCacheStartIndex) {
                    bUpdateLineRows = true;
                }
            }
            else if (!bAppendUpdateAtEnd) {
                //无修改，只有删除；或者新修改的行只是空行的情况
                if (pendingData.m_nLineNumber >= nStartLine) {
                    bUpdateLineRows = true;
                }
            }
        }
        if (bUpdateLineRows) {
            if (nLineNumberDiff > 0) {
                pendingData.m_nLineNumber += (uint32_t)nLineNumberDiff;
            }
            else if (nLineNumberDiff < 0) {
                pendingData.m_nLineNumber -= (uint32_t)-nLineNumberDiff;
            }
            if (nRowIndexDiff > 0) {
                pendingData.m_nRowIndex += (uint32_t)nRowIndexDiff;
            }
            else if (nRowIndexDiff < 0) {
                pendingData.m_nRowIndex -= (uint32_t)-nRowIndexDiff;
            }
        }

        if (pendingData.m_nLineNumber >= nStartLine) {
            //对应数据的索引下标更新为无效值，后续不再使用
            pendingData.m_nDataIndex = (uint32_t)-1;

            //更新本行的绘制目标区域
            ASSERT(pendingData.m_nRowIndex < nRowRectCount);
            if (pendingData.m_nRowIndex < nRowRectCount) {
                pendingData.m_destRect.bottom = rowRectTopList[pendingData.m_nRowIndex] + pendingData.m_destRect.Height();
                pendingData.m_destRect.top = rowRectTopList[pendingData.m_nRowIndex];
            }
        }
    }
    //增量更新后，每行的宽度数据已失效，避免使用过期的数据做逐行对齐
    oldData.m_rowWidthVector.clear();
    return true;
}

bool DrawRichText::IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const
{
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return false;
    }

    ASSERT(first.m_textRect == second.m_textRect);
    if (first.m_textRect != second.m_textRect) {
        return false;
    }

    ASSERT(first.m_textEncoding == second.m_textEncoding);
    if (first.m_textEncoding != second.m_textEncoding) {
        return false;
    }

    ASSERT(first.m_textCharSize == second.m_textCharSize);
    if (first.m_textCharSize != second.m_textCharSize) {
        return false;
    }

    ASSERT(first.m_richTextData.size() == second.m_richTextData.size());
    if (first.m_richTextData.size() != second.m_richTextData.size()) {
        return false;
    }
    const size_t nDataCount = first.m_richTextData.size();
    for (size_t nIndex = 0; nIndex < nDataCount; ++nIndex) {
        const RichTextData& v1 = first.m_richTextData[nIndex];
        const RichTextData& v2 = second.m_richTextData[nIndex];
        ASSERT(v1.m_textView == v2.m_textView);
        if (v1.m_textView != v2.m_textView) {
            return false;
        }
        ASSERT(v1.m_textColor == v2.m_textColor);
        if (v1.m_textColor != v2.m_textColor) {
            return false;
        }
        ASSERT(v1.m_bgColor == v2.m_bgColor);
        if (v1.m_bgColor != v2.m_bgColor) {
            return false;
        }
        ASSERT((v1.m_pFontInfo != nullptr) && (v2.m_pFontInfo != nullptr));
        if ((v1.m_pFontInfo == nullptr) || (v2.m_pFontInfo == nullptr)) {
            return false;
        }
        ASSERT(*v1.m_pFontInfo == *v2.m_pFontInfo);
        if (*v1.m_pFontInfo != *v2.m_pFontInfo) {
            return false;
        }
        ASSERT(v1.m_fRowSpacingMul == v2.m_fRowSpacingMul);
        if (v1.m_fRowSpacingMul != v2.m_fRowSpacingMul) {
            return false;
        }
        ASSERT(v1.m_fRowSpacingAdd == v2.m_fRowSpacingAdd);
        if (v1.m_fRowSpacingAdd != v2.m_fRowSpacingAdd) {
            return false;
        }
        ASSERT(v1.m_textStyle == v2.m_textStyle);
        if (v1.m_textStyle != v2.m_textStyle) {
            return false;
        }
    }

    ASSERT(first.m_pendingTextData.size() == second.m_pendingTextData.size());
    if (first.m_pendingTextData.size() != second.m_pendingTextData.size()) {
        return false;
    }

    const size_t nCount = first.m_pendingTextData.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        const TPendingDrawRichText& v1 = first.m_pendingTextData[nIndex];
        const TPendingDrawRichText& v2 = second.m_pendingTextData[nIndex];

        //m_nDataIndex 此值不需要比较
        ASSERT(v1.m_nLineNumber == v2.m_nLineNumber);
        if (v1.m_nLineNumber != v2.m_nLineNumber) {
            return false;
        }
        ASSERT(v1.m_nRowIndex == v2.m_nRowIndex);
        if (v1.m_nRowIndex != v2.m_nRowIndex) {
            return false;
        }
        ASSERT(v1.m_textView == v2.m_textView);
        if (v1.m_textView != v2.m_textView) {
            return false;
        }
        ASSERT(v1.m_destRect == v2.m_destRect);
        if (v1.m_destRect != v2.m_destRect) {
            return false;
        }

        ASSERT((v1.m_spFont != nullptr) && (v2.m_spFont != nullptr));
        if ((v1.m_spFont == nullptr) || (v2.m_spFont == nullptr)) {
            return false;
        }
        ASSERT(v1.m_spFont->FontName() == v2.m_spFont->FontName());
        if (v1.m_spFont->FontName() != v2.m_spFont->FontName()) {
            return false;
        }
        ASSERT(v1.m_spFont->FontSize() == v2.m_spFont->FontSize());
        if (v1.m_spFont->FontSize() != v2.m_spFont->FontSize()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsBold() == v2.m_spFont->IsBold());
        if (v1.m_spFont->IsBold() != v2.m_spFont->IsBold()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsUnderline() == v2.m_spFont->IsUnderline());
        if (v1.m_spFont->IsUnderline() != v2.m_spFont->IsUnderline()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsItalic() == v2.m_spFont->IsItalic());
        if (v1.m_spFont->IsItalic() != v2.m_spFont->IsItalic()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsStrikeOut() == v2.m_spFont->IsStrikeOut());
        if (v1.m_spFont->IsStrikeOut() != v2.m_spFont->IsStrikeOut()) {
            return false;
        }
        
        ASSERT(v1.m_textColor == v2.m_textColor);
        if (v1.m_textColor != v2.m_textColor) {
            return false;
        }
        ASSERT(v1.m_bgColor == v2.m_bgColor);
        if (v1.m_bgColor != v2.m_bgColor) {
            return false;
        }
        ASSERT(v1.m_textStyle == v2.m_textStyle);
        if (v1.m_textStyle != v2.m_textStyle) {
            return false;
        }
    }
    return true;
}

void DrawRichText::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                         const UiRect& rcNewTextRect,
                                         const UiSize& szNewScrollOffset,
                                         const std::vector<int32_t>& rowXOffset,
                                         uint8_t uFade,
                                         std::vector<std::vector<UiRect>>* pRichTextRects,
                                         const std::vector<UiColor>* pTextColors)
{
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return;
    }

    ASSERT(spDrawRichTextCache != nullptr);
    if (spDrawRichTextCache == nullptr) {
        return;
    }

#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("DrawRichText::DrawRichTextCacheData");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    const SkTextEncoding textEncoding = spDrawRichTextCache->m_textEncoding;
    const size_t textCharSize = spDrawRichTextCache->m_textCharSize;

    const std::vector<TPendingDrawRichText>& pendingTextData = spDrawRichTextCache->m_pendingTextData;

    UiRect rcTemp;
    UiRect rcDestRect;
    //绘制属性
    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        //透明度
        skPaint.setAlpha(uFade);
    }

    if (pRichTextRects != nullptr) {
        pRichTextRects->clear();
        pRichTextRects->resize(spDrawRichTextCache->m_richTextData.size());
    }

    UiColor textColor;
    for (const TPendingDrawRichText& textData : pendingTextData) {
        //通过缓存绘制的时候，不能使用textData.m_nDataIndex值，此值再增量绘制的情况下是不正确的
        
        //执行绘制        
        rcDestRect = textData.m_destRect;
        rcDestRect.Offset(rcNewTextRect.left, rcNewTextRect.top);
        rcDestRect.Offset(-szNewScrollOffset.cx, -szNewScrollOffset.cy);
        if (!rowXOffset.empty()) {
            ASSERT(textData.m_nRowIndex < rowXOffset.size());
            if ((textData.m_nRowIndex < rowXOffset.size()) && (rowXOffset[textData.m_nRowIndex] > 0)) {
                rcDestRect.Offset(rowXOffset[textData.m_nRowIndex], 0);
            }
        }
        else if (textData.m_nRowIndex < spDrawRichTextCache->m_rowWidthVector.size()) {
            //逐行水平对齐：按行计算起始偏移（水平居中或右对齐）
            const int32_t nRowWidth = spDrawRichTextCache->m_rowWidthVector[textData.m_nRowIndex];
            const int32_t nAvailableWidth = rcNewTextRect.Width();
            if ((nRowWidth > 0) && (nRowWidth < nAvailableWidth)) {
                int32_t nXOffset = 0;
                if (textData.m_textStyle & DrawStringFormat::TEXT_HCENTER) {
                    nXOffset = (nAvailableWidth - nRowWidth) / 2;
                }
                else if (textData.m_textStyle & DrawStringFormat::TEXT_RIGHT) {
                    nXOffset = nAvailableWidth - nRowWidth;
                }
                if (nXOffset > 0) {
                    rcDestRect.Offset(nXOffset, 0);
                }
            }
        }

        //记录绘制区域
        if (pRichTextRects != nullptr) {
            //保存绘制的目标区域，同一个文本，可能会有多个区域（换行时）
            ASSERT(textData.m_nDataIndex < pRichTextRects->size());
            std::vector<UiRect>& textRects = (*pRichTextRects)[textData.m_nDataIndex];
            textRects.push_back(rcDestRect);
        }

        if (!UiRect::Intersect(rcTemp, rcDestRect, rcNewTextRect)) {
            continue;
        }

        //绘制水平分割线
        if (textData.m_bHorizontalRule) {
            const UiColor* pDrawTextColor = &textData.m_textColor;
            if ((pTextColors != nullptr) && (textData.m_nDataIndex < pTextColors->size())) {
                pDrawTextColor = &((*pTextColors)[textData.m_nDataIndex]);
            }
            m_pRender->FillRect(UiRectF::MakeFromRect(rcTemp), *pDrawTextColor, uFade);
            continue;
        }

        //绘制内联图片
        if (textData.m_spImage != nullptr) {
            const int32_t nImageWidth = (int32_t)textData.m_spImage->GetWidth();
            const int32_t nImageHeight = (int32_t)textData.m_spImage->GetHeight();
            if ((nImageWidth > 0) && (nImageHeight > 0)) {
                if (!textData.m_bgColor.IsEmpty()) {
                    m_pRender->FillRect(UiRectF::MakeFromRect(rcDestRect), textData.m_bgColor, uFade);
                }
                m_pRender->DrawImageRect(rcTemp, textData.m_spImage.get(), rcDestRect,
                                         UiRect(0, 0, nImageWidth, nImageHeight), uFade);
            }
            continue;
        }

        //绘制文字的背景色
        if (!textData.m_bgColor.IsEmpty()) {
            m_pRender->FillRect(UiRectF::MakeFromRect(rcDestRect), textData.m_bgColor, uFade);
        }

        //设置文本颜色（优先使用外部传入的颜色，用于悬停/按下等仅改变颜色的场景）
        const UiColor* pDrawTextColor = &textData.m_textColor;
        if ((pTextColors != nullptr) && (textData.m_nDataIndex < pTextColors->size())) {
            pDrawTextColor = &((*pTextColors)[textData.m_nDataIndex]);
        }
        if (textColor != *pDrawTextColor) {
            const UiColor& color = *pDrawTextColor;
            //颜色自身的Alpha与叠加透明度相乘（uFade 不再覆盖颜色的Alpha）
            skPaint.setARGB((uint8_t)(((uint32_t)color.GetA() * (uint32_t)uFade + 127) / 255),
                            color.GetR(), color.GetG(), color.GetB());
            textColor = *pDrawTextColor;
        }

        //不自动换行时，超出的部分裁剪到控件区域内，并用省略号表示
        uint32_t uDrawFormat = textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE;
        UiRect rcDrawText = rcDestRect;
        rcDrawText.right = std::min(rcDrawText.right, rcNewTextRect.right);
        if (!(textData.m_textStyle & DrawStringFormat::TEXT_WORD_WRAP) && (rcDestRect.right > rcNewTextRect.right)) {
            uDrawFormat |= DrawStringFormat::TEXT_END_ELLIPSIS;
        }
        if (rcDrawText.right <= rcDrawText.left) {
            continue;
        }

        //绘制文字
        if ((textData.m_textView.size() == 1) && (textData.m_textView[0] == L'\t')) {
            //绘制TAB键
            const char* text = (const char*)" ";
            const size_t len = 1; //字节数
            DrawTextString(rcDrawText, text, len, SkTextEncoding::kUTF8,
                            uDrawFormat,
                            skPaint, textData.m_spFont.get());
        }
        else {
            //字间距 + 两端对齐的额外间距
            const SkScalar fSpacing = textData.m_fLetterSpacing + textData.m_fJustifySpacing;
            DrawTextRun(rcDrawText, textData.m_textView, textCharSize, textEncoding,
                        uDrawFormat, skPaint, textData.m_spFont.get(), fSpacing,
                        textData.m_spGlyphRuns.get(), textData.m_fGlyphRunsWidth);
        }
    }
}

void DrawRichText::DrawTextString(const UiRect& textRect, const DString& strText, uint32_t uFormat,
                                 const SkPaint& skPaint, IFont* pFont) const
{
    ASSERT(!strText.empty());
    if (strText.empty()) {
        return;
    }
    const char* text = (const char*)strText.c_str();
    size_t len = strText.size() * sizeof(DString::value_type);
    SkTextEncoding textEncoding = GetTextEncoding();
    DrawTextString(textRect, text, len, textEncoding, uFormat, skPaint, pFont);
}

void DrawRichText::DrawTextString(const UiRect& textRect,
                                  const char* text, size_t len, SkTextEncoding textEncoding,
                                  uint32_t uFormat, const SkPaint& skPaint, IFont* pFont) const
{
    SkCanvas* skCanvas = m_pSkCanvas;
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return;
    }
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFont);
    ASSERT(pSkiaFont != nullptr);
    if (pSkiaFont == nullptr) {
        return;
    }
    const SkFont* pSkFont = pSkiaFont->GetFontHandle();
    ASSERT(pSkFont != nullptr);
    if (pSkFont == nullptr) {
        return;
    }

    //绘制区域
    SkIRect rcSkDestI = { textRect.left, textRect.top, textRect.right, textRect.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    //设置绘制属性
    SkTextBox skTextBox;
    skTextBox.SetBox(rcSkDest);
    if (uFormat & DrawStringFormat::TEXT_SINGLELINE) {
        //单行文本
        skTextBox.SetLineMode(TextBoxLineMode::kOneLine_Mode);
    }

    //绘制区域不足时，自动在末尾绘制省略号
    bool bEndEllipsis = false;
    if (uFormat & DrawStringFormat::TEXT_END_ELLIPSIS) {
        bEndEllipsis = true;
    }
    skTextBox.SetEndEllipsis(bEndEllipsis);

    bool bPathEllipsis = false;
    if (uFormat & DrawStringFormat::TEXT_PATH_ELLIPSIS) {
        bPathEllipsis = true;
    }
    skTextBox.SetPathEllipsis(bPathEllipsis);

    //绘制文字时，不使用裁剪区域（可能会导致文字绘制超出边界）
    if (uFormat & DrawStringFormat::TEXT_NOCLIP) {
        skTextBox.SetClipBox(false);
    }
    //删除线
    skTextBox.SetStrikeOut(pSkiaFont->IsStrikeOut());
    //下划线
    skTextBox.SetUnderline(pSkiaFont->IsUnderline());

    if (uFormat & DrawStringFormat::TEXT_HCENTER) {
        //横向对齐：居中对齐
        skTextBox.SetTextAlign(SkTextBox::kCenter_Align);
    }
    else if (uFormat & DrawStringFormat::TEXT_RIGHT) {
        //横向对齐：右对齐
        skTextBox.SetTextAlign(SkTextBox::kRight_Align);
    }
    else {
        //横向对齐：左对齐
        skTextBox.SetTextAlign(SkTextBox::kLeft_Align);
    }

    if (uFormat & DrawStringFormat::TEXT_VCENTER) {
        //纵向对齐：居中对齐
        skTextBox.SetSpacingAlign(SkTextBox::kCenter_SpacingAlign);
    }
    else if (uFormat & DrawStringFormat::TEXT_BOTTOM) {
        //纵向对齐：下对齐
        skTextBox.SetSpacingAlign(SkTextBox::kEnd_SpacingAlign);
    }
    else {
        //纵向对齐：上对齐
        skTextBox.SetSpacingAlign(SkTextBox::kStart_SpacingAlign);
    }
    FallbackFontCreator fallbackFontCreator = [this, pFont](SkUnichar unicodeChar, SkGlyphID* glyphId) {
        return DrawSkiaText::CreateFallbackFont(pFont, unicodeChar, glyphId);
        };
    skTextBox.Draw(skCanvas, SkiaTextData(text, len, textEncoding), *pSkFont, skPaint, fallbackFontCreator);
}

SkTextEncoding DrawRichText::GetTextEncoding() const
{
    constexpr const size_t nValueLen = sizeof(DString::value_type);
    if constexpr (nValueLen == 1) {
        return SkTextEncoding::kUTF8;
    }
    else if constexpr (nValueLen == 2) {
        return SkTextEncoding::kUTF16;
    }
    else if constexpr (nValueLen == 4) {
        return SkTextEncoding::kUTF32;
    }
    else {
#ifdef DUILIB_UNICODE
        return SkTextEncoding::kUTF16;
#else
        return SkTextEncoding::kUTF8;
#endif
    }
}


} //namespace ui {
