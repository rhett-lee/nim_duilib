#include "FontMgr_Skia.h"
#include "duilib/Utils/StringUtil.h"

#pragma warning (push)
#pragma warning (disable: 4244)

#include "include/core/SkFontMgr.h"
#include "include/core/SkFont.h"
#include "include/core/SkData.h"

#if defined(SK_BUILD_FOR_WIN)
    #include "include/ports/SkTypeface_win.h"
#endif

#pragma warning (pop)

namespace ui
{
/** 从文件加载的字体数据
*/
struct FontFromFile
{
    //字体名称
    DString m_fontFamilyName;

    //字体风格 -> 字体数据
    std::vector<sk_sp<SkTypeface>> m_fontTypefaceList;
};

/** 从文件加载的字体管理器
*/
class FontFileManager
{
public:
    /** 添加字体文件加载的结果
    */
    bool AddFontTypeface(const sk_sp<SkTypeface>& spTypeface)
    {
        ASSERT(spTypeface != nullptr);
        if (spTypeface == nullptr) {
            return false;
        }

        //加载成功后，添加到管理器中集中管理
        SkString fontName;
        spTypeface->getFamilyName(&fontName);
        ASSERT(!fontName.isEmpty());
        if (fontName.isEmpty()) {
            return false;
        }

        DString fontFamilyName = StringUtil::UTF8ToT(fontName.c_str());
        ASSERT(!fontFamilyName.empty());
        if (fontFamilyName.empty()) {
            return false;
        }
        size_t nCount = m_fontFamilies.size();
        size_t nNewIndex = nCount;
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            FontFromFile& fontFromFile = m_fontFamilies[nIndex];
            if (fontFromFile.m_fontFamilyName == fontFamilyName) {
                nNewIndex = nIndex;
                break;
            }
        }

        if (nNewIndex >= m_fontFamilies.size()) {
            //添加
            FontFromFile fontFromFile;
            fontFromFile.m_fontFamilyName = fontFamilyName;
            fontFromFile.m_fontTypefaceList.push_back(spTypeface);
            m_fontFamilies.push_back(fontFromFile);
        }
        else {
            //更新
            bool bFoundSame = false;
            FontFromFile& fontFromFile = m_fontFamilies[nNewIndex];
            for (const sk_sp<SkTypeface>& typeface : fontFromFile.m_fontTypefaceList) {
                if (typeface->fontStyle() == spTypeface->fontStyle()) {
                    //不重复添加
                    bFoundSame = true;
                    break;
                }
            }
            if (!bFoundSame) {
                fontFromFile.m_fontTypefaceList.push_back(spTypeface);
            }
        }
        return true;
    }

    /** 创建字体
    */
    sk_sp<SkTypeface> MakeTypeface(const DString& fontName, SkFontStyle style)
    {
        sk_sp<SkTypeface> skTypeface;
        if (m_fontFamilies.empty()) {
            return skTypeface;
        }
        for (const FontFromFile& fontFromFile : m_fontFamilies) {
            if (fontName == fontFromFile.m_fontFamilyName) {
                for (const sk_sp<SkTypeface>& typeface : fontFromFile.m_fontTypefaceList) {
                    if (typeface->fontStyle() == style) {
                        //找到一致的字体
                        skTypeface = typeface;
                        break;
                    }
                }
            }
        }
        return skTypeface;
    }

    /** 获取字体个数
    */
    uint32_t GetFontCont() const
    {
        return (uint32_t)m_fontFamilies.size();
    }

    /** 获取字体名称
    */
    DString GetFontName(uint32_t nIndex) const
    {
        DString fontName;
        if (nIndex < m_fontFamilies.size()) {
            const FontFromFile& fontFromFile = m_fontFamilies[nIndex];
            fontName = fontFromFile.m_fontFamilyName;
        }
        return fontName;
    }

    /** 是否包括该字体
    */
    bool HasFontName(const DString& fontName) const
    {
        for (const FontFromFile& fontFromFile : m_fontFamilies) {
            if (fontName == fontFromFile.m_fontFamilyName) {
                return true;
            }
        }
        return false;
    }

