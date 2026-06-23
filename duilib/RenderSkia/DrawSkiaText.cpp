#include "DrawSkiaText.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/SkUTF.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/PerformanceUtil.h"

namespace ui
{

IFallbackFontMgr* DrawSkiaText::GetFallbackFontMgr(const IFont* pFont)
{
    IFallbackFontMgr* pFallbackFontMgr = nullptr;
    if (pFont != nullptr) {
        IFontMgr* pFontMgr = nullptr;
        const Font_Skia* pSkiaFont = dynamic_cast<const Font_Skia*>(pFont);
        ASSERT(pSkiaFont != nullptr);
        if (pSkiaFont != nullptr) {
            pFontMgr = pSkiaFont->GetFontMgr();
        }
        if (pFontMgr != nullptr) {
            pFallbackFontMgr = pFontMgr->GetFallbackFontMgr();
        }
    }
    return pFallbackFontMgr;
}

const SkFont* DrawSkiaText::CreateFallbackFont(const IFont* pFont, SkUnichar unicodeChar, SkGlyphID* glyphId)
{
    IFont* pFallbackFont = nullptr;
    IFallbackFontMgr* pFallbackFontMgr = GetFallbackFontMgr(pFont);
    if (pFallbackFontMgr != nullptr) {
        ASSERT(sizeof(SkUnichar) == sizeof(uint32_t));
        pFallbackFont = pFallbackFontMgr->CreateFallbackFont(pFont, (uint32_t)unicodeChar, glyphId);
    }

    const SkFont* pFallbackSkFont = nullptr;
    if (pFallbackFont != nullptr) {
        Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFallbackFont);
        ASSERT(pSkiaFont != nullptr);
        if (pSkiaFont != nullptr) {
            pFallbackSkFont = pSkiaFont->GetFontHandle();
            ASSERT(pFallbackSkFont != nullptr);
        }
    }
    return pFallbackSkFont;
}


UTF32String DrawSkiaText::GetDrawStringUTF32(const void* text, size_t byteLength, SkTextEncoding textEncoding)
{
    if ((text == nullptr) || (byteLength == 0)) {
        return UTF32String();
    }
    if (textEncoding == SkTextEncoding::kUTF8) {
        return StringConvert::UTF8ToUTF32((const DUTF8Char*)text, byteLength / sizeof(DUTF8Char));
    }
    else if (textEncoding == SkTextEncoding::kUTF16) {
        ASSERT(byteLength % sizeof(DUTF16Char) == 0);
        return StringConvert::UTF16ToUTF32((const DUTF16Char*)text, byteLength / sizeof(DUTF16Char));
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        ASSERT(byteLength % sizeof(DUTF32Char) == 0);
        return UTF32String((const DUTF32Char*)text, byteLength / sizeof(DUTF32Char));
    }
    ASSERT(0);
    return UTF32String();
}

DUTF32Char DrawSkiaText::GetMeasureDefaultChar()
{
    return L'A';
}

SkScalar DrawSkiaText::MeasureTextChar(const SkFont& font, DUTF32Char ch,
                                       SkRect* bounds, const SkPaint* paint,
                                       const IFont* pFont,
                                       bool bUseDefaultCharWhenFailed)
{
    if (pFont != nullptr) {
        FallbackFontCreator fallbackFontCreator = [pFont](SkUnichar unicodeChar, SkGlyphID* glyphId) {
            return DrawSkiaText::CreateFallbackFont(pFont, unicodeChar, glyphId);
            };
        return MeasureTextChar(font, ch, bounds, paint, fallbackFontCreator, bUseDefaultCharWhenFailed);
    }
    else {
        SkGlyphID glyphID = font.unicharToGlyph((SkUnichar)ch);
        if (glyphID != 0) {
            //只有当前不为0时可以调用，否则skia内部会导致崩溃
            return font.measureText(&ch, sizeof(ch), SkTextEncoding::kGlyphID, bounds, paint);
        }        
    }
    if (bUseDefaultCharWhenFailed) {
        DUTF32Char defaultChar = GetMeasureDefaultChar();
        return font.measureText(&defaultChar, sizeof(defaultChar), SkTextEncoding::kUTF32, bounds, paint);
    }
    if (bounds != nullptr) {
        bounds->setEmpty();
    }
    return 0.0f;
}

