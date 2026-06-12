#include "FontMgr_Skia.h"
#include "duilib/Utils/StringConvert.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFont.h"
#include "include/core/SkData.h"

#include "include/core/SkRefCnt.h"      // 包含sk_sp的完整定义
#include "include/core/SkTypeface.h"    // 确保SkFontMgr完整定义

#if defined(SK_BUILD_FOR_MAC)
    #include "include/ports/SkFontMgr_mac_ct.h"  // macOS专用API
#endif

#if defined(SK_BUILD_FOR_WIN)
    #include "include/ports/SkTypeface_win.h"
#elif defined(SK_BUILD_FOR_MAC)
    #include "include/ports/SkTypeface_mac.h"
#else
    #include "include/ports/SkFontMgr_fontconfig.h"
    #include "include/ports/SkFontScanner_FreeType.h"
#endif

#include "SkiaHeaderEnd.h"

#include <unordered_map>

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

        DString fontFamilyName = StringConvert::UTF8ToT(fontName.c_str());
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

    /** 字体名称对应的FontStyleSet缓存（发现部分Linux系统创建字体时，速度特别慢，调用一次需要几十毫秒，所以有必要做缓存）
    */
    std::unordered_map<std::string, sk_sp<SkFontStyleSet>> m_fontStyleSetMap;

    /** 字体名称缓存
    */
    std::unordered_map<DString, bool> m_fontNameMap;

    /** 字体回退管理器（生命周期由设置者管理）
    */
    IFallbackFontMgr* m_pFallbackFontMgr;
};

FontMgr_Skia::FontMgr_Skia()
{
    m_impl = new TImpl;
    m_impl->m_pFallbackFontMgr = nullptr;

    //创建Skia的字体管理器对象，进程内唯一
#if defined(SK_BUILD_FOR_WIN)
    //Windows系统
    m_impl->m_pSkFontMgr = SkFontMgr_New_DirectWrite();
#elif defined(SK_BUILD_FOR_MAC) 
    // macOS/iOS系统
    m_impl->m_pSkFontMgr = SkFontMgr_New_CoreText(nullptr);  // 使用nullptr表示系统默认字体集
#else
    //Linux系统, FreeBSD系统
    m_impl->m_pSkFontMgr = SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());
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
        fontName = StringConvert::UTF8ToT(fontFamilyName.c_str());
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
    auto iterFontName = m_impl->m_fontNameMap.find(fontName);
    if (iterFontName != m_impl->m_fontNameMap.end()) {
        //优先从缓存中匹配
        return iterFontName->second;
    }

    ASSERT(m_impl->m_pSkFontMgr != nullptr);
    if (m_impl->m_pSkFontMgr == nullptr) {
        return false;
    }
    bool bFound = false;
    const std::string dstFontFamily = StringConvert::TToUTF8(fontName); //转换为UTF8格式
    const int nCountFamilies = m_impl->m_pSkFontMgr->countFamilies();
    for (int nIndex = 0; nIndex < nCountFamilies; ++nIndex) {
        SkString fontFamilyName;//字体名称，该名称为与系统语言相同的字体名称，比如中文版时返回"微软雅黑"，英文版时返回"Microsoft YaHei"，所以很可能匹配不到设置的字体
        m_impl->m_pSkFontMgr->getFamilyName((int)nIndex, &fontFamilyName);
        if (!fontFamilyName.isEmpty() && (dstFontFamily == fontFamilyName.c_str())) {
            bFound = true;
            break;
        }
    }
    if (!bFound) {
        bFound = m_impl->m_fontFileMgr.HasFontName(fontName);
    }
    if (!bFound) {
        //使用字体本身的名称（每个语言一个名称），进行精确匹配字体名称
        for (int nIndex = 0; nIndex < nCountFamilies; ++nIndex) {
            SkString fontFamilyName;
            m_impl->m_pSkFontMgr->getFamilyName((int)nIndex, &fontFamilyName);
            if (!fontFamilyName.isEmpty()) {
                auto skTypeFace = m_impl->m_pSkFontMgr->legacyMakeTypeface(fontFamilyName.c_str(), SkFontStyle::Normal());
                if (skTypeFace != nullptr) {
                    SkTypeface::LocalizedStrings* iter = skTypeFace->createFamilyNameIterator();
                    if (iter != nullptr) {
                        SkTypeface::LocalizedString ls;
                        while (iter->next(&ls)) {
                            if (!ls.fString.isEmpty() && (dstFontFamily == ls.fString.c_str())) {
                                bFound = true;
                                break;
                            }
                        }
                        iter->unref();
                    }
                }                        
            }
            if (bFound) {
                break;
            }
        }
    }
    m_impl->m_fontNameMap[fontName] = bFound;
    return bFound;
}

