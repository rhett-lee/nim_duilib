/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef UI_RENDER_SKIA_SK_TEXT_BOX_H_
#define UI_RENDER_SKIA_SK_TEXT_BOX_H_

#include "duilib/RenderSkia/DrawSkiaText.h"

//该文件原始文件的出处：skia/chrome_67/include/utils/SkTextBox.h
//基于原始文件，有修改，以兼容最新版本的skia代码（2023-06-25）
//原始文件从chrome 68以后就删除了。

namespace ui
{

/** SkTextBox 文字排版类
 *
 * SkTextBox 是一个用于在矩形区域内绘制单行或多行文本的帮助类。
 * 文本框通过其绘制区域（Frame）进行定位和裁剪。
 * 边距矩形控制文本相对于绘制区域的绘制位置。
 * 换行发生在边距矩形内部。
 *
 * 间距是一个线性方程，用于计算文本行之间的距离。
 * 间距由两个标量组成：mul（倍数）和 add（附加量），
 * 行间距的计算公式为：spacing = font.getSize() * mul + add
 */
class SkTextBox
{
public:
    SkTextBox();

public:
    /** 获取换行模式
     * @return 换行模式
     */
    TextBoxLineMode GetLineMode() const { return m_lineMode; }

    /** 设置换行模式
     * @param [in] lineMode 换行模式
     */
    void SetLineMode(TextBoxLineMode lineMode);

    /** 文字纵向对齐方式
    */
    enum SpacingAlign {
        kStart_SpacingAlign,     //上对齐，相当于：top 对齐(默认)
        kCenter_SpacingAlign,    //中对齐，相当于：vcenter 对齐
        kEnd_SpacingAlign,       //下对齐，相当于：bottom 对齐

        kSpacingAlignCount
    };

    /** 获取纵向对齐方式
     * @return 纵向对齐方式
     */
    SpacingAlign GetSpacingAlign() const { return (SpacingAlign)m_spacingAlign; }

    /** 设置纵向对齐方式
     * @param [in] align 纵向对齐方式
     */
    void SetSpacingAlign(SpacingAlign align);

    /** 文字横向对齐方式
    */
    enum TextAlign {
        kLeft_Align,     //左对齐(默认)
        kCenter_Align,   //中对齐
        kRight_Align,    //右对齐

        kAlignCount
    };

    /** 获取横向对齐方式
     * @return 横向对齐方式
     */
    TextAlign GetTextAlign() const { return (TextAlign)m_textAlign; }

    /** 设置横向对齐方式
     * @param [in] align 横向对齐方式
     */
    void SetTextAlign(TextAlign align);

    /** 获取省略号模式：绘制区域不足时，自动在末尾绘制省略号
     * @return true：启用省略号模式；false：不启用
     */
    bool GetEndEllipsis() const { return m_endEllipsis; };

    /** 设置省略号模式：绘制区域不足时，自动在末尾绘制省略号
     * @param [in] bEndEllipsis true：启用省略号模式；false：不启用
     */
    void SetEndEllipsis(bool bEndEllipsis);

    /** 获取路径省略号模式：绘制区域不足时，自动在绘制省略号代替文本
     * @return true：启用路径省略号模式；false：不启用
     * @note 仅限单行文本模式，多行文本模式不支持此属性。
     *       如果字符串包含反斜杠 (\\) 或正斜杠 (/) 字符，在最后一个分隔符之后保留尽可能多的文本。
     */
    bool GetPathEllipsis() const { return m_pathEllipsis; };

    /** 设置路径省略号模式：绘制区域不足时，自动在绘制省略号代替文本
     * @param [in] bPathEllipsis true：启用路径省略号模式；false：不启用
     * @note 仅限单行文本模式，多行文本模式不支持此属性。
     *       如果字符串包含反斜杠 (\\) 或正斜杠 (/) 字符，在最后一个分隔符之后保留尽可能多的文本。
     */
    void SetPathEllipsis(bool bPathEllipsis);

    /** 获取下划线属性
     * @return true：启用下划线；false：不启用
     */
    bool GetUnderline() const { return m_underline; }

    /** 设置下划线属性
     * @param [in] bUnderline true：启用下划线；false：不启用
     */
    void SetUnderline(bool bUnderline);

    /** 获取删除线属性
     * @return true：启用删除线；false：不启用
     */
    bool GetStrikeOut() const { return m_strikeOut; }

    /** 设置删除线属性
     * @param [in] bStrikeOut true：启用删除线；false：不启用
     */
    void SetStrikeOut(bool bStrikeOut);

    /** 获取绘制区域
     * @param [out] pBox 绘制区域
     */
    void GetBox(SkRect* pBox) const;

    /** 设置绘制区域
     * @param [in] box 绘制区域
     */
    void SetBox(const SkRect& box);

    /** 设置绘制区域
     * @param [in] left 左边距
     * @param [in] top 上边距
     * @param [in] right 右边距
     * @param [in] bottom 下边距
     */
    void SetBox(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom);