SkScalar DrawSkiaText::MeasureTextChar(const SkFont& font, DUTF32Char ch,
                                       SkRect* bounds, const SkPaint* paint,
                                       FallbackFontCreator fallbackFontCreator,
                                       bool bUseDefaultCharWhenFailed)
{
    SkGlyphID glyphId = font.unicharToGlyph((SkUnichar)ch);
    if (glyphId != 0) {
        //只有当前不为0时可以调用，否则skia内部会导致崩溃
        return font.measureText(&glyphId, sizeof(SkGlyphID), SkTextEncoding::kGlyphID, bounds, paint);
    }
    else if (fallbackFontCreator != nullptr) {
        //当前设置的字体不支持这个字，需要使用回退字体
        glyphId = 0;
        const SkFont* pFallbackSkFont = fallbackFontCreator(ch, &glyphId);
        if (pFallbackSkFont != nullptr) {
            return pFallbackSkFont->measureText(&glyphId, sizeof(SkGlyphID), SkTextEncoding::kGlyphID, bounds, paint);
        }
    }
    if (bUseDefaultCharWhenFailed) {
        DUTF32Char defaultChar = GetMeasureDefaultChar();
        return font.measureText(&defaultChar, sizeof(defaultChar), SkTextEncoding::kUTF32, bounds, paint);
    }
    if (bounds != nullptr) {
        bounds->setEmpty();
    }
    return 0.0f;
}

SkScalar DrawSkiaText::MeasureText(const SkFont& font, const void* text, size_t byteLength, SkTextEncoding textEncoding,
                                   SkRect* bounds, const SkPaint* paint,
                                   const IFont* pFont,
                                   MeasureTextTempData& tempData)
{
    if (pFont != nullptr) {
        FallbackFontCreator fallbackFontCreator = [pFont](SkUnichar unicodeChar, SkGlyphID* glyphId) {
            return DrawSkiaText::CreateFallbackFont(pFont, unicodeChar, glyphId);
            };
        return MeasureText(font, text, byteLength, textEncoding, bounds, paint, fallbackFontCreator, tempData);
    }
    else {
        return font.measureText(text, byteLength, textEncoding, bounds, paint);
    }
}

/** 计算给定编码格式下一个字符的字节数
 * @param [in] textEncoding 文本编码格式
 * @return 字符字节数（UTF8:1, UTF16:2, UTF32:4）
 */
static inline size_t GetCharBytes(SkTextEncoding textEncoding)
{
    switch (textEncoding) {
    case SkTextEncoding::kUTF8:
        return 1;
    case SkTextEncoding::kUTF16:
        return 2;
    case SkTextEncoding::kUTF32:
        return 4;
    default:
        ASSERT(false);
        break;
    }
    return 1;
}

