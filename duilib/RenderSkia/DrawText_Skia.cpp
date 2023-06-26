#include "DrawText_Skia.h"
#include "duilib/RenderSkia/SkTextBox.h"

#include "include/core/SkPaint.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontMetrics.h"

namespace ui {

UiRect DrawText_Skia::MeasureString(const std::wstring& strText,
								    const SkFont& skFont,
								    const SkPaint& skPaint,
									bool isSingleLineMode,
	                                float spacingMul,
								    int width)
{
	if (strText.empty()) {
		return UiRect();
	}

	//计算行高
	SkFontMetrics fontMetrics;
	SkScalar fontHeight = skFont.getMetrics(&fontMetrics);

	if (isSingleLineMode || (width <= 0)) {
		//单行模式, 或者没有限制宽度
		SkScalar textWidth = skFont.measureText(strText.c_str(), 
												strText.size() * sizeof(std::wstring::value_type), 
												SkTextEncoding::kUTF16, 
												nullptr,
												&skPaint);
		int textIWidth = SkScalarTruncToInt(textWidth + 0.5f);
		if (textWidth > textIWidth) {
			textIWidth += 1;
		}
		if (textIWidth <= 0) {
			return UiRect();
		}
		UiRect rc;
		rc.left = 0;
		if (width <= 0) {
			rc.right = textIWidth;
		}
		else if (textIWidth < width) {
			rc.right = textIWidth;
		}
		else {
			//返回限制宽度
			rc.right = width;
		}
		rc.top = 0;
		rc.bottom = SkScalarTruncToInt(fontHeight + 0.5f);
		if (fontHeight > rc.bottom) {
			rc.bottom += 1;
		}
		return rc;
	}
	else {
		//多行模式，并且限制宽度width为有效值
		ASSERT(width > 0);
		int lineCount = SkTextLineBreaker::CountLines((const char*)strText.c_str(),
													  strText.size() * sizeof(std::wstring::value_type),
													  SkTextEncoding::kUTF16, 
													  skFont,
			                                          skPaint,
			                                          SkScalar(width));

		SkScalar scaledSpacing = fontHeight * spacingMul;
		SkScalar textHeight = fontHeight;
		if (lineCount > 0) {
			textHeight += scaledSpacing * (lineCount - 1);
		}
		UiRect rc;
		rc.left = 0;
		rc.right = width;
		rc.top = 0;
		rc.bottom = SkScalarTruncToInt(textHeight + 0.5f);
		if (textHeight > rc.bottom) {
			rc.bottom += 1;
		}
		return rc;
	}
}

} // namespace ui