    /** 获取是否对Box区域做裁剪
     * @return true：启用裁剪；false：不启用裁剪
     * @note 设置裁剪可以避免文字绘制超出边界
     */
    bool GetClipBox() const { return m_clipBox; }

    /** 设置是否对Box区域做裁剪
     * @param [in] bClipBox true：启用裁剪；false：不启用裁剪
     * @note 设置裁剪可以避免文字绘制超出边界
     */
    void SetClipBox(bool bClipBox);

    /** 获取行间距
     * @param [out] pMul 行间距倍数
     * @param [out] pAdd 行间距附加量
     * @note 实际行间距 = fontHeight * mul + add
     */
    void GetSpacing(SkScalar* pMul, SkScalar* pAdd) const;

    /** 设置行间距
     * @param [in] mul 行间距倍数
     * @param [in] add 行间距附加量
     * @note 实际行间距 = fontHeight * mul + add
     */
    void SetSpacing(SkScalar mul, SkScalar add);

    /** 绘制文字（便捷封装函数）
     * @param [in] pSkCanvas 绘制画布
     * @param [in] textData 文本数据
     * @param [in] skFont 字体
     * @param [in] skPaint 绘制属性
     * @param [in] fallbackFontCreator 回退字体管理器
     * @note 等价于以下代码：
     *       SetText(textData, font, paint);
     *       Draw(canvas);
     */
    void Draw(SkCanvas* pSkCanvas,
              const SkiaTextData& textData,
              const SkFont& skFont, const SkPaint& skPaint,
              FallbackFontCreator fallbackFontCreator);

    /** 设置待绘制的文字
     * @param [in] textData 文本数据
     * @param [in] skFont 字体
     * @param [in] skPaint 绘制属性
     * @param [in] fallbackFontCreator 回退字体管理器
     */
    void SetText(const SkiaTextData& textData,
                 const SkFont& skFont, const SkPaint& skPaint,
                 FallbackFontCreator fallbackFontCreator);

    /** 执行文字绘制
     * @param [in] pSkCanvas 绘制画布
     */
    void Draw(SkCanvas* pSkCanvas);

    /** 获取行数
     * @return 行数
     */
    int32_t CountLines() const;

    /** 获取文字总高度
     * @return 文字总高度
     */
    SkScalar GetTextHeight() const;

    /** 文字绘制访客器接口
     * @note 用于遍历每一行文字的绘制回调
     */
    class Visitor {
    public:
        virtual ~Visitor() {}
        virtual void operator()(const SkiaTextData& textData,
                                SkScalar x, SkScalar y,
                                const SkFont& skFont, const SkPaint& skPaint,
                                bool bHasMoreText, bool bIsLastLine,
                                FallbackFontCreator fallbackFontCreator,
                                MeasureTextTempData& measureTempData) = 0;
    };

public:
    /** 绘制删除线/下划线的厚度因子（相对于字体大小）
    */
    static constexpr SkScalar kLineThicknessFactor = (SK_Scalar1 / 18);

    /** 删除线高于基线的偏移量因子
     */
    static constexpr SkScalar kStrikeThroughOffset = (SK_Scalar1 * 65 / 252);

    /** 下划线低于基线的偏移量因子
     */
    static constexpr SkScalar kUnderlineOffset = (SK_Scalar1 / 9);

    /** 下划线厚度因子（相对于删除线的倍数）
     */
    static constexpr SkScalar kUnderlineThicknessFactor = (SK_Scalar1 * 3 / 2);

private:
    /** 遍历访问每一行文字
     * @param [in] visitor 访客器
     * @return 绘制后的Y坐标
     */
    SkScalar Visit(Visitor& visitor) const;

private:
    /** 文字绘制区域
     */
    SkRect m_box;

    /** 是否对Box区域做裁剪，默认为true
     */
    bool m_clipBox;

    /** 行间距倍数
     */
    SkScalar m_spacingMul;

    /** 行间距附加量
     */
    SkScalar m_spacingAdd;

    /** 换行模式
     */
    TextBoxLineMode m_lineMode;

    /** 文字纵向对齐方式
     */
    uint8_t m_spacingAlign;

    /** 文字横向对齐方式
     */
    uint8_t m_textAlign;

    /** 文本数据
     */
    SkiaTextData m_textData;

    /** 绘制属性
     */
    const SkPaint* m_pPaint;

    /** 绘制字体
     */
    const SkFont* m_pFont;

    /** 回退字体管理器
     */
    FallbackFontCreator m_fallbackFontCreator;

    /** 省略号模式：绘制区域不足时，自动在末尾绘制省略号
     */
    bool m_endEllipsis;

    /** 路径省略号模式：绘制区域不足时，自动在绘制省略号代替文本
     * @note 如果字符串包含反斜杠 (\\) 或正斜杠 (/) 字符，在最后一个分隔符之后保留尽可能多的文本。
     *       仅限单行文本模式，多行文本模式不支持此属性。
     */
    bool m_pathEllipsis;

    /** 下划线属性
     */
    bool m_underline;

    /** 删除线属性
     */
    bool m_strikeOut;
};

} //namespace ui

#endif //UI_RENDER_SKIA_SK_TEXT_BOX_H_