SkScalar DrawSkiaText::MeasureText(const SkFont& font, const void* text, size_t byteLength, SkTextEncoding textEncoding,
                                   SkRect* bounds, const SkPaint* paint,
                                   FallbackFontCreator fallbackFontCreator,
                                   MeasureTextTempData& tempData)
{
#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("DrawSkiaText::MeasureText");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    if ((text == nullptr) || (byteLength == 0)) {
        return 0.0f;
    }    
    if (fallbackFontCreator != nullptr) {
        //UTF32字符列表
        FastVector<SkUnichar>& unicharList = tempData.unicharList;
        unicharList.clear();
        unicharList.reserve(byteLength / GetCharBytes(textEncoding));

        EnumTextCallback enumTextCallback = [&](SkUnichar unicodeChar, size_t /*charByteLength*/) {
            unicharList.push_back(unicodeChar);
            return true;
            };
        SkiaTextData textData(text, byteLength, textEncoding);
        textData.EnumChars(enumTextCallback);
        if (unicharList.empty()) {
            return 0.0f;
        }

        //GlyphID列表
        FastVector<SkGlyphID>& glyphIDList = tempData.glyphIDList;
        glyphIDList.clear();
        glyphIDList.resize(unicharList.size());
        font.unicharsToGlyphs(SkSpan(unicharList.data(), unicharList.size()), SkSpan(glyphIDList.data(), glyphIDList.size()));

        bool bNeedFallback = false;
        for (SkGlyphID glyphID : glyphIDList) {
            if (glyphID == 0) {
                bNeedFallback = true;
                break;
            }
        }
        if (!bNeedFallback) {
            //不需要字体回退，直接计算
            return font.measureText(glyphIDList.data(), glyphIDList.size() * sizeof(SkGlyphID), SkTextEncoding::kGlyphID, bounds, paint);
        }

        //需要字体回退，拆分为两个部分（不需要字体回退的SkGlyphID列表，需要字体回退的UTF32字符列表）
        FastVector<SkGlyphID>& normalGlyphIDList = tempData.normalGlyphIDList;
        FastVector<SkUnichar>& fallbackUnicharList = tempData.fallbackUnicharList;
        normalGlyphIDList.clear();
        fallbackUnicharList.clear();
        normalGlyphIDList.reserve(unicharList.size());
        fallbackUnicharList.reserve(unicharList.size());
        const size_t nUnicharCount = unicharList.size();
        for (size_t nIndex = 0; nIndex < nUnicharCount; ++nIndex) {
            if (glyphIDList[nIndex] == 0) {
                fallbackUnicharList.push_back(unicharList[nIndex]);
            }
            else {
                normalGlyphIDList.push_back(glyphIDList[nIndex]);
            }
        }

        //先计算正常的字列表
        SkScalar fTotalWidth = 0;
        SkRect totalBounds;
        const bool bHasBounds = bounds != nullptr;
        SkRect* calcBounds = bHasBounds ? &totalBounds : nullptr;
        if (!normalGlyphIDList.empty()) {
            fTotalWidth = font.measureText(normalGlyphIDList.data(),
                                           normalGlyphIDList.size() * sizeof(SkGlyphID),
                                           SkTextEncoding::kGlyphID,
                                           calcBounds, paint);
        }

        //再计算需要字体回退的字符
        SkScalar fCalcCharWidth = 0;
        SkRect calcCharBounds;
        calcBounds = bHasBounds ? &calcCharBounds : nullptr;
        EnumTextCallback enumTextCallback2 = [&](SkUnichar unicodeChar, size_t /*charByteLength*/) {
            fCalcCharWidth = MeasureTextChar(font, unicodeChar, calcBounds, paint, fallbackFontCreator, true);            
            if (bHasBounds) {
                //算法保持与SkFont::measureText函数内部的实现保持一致
                if (totalBounds.isEmpty()) {
                    totalBounds = *calcBounds;
                }
                else {
                    calcBounds->offset(fTotalWidth, 0);
                    totalBounds.join(*calcBounds);
                }
            }
            fTotalWidth += fCalcCharWidth;
            return true;
            };

        SkiaTextData textData2(fallbackUnicharList.data(), fallbackUnicharList.size() * sizeof(SkUnichar), SkTextEncoding::kUTF32);
        textData2.EnumChars(enumTextCallback2);

        if (bounds != nullptr) {
            *bounds = totalBounds;
        }
        return fTotalWidth;
    }
    else {
        return font.measureText(text, byteLength, textEncoding, bounds, paint);
    }
}

