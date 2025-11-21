#ifndef UI_RENDER_SKIA_HORIZONTAL_DRAW_TEXT_H_
#define UI_RENDER_SKIA_HORIZONTAL_DRAW_TEXT_H_

#include "duilib/Render/IRender.h"

//Skia相关类的前置声明
class SkCanvas;
class SkPaint;
struct SkRect;
class SkFont;
struct SkPoint;

namespace ui
{
/** 横向绘制文本的字符属性
*/
struct THorizontalChar;

/** 横向文本绘制的实现封装（从左到右，从上到下）
*/
class UILIB_API HorizontalDrawText
{
public:
    HorizontalDrawText(SkCanvas* pSkCanvas, SkPaint* pSkPaint, SkPoint* pSkPointOrg);
    HorizontalDrawText(const HorizontalDrawText& r) = delete;
    HorizontalDrawText& operator = (const HorizontalDrawText& r) = delete;
    ~HorizontalDrawText() = default;

public:
    /** 横向绘制文本的评估函数：文本绘制方向为从左到右，从上到下
    * @param [in] strText 需要评估的文本内容
    * @param [in] measureParam 评估所需的参数
    */
    UiRect MeasureString(const DString& strText, const MeasureStringParam& measureParam);

    /** 横向绘制文本：文本绘制方向为从左到右，从上到下
    * @param [in] strText 需要绘制的文本内容
    * @param [in] measureParam 绘制所需的参数
    */
    void DrawString(const DString& strText, const DrawStringParam& drawParam);

private:
    /** 获取UTF16字符串，并做预处理（横向绘制文本）
    */
    UTF16String GetDrawStringUTF16(const DString& strText, bool bSingleLineMode) const;

    /** 计算每个字符的绘制所占的矩形范围
    * @param [in] textUTF16 字符串
    * @param [in] pSkFont 字体
    * @param [in] skPaint 绘制属性
    * @param [in] fFontHeight 字体高度
    */
    bool CalculateTextCharBounds(const UTF16String& textUTF16, const SkFont* pSkFont, const SkPaint* skPaint,
                                 float fFontHeight, std::vector<THorizontalChar>& charRects) const;

    /** 计算横向文本（从左到右、从上到下）的绘制区域总矩形
     * @param [in] charRects 每个字符的绘制矩形（宽或高为0表示换行）
     * @param [in] width 目标绘制区域宽度
     * @param [in] bSingleLineMode true表示单行文本模式，true表示多行文本模式
     * @param [in] fSpacingMul 行间距倍数: 字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小），用于按比例调整行间距
     * @param [in] fSpacingAdd 行间距附加量: 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移（像素）
     * @param [in] fWordHorizontalSpacing 每个字在横向的间隔（像素）
     * @param [in] fDefaultCharWidth 默认的列宽
     * @param [in] fDefaultCharHeight 默认的行高
     * @param [out] pColumnRows 返回每列每行字符在charRects容器中对应的下标值
     * @param [out] pColumnWidths 返回每列的列宽
     * @param [out] pColumnHeights 返回每列的列高
     */
    SkRect CalculateHorizontalTextBounds(const std::vector<THorizontalChar>& charRects, int32_t width, bool bSingleLineMode,
                                         float fSpacingMul, float fSpacingAdd, float fWordHorizontalSpacing,
                                         float fDefaultCharWidth, float fDefaultCharHeight,
                                         std::vector<std::vector<int32_t>>* pRowColumns,
                                         std::vector<float>* pRowHeights,
                                         std::vector<float>* pRowWidths) const;

    /** 计算默认字符的宽度(用于空行的宽度计算)
    */
    float CalculateDefaultCharWidth(const SkFont* pSkFont, const SkPaint* skPaint) const;

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

#endif // UI_RENDER_SKIA_HORIZONTAL_DRAW_TEXT_H_
