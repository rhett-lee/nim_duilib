#ifndef UI_RENDER_SKIA_DRAW_TEXT_H_
#define UI_RENDER_SKIA_DRAW_TEXT_H_

#pragma once

#include "duilib/duilib_defs.h"

class SkFont;
class SkPaint;

namespace ui 
{

class UILIB_API DrawText_Skia
{
public:
	/** 计算指定文本字符串的宽度和高度
	* @param [in] strText 文字内容
	* @param [in] skFont 字体, 不可以是nullptr
	* @param [in] skPaint 绘制接口，包括文字的样式等属性信息, 可以是nullptr
	* @param [in] isSingleLineMode true表示单行模式，否则为多行模式
	* @param [in] spacingMul 行间距，默认为1.0，可以通过这个参数设置多行模式下的行间距，比如1.2表示1.2倍行间距
	* @param [in] width 当前区域的限制宽度，如果是整数，表示限制宽度，否则不限制宽度	
	* @return 返回文本字符串的宽度和高度，以矩形表示结果
	*/
	static UiRect MeasureString(const std::wstring& strText, 
		                        const SkFont& skFont,
		                        const SkPaint& skPaint,
		                        bool isSingleLineMode,
		                        float spacingMul,
		                        int width);
};

} // namespace ui

#endif // UI_RENDER_SKIA_DRAW_TEXT_H_