/** 绘制一个Glyph字符，并返回其宽度
* @param [in] skCanvas Canvas 接口
* @param [in] font 字体
* @param [in] paint 绘制属性
* @param [in] x 绘制的x坐标
* @param [in] y 绘制的y坐标
* @param [in] glyphID 字符ID
* @return 返回字符宽度
*/
inline static SkScalar DrawSkGlyphChar(SkCanvas* skCanvas, const SkFont& font, const SkPaint& paint,
                                       SkScalar x, SkScalar y, SkGlyphID glyphID, DUTF32Char ch)
{
    //如果glyphID为0，绘制未知字符，实际会显示一个方框
    SkRect bounds;
    SkScalar fWidth = 0;
    font.getWidthsBounds(SkSpan<const SkGlyphID>(&glyphID, 1),
                         SkSpan<SkScalar>(&fWidth, 1),
                         SkSpan<SkRect>(&bounds, 1),
                         &paint);
    if (bounds.isEmpty() && !iswspace((wchar_t)ch)) {
        //bounds是空的话，表示该字符无法显示，但此时fWidth却不是0，所以绘制一个方框，作为占位符
        glyphID = 0;
        skCanvas->drawSimpleText(&glyphID, sizeof(SkGlyphID), SkTextEncoding::kGlyphID, x, y, font, paint);
    }
    else {
        //skCanvas->drawSimpleText(&glyphID, sizeof(SkGlyphID), SkTextEncoding::kGlyphID, x, y, font, paint);
        SkPoint pt = SkPoint::Make(x, y);
        skCanvas->drawGlyphs(SkSpan<const SkGlyphID>(&glyphID, 1), SkSpan<const SkPoint>(&pt, 1), SkPoint(), font, paint);
    }
    return fWidth;
}

SkScalar DrawSkiaText::DrawSimpleText(SkCanvas* skCanvas, DUTF32Char ch,
                                      SkScalar x, SkScalar y,
                                      const SkFont& font, const SkPaint& paint,
                                      const IFont* pFont)
{
    SkScalar fWidth = 0;
    if (pFont != nullptr) {
        FallbackFontCreator fallbackFontCreator = [pFont](SkUnichar unicodeChar, SkGlyphID* glyphId) {
            return DrawSkiaText::CreateFallbackFont(pFont, unicodeChar, glyphId);
            };
        fWidth = DrawSimpleText(skCanvas, ch, x, y, font, paint, fallbackFontCreator);
    }
    else {
        SkGlyphID glyphID = font.unicharToGlyph((SkUnichar)ch);
        //如果glyphID为0，绘制未知字符，实际会显示一个方框        
        fWidth = DrawSkGlyphChar(skCanvas, font, paint, x, y, glyphID, ch);
    }
    return fWidth;
}

SkScalar DrawSkiaText::DrawSimpleText(SkCanvas* skCanvas, DUTF32Char ch, SkScalar x, SkScalar y,
                                      const SkFont& font, const SkPaint& paint,
                                      FallbackFontCreator fallbackFontCreator)
{
    SkScalar fWidth = 0;
    if (skCanvas == nullptr) {
        return fWidth;
    }
    SkGlyphID glyphID = font.unicharToGlyph((SkUnichar)ch);
    if (glyphID != 0) {
        fWidth = DrawSkGlyphChar(skCanvas, font, paint, x, y, glyphID, ch);
    }
    else if (fallbackFontCreator != nullptr) {
        //该字体无法绘制，需要请求回退字体绘制
        glyphID = 0;
        const SkFont* pFallbackSkFont = fallbackFontCreator((uint32_t)ch, &glyphID);
        if ((pFallbackSkFont != nullptr) && (glyphID != 0)) {
            fWidth = DrawSkGlyphChar(skCanvas, *pFallbackSkFont, paint, x, y, glyphID, ch);
        }
    }
    if (glyphID == 0) {
        //使用默认字体获取宽度，避免不绘制，从而使得界面显示字符位置和实际位置不符
        fWidth = DrawSkGlyphChar(skCanvas, font, paint, x, y, glyphID, ch);
    }
    return fWidth;
}

void DrawSkiaText::DrawSimpleText(SkCanvas* skCanvas, const void* text, size_t byteLength, SkTextEncoding textEncoding,
                                  SkScalar x, SkScalar y,
                                  const SkFont& font, const SkPaint& paint,
                                  const IFont* pFont)
{
    if (pFont != nullptr) {
        FallbackFontCreator fallbackFontCreator = [pFont](SkUnichar unicodeChar, SkGlyphID* glyphId) {
            return DrawSkiaText::CreateFallbackFont(pFont, unicodeChar, glyphId);
            };
        return DrawSimpleText(skCanvas, text, byteLength, textEncoding, x, y, font, paint, fallbackFontCreator);
    }
    else {
        skCanvas->drawSimpleText(text, byteLength, textEncoding, x, y, font, paint);
    }
}

