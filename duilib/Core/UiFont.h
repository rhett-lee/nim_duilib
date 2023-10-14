#ifndef UI_CORE_UIFONT_H_
#define UI_CORE_UIFONT_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <cstdint>
#include <string>

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

    /** 字体名称
    */
    std::wstring m_fontName;

    /** 字体大小（单位：像素）
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

} // namespace ui

#endif // UI_CORE_UIFONT_H_
