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

bool FontManager::AddFont(const std::wstring& fontId, const UiFont& fontInfo, bool bDefault)
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

std::wstring FontManager::GetDpiFontId(const std::wstring& fontId, const DpiManager& dpi) const
{
    std::wstring dpiFontId;
    if (!fontId.empty()) {
        dpiFontId = fontId + L"@" + StringHelper::UInt32ToString(dpi.GetScale());
    }
    return dpiFontId;
}

IFont* FontManager::GetIFont(const std::wstring& fontId, const DpiManager& dpi)
{
    //先在缓存中查找
    IFont* pFont = nullptr;
    if (!fontId.empty()) {        
        std::wstring dpiFontId = GetDpiFontId(fontId, dpi);
        auto iter = m_fontMap.find(dpiFontId);
        if (iter != m_fontMap.end()) {
            pFont = iter->second;
        }
    }
    if (pFont == nullptr) {
        auto iter = m_fontIdMap.find(fontId);
        if ((iter == m_fontIdMap.end()) && !m_defaultFontId.empty()) {
            //没有这个字体ID，使用默认的字体ID
            std::wstring dpiFontId = GetDpiFontId(m_defaultFontId, dpi);
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
    std::wstring realFontId = fontId;
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
    std::wstring dpiFontId = GetDpiFontId(realFontId, dpi);    
    if (fontInfo.m_fontName.empty() || 
        StringHelper::IsEqualNoCase(fontInfo.m_fontName, L"system")) {
        //字体使用英文名称，保持兼容性
        static bool bOsOverXp = IsWindowsVistaOrGreater();
        fontInfo.m_fontName = bOsOverXp ? L"Microsoft YaHei" : L"SimSun";
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

void FontManager::AddFontFile(const std::wstring& strFontFile, const std::wstring& strFontName)
{
    FontFileInfo fontFileInfo;
    std::wstring path = GlobalManager::Instance().GetResFullPath(L"", L"font\\" + strFontFile);
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
static void GetFontNameList(std::vector<std::pair<std::wstring, std::wstring>>& fontNameList)
{
    //系统字体
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"宋体", L"SimSun"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"黑体", L"SimHei"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"微软雅黑", L"Microsoft YaHei"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"微软正黑体", L"Microsoft JhengHei"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"楷体", L"KaiTi"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"新宋体", L"NSimSun"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"仿宋", L"FangSong"));
    
    //Office字体
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"幼圆", L"YouYuan"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"隶书", L"LiSu"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文细黑", L"STXiHei"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文楷体", L"STKaiTi"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文宋体", L"STSong"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文仿宋", L"STFangSong"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文中宋", L"STZhongSong"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文彩云", L"STCaiYun"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文琥珀", L"STHuPo"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文新魏", L"STXinWei"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文隶书", L"STLiTi"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"华文行楷", L"STXingKai"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"方正舒体", L"FZShuTi"));
    fontNameList.push_back(std::make_pair<std::wstring, std::wstring>(L"方正姚体", L"FZYaoTi"));
}

std::wstring FontManager::GetFontEnglishName(const std::wstring& fontName)
{
    std::vector<std::pair<std::wstring, std::wstring>> fontNameList;
    GetFontNameList(fontNameList);
    for (const auto& pair : fontNameList) {
        if (fontName == pair.first) {
            return pair.second;
        }
    }
    return fontName;
}

std::wstring FontManager::GetFontSystemName(const std::wstring& fontName)
{
    std::vector<std::pair<std::wstring, std::wstring>> fontNameList;
    GetFontNameList(fontNameList);
    for (const auto& pair : fontNameList) {
        if (ui::StringHelper::IsEqualNoCase(fontName, pair.second)) {
            return pair.first;
        }
    }
    return fontName;
}

}