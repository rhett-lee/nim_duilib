#ifndef UI_RENDER_SKIA_DRAW_SKIA_TEXT_H_
#define UI_RENDER_SKIA_DRAW_SKIA_TEXT_H_

#include "duilib/Render/IRender.h"
#include "duilib/RenderSkia/SkiaTextData.h"
#include "duilib/RenderSkia/FastVector.h"

namespace ui
{
/** 回退字体管理器
* @param [in] unicodeChar UTF32字符
* @param [out] glyphId 如果unicodeChar不为0，返回对应的SkGlyphID值
* @return 返回对应的Skia字体接口
*/
typedef std::function<const SkFont* (SkUnichar unicodeChar, SkGlyphID* glyphId)> FallbackFontCreator;

/** 换行模式
*/
enum TextBoxLineMode {
    kOneLine_Mode,   //单行模式
    kWordBreak_Mode, //Word Break模式，多行模式，按单词分行（默认）
    kCharBreak_Mode, //Char Break模式，多行模式，按字符分行（英文单词和数字都会被换行切分）
    kModeCount       //有效值总个数
};

/** 评估字符宽度的临时变量（外部管理，以减少内存分配，提高性能）
*   外部定义变量时，应放置在循环的最外层
*/
struct MeasureTextTempData
{
    FastVector<SkUnichar> unicharList;
    FastVector<SkGlyphID> glyphIDList;
    FastVector<SkGlyphID> normalGlyphIDList;
    FastVector<SkUnichar> fallbackUnicharList;

    FastVector<uint8_t> glyphChars;   //每个Glyph字符，占几个字节
    FastVector<SkScalar> glyphWidths; //每个SkGlyphID字符的所占的宽度，该宽度与字体名称和字体大小、是否斜体等属性均相关，容器大小与glyphIDs容器相同
};

/** 评估可绘制字符数量的临时变量（外部管理，以减少内存分配，提高性能）
*   外部定义变量时，应放置在循环的最外层
*/
struct BreakTextTempData
{
    FastVector<SkGlyphID> glyphIDs;    //文本对应的SkGlyphID列表（只可用于统计字符数量，不可用于绘制，因为对应的字体可能不同）
    FastVector<uint8_t> glyphChars;    //每个SkGlyphID对应的输入字节数，容器大小与glyphIDs容器相同
    FastVector<SkScalar> glyphWidths;  //每个SkGlyphID字符的所占的宽度，该宽度与字体名称和字体大小、是否斜体等属性均相关，容器大小与glyphIDs容器相同
};

/** 调用Canvas绘制Skia的文本，二次封装Skia文本相关函数以支持回退字体的功能
 *  该功能的目标是支持emoji文字和扩展汉字（Unicode扩展B/C区等其他区域的字符）的显示
 */
class DrawSkiaText
{
public:
    /** 获取回退字体管理器
    */
    static IFallbackFontMgr* GetFallbackFontMgr(const IFont* pFont);

    /** 创建指定字体的回退字体接口
    * @param [in] pFont 当前字体接口
    * @param [in] unicodeChar UTF32字符，如果为0表示不支持字符检测
    * @param [out] glyphId 如果unicodeChar不为0，返回对应的SkGlyphID值
    * @return 返回对应的回退字体接口
    */
    static const SkFont* CreateFallbackFont(const IFont* pFont, SkUnichar unicodeChar, SkGlyphID* glyphId);

public:
    /** 估算的默认字符（UTF32字符）
    * @return 返回估算默认使用的字符，仅当字符估算失败时使用
    */
    static DUTF32Char GetMeasureDefaultChar();

    /** 评估文字的宽度和绘制区域，支持字体回退
    * @param [in] font 字体
    * @param [in] ch 待估算的字符（UTF32字符）
    * @param [out] bounds 如果不为nullptr则输出该字符的绘制范围
    * @param [in] paint 绘制属性
    * @param [in] pFont font字体关联的IFont接口，用于支持字体回退，如果为nullptr则不支持字体回退
    * @param [in] bUseDefaultCharWhenFailed 当估算失败时，是否使用默认字符估算
    * @return 返回字符宽度，如果不支持绘制该字符(当bUseDefaultCharWhenFailed为false时)，返回0
    */
    static SkScalar MeasureTextChar(const SkFont& font, DUTF32Char ch,
                                    SkRect* bounds, const SkPaint* paint,
                                    const IFont* pFont,
                                    bool bUseDefaultCharWhenFailed);

    /** 评估文字的宽度和绘制区域，支持字体回退
    * @param [in] font 字体
    * @param [in] ch 待估算的字符（UTF32字符）
    * @param [out] bounds 如果不为nullptr则输出该字符的绘制范围
    * @param [in] paint 绘制属性
    * @param [in] fallbackFontCreator 字体回退函数，如果为nullptr则不支持字体回退
    * @param [in] bUseDefaultCharWhenFailed 当估算失败时，是否使用默认字符估算
    * @return 返回字符宽度，如果不支持绘制该字符(当bUseDefaultCharWhenFailed为false时)，返回0
    */
    static SkScalar MeasureTextChar(const SkFont& font, DUTF32Char ch,
                                    SkRect* bounds, const SkPaint* paint,
                                    FallbackFontCreator fallbackFontCreator,
                                    bool bUseDefaultCharWhenFailed);

