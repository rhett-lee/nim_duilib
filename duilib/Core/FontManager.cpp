#include "FontManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui 
{

FontManager::FontManager():
    m_bDefaultFontInited(false)
{
}

FontManager::~FontManager()
{
    RemoveAllFonts();
    RemoveAllFontFiles();
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

    auto iter = m_fontIdMap.find(fontId);
    ASSERT(iter == m_fontIdMap.end());
    if (iter != m_fontIdMap.end()) {
        //避免相同的字体ID重复添加
        return false;
    }

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
    return GetIFont(fontId, dpi.GetScale());
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
    IFontMgr* pFontMgr = pRenderFactory->GetFontMgr();
    if (!m_bDefaultFontInited && !m_defaultFontFamilyNames.empty() && (pFontMgr != nullptr)) {
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

    DString dpiFontId = GetDpiFontId(realFontId, nZoomPercent);
    if (fontInfo.m_fontName.empty() || 
        StringUtil::IsEqualNoCase(fontInfo.m_fontName.c_str(), _T("system"))) {
        if (!m_defaultFontFamilyNames.empty()) {
            fontInfo.m_fontName = m_defaultFontFamilyNames.front();
        }
        else {
            fontInfo.m_fontName = _T("Microsoft YaHei"); //微软雅黑，如果设置了默认字体，走不到这里
        }
    }

    //对字体大小进行DPI缩放
    ASSERT(fontInfo.m_fontSize > 0);
    if (nZoomPercent != 100) {
        fontInfo.m_fontSize = DpiManager::MulDiv(fontInfo.m_fontSize, nZoomPercent, 100);
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
    for (auto fontInfo : m_fontMap) {
        IFont* pFont = fontInfo.second;
        if (pFont != nullptr) {
            delete pFont;
        }
    }
    m_fontMap.clear();
    m_defaultFontId.clear();
    m_fontIdMap.clear();

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
    FilePath fontFilePath = FilePathUtil::JoinFilePath(GlobalManager::Instance().GetFontFilePath(), FilePath(strFontFile));
    IFontMgr* pFontMgr = nullptr;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        pFontMgr = pRenderFactory->GetFontMgr();
    }
    ASSERT(pFontMgr != nullptr);
    if (pFontMgr == nullptr) {
        return false;
    }

    bool bRet = false;
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

void FontManager::GetFontSizeList(const DpiManager& dpi, std::vector<FontSizeInfo>& fontSizeList) const
{
    fontSizeList.clear();
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

#ifdef DUILIB_BUILD_FOR_WIN
    if (::GetACP() == 936) {
        //仅中文环境使用
        fontSizeList.push_back({ _T("1英寸"), 95.6f, 0 });
        fontSizeList.push_back({ _T("大特号"), 83.7f, 0 });
        fontSizeList.push_back({ _T("特号"), 71.7f, 0 });
        fontSizeList.push_back({ _T("初号"), 56.0f, 0 });
        fontSizeList.push_back({ _T("小初"), 48.0f, 0 });
        fontSizeList.push_back({ _T("一号"), 34.7f, 0 });
        fontSizeList.push_back({ _T("小一"), 32.0f, 0 });
        fontSizeList.push_back({ _T("二号"), 29.3f, 0 });
        fontSizeList.push_back({ _T("小二"), 24.0f, 0 });
        fontSizeList.push_back({ _T("三号"), 21.3f, 0 });
        fontSizeList.push_back({ _T("小三"), 20.0f, 0 });
        fontSizeList.push_back({ _T("四号"), 18.7f, 0 });
        fontSizeList.push_back({ _T("小四"), 16.0f, 0 });
        fontSizeList.push_back({ _T("五号"), 14.0f, 0 });
        fontSizeList.push_back({ _T("小五"), 12.0f, 0 });
        fontSizeList.push_back({ _T("六号"), 10.0f, 0 });
        fontSizeList.push_back({ _T("小六"), 8.7f, 0 });
        fontSizeList.push_back({ _T("七号"), 7.3f, 0 });
        fontSizeList.push_back({ _T("八号"), 6.7f, 0 });
    }
#endif

    //更新DPI自适应值
    for (FontSizeInfo& fontSize : fontSizeList) {
        int32_t nSize = static_cast<int32_t>(fontSize.fFontSize * 1000);
        dpi.ScaleInt(nSize);
        fontSize.fDpiFontSize = nSize / 1000.0f;
    }
}

}
