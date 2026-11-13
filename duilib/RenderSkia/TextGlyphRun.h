#ifndef UI_RENDER_SKIA_TEXT_GLYPH_RUN_H_
#define UI_RENDER_SKIA_TEXT_GLYPH_RUN_H_

#include "duilib/RenderSkia/FastVector.h"
#include "duilib/RenderSkia/SkiaTextData.h"

#include <vector>

namespace ui
{

/** 文本运行结果（run）：一段命中同一字体的连续字形
 *
 * 设计目的：把"字符 -> 字形 -> 命中字体"的解析结果从栈上临时变量变成可传递的数据，
 *          由测量/布局阶段产出（BuildGlyphRuns），绘制阶段直接批量提交（DrawGlyphRuns），
 *          避免同一段文本在测量和绘制时各解析一次。
 *
 * 生命周期：m_pGlyphs / m_pAdvances 指向 TextGlyphRunList 内部的缓冲区，
 *          TextGlyphRunList 被 Clear 或析构后即失效，调用方不要单独保存本结构。
 */
struct TextGlyphRun
{
    /** 命中的字体接口（含回退字体）；调用方无法获取时为 nullptr
    */
    const IFont*     m_pFont = nullptr;

    /** 命中的Skia字体句柄，绘制时直接使用
    */
    const SkFont*    m_pSkFont = nullptr;

    /** 字形数组（批量），指向 TextGlyphRunList 内部缓冲区
    */
    const SkGlyphID* m_pGlyphs = nullptr;

    /** 每个字形的步进宽度（与 m_pGlyphs 一一对应），指向 TextGlyphRunList 内部缓冲区
    */
    const SkScalar*  m_pAdvances = nullptr;

    /** 每个字形相对 run 起点的偏移（与 m_pGlyphs 一一对应），指向 TextGlyphRunList 内部缓冲区
    * 仅在构建 run 时启用 positions 模式时有效，非空时优先于 m_pAdvances
    */
    const SkPoint*   m_pPositions = nullptr;

    /** run 起点X坐标（第一个字形的绘制位置）
    */
    SkScalar         m_x = 0;

    /** run 基线Y坐标
    */
    SkScalar         m_y = 0;

    /** 字形个数
    */
    uint32_t         m_nCount = 0;

    /** 是否旋转90度绘制（纵向文本路径使用）
    */
    bool             m_bRotate90 = false;
};

/** 文本运行结果列表：持有所有 run 的字形与宽度缓冲区
 *
 * 用法（游标方式，适合逐字符解析的场景）：
 *   runs.BeginRun(pFont, pSkFont, x, y);
 *   runs.AddGlyph(glyphId, advance);   // 可调用多次
 *   runs.EndRun();
 *
 * 用法（整段方式，适合已有解析结果的场景）：
 *   runs.AddRun(pFont, pSkFont, pGlyphs, pAdvances, nCount, x, y);
 *
 * 缓冲区在 Clear 时只清空计数、保留容量，因此跨帧复用时不会反复分配内存。
 */
class TextGlyphRunList
{
public:
    TextGlyphRunList() = default;
    TextGlyphRunList(const TextGlyphRunList&) = delete;
    TextGlyphRunList& operator=(const TextGlyphRunList&) = delete;
    ~TextGlyphRunList() = default;

    /** 清空全部 run（保留已分配的缓冲区，便于跨帧复用）
    */
    void Clear();

    /** 判断是否没有有效的 run
    */
    bool IsEmpty() const { return m_nRunCount == 0; }

    /** 获取 run 的个数
    */
    size_t GetRunCount() const { return m_nRunCount; }

    /** 获取指定下标的 run
    * @param [in] nIndex run 下标，必须小于 GetRunCount()
    */
    const TextGlyphRun& GetRun(size_t nIndex) const;

    /** 是否处于 BeginRun 与 EndRun 之间
    */
    bool IsInRun() const { return m_bInRun; }

    /** 开始一个新的 run（若上一个 run 未结束，会先自动结束它）
    * @param [in] pFont 命中的字体接口，可以为 nullptr
    * @param [in] pSkFont 命中的Skia字体句柄，可以为 nullptr
    * @param [in] x run 起点X坐标
    * @param [in] y 基线Y坐标
    * @param [in] bRotate90 是否旋转90度绘制
    * @param [in] bUsePositions 是否启用positions模式（每个字形携带相对run起点的偏移，优先于步进宽度）
    */
    void BeginRun(const IFont* pFont, const SkFont* pSkFont,
                  SkScalar x, SkScalar y, bool bRotate90 = false, bool bUsePositions = false);

    /** 向当前 run 追加一个字形（需先调用 BeginRun）
    * @param [in] glyphId 字形ID
    * @param [in] advance 该字形的步进宽度
    * @param [in] offset 字形相对run起点的偏移（仅在positions模式下有效）
    */
    void AddGlyph(SkGlyphID glyphId, SkScalar advance, const SkPoint& offset = SkPoint());

    /** 结束当前 run（回填字形/宽度指针与计数；没有字形的 run 会被丢弃）
    */
    void EndRun();

    /** 直接追加一个完整的 run（字形与宽度数据会被拷贝）
    * @param [in] pFont 命中的字体接口，可以为 nullptr
    * @param [in] pSkFont 命中的Skia字体句柄，可以为 nullptr
    * @param [in] pGlyphs 字形数组
    * @param [in] pAdvances 步进宽度数组，可以为 nullptr（此时按 0 处理）
    * @param [in] nCount 字形个数
    * @param [in] x run 起点X坐标
    * @param [in] y 基线Y坐标
    * @param [in] bRotate90 是否旋转90度绘制
    */
    void AddRun(const IFont* pFont, const SkFont* pSkFont,
                const SkGlyphID* pGlyphs, const SkScalar* pAdvances, uint32_t nCount,
                SkScalar x, SkScalar y, bool bRotate90 = false);

    /** 获取所有 run 的宽度之和（按步进宽度累加）
    */
    SkScalar GetTotalWidth() const;

private:
    /** 单个 run 的数据：run 头信息 + 自己持有的字形/宽度缓冲区
    */
    struct RunData
    {
        /** run 头信息（其中的指针在 EndRun 时回填）
        */
        TextGlyphRun m_run;

        /** 字形缓冲区
        */
        FastVector<SkGlyphID> m_glyphs;

        /** 步进宽度缓冲区（与 m_glyphs 一一对应）
        */
        FastVector<SkScalar> m_advances;

        /** 字形偏移缓冲区（与 m_glyphs 一一对应，仅positions模式下使用）
        */
        FastVector<SkPoint> m_positions;

        /** 是否启用positions模式
        */
        bool m_bUsePositions = false;
    };

private:
    /** 已分配的 run 缓冲区（下标小于 m_nRunCount 的是有效 run，其余是复用池）
    */
    std::vector<RunData> m_runPool;

    /** 有效 run 的个数
    */
    size_t m_nRunCount = 0;

    /** 是否处于 BeginRun 与 EndRun 之间
    */
    bool m_bInRun = false;
};

} // namespace ui

#endif //UI_RENDER_SKIA_TEXT_GLYPH_RUN_H_