void DrawSkiaText::DrawSimpleText(SkCanvas* skCanvas, const void* text, size_t byteLength, SkTextEncoding textEncoding,
                                  SkScalar x, SkScalar y,
                                  const SkFont& font, const SkPaint& paint,
                                  FallbackFontCreator fallbackFontCreator)
{
#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("DrawSkiaText::DrawSimpleText");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED
    if ((skCanvas == nullptr) || (text == nullptr) || (byteLength == 0)) {
        return;
    }
    if (fallbackFontCreator != nullptr) {
        UTF32String utf32 = GetDrawStringUTF32(text, byteLength, textEncoding);
        SkScalar fTotalWidth = 0;
        const size_t nCount = utf32.size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            fTotalWidth += DrawSimpleText(skCanvas, utf32[nIndex], x + fTotalWidth, y, font, paint, fallbackFontCreator);
        }
    }
    else {
        skCanvas->drawSimpleText(text, byteLength, textEncoding, x, y, font, paint);
    }
}

size_t DrawSkiaText::BreakText(const void* text, size_t byteLength, SkTextEncoding textEncoding,
                               const SkFont& font, FallbackFontCreator fallbackFontCreator,
                               const SkPaint& paint, SkScalar maxWidth, SkScalar* measuredWidth,
                               MeasureTextTempData& tempData)
{
    return BreakText(text, byteLength, textEncoding,
                     font, fallbackFontCreator, paint, maxWidth,
                     measuredWidth, tempData, nullptr);
}

size_t DrawSkiaText::BreakText(const void* text, size_t byteLength, SkTextEncoding textEncoding,
                               const SkFont& font, FallbackFontCreator fallbackFontCreator,
                               const SkPaint& paint, SkScalar maxWidth, SkScalar* measuredWidth,
                               MeasureTextTempData& tempData,
                               BreakTextTempData* breakTextData)
{
    //PerformanceUtil statPerformance(_T("DrawSkiaText::BreakText"));
    if (measuredWidth != nullptr) {
        *measuredWidth = 0;
    }
    if (breakTextData != nullptr) {
        breakTextData->glyphIDs.clear();
        breakTextData->glyphChars.clear();
        breakTextData->glyphWidths.clear();
    }
    if ((text == nullptr) || (maxWidth <= 0) || (byteLength == 0) || (textEncoding == SkTextEncoding::kGlyphID)) {
        return 0;
    }

    //文本 -> SkUnichar -> SkGlyphID -> 估算
    tempData.unicharList.clear();
    tempData.glyphChars.clear();
    const size_t charBytes = GetCharBytes(textEncoding);
    tempData.unicharList.reserve(byteLength / charBytes);
    tempData.glyphChars.reserve(byteLength / charBytes);

    EnumTextCallback enumTextCallback = [&tempData](SkUnichar unicodeChar, size_t charByteLength) {
        tempData.unicharList.push_back(unicodeChar);
        tempData.glyphChars.push_back((uint8_t)charByteLength);
        return true;
        };
    SkiaTextData textData(text, byteLength, textEncoding);
    textData.EnumChars(enumTextCallback);

    // 评估功能正确性时开启，其他情况关闭，该函数比较耗时
    // ASSERT(font.countText(text, byteLength, textEncoding) == tempData.unicharList.size()); 

    ASSERT(!tempData.unicharList.empty() && (tempData.unicharList.size() == tempData.glyphChars.size()));
    if (tempData.unicharList.empty() || (tempData.unicharList.size() != tempData.glyphChars.size())) {
        return 0;
    }

    tempData.glyphIDList.resize(tempData.unicharList.size());
    font.unicharsToGlyphs(SkSpan<const SkUnichar>(tempData.unicharList.data(), tempData.unicharList.size()),
                          SkSpan<SkGlyphID>(tempData.glyphIDList.data(), tempData.glyphIDList.size()));

    tempData.glyphWidths.resize(tempData.glyphIDList.size());
    font.getWidthsBounds(SkSpan<const SkGlyphID>(tempData.glyphIDList.data(), tempData.glyphIDList.size()),
                         SkSpan<SkScalar>(tempData.glyphWidths.data(), tempData.glyphWidths.size()),
                         {}, &paint);

    const bool bHasFallbackFontCreator = fallbackFontCreator != nullptr;
    SkGlyphID glyphId = 0;      //Glyph字符ID
    const SkFont* pFallbackSkFont = nullptr; //回退字体的接口

    size_t breakTextBytes = 0;  //可绘制文本数据的字节数
    SkScalar totalWidth = 0;    //绘制字符的总宽度
    SkScalar glyphWidth = 0;    //单个字符的宽度
    const size_t nCount = tempData.glyphWidths.size();
    size_t glyphCount = nCount; //可绘制的glyph字符数
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if ((tempData.glyphIDList[nIndex] == 0) && bHasFallbackFontCreator) {
            glyphId = 0;
            pFallbackSkFont = fallbackFontCreator(tempData.unicharList[nIndex], &glyphId);
            if (pFallbackSkFont != nullptr) {
                //字体回退成功, 更新字符宽度
                pFallbackSkFont->getWidthsBounds(SkSpan<const SkGlyphID>(&glyphId, 1),
                                                 SkSpan<SkScalar>(&tempData.glyphWidths[nIndex], 1),
                                                 {}, &paint);
           }
        }
        glyphWidth = tempData.glyphWidths[nIndex];
        if ((totalWidth > 0.01) && ((totalWidth + glyphWidth) > maxWidth)) {
            //已经达到宽度限制, 不需要再计算(但至少需要绘制一个字符，避免宽度过窄时，无法绘制)
            glyphCount = nIndex;
            break;
        }
        totalWidth += glyphWidth;
        breakTextBytes += tempData.glyphChars[nIndex];
    }
    if (breakTextData != nullptr) {
        tempData.glyphIDList.resize(glyphCount);
        tempData.glyphChars.resize(glyphCount);
        tempData.glyphWidths.resize(glyphCount);
        breakTextData->glyphIDs.swap(tempData.glyphIDList);
        breakTextData->glyphChars.swap(tempData.glyphChars);
        breakTextData->glyphWidths.swap(tempData.glyphWidths);
    }
    if (measuredWidth != nullptr) {
        *measuredWidth = totalWidth;
    }
    return breakTextBytes;
}

