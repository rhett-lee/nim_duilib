#include "duilib/RenderSkia/TextGlyphRun.h"

namespace ui
{

void TextGlyphRunList::Clear()
{
    m_nRunCount = 0;
    m_bInRun = false;
}

const TextGlyphRun& TextGlyphRunList::GetRun(size_t nIndex) const
{
    ASSERT(nIndex < m_nRunCount);
    if (nIndex >= m_nRunCount) {
        static const TextGlyphRun s_emptyRun;
        return s_emptyRun;
    }
    return m_runPool[nIndex].m_run;
}

void TextGlyphRunList::BeginRun(const IFont* pFont, const SkFont* pSkFont,
                                SkScalar x, SkScalar y, bool bRotate90, bool bUsePositions)
{
    if (m_bInRun) {
        //上一个 run 未结束，先结束它
        EndRun();
    }
    if (m_nRunCount >= m_runPool.size()) {
        m_runPool.emplace_back();
    }
    RunData& runData = m_runPool[m_nRunCount];
    runData.m_glyphs.clear();
    runData.m_advances.clear();
    runData.m_positions.clear();
    runData.m_bUsePositions = bUsePositions;
    runData.m_run = TextGlyphRun();
    runData.m_run.m_pFont = pFont;
    runData.m_run.m_pSkFont = pSkFont;
    runData.m_run.m_x = x;
    runData.m_run.m_y = y;
    runData.m_run.m_bRotate90 = bRotate90;
    ++m_nRunCount;
    m_bInRun = true;
}

void TextGlyphRunList::AddGlyph(SkGlyphID glyphId, SkScalar advance, const SkPoint& offset)
{
    if (!m_bInRun || (m_nRunCount == 0)) {
        ASSERT(false);
        return;
    }
    RunData& runData = m_runPool[m_nRunCount - 1];
    runData.m_glyphs.push_back(glyphId);
    if (runData.m_bUsePositions) {
        runData.m_positions.push_back(offset);
    }
    else {
        runData.m_advances.push_back(advance);
    }
}

void TextGlyphRunList::EndRun()
{
    if (!m_bInRun) {
        return;
    }
    m_bInRun = false;
    if (m_nRunCount == 0) {
        return;
    }
    RunData& runData = m_runPool[m_nRunCount - 1];
    if (runData.m_glyphs.empty()) {
        //没有字形的 run 没有意义，直接丢弃
        --m_nRunCount;
        return;
    }
    runData.m_run.m_pGlyphs = runData.m_glyphs.data();
    if (runData.m_bUsePositions) {
        ASSERT(runData.m_positions.size() == runData.m_glyphs.size());
        runData.m_run.m_pPositions = runData.m_positions.data();
    }
    else {
        ASSERT(runData.m_advances.size() == runData.m_glyphs.size());
        runData.m_run.m_pAdvances = runData.m_advances.data();
    }
    runData.m_run.m_nCount = (uint32_t)runData.m_glyphs.size();
}

void TextGlyphRunList::AddRun(const IFont* pFont, const SkFont* pSkFont,
                              const SkGlyphID* pGlyphs, const SkScalar* pAdvances, uint32_t nCount,
                              SkScalar x, SkScalar y, bool bRotate90)
{
    if ((pGlyphs == nullptr) || (nCount == 0)) {
        return;
    }
    BeginRun(pFont, pSkFont, x, y, bRotate90);
    for (uint32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        AddGlyph(pGlyphs[nIndex], (pAdvances != nullptr) ? pAdvances[nIndex] : 0);
    }
    EndRun();
}

SkScalar TextGlyphRunList::GetTotalWidth() const
{
    SkScalar fTotalWidth = 0;
    for (size_t nRun = 0; nRun < m_nRunCount; ++nRun) {
        const TextGlyphRun& run = m_runPool[nRun].m_run;
        if (run.m_pAdvances == nullptr) {
            continue;
        }
        for (uint32_t nIndex = 0; nIndex < run.m_nCount; ++nIndex) {
            fTotalWidth += run.m_pAdvances[nIndex];
        }
    }
    return fTotalWidth;
}

} // namespace ui