    /** 评估文字的宽度和绘制区域，支持字体回退
    * @return 返回字符宽度
    */
    static SkScalar MeasureText(const SkFont& font, const void* text, size_t byteLength, SkTextEncoding textEncoding,
                                SkRect* bounds, const SkPaint* paint,
                                const IFont* pFont,
                                MeasureTextTempData& tempData);

    /** 评估文字的宽度和绘制区域，支持字体回退
    * @return 返回字符宽度
    */
    static SkScalar MeasureText(const SkFont& font, const void* text, size_t byteLength, SkTextEncoding textEncoding,
                                SkRect* bounds, const SkPaint* paint,
                                FallbackFontCreator fallbackFontCreator,
                                MeasureTextTempData& tempData);

    /** 评估文字的宽度和绘制区域，支持字体回退
    * @param [in] textData 文本数据
    * @return 返回字符宽度
    */
    static SkScalar MeasureText(const SkFont& font, const SkiaTextData& textData,
                                SkRect* bounds, const SkPaint* paint,
                                const IFont* pFont,
                                MeasureTextTempData& tempData);

    /** 评估文字的宽度和绘制区域，支持字体回退
    * @param [in] textData 文本数据
    * @return 返回字符宽度
    */
    static SkScalar MeasureText(const SkFont& font, const SkiaTextData& textData,
                                SkRect* bounds, const SkPaint* paint,
                                FallbackFontCreator fallbackFontCreator,
                                MeasureTextTempData& tempData);

public:
    /** 绘制文本，支持字体回退
    */
    static SkScalar DrawSimpleText(SkCanvas* skCanvas, DUTF32Char ch, SkScalar x, SkScalar y,
                                   const SkFont& font, const SkPaint& paint,
                                   const IFont* pFont);

    /** 绘制文本，支持字体回退
    */
    static SkScalar DrawSimpleText(SkCanvas* skCanvas, DUTF32Char ch, SkScalar x, SkScalar y,
                                   const SkFont& font, const SkPaint& paint,
                                   FallbackFontCreator fallbackFontCreator);

    /** 绘制文本，支持字体回退
    */
    static void DrawSimpleText(SkCanvas* skCanvas, const void* text, size_t byteLength, SkTextEncoding textEncoding,
                               SkScalar x, SkScalar y,
                               const SkFont& font, const SkPaint& paint,
                               const IFont* pFont);

    /** 绘制文本，支持字体回退
    */
    static void DrawSimpleText(SkCanvas* skCanvas, const void* text, size_t byteLength, SkTextEncoding textEncoding,
                               SkScalar x, SkScalar y,
                               const SkFont& font, const SkPaint& paint,
                               FallbackFontCreator fallbackFontCreator);

    /** 绘制文本，支持字体回退
    * @param [in] textData 文本数据
    */
    static void DrawSimpleText(SkCanvas* skCanvas, const SkiaTextData& textData,
                               SkScalar x, SkScalar y,
                               const SkFont& font, const SkPaint& paint,
                               const IFont* pFont);

    /** 绘制文本，支持字体回退
    * @param [in] textData 文本数据
    */
    static void DrawSimpleText(SkCanvas* skCanvas, const SkiaTextData& textData,
                               SkScalar x, SkScalar y,
                               const SkFont& font, const SkPaint& paint,
                               FallbackFontCreator fallbackFontCreator);

public:
    //原始函数：SkPaint::breakText 这个函数在最新的Skia代码里面已经删除了，这个是自己实现的版本。
    /** 返回适合最大宽度的文本字节
     * @param [in] text 文本字符串的起始地址
     * @param [in] len 文本字符串的长度（字节）
     * @param [in] textEncoding 文本的编码, SkTextEncoding::kUTF8 or kUTF16 or kUTF32
     * @param [in] font 字体
     * @param [in] fallbackFontCreator 回退字体管理器
     * @param [in] paint 绘制属性
     * @param [in] maxWidth 绘制的最大宽度
     * @param [out] measuredWidth  返回估算的绘制宽度，小于或等于 maxWidth, 可以为nullptr
     * @param [in] tempData 临时变量，用以提高性能
     * @return 返回本行可绘制的字节数，总是小于或者等于byteLength     
     */
    static size_t BreakText(const void* text, size_t byteLength, SkTextEncoding textEncoding,
                            const SkFont& font, FallbackFontCreator fallbackFontCreator,
                            const SkPaint& paint, SkScalar maxWidth, SkScalar* measuredWidth,
                            MeasureTextTempData& tempData);