/** 判断是否为空格、不可见字符
* @param [in] c 是 Unicode 字符
* 返回true表示可以在当前字符进行换行
* 返回false表示不可以在当前字符换行
*/
static inline bool SkUTF_IsWhiteSpace(int c)
{
    //ASCII值（c）小于32的时候（含控制字符等不可见字符、空格），返回true；否则返回false
    return !((c - 1) >> 5);
}

/** 判断在当前字符处是否可以分行
* @param [in] c 是 Unicode 字符
*/
static inline bool SkUTF_IsLineBreaker(int c)
{
    //在数字和字母上不分行（返回false），尽量不换行，确保数字和英文单词的完整性
    //非字母数字（返回true）均可以分行
    if ((c >= -1) && (c <= 255)) {
        //ASCII值（c）
        if (::isalnum(c)) {
            return false;
        }
    }
    return true;
}

/** 获取下一个Unicode（UTF32）字符
* @param [in,out] ptr 输入当前字符串的起始地址，返回下一个字符的起始地址（ptr指针的值会增加）
* @param [in] end 字符串结束地址
* @param [in] textEncoding 文本编码
* @return 返回ptr地址当前的Unicode（UTF32）字符, 如果出错返回-1
*/
static inline SkUnichar SkUTF_NextUnichar(const void** ptr, const void* end, SkTextEncoding textEncoding)
{
    if (textEncoding == SkTextEncoding::kUTF16) {
        return SkUTF::NextUTF16((const uint16_t**)ptr, (const uint16_t*)end);
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        return SkUTF::NextUTF32((const int32_t**)ptr, (const int32_t*)end);
    }
    else if (textEncoding == SkTextEncoding::kUTF8){
        return SkUTF::NextUTF8((const char**)ptr, (const char*)end);
    }
    else {
        ASSERT(0);
        *ptr = end;
        return -1;
    }
}