void FontMgr_Skia::SetDefaultFontName(const DString& fontName)
{
    if (HasFontName(fontName)) {
        //字体必须存在
        m_impl->m_defaultFontName = fontName;
    }
    else {
        ASSERT(0);
    }
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

    std::string fontFile = StringConvert::TToUTF8(fontFilePath); //转换为UTF8格式的路径
    ASSERT(!fontFile.empty());
    if (fontFile.empty()) {
        return false;
    }
    sk_sp<SkTypeface> spTypeface = m_impl->m_pSkFontMgr->makeFromFile(fontFile.c_str());
    if (spTypeface == nullptr) {
        //加载失败不加断言
        return false;
    }
    m_impl->m_fontNameMap.clear();
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
    m_impl->m_fontNameMap.clear();
    return m_impl->m_fontFileMgr.AddFontTypeface(spTypeface);
}

void FontMgr_Skia::ClearFontFiles()
{
    m_impl->m_fontNameMap.clear();
    m_impl->m_fontFileMgr.Clear();
}

void FontMgr_Skia::ClearFontCache()
{
    m_impl->m_fontNameMap.clear();
    m_impl->m_fontStyleSetMap.clear();
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
    sk_sp<SkFontMgr> pSkFontMgr = m_impl->m_pSkFontMgr;
    ASSERT(pSkFontMgr != nullptr);
    if (pSkFontMgr == nullptr) {
        return nullptr;
    }

    //需要创建的字体列表（包含默认字体）
    std::vector<DString> fontNameList;
    if (!fontInfo.m_fontName.empty() && HasFontName(fontInfo.m_fontName.c_str())) {
        fontNameList.push_back(fontInfo.m_fontName.c_str());
    }
    if (!m_impl->m_defaultFontName.empty() && (m_impl->m_defaultFontName != fontInfo.m_fontName) && HasFontName(m_impl->m_defaultFontName)) {
        fontNameList.push_back(m_impl->m_defaultFontName);
    }

    sk_sp<SkTypeface> spTypeface;
    for (const DString& inFontName : fontNameList) {
        //优先检查外部加载的字体是否满足要求, 如果未能匹配，再通过系统字体创建
        spTypeface = m_impl->m_fontFileMgr.MakeTypeface(inFontName.c_str(), fontStyle);
        if (spTypeface != nullptr) {
            break;
        }

        //使用FontMgr接口创建字体
        std::string fontName = StringConvert::TToUTF8(inFontName);
        ASSERT(!fontName.empty());
        if (fontName.empty()) {
            continue;
        }
        sk_sp<SkFontStyleSet> skFontStyleSet;
        auto iter = m_impl->m_fontStyleSetMap.find(fontName.c_str());
        if (iter != m_impl->m_fontStyleSetMap.end()) {
            skFontStyleSet = iter->second;
        }
        if (skFontStyleSet == nullptr) {
            skFontStyleSet = pSkFontMgr->matchFamily(fontName.c_str());
            if (skFontStyleSet != nullptr) {
                m_impl->m_fontStyleSetMap[fontName] = skFontStyleSet;
            }
        }
        if (skFontStyleSet != nullptr) {
            spTypeface = skFontStyleSet->matchStyle(fontStyle);
        }
        if (spTypeface != nullptr) {
            break;
        }
    }

    if (spTypeface == nullptr) {
        //使用系统默认字体（但不能保证正确，比如Windows平台，该接口创建的字体是不能显示中文的）
        spTypeface = pSkFontMgr->legacyMakeTypeface(nullptr, fontStyle);
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

void* FontMgr_Skia::GetSkiaFontMgrPtr() const
{
    return &(m_impl->m_pSkFontMgr);
}

void FontMgr_Skia::SetFallbackFontMgr(IFallbackFontMgr* pFallbackFontMgr)
{
    m_impl->m_pFallbackFontMgr = pFallbackFontMgr;
}

IFallbackFontMgr* FontMgr_Skia::GetFallbackFontMgr() const
{
    return m_impl->m_pFallbackFontMgr;
}

} // namespace ui
