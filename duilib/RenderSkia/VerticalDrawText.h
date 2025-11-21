#ifndef UI_RENDER_SKIA_VERTICAL_DRAW_TEXT_H_
#define UI_RENDER_SKIA_VERTICAL_DRAW_TEXT_H_

#include "duilib/Render/IRender.h"

//Skia相关类的前置声明
class SkCanvas;
class SkPaint;
struct SkRect;
class SkFont;
struct SkPoint;

namespace ui 
{
/** 纵向绘制文本的字符属性
*/
struct TVerticalChar;

/** 纵向文本绘制的实现封装（从上到下，从右到左）
*/
class UILIB_API VerticalDrawText
{
public:
    VerticalDrawText(SkCanvas* pSkCanvas, SkPaint* pSkPaint, SkPoint* pSkPointOrg);
    VerticalDrawText(const VerticalDrawText& r) = delete;
    VerticalDrawText& operator = (const VerticalDrawText& r) = delete;
    ~VerticalDrawText() = default;

public:
    /** 纵向绘制文本的评估函数：文本绘制方向为从上到下，从右到左
    * @param [in] strText 需要评估的文本内容
    * @param [in] measureParam 评估所需的参数
    */
    UiRect MeasureString(const DString& strText, const MeasureStringParam& measureParam);

    /** 纵向绘制文本：文本绘制方向为从上到下，从右到左
    * @param [in] strText 需要绘制的文本内容
    * @param [in] measureParam 绘制所需的参数
    */
    void DrawString(const DString& strText, const DrawStringParam& drawParam);

private:
    /** 获取UTF16字符串，并做预处理（纵向绘制文本）
    */
    UTF16String GetDrawStringUTF16(const DString& strText, bool bSingleLineMode) const;

    /** 计算每个字符的绘制所占的矩形范围
    * @param [in] textUTF16 字符串
    * @param [in] pSkFont 字体
    * @param [in] skPaint 绘制属性
    * @param [in] bUseFontHeight 纵向绘制时，使用字体的默认高度，而不是每个字体的高度（显示时所有字体等高）
    * @param [in] fFontHeight 字体高度
    * @param [in] bRotate90ForAscii 纵向绘制时，对于字母数字等，旋转90度显示
    * @param [out] charRects 返回每个字符绘制所占的矩形范围
    */
    bool CalculateTextCharBounds(const UTF16String& textUTF16, const SkFont* pSkFont, const SkPaint* skPaint,
                                 bool bUseFontHeight, float fFontHeight, bool bRotate90ForAscii,
                                 std::vector<TVerticalChar>& charRects) const;

    /** 计算纵向文本（从上到下、从右向左）的绘制区域总矩形
     * @param [in] charRects 每个字符的绘制矩形（宽或高为0表示换行）
     * @param [in] height 目标绘制区域高度
     * @param [in] bSingleLineMode true表示单行文本模式，true表示多行文本模式
     * @param [in] fSpacingMul 行间距倍数: 字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小），用于按比例调整行间距
     * @param [in] fSpacingAdd 行间距附加量: 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移（像素）
     * @param [in] fWordVerticalSpacing 每个字在纵向的间隔（像素）
     * @param [in] fDefaultCharWidth 默认的列宽
     * @param [in] fDefaultCharHeight 默认的行高
     * @param [out] pColumnRows 返回每列每行字符在charRects容器中对应的下标值
     * @param [out] pColumnWidths 返回每列的列宽
     * @param [out] pColumnHeights 返回每列的列高
     * @return 包含所有字符的总绘制矩形（SkRect）
     */
    SkRect CalculateVerticalTextBounds(const std::vector<TVerticalChar>& charRects, int32_t height, bool bSingleLineMode,
                                       float fSpacingMul, float fSpacingAdd, float fWordVerticalSpacing,
                                       float fDefaultCharWidth, float fDefaultCharHeight,
                                       std::vector<std::vector<int32_t>>* pColumnRows,
                                       std::vector<float>* pColumnWidths,
                                       std::vector<float>* pColumnHeights) const;

    /** 计算默认字符的宽度(用于空列的宽度计算)
    */
    float CalculateDefaultCharWidth(const SkFont* pSkFont, const SkPaint* skPaint) const;

    /** 判断竖排文本中字符是否需要旋转90度显示
     * @param ch 宽字符（wchar_t）
     * @return true：需要旋转90度；false：保持正立
    */
    bool NeedRotateForVertical(DUTF16Char ch) const;

private:
    /** 绘制的画布
    */
    SkCanvas* m_pSkCanvas;

    /** 绘制属性
    */
    SkPaint* m_pSkPaint;

    /** 视图的原点坐标
    */
    SkPoint* m_pSkPointOrg;
};

} // namespace ui

#endif // UI_RENDER_SKIA_VERTICAL_DRAW_TEXT_H_