size_t DrawSkiaText::Linebreak(const char* text, const char* stop, SkTextEncoding textEncoding,
                               const SkFont& font, FallbackFontCreator fallbackFontCreator, const SkPaint& paint,
                               SkScalar margin, TextBoxLineMode lineMode, MeasureTextTempData& tempData,
                               size_t* trailing)
{
    size_t lengthBreak = stop - text;//单行模式
    if (lineMode != TextBoxLineMode::kOneLine_Mode) {
        //多行模式
        lengthBreak = DrawSkiaText::BreakText(text, stop - text, textEncoding, font, fallbackFontCreator,
                                              paint, margin, nullptr, tempData);
    }

    //Check for white space or line breakers before the lengthBreak
    const char* start = text;
    const char* word_start = text;

    //标记是否可以分行
    bool prevIsLineBreaker = true;

    //如果设置trailing的值，这部分字符串在绘制的时候，会被忽略，不绘制
    if (trailing) {
        *trailing = 0;
    }

    SkUnichar uni = 0;
    while (text < stop) {
        const char* prevText = text;
        uni = SkUTF_NextUnichar((const void**)&text, stop, textEncoding);
        if (uni == -1) {
            text = stop;//有错误
            break;
        }

        //当前字符是否为空格（或非可见字符）
        bool currIsWhiteSpace = SkUTF_IsWhiteSpace(uni);

        //当前字符是否可以分行，分行条件：当前字符是空格（或非可见字符），或者不是字母/数字
        //Word分行逻辑：按Word分行，保证一个英文单词或者一个完整的数字不被分行显示
        bool currIsLineBreaker = SkUTF_IsLineBreaker(uni);
        if (lineMode == TextBoxLineMode::kCharBreak_Mode) {
            //按字符分行，每个字符都可以分行
            currIsLineBreaker = true;
        }
        if (prevIsLineBreaker) {
            //如果前面字符可以分行, 就执行前面一个字符
            word_start = prevText;
        }
        prevIsLineBreaker = currIsLineBreaker;

        if (text > start + lengthBreak) {
            if (currIsWhiteSpace) {
                // eat the rest of the whitespace
                while (text < stop && SkUTF_IsWhiteSpace(SkUTF_NextUnichar((const void**)&text, stop, textEncoding))) {
                    ; //什么也不用做，SkUTF_NextUnichar函数会自动增加text的指针, 直到等于stop值
                }
                if (trailing) {
                    *trailing = text - prevText;
                }
            }
            else {
                // backup until a whitespace (or 1 char)
                if (word_start == start) {
                    if (prevText > start) {
                        text = prevText;
                    }
                }
                else {
                    text = word_start;
                }
            }
            break;
        }

        if ('\n' == uni) {
            size_t ret = text - start;
            size_t lineBreakSize = 1;
            if (text < stop) {
                uni = SkUTF_NextUnichar((const void**)&text, stop, textEncoding);
                if ('\r' == uni) {
                    ret = text - start;
                    ++lineBreakSize;
                }
            }
            if (textEncoding == SkTextEncoding::kUTF16) {
                //每个字符串占2个字节
                lineBreakSize *= 2;
            }
            else if (textEncoding == SkTextEncoding::kUTF32) {
                //每个字符串占4个字节
                lineBreakSize *= 4;
            }
            if (trailing) {
                *trailing = lineBreakSize;
            }
            return ret;
        }

        if ('\r' == uni) {
            size_t ret = text - start;
            size_t lineBreakSize = 1;
            if (text < stop) {
                uni = SkUTF_NextUnichar((const void**)&text, stop, textEncoding);
                if ('\n' == uni) {
                    ret = text - start;
                    ++lineBreakSize;
                }
            }
            if (textEncoding == SkTextEncoding::kUTF16) {
                //每个字符串占2个字节
                lineBreakSize *= 2;
            }
            else if (textEncoding == SkTextEncoding::kUTF32) {
                //每个字符串占4个字节
                lineBreakSize *= 4;
            }
            if (trailing) {
                *trailing = lineBreakSize;
            }
            return ret;
        }
    }

    return text - start;
}

