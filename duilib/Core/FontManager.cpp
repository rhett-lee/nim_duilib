#include "FontManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include <memory>

namespace ui 
{
/** 字体回退管理器（当支持的字体无法显示字符时，会查询回退字体管理器，以正确显示文字）
*   当前用于Emoji文字的显示，用于扩展汉字的显示等
*/
class FontManager::FallbackFontMgrImpl : public IFallbackFontMgr
{
public:
    /** 构造函数
    */
    explicit FallbackFontMgrImpl(FontManager* pFontManager) :
        m_pFontManager(pFontManager)
    {
    }

    /** 创建指定字体的回退字体接口
    * @param [in] pFont 当前字体接口
    * @param [in] unicodeChar UTF32字符，如果为0表示不支持字符检测
    * @param [out] glyphId 如果unicodeChar不为0，返回对应的SkGlyphID值
    * @return 返回对应的回退字体接口
    */
    virtual IFont* CreateFallbackFont(const IFont* pFont, uint32_t unicodeChar, uint16_t* glyphId) override
    {
        ASSERT((pFont != nullptr) && (m_pFontManager != nullptr));
        if ((pFont == nullptr) || (m_pFontManager == nullptr)) {
            return nullptr;
        }
        std::vector<DString> fallbackFontFamilyNames = m_pFontManager->m_fallbackFontFamilyNames;
        if (fallbackFontFamilyNames.empty()) {
            return nullptr;
        }

        //首先查询缓存，如果缓存中存在，则直接返回
        std::vector<IFont*>& fallbackFontList = m_pFontManager->m_fallbackFontMap[pFont];
        for (IFont* pCacheFont : fallbackFontList) {
            if (pCacheFont == nullptr) {
                continue;
            }
            if (unicodeChar == 0) {
                //不校验是否支持该字符
                return pCacheFont;
            }
            else if (pCacheFont->IsUnicodeCharSupported(unicodeChar, glyphId)) {
                //支持该字符，直接返回
                return pCacheFont;
            }
        }

        IFontMgr* pFontMgr = nullptr;
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        if (pRenderFactory != nullptr) {
            pFontMgr = pRenderFactory->GetFontMgr();
        }
        ASSERT(pFontMgr != nullptr);
        if (pFontMgr == nullptr) {
            return nullptr;
        }

        IFont* pRetFallbackFont = nullptr;
        for (const DString& fallbackFontName : fallbackFontFamilyNames) {
            if (fallbackFontName.empty()) {
                continue;
            }
            if (!pFontMgr->HasFontName(fallbackFontName)) {
                //不存在的字体
                continue;
            }
            IFont* pFallbackFont = pRenderFactory->CreateIFont();
            ASSERT(pFallbackFont != nullptr);
            if (pFallbackFont == nullptr) {
                continue;
            }
            // RAII 包装：异常路径下也能正确释放
            std::unique_ptr<IFont> pFallbackFontHolder(pFallbackFont);
            UiFont fontInfo;
            fontInfo.m_fontSize = pFont->FontSize();
            fontInfo.m_bUnderline = pFont->IsUnderline();
            fontInfo.m_bStrikeOut = pFont->IsStrikeOut();
            fontInfo.m_bItalic = pFont->IsItalic();
            fontInfo.m_bBold = pFont->IsBold();

            fontInfo.m_fontName = fallbackFontName; //设置回退的字体名称
            bool isInitOk = pFallbackFont->InitFont(fontInfo);
            ASSERT(isInitOk);
            if (unicodeChar != 0) {
                if (!pFallbackFont->IsUnicodeCharSupported(unicodeChar, glyphId)) {
                    //该字体不支持这个字符
                    isInitOk = false;
                }
            }
            if (!isInitOk) {
                // RAII 自动释放 pFallbackFont
                continue;
            }

            //找到第一个匹配的，停止
            //转移所有权到 pRetFallbackFont，RAII 释放器置空
            pRetFallbackFont = pFallbackFontHolder.release();
            break;
        }
        if (pRetFallbackFont != nullptr) {
            //放入缓存列表，管理其生命周期
            fallbackFontList.push_back(pRetFallbackFont);
        }
        return pRetFallbackFont;
    }

private:
    //字体管理器接口
    FontManager* m_pFontManager;
};

FontManager::FontManager():
    m_bDefaultFontInited(false),
    m_bFallbackFontInited(false)
{
    m_pFallbackFontMgr = std::make_unique<FallbackFontMgrImpl>(this);
}

FontManager::~FontManager()
{
    RemoveAllFonts();
    RemoveAllFontFiles();
    m_pFallbackFontMgr.reset();
}

bool FontManager::AddFont(const DString& fontId, const UiFont& fontInfo, bool bDefault)
{
    ASSERT(!fontId.empty());
    if (fontId.empty()) {
        return false;
    }

    ASSERT(fontInfo.m_fontSize > 0);
    if (fontInfo.m_fontSize <= 0) {
        return false;
    }
#ifdef _DEBUG
    if (!GlobalManager::Instance().Theme().IsSwitchingTheme()) {
        //在切换主题的时候，允许覆盖，其他情况下覆盖时断言
        auto iter = m_fontIdMap.find(fontId);
        if (iter != m_fontIdMap.end()) {
            ASSERT(iter->second == fontInfo);
        }
    }
#endif

    //保存字体信息，但不创建字体数据
    m_fontIdMap[fontId] = fontInfo;
    if (bDefault) {
        //默认字体ID
        m_defaultFontId = fontId;
    }
    return true;
}

const DString& FontManager::GetDefaultFontId() const
{
    return m_defaultFontId;
}

void FontManager::SetDefaultFontFamilyNames(const DString& defaultFontFamilyNames)
{
    m_defaultFontFamilyNames.clear();
    m_bDefaultFontInited = false;
    if (!defaultFontFamilyNames.empty()) {
        std::list<DString> fontFamilyNames = StringUtil::Split(defaultFontFamilyNames, _T(","));
        for (DString fontFamilyName : fontFamilyNames) {
            StringUtil::Trim(fontFamilyName);
            if (!fontFamilyName.empty()) {
                m_defaultFontFamilyNames.push_back(fontFamilyName);
            }
        }
    }
    //清理字体缓存数据
    ClearFontCache();
}

void FontManager::SetFallbackFontFamilyNames(const DString& fallbackFontFamilyNames)
{
    m_fallbackFontFamilyNames.clear();
    m_bFallbackFontInited = false;
    if (!fallbackFontFamilyNames.empty()) {
        std::list<DString> fontFamilyNames = StringUtil::Split(fallbackFontFamilyNames, _T(","));
        for (DString fontFamilyName : fontFamilyNames) {
            StringUtil::Trim(fontFamilyName);
            if (!fontFamilyName.empty()) {
                m_fallbackFontFamilyNames.push_back(fontFamilyName);
            }
        }
    }
    //清理字体缓存数据
    ClearFontCache();
}

void FontManager::InitDefaultFont()
{
    if (m_bDefaultFontInited && m_bFallbackFontInited) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }
    IFontMgr* pFontMgr = pRenderFactory->GetFontMgr();
    if (pFontMgr == nullptr) {
        return;
    }
    if (!m_bDefaultFontInited && !m_defaultFontFamilyNames.empty()) {
        auto pos = m_defaultFontFamilyNames.begin();
        while (pos != m_defaultFontFamilyNames.end()) {
            const DString& fontFamilyName = *pos;
            if (!pFontMgr->HasFontName(fontFamilyName)) {
                //移除不存在的字体
                pos = m_defaultFontFamilyNames.erase(pos);
            }
            else {
                break;
            }
        }
        m_bDefaultFontInited = true;
        if (!m_defaultFontFamilyNames.empty()) {
            pFontMgr->SetDefaultFontName(m_defaultFontFamilyNames.front());
        }
    }

