#ifndef UI_CORE_UIFONT_H_
#define UI_CORE_UIFONT_H_

#include "duilib/Core/UiString.h"
#include "duilib/Core/SharePtr.h"
#include <cstdint>

namespace ui
{

/** 字体接口的基本数据结构
*/
class UILIB_API UiFont
{
public:
    UiFont() :
        m_fontSize(0),
        m_bBold(false),
        m_bUnderline(false),
        m_bItalic(false),
        m_bStrikeOut(false)
    {}

    /** 字体名称（如果为空，表示不含有效字体名称）
    */
    UiString m_fontName;

    /** 字体大小（单位：像素），如果为0表示不含字体大小信息
    */
    int32_t m_fontSize;

    /** 是否为粗体
    */
    bool m_bBold;

    /** 字体下划线状态
    */
    bool m_bUnderline;

    /** 字体的斜体状态
    */
    bool m_bItalic;

    /** 字体的删除线状态
    */
    bool m_bStrikeOut;
};

#pragma pack(4)
class UILIB_API UiFontEx: public UiFont, public NVRefCount<UiFontEx>
{
public:
    /** 复制数据
    */
    void CopyFrom(const UiFont& r)
    {
        m_fontName = r.m_fontName;
        m_fontSize = r.m_fontSize;
        m_bBold = r.m_bBold;
        m_bUnderline = r.m_bUnderline;
        m_bItalic = r.m_bItalic;
        m_bStrikeOut = r.m_bStrikeOut;
    }
};
#pragma pack()


/** 比较操作符
*/
static inline bool operator == (const UiFont& a, const UiFont& b)
{
    return (a.m_fontName == b.m_fontName) && (a.m_fontSize == b.m_fontSize)      &&
           (a.m_bBold == b.m_bBold)       && (a.m_bUnderline == b.m_bUnderline)  &&
           (a.m_bItalic == b.m_bItalic)   && (a.m_bStrikeOut == b.m_bStrikeOut);
}

static inline bool operator != (const UiFont& a, const UiFont& b)
{
    return (a.m_fontName != b.m_fontName) || (a.m_fontSize != b.m_fontSize)       ||
           (a.m_bBold != b.m_bBold)       || (a.m_bUnderline != b.m_bUnderline)   ||
           (a.m_bItalic != b.m_bItalic)   || (a.m_bStrikeOut != b.m_bStrikeOut);
}

/** 比较操作符
*/
static inline bool operator == (const UiFontEx& a, const UiFontEx& b)
{
    return (a.m_fontName == b.m_fontName) && (a.m_fontSize == b.m_fontSize) &&
           (a.m_bBold == b.m_bBold) && (a.m_bUnderline == b.m_bUnderline) &&
           (a.m_bItalic == b.m_bItalic) && (a.m_bStrikeOut == b.m_bStrikeOut);
}

static inline bool operator != (const UiFontEx& a, const UiFontEx& b)
{
    return (a.m_fontName != b.m_fontName) || (a.m_fontSize != b.m_fontSize) ||
           (a.m_bBold != b.m_bBold) || (a.m_bUnderline != b.m_bUnderline) ||
           (a.m_bItalic != b.m_bItalic) || (a.m_bStrikeOut != b.m_bStrikeOut);
}

} // namespace ui

#endif // UI_CORE_UIFONT_H_