int32_t DrawSkiaText::CountLines(const char* text, size_t len, SkTextEncoding textEncoding,
                                 const SkFont& font, FallbackFontCreator fallbackFontCreator, const SkPaint& paint,
                                 SkScalar width, TextBoxLineMode lineMode,
                                 std::vector<size_t>* lineLenList)
{
    MeasureTextTempData tempData;
    const char* stop = text + len;
    int32_t count = 0;
    if (width > 0) {
        do {
            count += 1;
            size_t lineLen = DrawSkiaText::Linebreak(text, stop, textEncoding, font, fallbackFontCreator, paint, width, lineMode, tempData);
            if (lineLenList != nullptr) {
                lineLenList->push_back(lineLen);
            }
            text += lineLen;
        } while (text < stop);
    }
    return count;
}

SkScalar DrawSkiaText::MeasureText(const SkFont& font, const SkiaTextData& textData,
                                   SkRect* bounds, const SkPaint* paint,
                                   const IFont* pFont,
                                   MeasureTextTempData& tempData)
{
    return MeasureText(font, textData.GetText(), textData.GetByteLength(), textData.GetTextEncoding(),
                       bounds, paint, pFont, tempData);
}

SkScalar DrawSkiaText::MeasureText(const SkFont& font, const SkiaTextData& textData,
                                   SkRect* bounds, const SkPaint* paint,
                                   FallbackFontCreator fallbackFontCreator,
                                   MeasureTextTempData& tempData)
{
    return MeasureText(font, textData.GetText(), textData.GetByteLength(), textData.GetTextEncoding(),
                       bounds, paint, fallbackFontCreator, tempData);
}

void DrawSkiaText::DrawSimpleText(SkCanvas* skCanvas, const SkiaTextData& textData,
                                   SkScalar x, SkScalar y,
                                   const SkFont& font, const SkPaint& paint,
                                   const IFont* pFont)
{
    DrawSimpleText(skCanvas, textData.GetText(), textData.GetByteLength(), textData.GetTextEncoding(),
                   x, y, font, paint, pFont);
}

void DrawSkiaText::DrawSimpleText(SkCanvas* skCanvas, const SkiaTextData& textData,
                                   SkScalar x, SkScalar y,
                                   const SkFont& font, const SkPaint& paint,
                                   FallbackFontCreator fallbackFontCreator)
{
    DrawSimpleText(skCanvas, textData.GetText(), textData.GetByteLength(), textData.GetTextEncoding(),
                   x, y, font, paint, fallbackFontCreator);
}

size_t DrawSkiaText::BreakText(const SkiaTextData& textData,
                               const SkFont& font, FallbackFontCreator fallbackFontCreator,
                               const SkPaint& paint, SkScalar maxWidth,
                               SkScalar* measuredWidth, MeasureTextTempData& tempData)
{
    return BreakText(textData.GetText(), textData.GetByteLength(), textData.GetTextEncoding(),
                     font, fallbackFontCreator, paint, maxWidth, measuredWidth, tempData);
}

int32_t DrawSkiaText::CountLines(const SkiaTextData& textData,
                                  const SkFont& font, FallbackFontCreator fallbackFontCreator,
                                  const SkPaint& paint, SkScalar width, TextBoxLineMode lineMode,
                                  std::vector<size_t>* lineLenList)
{
    const char* text = static_cast<const char*>(textData.GetText());
    return CountLines(text, textData.GetByteLength(), textData.GetTextEncoding(),
                      font, fallbackFontCreator, paint, width, lineMode, lineLenList);
}

size_t DrawSkiaText::Linebreak(const SkiaTextData& textData, const char* stop,
                                const SkFont& font, FallbackFontCreator fallbackFontCreator, const SkPaint& paint,
                                SkScalar margin, TextBoxLineMode lineMode, MeasureTextTempData& tempData,
                                size_t* trailing)
{
    const char* text = static_cast<const char*>(textData.GetText());
    return Linebreak(text, stop, textData.GetTextEncoding(),
                     font, fallbackFontCreator, paint, margin, lineMode, tempData, trailing);
}

} // namespace ui