    /** 清除已经加载的字体数据
    */
    void Clear()
    {
        m_fontFamilies.clear();
    }

private:
    /** 字体名称->字体数据
    */
    std::vector<FontFromFile> m_fontFamilies;
};

class FontMgr_Skia::TImpl
{
public:
    /** Skia的字体管理器
    */
    sk_sp<SkFontMgr> m_pSkFontMgr;

    /** 从文件加载的字体列表
    */
    FontFileManager m_fontFileMgr;

    /** 默认的字体名称
    */
    DString m_defaultFontName;
};

FontMgr_Skia::FontMgr_Skia()
{
    m_impl = new TImpl;

    //创建Skia的字体管理器对象，进程内唯一
#if defined(SK_BUILD_FOR_WIN)
    m_impl->m_pSkFontMgr = SkFontMgr_New_DirectWrite();
#else
    m_impl->m_pSkFontMgr = nullptr;
#endif

    ASSERT(m_impl->m_pSkFontMgr != nullptr);
}

FontMgr_Skia::~FontMgr_Skia()
{
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

uint32_t FontMgr_Skia::GetFontCount() const
{
    uint32_t nFontCount = 0;
    if (m_impl->m_pSkFontMgr != nullptr) {
        nFontCount = (uint32_t)m_impl->m_pSkFontMgr->countFamilies();
    }
    nFontCount += m_impl->m_fontFileMgr.GetFontCont();
    return nFontCount;
}

bool FontMgr_Skia::GetFontName(uint32_t nIndex, DString& fontName) const
{
    fontName.clear();
    ASSERT(m_impl->m_pSkFontMgr != nullptr);
    if (m_impl->m_pSkFontMgr == nullptr) {
        return false;
    }

    uint32_t nFontCount = GetFontCount();
    ASSERT(nIndex < nFontCount);
    if (nIndex >= nFontCount) {
        return false;
    }
    if (nIndex < (uint32_t)m_impl->m_pSkFontMgr->countFamilies()) {
        SkString fontFamilyName;
        m_impl->m_pSkFontMgr->getFamilyName((int)nIndex, &fontFamilyName);
        fontName = StringUtil::UTF8ToT(fontFamilyName.c_str());
    }
    else {
        uint32_t nFontFileIndex = nIndex - (uint32_t)m_impl->m_pSkFontMgr->countFamilies();
        fontName = m_impl->m_fontFileMgr.GetFontName(nFontFileIndex);
    }

    return !fontName.empty();
}

bool FontMgr_Skia::HasFontName(const DString& fontName) const
{
    if (fontName.empty()) {
        return false;
    }

    ASSERT(m_impl->m_pSkFontMgr != nullptr);
    if (m_impl->m_pSkFontMgr == nullptr) {
        return false;
    }
    std::string fontFamily = StringUtil::TToUTF8(fontName); //转换为UTF8格式
    auto styleSet = m_impl->m_pSkFontMgr->matchFamily(fontFamily.c_str());
    if ((styleSet != nullptr) && (styleSet->count() > 0)) {
        return true;
    }
    return m_impl->m_fontFileMgr.HasFontName(fontName);
}

void FontMgr_Skia::SetDefaultFontName(const DString& fontName)
{
    m_impl->m_defaultFontName = fontName;
}

bool FontMgr_Skia::LoadFontFile(const DString& fontFilePath)
{
    ASSERT(!fontFilePath.empty());
    if (fontFilePath.empty()) {
        return false;
    }

    ASSERT(m_impl->m_pSkFontMgr != nullptr);
    if (m_impl->m_pSkFontMgr == nullptr) {
        return false;
    }

    std::string fontFile = StringUtil::TToUTF8(fontFilePath); //转换为UTF8格式的路径
    ASSERT(!fontFile.empty());
    if (fontFile.empty()) {
        return false;
    }
    sk_sp<SkTypeface> spTypeface = m_impl->m_pSkFontMgr->makeFromFile(fontFile.c_str());
    return m_impl->m_fontFileMgr.AddFontTypeface(spTypeface);
}

bool FontMgr_Skia::LoadFontFileData(const void* data, size_t length)
{
    ASSERT(data != nullptr);
    ASSERT(length > 0);
    if ((data == nullptr) || (length == 0)) {
        return false;
    }
    sk_sp<SkData> skData = SkData::MakeWithCopy(data, length);
    ASSERT(skData != nullptr);
    if (skData == nullptr) {
        return false;
    }
    sk_sp<SkTypeface> spTypeface = m_impl->m_pSkFontMgr->makeFromData(skData);
    return m_impl->m_fontFileMgr.AddFontTypeface(spTypeface);
}

void FontMgr_Skia::ClearFontFiles()
{
    m_impl->m_fontFileMgr.Clear();
}

SkFont* FontMgr_Skia::CreateSkFont(const UiFont& fontInfo)
{
    ASSERT(!fontInfo.m_fontName.empty());
    if (fontInfo.m_fontName.empty()) {
        return nullptr;
    }

    ASSERT(fontInfo.m_fontSize != 0);
    if (fontInfo.m_fontSize == 0) {
        return nullptr;
    }

    SkFontStyle fontStyle = SkFontStyle::Normal();
    if (fontInfo.m_bBold && fontInfo.m_bItalic) {
        fontStyle = SkFontStyle::BoldItalic();
    }
    else if (fontInfo.m_bBold) {
        fontStyle = SkFontStyle::Bold();
    }
    else if (fontInfo.m_bItalic) {
        fontStyle = SkFontStyle::Italic();
    }

    //优先检查外部加载的字体是否满足要求, 如果未能匹配，再通过系统字体创建
    sk_sp<SkTypeface> spTypeface = m_impl->m_fontFileMgr.MakeTypeface(fontInfo.m_fontName.c_str(), fontStyle);
    if ((spTypeface == nullptr) && !m_impl->m_defaultFontName.empty()){
        //未能匹配则查询默认字体是否能匹配
        spTypeface = m_impl->m_fontFileMgr.MakeTypeface(m_impl->m_defaultFontName, fontStyle);
    }
    if (spTypeface == nullptr) {
        //UTF8编码的字体名称
        std::string fontName = StringUtil::TToUTF8(fontInfo.m_fontName.c_str());
        ASSERT(!fontName.empty());
        if (fontName.empty()) {
            return nullptr;
        }
        sk_sp<SkFontMgr> pSkFontMgr = m_impl->m_pSkFontMgr;
        ASSERT(pSkFontMgr != nullptr);
        if (pSkFontMgr != nullptr) {
            sk_sp<SkFontStyleSet> skFontStyleSet = pSkFontMgr->matchFamily(fontName.c_str());
            if (skFontStyleSet != nullptr) {
                spTypeface = skFontStyleSet->matchStyle(fontStyle);
            }
            if ((spTypeface == nullptr) && !m_impl->m_defaultFontName.empty()) {
                //未能匹配则查询默认字体是否能匹配
                std::string defaultFontName = StringUtil::TToUTF8(m_impl->m_defaultFontName);
                skFontStyleSet = pSkFontMgr->matchFamily(defaultFontName.c_str());
                if (skFontStyleSet != nullptr) {
                    spTypeface = skFontStyleSet->matchStyle(fontStyle);
                }
            }
            if (spTypeface == nullptr) {
                spTypeface = pSkFontMgr->legacyMakeTypeface(fontName.c_str(), fontStyle);
            }            
        }          
    }
    ASSERT(spTypeface != nullptr);
    if (spTypeface == nullptr) {
        return nullptr;
    }
    SkFont* skFont = new SkFont();
    skFont->setTypeface(spTypeface);
    skFont->setSize(SkIntToScalar(std::abs(fontInfo.m_fontSize)));
    skFont->setEdging(SkFont::Edging::kSubpixelAntiAlias);
    skFont->setSubpixel(true);
    return skFont;
}

void FontMgr_Skia::DeleteSkFont(SkFont* pSkFont)
{
    if (pSkFont != nullptr) {
        delete pSkFont;
    }
}

} // namespace ui
