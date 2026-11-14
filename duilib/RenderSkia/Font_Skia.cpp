#include "Font_Skia.h"
#include "duilib/RenderSkia/FontMgr_Skia.h"
#include "duilib/RenderSkia/DrawSkiaText.h"
#include "duilib/Utils/StringUtil.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPaint.h"
#include "SkiaHeaderEnd.h"

namespace ui 
{

/** 计算字体签名：混合字体名、字号与字体样式
*   注意：该签名在InitFont时只计算一次并缓存，避免在热路径中重复计算
*/
static uint64_t CalcFontKey(const UiFont& fontInfo)
{
    const uint64_t fnvPrime = 1099511628211ULL;
    uint64_t fontKey = 14695981039346656037ULL;
    const uint8_t* pData = (const uint8_t*)fontInfo.m_fontName.c_str();
    const size_t byteCount = StringUtil::StringLen(fontInfo.m_fontName.c_str()) * sizeof(DString::value_type);
    for (size_t i = 0; i < byteCount; ++i) {
        fontKey ^= pData[i];
        fontKey *= fnvPrime;
    }
    uint8_t flags = 0;
    if (fontInfo.m_bBold) {
        flags |= 0x01;
    }
    if (fontInfo.m_bItalic) {
        flags |= 0x02;
    }
    if (fontInfo.m_bUnderline) {
        flags |= 0x04;
    }
    if (fontInfo.m_bStrikeOut) {
        flags |= 0x08;
    }
    fontKey ^= (uint64_t)(uint32_t)fontInfo.m_fontSize;
    fontKey *= fnvPrime;
    fontKey ^= flags;
    fontKey *= fnvPrime;
    //0表示字体不支持签名，不能作为缓存键
    return (fontKey != 0) ? fontKey : 1;
}

Font_Skia::Font_Skia(std::shared_ptr<IFontMgr>& spFontMgr):
    m_skFont(nullptr),
    m_spFontMgr(spFontMgr),
    m_fontKey(0),
    m_pSkiaFontMgr(nullptr)
{
    ASSERT(m_spFontMgr != nullptr);
    //缓存字体管理器指针，避免在热路径中重复dynamic_cast
    m_pSkiaFontMgr = dynamic_cast<FontMgr_Skia*>(m_spFontMgr.get());
    ASSERT(m_pSkiaFontMgr != nullptr);
}

Font_Skia::~Font_Skia()
{
    ClearSkFont();
    m_pSkiaFontMgr = nullptr;
    m_spFontMgr.reset();
}

void Font_Skia::ClearSkFont()
{
    if (m_skFont != nullptr) {
        ASSERT(m_pSkiaFontMgr != nullptr);
        if (m_pSkiaFontMgr != nullptr) {
            m_pSkiaFontMgr->DeleteSkFont(m_skFont);
        }
        m_skFont = nullptr;
    }
}

bool Font_Skia::InitFont(const UiFont& fontInfo)
{
    ASSERT(!fontInfo.m_fontName.empty());
    if (fontInfo.m_fontName.empty()) {
        return false;
    }
    // 仅更新字体信息并清理旧的 SkFont 实例
    // 实际的 SkFont 创建会延迟到首次调用 GetFontHandle() 时（懒加载模式）
    m_uiFont = fontInfo;
    m_fontKey = CalcFontKey(m_uiFont);
    ClearSkFont();
    return true;
}

const SkFont* Font_Skia::GetFontHandle() const
{
    if (m_skFont != nullptr) {
        return m_skFont;
    }
    ASSERT(m_pSkiaFontMgr != nullptr);
    if (m_pSkiaFontMgr != nullptr) {
        m_skFont = m_pSkiaFontMgr->CreateSkFont(m_uiFont);
    }
    return m_skFont;
}

bool Font_Skia::IsUnicodeCharSupported(uint32_t unicodeChar) const
{
    if (unicodeChar != 0) {
        const SkFont* pSkFont = GetFontHandle();
        if (pSkFont != nullptr) {
            ASSERT(sizeof(SkUnichar) == sizeof(uint32_t));
            return pSkFont->unicharToGlyph((SkUnichar)unicodeChar) != 0;
        }
    }
    return false;
}

IFontMgr* Font_Skia::GetFontMgr() const
{
    return m_spFontMgr.get();
}

uint64_t Font_Skia::GetFontKey() const
{
    return m_fontKey;
}

bool Font_Skia::GetFontMetrics(FontMetrics& metrics) const
{
    const SkFont* pSkFont = GetFontHandle();
    if (pSkFont == nullptr) {
        return false;
    }
    SkFontMetrics skMetrics;
    const SkScalar fLineHeight = pSkFont->getMetrics(&skMetrics);
    metrics.fAscent = skMetrics.fAscent;
    metrics.fDescent = skMetrics.fDescent;
    metrics.fLeading = skMetrics.fLeading;
    metrics.fLineHeight = fLineHeight;
    return true;
}

float Font_Skia::MeasureTextWidth(const DString& text) const
{
    if (text.empty()) {
        return 0.0f;
    }
    const SkFont* pSkFont = GetFontHandle();
    if (pSkFont == nullptr) {
        return 0.0f;
    }
    SkPaint skPaint;
    skPaint.setAntiAlias(true);
    MeasureTextTempData tempData;
    SkRect bounds;
    //DString 的字符宽度决定Skia的文本编码（Windows/macOS 为 UTF-16，Linux 为 UTF-32）
    constexpr SkTextEncoding textEncoding = (sizeof(DString::value_type) == sizeof(uint32_t))
                                            ? SkTextEncoding::kUTF32 : SkTextEncoding::kUTF16;
    return DrawSkiaText::MeasureText(*pSkFont,
                                     text.c_str(), text.size() * sizeof(DString::value_type),
                                     textEncoding, &bounds, &skPaint, this, tempData);
}

bool Font_Skia::GetFontInfo(UiFont& fontInfo) const
{
    if (m_uiFont.m_fontName.empty()) {
        return false;
    }
    fontInfo = m_uiFont;
    return true;
}

} // namespace ui