    if (!m_bFallbackFontInited && !m_fallbackFontFamilyNames.empty()) {
        //所有字体均需要校验一次
        auto pos = m_fallbackFontFamilyNames.begin();
        while (pos != m_fallbackFontFamilyNames.end()) {
            const DString& fontFamilyName = *pos;
            if (!pFontMgr->HasFontName(fontFamilyName)) {
                //移除不存在的字体
                pos = m_fallbackFontFamilyNames.erase(pos);
            }
            else {
                ++pos;
            }
        }
        m_bFallbackFontInited = true;
    }
}

/** 获取默认的字体名称
*/
static DString GetDefaultFontName()
{
#ifdef DUILIB_BUILD_FOR_WIN
    return _T("Microsoft YaHei");
#elif defined DUILIB_BUILD_FOR_MACOS
    return _T("PingFang SC");
#elif defined DUILIB_BUILD_FOR_LINUX
    return _T("Noto Sans CJK SC");
#elif defined DUILIB_BUILD_FOR_FREEBSD
    return _T("Noto Sans CJK SC");
#else
    return _T("RobotoMono");
#endif
}

DString FontManager::GetDpiFontId(const DString& fontId, uint32_t nZoomPercent) const
{
    DString dpiFontId;
    if (!fontId.empty()) {
        dpiFontId = fontId + _T("@") + StringUtil::UInt32ToString(nZoomPercent);
    }
    return dpiFontId;
}

