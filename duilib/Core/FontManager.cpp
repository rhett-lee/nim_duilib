#include "FontManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/StringUtil.h"
#include <VersionHelpers.h>

namespace ui 
{

FontManager::FontManager()
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

DString FontManager::GetDpiFontId(const DString& fontId, const DpiManager& dpi) const
{
    DString dpiFontId;
    if (!fontId.empty()) {
        dpiFontId = fontId + _T("@") + StringUtil::UInt32ToString(dpi.GetScale());
    }
    return dpiFontId;
}

IFont* FontManager::GetIFont(const DString& fontId, const DpiManager& dpi)
{
    //先在缓存中查找
    IFont* pFont = nullptr;
    if (!fontId.empty()) {        
        DString dpiFontId = GetDpiFontId(fontId, dpi);
        auto iter = m_fontMap.find(dpiFontId);
        if (iter != m_fontMap.end()) {
            pFont = iter->second;
        }
    }
    if (pFont == nullptr) {
        auto iter = m_fontIdMap.find(fontId);
        if ((iter == m_fontIdMap.end()) && !m_defaultFontId.empty()) {
            //没有这个字体ID，使用默认的字体ID
            DString dpiFontId = GetDpiFontId(m_defaultFontId, dpi);
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
    DString dpiFontId = GetDpiFontId(realFontId, dpi);    
    if (fontInfo.m_fontName.empty() || 
        StringUtil::IsEqualNoCase(fontInfo.m_fontName, _T("system"))) {
        //字体使用英文名称，保持兼容性
        static bool bOsOverXp = IsWindowsVistaOrGreater();
        fontInfo.m_fontName = bOsOverXp ? _T("Microsoft YaHei") : _T("SimSun");
    }

    //对字体大小进行DPI缩放
    ASSERT(fontInfo.m_fontSize > 0);
    dpi.ScaleInt(fontInfo.m_fontSize);
    ASSERT(fontInfo.m_fontSize > 0);

    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        pFont = pRenderFactory->CreateIFont();
    }
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
}

void FontManager::AddFontFile(const DString& strFontFile, const DString& strFontName)
{
    FontFileInfo fontFileInfo;
    DString path = GlobalManager::Instance().GetResFullPath(_T(""), _T("font\\") + strFontFile);
    bool res = false;
    if (GlobalManager::Instance().Zip().IsUseZip()) {
        std::vector<unsigned char> file_data;
        if (GlobalManager::Instance().Zip().GetZipData(path, file_data)) {
            DWORD dwFonts = 0;
            fontFileInfo.m_hFontFile = ::AddFontMemResourceEx(file_data.data(), static_cast<DWORD>(file_data.size()), NULL, &dwFonts);
            res = fontFileInfo.m_hFontFile != nullptr;
        }
    }
    else {
        res = ::AddFontResourceEx(path.c_str(), FR_PRIVATE, 0) != 0;
    }
    ASSERT(res);
    if (res) {
        fontFileInfo.m_fontFilePath = path;
        fontFileInfo.m_fontName = strFontName;
        m_fontFileInfo.push_back(fontFileInfo);
    }
}

void FontManager::RemoveAllFontFiles()
{
    for (const auto& it : m_fontFileInfo) {
        if (it.m_hFontFile != nullptr) {
            ::RemoveFontMemResourceEx(it.m_hFontFile);
        }
        else if(!it.m_fontFilePath.empty()){
            ::RemoveFontResourceEx(it.m_fontFilePath.c_str(), FR_PRIVATE, 0);
        }
    }
    m_fontFileInfo.clear();
}

//字体的中英文名称映射表
static void GetFontNameList(std::vector<std::pair<DString, DString>>& fontNameList)
{
    //系统字体
    fontNameList.push_back(std::make_pair<DString, DString>(_T("宋体"), _T("SimSun")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("黑体"), _T("SimHei")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("微软雅黑"), _T("Microsoft YaHei")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("微软正黑体"), _T("Microsoft JhengHei")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("楷体"), _T("KaiTi")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("新宋体"), _T("NSimSun")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("仿宋"), _T("FangSong")));
    
    //Office字体
    fontNameList.push_back(std::make_pair<DString, DString>(_T("幼圆"), _T("YouYuan")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("隶书"), _T("LiSu")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文细黑"), _T("STXiHei")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文楷体"), _T("STKaiTi")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文宋体"), _T("STSong")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文仿宋"), _T("STFangSong")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文中宋"), _T("STZhongSong")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文彩云"), _T("STCaiYun")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文琥珀"), _T("STHuPo")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文新魏"), _T("STXinWei")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文隶书"), _T("STLiTi")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("华文行楷"), _T("STXingKai")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("方正舒体"), _T("FZShuTi")));
    fontNameList.push_back(std::make_pair<DString, DString>(_T("方正姚体"), _T("FZYaoTi")));
}

DString FontManager::GetFontEnglishName(const DString& fontName)
{
    std::vector<std::pair<DString, DString>> fontNameList;
    GetFontNameList(fontNameList);
    for (const auto& pair : fontNameList) {
        if (fontName == pair.first) {
            return pair.second;
        }
    }
    return fontName;
}

DString FontManager::GetFontSystemName(const DString& fontName)
{
    std::vector<std::pair<DString, DString>> fontNameList;
    GetFontNameList(fontNameList);
    for (const auto& pair : fontNameList) {
        if (ui::StringUtil::IsEqualNoCase(fontName, pair.second)) {
            return pair.first;
        }
    }
    return fontName;
}

}