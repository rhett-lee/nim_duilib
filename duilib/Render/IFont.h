#ifndef UI_RENDER_IFONT_H_
#define UI_RENDER_IFONT_H_

#pragma once

#include "duilib/duilib_defs.h"

namespace ui 
{

/** 字体接口的封装
*/
class UILIB_API IFont
{
public:
    virtual ~IFont() = default;

    /**@brief 获得字体的LOGFONT
     */
    virtual const LOGFONT* LogFont() const = 0;

    /**@brief 获取字体名
     */
    virtual const wchar_t* FontName() const = 0;

    /**@brief 获取字体大小
     */
    virtual int FontSize() const = 0;

    /**@brief 是否为粗体
     */
    virtual bool IsBold() const = 0;

    /**@brief 字体下划线状态
     */
    virtual bool IsUnderline() const = 0;

    /**@brief 字体的斜体状态
     */
    virtual bool IsItalic() const = 0;

    /**@brief 字体的删除线状态
     */
    virtual bool IsStrikeOut() const = 0;

    /** 设置字体信息
    */
    virtual bool UpdateFont(const LOGFONT& logFont) = 0;
};

} // namespace ui

#endif // UI_RENDER_IFONT_H_