IFont* FontManager::GetIFont(const DString& fontId, const DpiManager& dpi)
{
    return GetIFont(fontId, dpi.GetDisplayScaleFactor());
}

IFont* FontManager::GetIFont(const DString& fontId, uint32_t nZoomPercent)
{
    ASSERT(nZoomPercent != 0);
    if (nZoomPercent == 0) {
        nZoomPercent = 100;
    }
    //先在缓存中查找
    IFont* pFont = nullptr;
    if (!fontId.empty()) {        
        DString dpiFontId = GetDpiFontId(fontId, nZoomPercent);
        auto iter = m_fontMap.find(dpiFontId);
        if (iter != m_fontMap.end()) {
            pFont = iter->second;
        }
    }
    if (pFont == nullptr) {
        auto iter = m_fontIdMap.find(fontId);
#ifdef _DEBUG
        //如果字体ID不存在，则给出断言，便于在开发阶段发现配置错误
        ASSERT(fontId.empty() || (iter != m_fontIdMap.end()));
#endif
        if ((iter == m_fontIdMap.end()) && !m_defaultFontId.empty()) {
            //没有这个字体ID，使用默认的字体ID
            DString dpiFontId = GetDpiFontId(m_defaultFontId, nZoomPercent);
            auto pos = m_fontMap.find(dpiFontId);
            if (pos != m_fontMap.end()) {
                pFont = pos->second;
            }
        }
    }
    if (pFont != nullptr) {
        //使用缓存中已经创建好的字体数据
        return pFont;
    }

    //缓存中不存在，需要创建字体
    UiFont fontInfo;
    DString realFontId = fontId;
    auto iter = m_fontIdMap.find(realFontId);
    if (iter == m_fontIdMap.end()) {
        realFontId = m_defaultFontId;
        iter = m_fontIdMap.find(realFontId);
        if (iter != m_fontIdMap.end()) {
            fontInfo = iter->second;
        }
        else {
            realFontId.clear();
        }
    }
    else {
        fontInfo = iter->second;
    }
    ASSERT(!realFontId.empty());
    if (realFontId.empty()) {
        //无此字体ID
        return nullptr;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return nullptr;
    }

    //初始化默认字体名称
    InitDefaultFont();

    DString dpiFontId = GetDpiFontId(realFontId, nZoomPercent);
    if (fontInfo.m_fontName.empty() || 
        StringUtil::IsEqualNoCase(fontInfo.m_fontName.c_str(), _T("system"))) {
        if (!m_defaultFontFamilyNames.empty()) {
            fontInfo.m_fontName = m_defaultFontFamilyNames.front();
        }
        else {
            //保底设置(如果设置了默认字体，走不到这里)
            fontInfo.m_fontName = GetDefaultFontName();
        }
    }

    //对字体大小进行DPI缩放
    ASSERT(fontInfo.m_fontSize > 0);
    if (nZoomPercent != 100) {
        fontInfo.m_fontSize = DpiManager::MulDiv(fontInfo.m_fontSize, (int32_t)nZoomPercent, 100);
        if (fontInfo.m_fontSize < 1) {
            fontInfo.m_fontSize = 1;
        }
    }

    pFont = pRenderFactory->CreateIFont();
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return nullptr;
    }
    bool isInitOk = pFont->InitFont(fontInfo);
    ASSERT(isInitOk);
    if (!isInitOk) {
        delete pFont;
        pFont = nullptr;
        return nullptr;
    }
    m_fontMap.insert(std::make_pair(dpiFontId, pFont));
    return pFont;
}

bool FontManager::HasFontId(const DString& fontId) const
{
    auto pos = m_fontIdMap.find(fontId);
    bool bFound = pos != m_fontIdMap.end();
    return bFound;
}