    /** 特殊版本，临时变量（glyphs，glyphChars，glyphWidths）由外部提供，以改进执行性能
    * @param [in] tempData 临时变量，用以提高性能
    * @param [int/out] breakTextData Break计算相关临时变量，用以提高性能，也可以返回有效数据
    */
    static size_t BreakText(const void* text, size_t byteLength, SkTextEncoding textEncoding,
                            const SkFont& font, FallbackFontCreator fallbackFontCreator,
                            const SkPaint& paint, SkScalar maxWidth, SkScalar* measuredWidth,
                            MeasureTextTempData& tempData,
                            BreakTextTempData* breakTextData);

    /** 返回适合最大宽度的文本字节
    * @param [in] textData 文本数据
    * @param [in] font 字体
    * @param [in] fallbackFontCreator 回退字体管理器
    * @param [in] paint 绘制属性
    * @param [in] maxWidth 绘制的最大宽度
    * @param [out] measuredWidth  返回估算的绘制宽度，小于或等于 maxWidth, 可以为nullptr
    * @param [in] tempData 临时变量，用以提高性能
    * @return 返回本行可绘制的字节数，总是小于或者等于byteLength
    */
    static size_t BreakText(const SkiaTextData& textData,
                            const SkFont& font, FallbackFontCreator fallbackFontCreator,
                            const SkPaint& paint, SkScalar maxWidth, SkScalar* measuredWidth,
                            MeasureTextTempData& tempData);

public:
    /** 统计绘制需要多少行
     * @param [in] text 文本字符串的起始地址
     * @param [in] len 文本字符串的长度（字节）
     * @param [in] textEncoding 文本的编码, SkTextEncoding::kUTF8 or kUTF16 or kUTF32
     * @param [in] font 字体
     * @param [in] fallbackFontCreator 回退字体管理器
     * @param [in] paint 绘制属性
     * @param [in] width 绘制区域的宽度
     * @param [in] lineMode 换行模式
     * @param [out] lineLenList 返回每行文本数据的长度（字节）
     */
    static int32_t CountLines(const char* text, size_t len, SkTextEncoding textEncoding,
                              const SkFont& font, FallbackFontCreator fallbackFontCreator,
                              const SkPaint& paint, SkScalar width, TextBoxLineMode lineMode,
                              std::vector<size_t>* lineLenList = nullptr);

    /** 统计绘制需要多少行
    * @param [in] textData 文本数据
    * @param [in] font 字体
    * @param [in] fallbackFontCreator 回退字体管理器
    * @param [in] paint 绘制属性
    * @param [in] width 绘制区域的宽度
    * @param [in] lineMode 换行模式
    * @param [out] lineLenList 返回每行文本数据的长度（字节）
    */
    static int32_t CountLines(const SkiaTextData& textData,
                              const SkFont& font, FallbackFontCreator fallbackFontCreator,
                              const SkPaint& paint, SkScalar width, TextBoxLineMode lineMode,
                              std::vector<size_t>* lineLenList = nullptr);


    /** 返回适合最大宽度的文本字节
     * @param [in] text 文本字符串的起始地址
     * @param [in] len 文本字符串的长度（字节）
     * @param [in] textEncoding 文本的编码, SkTextEncoding::kUTF8 or kUTF16 or kUTF32
     * @param [in] font 字体
     * @param [in] fallbackFontCreator 回退字体管理器
     * @param [in] paint 绘制属性
     * @param [in] margin 绘制区域的宽度
     * @param [in] lineMode 换行模式
     * @param [in] tempData 临时变量，用以提高性能
     * @param [out] trailing 返回尾部的字符占几个字节（字节）
     */
    static size_t Linebreak(const char* text, const char* stop, SkTextEncoding textEncoding,
                            const SkFont& font, FallbackFontCreator fallbackFontCreator, const SkPaint& paint,
                            SkScalar margin, TextBoxLineMode lineMode, MeasureTextTempData& tempData,
                            size_t* trailing = nullptr);

    /** 返回适合最大宽度的文本字节
    * @param [in] textData 文本数据
    * @param [in] stop 结束位置
    * @param [in] font 字体
    * @param [in] fallbackFontCreator 回退字体管理器
    * @param [in] paint 绘制属性
    * @param [in] margin 绘制区域的宽度
    * @param [in] lineMode 换行模式
    * @param [in] tempData 临时变量，用以提高性能
    * @param [out] trailing 返回尾部的字符占几个字节（字节）
    */
    static size_t Linebreak(const SkiaTextData& textData, const char* stop,
                            const SkFont& font, FallbackFontCreator fallbackFontCreator, const SkPaint& paint,
                            SkScalar margin, TextBoxLineMode lineMode, MeasureTextTempData& tempData,
                            size_t* trailing = nullptr);

private:
    /** 获取UTF32字符串，内部操作时按UTF32编码处理
    */
    static UTF32String GetDrawStringUTF32(const void* text, size_t byteLength, SkTextEncoding textEncoding);
};

} // namespace ui

#endif // UI_RENDER_SKIA_DRAW_SKIA_TEXT_H_
