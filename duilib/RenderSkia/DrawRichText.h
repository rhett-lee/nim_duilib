#ifndef UI_RENDER_SKIA_DRAW_RICH_TEXT_H_
#define UI_RENDER_SKIA_DRAW_RICH_TEXT_H_

#include "duilib/Render/IRender.h"

//Skia相关类的前置声明
class SkCanvas;
class SkPaint;
struct SkRect;
class SkFont;
struct SkPoint;
enum class SkTextEncoding;

namespace ui 
{
class IRender;

/** RichText格式文本的绘制实现
*/
class UILIB_API DrawRichText
{
public:
    DrawRichText(IRender* pRender, SkCanvas* pSkCanvas, SkPaint* pSkPaint, SkPoint* pSkPointOrg);
    DrawRichText(const DrawRichText& r) = delete;
    DrawRichText& operator = (const DrawRichText& r) = delete;
    ~DrawRichText() = default;

public:

    /** 绘制格式文本
    */
    void InternalDrawRichText(const UiRect& rcTextRect,
                              const UiSize& szScrollOffset,
                              IRenderFactory* pRenderFactory, 
                              const std::vector<RichTextData>& richTextData,                   
                              uint8_t uFade,
                              bool bMeasureOnly,
                              RichTextLineInfoParam* pLineInfoParam,
                              std::shared_ptr<DrawRichTextCache>* pDrawRichTextCache,
                              std::vector<std::vector<UiRect>>* pRichTextRects);

    /** 判断RichText的绘制缓存是否有效
    * @param [in] textRect 绘制文本的矩形区域
    * @param [in] richTextData 格式化文字内容，返回文字绘制的区域
    * @param [out] spDrawRichTextCache 返回绘制缓存
    */
    bool IsValidDrawRichTextCache(const UiRect& textRect,
                                  const std::vector<RichTextData>& richTextData,
                                  const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache);

    /** 更新RichText的绘制缓存(增量计算)
    * @param [in] spOldDrawRichTextCache 需要更新的缓存
    * @param [in] spUpdateDrawRichTextCache 增量绘制的缓存
    * @param [in,out] richTextDataNew 最新的完整数据, 数据会交换给内部容器
    * @param [in] nStartLine 重新计算的起始行号
    * @param [in] modifiedLines 有修改的行号
    * @param [in] nModifiedRows 修改后的文本，计算后切分为几行（逻辑行）
    * @param [in] deletedLines 删除的行
    * @param [in] nDeletedRows 删除了几个逻辑行
    * @param [in] rowRectTopList 每个逻辑行的top坐标，用于更新行的坐标(下标值为逻辑行，从0开始编号)
    */
    bool UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>& spOldDrawRichTextCache,
                                 const std::shared_ptr<DrawRichTextCache>& spUpdateDrawRichTextCache,
                                 std::vector<RichTextData>& richTextDataNew,
                                 size_t nStartLine,
                                 const std::vector<size_t>& modifiedLines,
                                 size_t nModifiedRows,
                                 const std::vector<size_t>& deletedLines,
                                 size_t nDeletedRows,
                                 const std::vector<int32_t>& rowRectTopList);

    /** 比较两个绘制缓存的数据是否一致
    */
    bool IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const;

    /** 绘制RichText的缓存中的内容（绘制前，需要使用IsValidDrawRichTextCache判断缓存是否失效）
    * @param [in] spDrawRichTextCache 缓存的数据
    * @param [in] rcNewTextRect 绘制文本的矩形区域
    * @param [in] szNewScrollOffset 新的滚动条位置
    * @param [in] rowXOffset 每行的横向偏移列表（逻辑行）
    * @param [in] uFade 透明度（0 - 255）
    * @param [out] pRichTextRects 如果不为nullptr，则返回richTextData中每个数据绘制的矩形范围列表
    */
    void DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                               const UiRect& textRect,
                               const UiSize& szNewScrollOffset,
                               const std::vector<int32_t>& rowXOffset,
                               uint8_t uFade,
                               std::vector<std::vector<UiRect>>* pRichTextRects = nullptr);

private:
    /** 将文本按照换行符（'\r'或者'\n'）切分为多行
    */
    void SplitLines(const std::wstring_view& lineText, std::vector<uint32_t>& lineSeprators, std::vector<std::wstring_view>& lineTextViewList);

    /** 绘制一个字符，记录字符属性
    * @param [in] pLineInfoParam 字符属性记录表
    * @param [in] ch 当前绘制的字符, 仅当回车和换行符等特殊字符时有效
    * @param [in] glyphChars 当前字对应的Unicode字符数（1或者2）
    * @param [in] glyphCount 字符总个数
    * @param [in] nLineTextIndex 文本物理行号（即换行符'\n'切分的行）
    * @param [in] nLineTextRowIndex 物理行内的逻辑行号(每个物理行中，从0开始编号)
    * @param [in] xPos 字符绘制的X坐标
    * @param [in] yPos 字符绘制的Y坐标
    * @param [in] glyphWidth 当前字符的绘制宽度
    * @param [in] nRowHeight 当前行高
    */
    void OnDrawUnicodeChar(RichTextLineInfoParam* pLineInfoParam, DStringW::value_type ch, uint8_t glyphChars, size_t glyphCount,
                           size_t nLineTextIndex, uint32_t nLineTextRowIndex, float xPos, int32_t yPos, float glyphWidth, int32_t nRowHeight);

    /** 按设置的属性，绘制文字，由外部指定字符编码
    */
    void DrawTextString(const UiRect& textRect,
                        const char* text, size_t len, SkTextEncoding textEncoding,
                        uint32_t uFormat, const SkPaint& skPaint, IFont* pFont) const;

    /** 按设置的属性，绘制文字
    */
    void DrawTextString(const UiRect& textRect, const DString& strText, uint32_t uFormat,
                        const SkPaint& skPaint, IFont* pFont) const;

    /** 获取文本编码
    */
    SkTextEncoding GetTextEncoding() const;

private:
    /** 关联的渲染接口
    */
    IRender* m_pRender;

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

#endif // UI_RENDER_SKIA_DRAW_RICH_TEXT_H_