bool FontManager::RemoveFontId(const DString& fontId)
{
    ASSERT(fontId != m_defaultFontId);
    if (fontId == m_defaultFontId) {
        return false;
    }
    bool bDeleted = false;
    const DString zoomFontId = fontId + _T("@");
    auto iter = m_fontMap.begin();
    while (iter != m_fontMap.end()) {
        if (iter->first.find(zoomFontId) == 0) {
            //匹配到字体ID
            if (iter->second != nullptr) {
                OnIFontDataRemoved(iter->second);
                delete iter->second;//IFont指针
            }
            bDeleted = true;
            iter = m_fontMap.erase(iter);
        }
        else {
            ++iter;
        }
    }
    auto pos = m_fontIdMap.find(fontId);
    if (pos != m_fontIdMap.end()) {
        m_fontIdMap.erase(pos);
        bDeleted = true;
    }
    return bDeleted;
}

bool FontManager::RemoveIFont(const DString& fontId, uint32_t nZoomPercent)
{
    bool bDeleted = false;
    if (!fontId.empty()) {
        DString realFontId = GetDpiFontId(fontId, nZoomPercent);
        auto iter = m_fontMap.find(realFontId);
        if (iter != m_fontMap.end()) {
            //匹配到字体ID
            if (iter->second != nullptr) {
                OnIFontDataRemoved(iter->second);
                delete iter->second;//IFont指针
            }
            bDeleted = true;
            m_fontMap.erase(iter);
        }
    }
    return bDeleted;
}

void FontManager::RemoveAllFonts()
{
    ClearFontCache();

    m_defaultFontId.clear();
    m_fontIdMap.clear();    
}

void FontManager::ClearFontCache()
{
    //清理字体ID对应的字体缓存数据
    for (auto fontInfo : m_fontMap) {
        IFont* pFont = fontInfo.second;
        if (pFont != nullptr) {
            delete pFont;
        }
    }
    m_fontMap.clear();

    //清理字体回退缓存
    for (auto fallbackFont : m_fallbackFontMap) {
        const std::vector<IFont*>& fontList = fallbackFont.second;
        for (IFont* pFont : fontList) {
            if (pFont != nullptr) {
                delete pFont;
            }
        }
    }    
    m_fallbackFontMap.clear();

    //清理字体管理器内部的字体缓存
    IFontMgr* pFontMgr = nullptr;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        pFontMgr = pRenderFactory->GetFontMgr();
    }
    if (pFontMgr != nullptr) {
        pFontMgr->ClearFontCache();
    }
}

bool FontManager::AddFontFile(const DString& strFontFile, const DString& /*strFontDesc*/)
{
    IFontMgr* pFontMgr = nullptr;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        pFontMgr = pRenderFactory->GetFontMgr();
    }
    ASSERT(pFontMgr != nullptr);
    if (pFontMgr == nullptr) {
        return false;
    }

    std::vector<FilePath> searchDirList;
    searchDirList.push_back(GlobalManager::Instance().GetFontFilePath());
    searchDirList.push_back(GlobalManager::Instance().GetResourceRootPath());
    bool bRet = false;
    for (const FilePath& searchDir : searchDirList) {
        FilePath fontFilePath = searchDir;
        fontFilePath /= FilePath(strFontFile);
        if (GlobalManager::Instance().Zip().IsUseZip()) {
            std::vector<unsigned char> file_data;
            if (GlobalManager::Instance().Zip().GetZipData(fontFilePath, file_data)) {
                //从内存流加载
                bRet = pFontMgr->LoadFontFileData(file_data.data(), file_data.size());
            }
        }
        else {
            //从文件加载
            bRet = pFontMgr->LoadFontFile(fontFilePath.ToString());
        }
        if (bRet) {
            break;
        }
    }
    ASSERT(bRet);
    return bRet;
}

void FontManager::RemoveAllFontFiles()
{
    IFontMgr* pFontMgr = nullptr;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        pFontMgr = pRenderFactory->GetFontMgr();
    }
    if (pFontMgr != nullptr) {
        pFontMgr->ClearFontFiles();
    }
}

void FontManager::GetFontNameList(std::vector<DString>& fontNameList) const
{
    fontNameList.clear();
    IFontMgr* pFontMgr = nullptr;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        pFontMgr = pRenderFactory->GetFontMgr();
    }
    ASSERT(pFontMgr != nullptr);
    if (pFontMgr == nullptr) {
        return;
    }
    DString fontName;
    uint32_t nFontCount = pFontMgr->GetFontCount();
    for (uint32_t nIndex = 0; nIndex < nFontCount; ++nIndex) {
        if (pFontMgr->GetFontName(nIndex, fontName)) {
            fontNameList.push_back(fontName);
        }
    }
    if (!fontNameList.empty()) {
        std::sort(fontNameList.begin(), fontNameList.end());
    }
}

