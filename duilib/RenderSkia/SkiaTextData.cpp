#include "duilib/RenderSkia/SkiaTextData.h"
#include "duilib/RenderSkia/SkUTF.h"
#include "duilib/Utils/StringConvert.h"

namespace ui
{

SkiaTextData::SkiaTextData()
    : m_text(nullptr)
    , m_byteLength(0)
    , m_textEncoding(SkTextEncoding::kUTF8)
{
}

SkiaTextData::SkiaTextData(const void* text, size_t byteLength, SkTextEncoding textEncoding)
    : m_text(text)
    , m_byteLength(byteLength)
    , m_textEncoding(textEncoding)
{
}

void SkiaTextData::SetText(const void* text, size_t byteLength, SkTextEncoding textEncoding)
{
    m_text = text;
    m_byteLength = byteLength;
    m_textEncoding = textEncoding;
    ASSERT(textEncoding != SkTextEncoding::kGlyphID);
#ifdef _DEBUG
    if (byteLength > 0) {
        switch (textEncoding) {
        case SkTextEncoding::kUTF8:
            ASSERT(SkUTF::CountUTF8((const char*)text, byteLength) != -1);
            break;
        case SkTextEncoding::kUTF16:
            ASSERT(SkUTF::CountUTF16((const uint16_t*)text, byteLength) != -1);
            break;
        case SkTextEncoding::kUTF32:
            ASSERT(SkUTF::CountUTF32((const int32_t*)text, byteLength) != -1);
            break;
        default:
            ASSERT(false);
            break;
        }
    }
#endif
}

bool SkiaTextData::IsEmpty() const
{
    if ((m_text == nullptr) || (m_byteLength == 0)) {
        return true;
    }
    switch (m_textEncoding) {
    case SkTextEncoding::kUTF8:
    case SkTextEncoding::kUTF16:
    case SkTextEncoding::kUTF32:
        return false;
    default:
        ASSERT(false);
        break;
    }
    return true;
}

size_t SkiaTextData::GetCharByteLength(const void* text, SkTextEncoding textEncoding)
{
    if (textEncoding == SkTextEncoding::kUTF8) {
        int32_t type = SkUTF::SkUTF8_ByteType(*(const uint8_t*)text);
        ASSERT((type >= 1) && (type <= 4));
        // type == -1   → 非法
        // type == 0    → 续流字节（不能在开头）
        // type == 1    → 1字节（ASCII）
        // type == 2    → 2字节序列
        // type == 3    → 3字节序列
        // type == 4    → 4字节序列
        if (type == 1) return 1;
        if (type == 2) return 2;
        if (type == 3) return 3;
        if (type == 4) return 4;
        return 1;
    }
    else if (textEncoding == SkTextEncoding::kUTF16) {
        uint16_t c = *(const uint16_t*)text;
        if (SkUTF::IsLeadingSurrogateUTF16(c)) {
            return 4;
        }
        return 2;
    }
    return 4;
}

bool SkiaTextData::EnumChars(EnumTextCallback callback) const
{
    const void* text = m_text;
    size_t byteLength = m_byteLength;
    SkTextEncoding textEncoding = m_textEncoding;
    if ((text == nullptr) || (byteLength == 0)) {
        return true;
    }
    if (textEncoding == SkTextEncoding::kUTF8) {
        int32_t type = 0;
        int32_t charByteLen = 0;
        SkUnichar unichar = 0;

        const uint8_t* utf8 = static_cast<const uint8_t*>(text);
        const uint8_t* stop = utf8 + byteLength;
        while (utf8 < stop) {
            // type == -1   → 非法
            // type == 0    → 续流字节（不能在开头）
            // type == 1    → 1字节（ASCII）
            // type == 2    → 2字节序列
            // type == 3    → 3字节序列
            // type == 4    → 4字节序列
            type = SkUTF::SkUTF8_ByteType(*utf8);

            // -1 = 非法UTF8
            if (type == -1) {
                return false;
            }

            // 0 = 续流字节，不能出现在开头
            if (type == 0) {
                return false;
            }

            // type = 1/2/3/4 表示字节长度
            charByteLen = type;
            unichar = 0;

            // 检查剩余长度是否足够
            if (utf8 + charByteLen > stop) {
                return false;
            }

            // --------------------------
            // 【正确】按长度解码
            // --------------------------
            switch (charByteLen) {
            case 1: // ASCII
                unichar = *utf8;
                break;

            case 2:
                unichar = ((utf8[0] & 0x1F) << 6) | ((utf8[1] & 0x3F));
                break;

            case 3:
                unichar = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | ((utf8[2] & 0x3F));
                break;

            case 4:
                unichar = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | ((utf8[3] & 0x3F));
                break;

            default:
                return false;
            }

            // --------------------------
            // 检查是否为合法 Unicode
            // --------------------------
            /*if (!SkUTF::IsValidUnichar(unichar)) {
                return false;
            }*/

            // 回调
            if (!callback(unichar, charByteLen)) {
                return true;
            }
            utf8 += charByteLen;
        }
    }
    else if (textEncoding == SkTextEncoding::kUTF16) {
        const uint16_t* utf16 = static_cast<const uint16_t*>(text);
        const uint16_t* stop = utf16 + (byteLength >> 1);
        uint16_t c = 0;
        size_t charByteLen = 0;
        SkUnichar unichar = 0;
        uint16_t low = 0;
        constexpr SkUnichar unicharLowOffset = (0x10000 - (0xD800 << 10) - 0xDC00);
        while (utf16 < stop) {
            c = *utf16++;
            charByteLen = sizeof(uint16_t);
            unichar = c;
            if (SkUTF::IsLeadingSurrogateUTF16(c)) {
                if (utf16 >= stop) {
                    return true;
                }
                low = *utf16++;
                if (!SkUTF::IsTrailingSurrogateUTF16(low)) {
                    return false;
                }
                unichar = (c << 10) + low + unicharLowOffset;
                charByteLen = sizeof(uint32_t);
            }
            if (!callback(unichar, charByteLen)) {
                return true;
            }
        }
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        ASSERT(byteLength % sizeof(SkUnichar) == 0);
        const SkUnichar* utf32 = static_cast<const SkUnichar*>(text);
        const SkUnichar* stop = utf32 + (byteLength / sizeof(SkUnichar));
        while (utf32 < stop) {
            if (!callback(*utf32, sizeof(SkUnichar))) {
                return true;
            }
            ++utf32;
        }
    }
    else {
        ASSERT(false);
        return false;
    }
    return true;
}

UTF32String SkiaTextData::ToUTF32String() const
{
    if (IsEmpty()) {
        return UTF32String();
    }
    if (m_textEncoding == SkTextEncoding::kUTF8) {
        return StringConvert::UTF8ToUTF32((const DUTF8Char*)m_text, m_byteLength / sizeof(DUTF8Char));
    }
    else if (m_textEncoding == SkTextEncoding::kUTF16) {
        ASSERT(m_byteLength % sizeof(DUTF16Char) == 0);
        return StringConvert::UTF16ToUTF32((const DUTF16Char*)m_text, m_byteLength / sizeof(DUTF16Char));
    }
    else if (m_textEncoding == SkTextEncoding::kUTF32) {
        ASSERT(m_byteLength % sizeof(DUTF32Char) == 0);
        return UTF32String((const DUTF32Char*)m_text, m_byteLength / sizeof(DUTF32Char));
    }
    ASSERT(0);
    return UTF32String();
}

} // namespace ui