void FontManager::SetFontSizeList(const std::vector<FontSizeInfo>& fontSizeList)
{
    m_fontSizeList = fontSizeList;
}

void FontManager::GetFontSizeList(std::vector<FontSizeInfo>& fontSizeList) const
{
    fontSizeList = m_fontSizeList; //默认以外部设置的字体大小列表为最高优先级
    if (fontSizeList.empty()) {
        fontSizeList.push_back({ _T("8"),  8.0f, 0 });
        fontSizeList.push_back({ _T("9"),  9.0f, 0 });
        fontSizeList.push_back({ _T("10"), 10.0f, 0 });
        fontSizeList.push_back({ _T("11"), 11.0f, 0 });
        fontSizeList.push_back({ _T("12"), 12.0f, 0 });
        fontSizeList.push_back({ _T("14"), 14.0f, 0 });
        fontSizeList.push_back({ _T("16"), 16.0f, 0 });
        fontSizeList.push_back({ _T("18"), 18.0f, 0 });
        fontSizeList.push_back({ _T("20"), 20.0f, 0 });
        fontSizeList.push_back({ _T("22"), 22.0f, 0 });
        fontSizeList.push_back({ _T("24"), 24.0f, 0 });
        fontSizeList.push_back({ _T("26"), 26.0f, 0 });
        fontSizeList.push_back({ _T("28"), 28.0f, 0 });
        fontSizeList.push_back({ _T("32"), 32.0f, 0 });
        fontSizeList.push_back({ _T("36"), 36.0f, 0 });
        fontSizeList.push_back({ _T("48"), 48.0f, 0 });
        fontSizeList.push_back({ _T("72"), 72.0f, 0 });

        //获取中文的字体大小列表
        //1英寸,95.6;大特号,83.7;特号,71.7;初号,56.0;小初,48.0;一号,34.7;小一,32.0;二号,29.3;小二,24.0;三号,21.3;小三,20.0;四号,18.7;小四,16.0;五号,14.0;小五,12.0;六号,10.0;小六,8.7;七号,7.3;八号,6.7
        DString fontSizeListString = GlobalManager::GetTextById(_T("STRID_PUBLIC_FONT_SIZE_LIST"));
        if (!fontSizeListString.empty()) {
            std::list<DString> fontSizePairList = StringUtil::Split(fontSizeListString, _T(";"));
            for (const DString& fontSizePair : fontSizePairList) {
                std::list<DString> fontSizeName = StringUtil::Split(fontSizePair, _T(","));
                if (fontSizeName.size() != 2) {
                    continue;
                }
                DString name = *fontSizeName.begin();
                DString value = *fontSizeName.rbegin();
                StringUtil::Trim(name);
                StringUtil::Trim(value);
                if (!name.empty() && !value.empty()) {
                    float fValue = StringUtil::StringToFloat(value.c_str(), nullptr);
                    fontSizeList.push_back({ name, fValue, 0 });
                }
            }
        }
    }    
}

void FontManager::GetDpiFontSizeList(const DpiManager& dpi, std::vector<FontSizeInfo>& fontSizeList) const
{
    //获取默认的字体列表，未DPI缩放的值
    GetFontSizeList(fontSizeList);

    //更新DPI自适应值
    DpiScaleFontSizeList(fontSizeList, dpi);
}

void FontManager::DpiScaleFontSizeList(std::vector<FontSizeInfo>& fontSizeList, const DpiManager& dpi) const
{
    //更新DPI自适应值
    for (FontSizeInfo& fontSize : fontSizeList) {
        int32_t nSize = static_cast<int32_t>(fontSize.fFontSize * 1000);
        dpi.ScaleInt(nSize);
        fontSize.fDpiFontSize = nSize / 1000.0f;
    }
}

IFallbackFontMgr* FontManager::GetFallbackFontMgr() const
{
    return m_pFallbackFontMgr.get();
}

void FontManager::OnIFontDataRemoved(IFont* pIFont)
{
    auto iter = m_fallbackFontMap.find(pIFont);
    if (iter != m_fallbackFontMap.end()) {
        std::vector<IFont*>& fontList = iter->second;
        for (IFont* pFont : fontList) {
            if (pFont != nullptr) {
                delete pFont;
            }
        }
        fontList.clear();
    }
}

}// namespace ui
