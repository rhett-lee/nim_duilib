#include "ThemeManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/PerformanceUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/StringUtil.h"
#include <unordered_set>

namespace ui 
{
ThemeManager::ThemeManager():
    m_themeChangedCallback(nullptr),
    m_bSwitchingTheme(false)
{
    m_defaultThemeInfo.m_bDefaultTheme = true;
    m_defaultThemeInfo.m_bSelectedTheme = false;
    m_colorThemeInfo.m_bDefaultTheme = false;
    m_colorThemeInfo.m_bSelectedTheme = false;
    m_iconThemeInfo.m_bDefaultTheme = false;
    m_iconThemeInfo.m_bSelectedTheme = false;
}

ThemeType ThemeManager::GetThemeTypeValue(DString themeType) const
{
    themeType = StringUtil::MakeLowerString(themeType);
    if (themeType == _T("color")) {
        return ThemeType::kColor;
    }
    else if (themeType == _T("icon")) {
        return ThemeType::kIcon;
    }
    else if (themeType == _T("combined")) {
        return ThemeType::kCombined;
    }
    else {
        return ThemeType::kUnknown;
    }
}

ThemeStyle ThemeManager::GetThemeStyleValue(DString themeStyle) const
{
    themeStyle = StringUtil::MakeLowerString(themeStyle);
    if (themeStyle == _T("light")) {
        return ThemeStyle::kLight;
    }
    else if (themeStyle == _T("dark")) {
        return ThemeStyle::kDark;
    }
    else {
        return ThemeStyle::kUnknown;
    }
}

bool ThemeManager::InitTheme(const FilePath& themeRootFullPath,
                             const FilePath& defaultThemePath,
                             const DString& globalXmlFileName)
{
    ASSERT(!themeRootFullPath.IsEmpty());
    if (themeRootFullPath.IsEmpty()) {
        return false;
    }
    ASSERT(!defaultThemePath.IsEmpty());
    if (defaultThemePath.IsEmpty()) {
        return false;
    }
    ASSERT(!globalXmlFileName.empty());
    if (globalXmlFileName.empty()) {
        return false;
    }

    m_themeRootPath = themeRootFullPath;
    m_defaultThemePath = defaultThemePath;
    m_globalXmlFileName = globalXmlFileName;

    FilePath globalXmlFileNameFullPath = themeRootFullPath;    //主题根目录：resources/themes
    globalXmlFileNameFullPath /= defaultThemePath;             //默认主题目录：default
    globalXmlFileNameFullPath /= FilePath(globalXmlFileName);  //配置文件：global.xml
    globalXmlFileNameFullPath.NormalizeFilePath();             //默认取值示例：resources/themes/default/global.xml

    //解析全局资源信息(默认是"global.xml"文件)
    PerformanceStat statPerformance(_T("ParseXml, ThemeManager::InitTheme load global.xml"));
    WindowBuilder globalbuilder;
    Window tempWnd;
    if (globalbuilder.ParseXmlFile(FilePath(globalXmlFileName))) {
        //初始化默认主题数据
        DString themeName;
        DString themeType;
        DString themeStyle;
        globalbuilder.ParseThemeInfo(themeName, themeType, themeStyle);

        m_defaultThemeInfo.m_bDefaultTheme = true;
        m_defaultThemeInfo.m_bSelectedTheme = true;
        m_defaultThemeInfo.m_themePath = defaultThemePath;
        m_defaultThemeInfo.m_themeName = themeName;
        m_defaultThemeInfo.m_themeStyle = GetThemeStyleValue(themeStyle);
        m_defaultThemeInfo.m_themeType = GetThemeTypeValue(themeType);

        globalbuilder.CreateControls(&tempWnd);
        return true;
    }
    return false;
}

bool ThemeManager::SwitchColorTheme(const FilePath& themePath)
{
    return SwitchTheme(themePath, ThemeType::kColor);
}

bool ThemeManager::SwitchIconTheme(const FilePath& themePath)
{
    return SwitchTheme(themePath, ThemeType::kIcon);
}

bool ThemeManager::SwitchTheme(const FilePath& themePath, ThemeType destThemeType)
{
    ASSERT(!themePath.IsEmpty());
    if (themePath.IsEmpty()) {
        return false;
    }
    if ((m_defaultThemePath == themePath)) {
        //默认主题
        destThemeType = ThemeType::kCombined;
    }
    else {
        //颜色主题或者图标主题
        ASSERT((destThemeType == ThemeType::kColor) || (destThemeType == ThemeType::kIcon));
        if ((destThemeType != ThemeType::kColor) && (destThemeType != ThemeType::kIcon)) {
            return false;
        }
    }

    FilePath globalXmlFileName = FilePath(GetGlobalXmlFileName());
    ASSERT(!globalXmlFileName.IsEmpty());
    if (globalXmlFileName.IsEmpty()) {
        return false;
    }

    FilePath themeFullPath = GetThemeRootPath();
    themeFullPath /= themePath;
    themeFullPath /= globalXmlFileName;
    WindowBuilder globalbuilder;
    Window tempWnd;
    if (!globalbuilder.ParseXmlFile(FilePath(themeFullPath))) {
        return false;
    }

    //初始化主题数据
    DString themeName;
    DString themeType;
    DString themeStyle;
    globalbuilder.ParseThemeInfo(themeName, themeType, themeStyle);
    ASSERT(!themeName.empty() && !themeType.empty() && !themeStyle.empty());
    if (themeName.empty() || themeType.empty() || themeStyle.empty()) {
        return false;
    }

    ThemeInfo themeInfo;
    if (destThemeType == ThemeType::kColor) {
        //颜色主题
        ThemeType readThemeType = GetThemeTypeValue(themeType);
        ASSERT((readThemeType == ThemeType::kColor) || (readThemeType == ThemeType::kCombined));
        if ((readThemeType != ThemeType::kColor) &&
            (readThemeType != ThemeType::kCombined)) {
            return false;
        }
        m_colorThemeInfo.m_bDefaultTheme = false;
        m_colorThemeInfo.m_bSelectedTheme = true;
        m_colorThemeInfo.m_themePath = themePath;
        m_colorThemeInfo.m_themeName = themeName;
        m_colorThemeInfo.m_themeStyle = GetThemeStyleValue(themeStyle);
        m_colorThemeInfo.m_themeType = readThemeType;

        if (readThemeType == ThemeType::kCombined) {
            //覆盖图标主题
            m_iconThemeInfo.m_bSelectedTheme = false;            
        }
        m_defaultThemeInfo.m_bSelectedTheme = false;
        themeInfo = m_colorThemeInfo;
    }
    else if (destThemeType == ThemeType::kIcon) {
        //图标主题
        ThemeType readThemeType = GetThemeTypeValue(themeType);
        m_iconThemeInfo.m_bDefaultTheme = false;
        m_iconThemeInfo.m_bSelectedTheme = true;
        m_iconThemeInfo.m_themePath = themePath;
        m_iconThemeInfo.m_themeName = themeName;
        m_iconThemeInfo.m_themeStyle = GetThemeStyleValue(themeStyle);
        m_iconThemeInfo.m_themeType = readThemeType;

        if (readThemeType == ThemeType::kCombined) {
            //覆盖颜色主题
            m_colorThemeInfo.m_bSelectedTheme = false;            
        }
        m_defaultThemeInfo.m_bSelectedTheme = false;
        themeInfo = m_iconThemeInfo;
    }
    else if (destThemeType == ThemeType::kCombined) {
        //默认主题
        ASSERT(m_defaultThemePath == themePath);
        m_defaultThemeInfo.m_bSelectedTheme = true;
        m_colorThemeInfo.m_bSelectedTheme = false;
        m_iconThemeInfo.m_bSelectedTheme = false;
    }
    else {
        return false;
    }
    m_bSwitchingTheme = true;
    globalbuilder.CreateControls(&tempWnd);
    m_bSwitchingTheme = false;
    if (m_themeChangedCallback) {
        m_themeChangedCallback(themeInfo);
    }
    return true;
}

bool ThemeManager::IsSwitchingTheme() const
{
    return m_bSwitchingTheme;
}


bool ThemeManager::GetAllThemes(const std::vector<FilePath>& themePathList,
                                std::vector<ThemeInfo>& themeInfoList) const
{
    FilePath themeRootPath = GetThemeRootPath();
    ASSERT(!themeRootPath.IsEmpty());
    if (themeRootPath.IsEmpty()) {
        return false;
    }
    FilePath globalXmlFileName = FilePath(GetGlobalXmlFileName());
    ASSERT(!globalXmlFileName.IsEmpty());
    if (globalXmlFileName.IsEmpty()) {
        return false;
    }
    themeInfoList.clear();
    std::vector<FilePath> destThemePathList;
    if (GlobalManager::Instance().Zip().IsUseZip()) {
        //使用Zip压缩包作为资源文件
        GetZipThemePathList(themeRootPath, themePathList, destThemePathList);
    }
    else if (themeRootPath.IsAbsolutePath()) {
        //使用本地磁盘文件
        GetLocalThemePathList(themeRootPath, themePathList, destThemePathList);
    }
    for (const FilePath& themePath : destThemePathList) {
        if (themePath.IsEmpty()) {
            continue;
        }
        FilePath configXmlFile = themeRootPath;
        configXmlFile /= themePath;
        configXmlFile /= globalXmlFileName;

        std::vector<uint8_t> configXmlData;
        if (GlobalManager::Instance().Zip().IsUseZip()) {
            if (!GlobalManager::Instance().Zip().GetZipData(configXmlFile, configXmlData)) {
                continue;
            }
        }

        ThemeInfo themeInfo;
        if (ParseXmlThemeInfo(configXmlFile, configXmlData, themePath, themeInfo)) {
            themeInfoList.push_back(themeInfo);
        }
    }
    return !themeInfoList.empty();
}

bool ThemeManager::ParseXmlThemeInfo(const FilePath& configXmlFile,
                                     const std::vector<uint8_t>& configXmlData,
                                     const FilePath& themePath,
                                     ThemeInfo& themeInfo) const
{
    WindowBuilder globalbuilder;    
    if (configXmlData.empty()) {
        if (!configXmlFile.IsExistsFile()) {
            return false;
        }
        if (!globalbuilder.ParseXmlFile(FilePath(configXmlFile))) {
            return false;
        }
    }
    else {
        if (!globalbuilder.ParseXmlData(configXmlData)) {
            return false;
        }
    }

    //初始化主题数据
    DString themeName;
    DString themeType;
    DString themeStyle;
    globalbuilder.ParseThemeInfo(themeName, themeType, themeStyle);
    if (themeName.empty() || themeType.empty() || themeStyle.empty()) {
        return false;
    }
    themeInfo.m_bDefaultTheme = false;
    themeInfo.m_bSelectedTheme = false;
    themeInfo.m_themePath = themePath;
    themeInfo.m_themeName = themeName;
    themeInfo.m_themeStyle = GetThemeStyleValue(themeStyle);
    themeInfo.m_themeType = GetThemeTypeValue(themeType);

    const ThemeInfo& defaultThemeInfo = GetDefaultThemeInfo();
    if (defaultThemeInfo.m_themePath == themeInfo.m_themePath) {
        themeInfo.m_bDefaultTheme = true;
        themeInfo.m_bSelectedTheme = defaultThemeInfo.m_bSelectedTheme;
    }
    const ThemeInfo& iconThemeInfo = GetCurrentIconThemeInfo();
    if ((iconThemeInfo.m_bSelectedTheme) &&
        (iconThemeInfo.m_themePath == themeInfo.m_themePath)) {
        themeInfo.m_bDefaultTheme = false;
        themeInfo.m_bSelectedTheme = true;
    }
    const ThemeInfo& colorThemeInfo = GetCurrentColorThemeInfo();
    if ((colorThemeInfo.m_bSelectedTheme) &&
        (colorThemeInfo.m_themePath == themeInfo.m_themePath)) {
        themeInfo.m_bDefaultTheme = false;
        themeInfo.m_bSelectedTheme = true;
    }
    return true;
}

bool ThemeManager::GetLocalThemePathList(const FilePath& themeRootPath,
                                         const std::vector<FilePath>& themePathList,
                                         std::vector<FilePath>& localThemePathList) const
{
    localThemePathList.clear();
    if (themeRootPath.IsEmpty() || !themeRootPath.IsAbsolutePath() || !themeRootPath.IsExistsDirectory()) {
        return false;
    }
    std::unordered_set<DString> themePathSet;
    for (const FilePath& inputPath : themePathList) {
        DString temp = inputPath.ToString();
        temp = StringUtil::MakeLowerString(temp);
        themePathSet.insert(temp);
    }
    try {
#ifdef DUILIB_BUILD_FOR_WIN
        std::filesystem::path stdPath(themeRootPath.ToStringW());
#else
        std::filesystem::path stdPath(themeRootPath.NativePathA());
#endif
        for (const auto& entry : std::filesystem::directory_iterator(stdPath)) {
            std::error_code errorCode;
            bool bDirectory = entry.is_directory(errorCode);
            if (bDirectory) {
                //目录
                FilePath localPath(entry.path().filename().native());
                if (!themePathSet.empty()) {
                    DString temp = localPath.ToString();
                    temp = StringUtil::MakeLowerString(temp);
                    if (themePathSet.find(temp) == themePathSet.end()) {
                        //过滤不在列表中的主题
                        localPath.Clear();
                    }
                }
                if (!localPath.IsEmpty()) {
                    localThemePathList.push_back(localPath);
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& /*e*/) {
        ASSERT(0);
    }
    return !localThemePathList.empty();
}

bool ThemeManager::GetZipThemePathList(const FilePath& themeRootPath,
                                       const std::vector<FilePath>& themePathList,
                                       std::vector<FilePath>& localThemePathList) const
{
    if (themeRootPath.IsEmpty()) {
        return false;
    }
    std::unordered_set<DString> themePathSet;
    for (const FilePath& inputPath : themePathList) {
        DString temp = inputPath.ToString();
        temp = StringUtil::MakeLowerString(temp);
        themePathSet.insert(temp);
    }
    std::vector<DString> dirList;
    GlobalManager::Instance().Zip().GetZipFileList(themeRootPath, nullptr, &dirList);
    for (const DString& dirName : dirList) {
        FilePath localPath(dirName);
        if (!themePathSet.empty()) {
            DString temp = localPath.ToString();
            temp = StringUtil::MakeLowerString(temp);
            if (themePathSet.find(temp) == themePathSet.end()) {
                //过滤不在列表中的主题
                localPath.Clear();
            }
        }
        if (!localPath.IsEmpty()) {
            localThemePathList.push_back(localPath);
        }
    }
    return !localThemePathList.empty();
}

ThemeStyle ThemeManager::GetCurrentThemeStyle() const
{
    if (m_colorThemeInfo.m_bSelectedTheme) {
        return m_colorThemeInfo.m_themeStyle;
    }
    else if ((m_iconThemeInfo.m_bSelectedTheme) &&
             (m_iconThemeInfo.m_themeType == ThemeType::kCombined)) {
        return m_iconThemeInfo.m_themeStyle;
    }
    else if ((m_defaultThemeInfo.m_bSelectedTheme) &&
             (m_defaultThemeInfo.m_themeType == ThemeType::kCombined)) {
        return m_defaultThemeInfo.m_themeStyle;
    }
    return ThemeStyle::kLight;
}

const ThemeInfo& ThemeManager::GetDefaultThemeInfo() const
{
    return m_defaultThemeInfo;
}

const ThemeInfo& ThemeManager::GetCurrentColorThemeInfo() const
{
    return m_colorThemeInfo;
}

const ThemeInfo& ThemeManager::GetCurrentIconThemeInfo() const
{
    return m_iconThemeInfo;
}

bool ThemeManager::GetResFile(const FilePath& resFilePath,
                              const FilePath& windowResPath,
                              FilePath& resFileFullPath,
                              std::vector<uint8_t>& resFileData) const
{
    return GetResFileData(resFilePath, windowResPath, &resFileFullPath, &resFileData);
}

bool ThemeManager::IsResFileExists(const FilePath& resFilePath, const FilePath& windowResPath) const
{
    return GetResFileData(resFilePath, windowResPath, nullptr, nullptr);
}

bool ThemeManager::GetResFileData(const FilePath& resFilePath,
                                  const FilePath& windowResPath,
                                  FilePath* pResFileFullPath,
                                  std::vector<uint8_t>* pResFileData) const
{
    ASSERT(!resFilePath.IsEmpty() && _T("resFilePath 参数为空！"));
    if (resFilePath.IsEmpty()) {
        return false;
    }
    if (pResFileFullPath != nullptr) {
        pResFileFullPath->Clear();
    }
    if (pResFileData != nullptr) {
        pResFileData->clear();
    }
    bool bFileExists = false;
    FilePath sFile;
    std::vector<FilePath> resFileSearchPathList;
    GetResFileSearchPath(windowResPath, resFileSearchPathList);
    for (const FilePath& resFileSearchPath : resFileSearchPathList) {
        if (GlobalManager::Instance().Zip().IsUseZip()) {
            sFile = FilePathUtil::JoinFilePath(resFileSearchPath, resFilePath);
            if (GlobalManager::Instance().Zip().IsZipResExist(sFile)) {
                bFileExists = true;                
                if (pResFileData != nullptr) {
                    if (!GlobalManager::Instance().Zip().GetZipData(sFile, *pResFileData)) {
                        bFileExists = false;
                    }
                }
            }            
        }
        else {
            sFile.Clear();
            if (resFilePath.IsRelativePath()) {
                //相对路径
                sFile = FilePathUtil::JoinFilePath(resFileSearchPath, resFilePath);
            }
            else {
                //绝对路径
                sFile = resFilePath;
            }
            bFileExists = sFile.IsExistsFile();
        }
        if (bFileExists) {
            if (pResFileFullPath != nullptr) {
                *pResFileFullPath = sFile;
            }
            break;
        }
    }
    return bFileExists;
}

void ThemeManager::GetResFileSearchPath(const FilePath& windowResPath, std::vector<FilePath>& resFileSearchPathList) const
{
    // 资源查找顺序：
    // 1. 首先在窗口对应的资源目录下查找（windowResPath目录）
    // 2. 在颜色主题目录查找
    // 3. 在图标主题目录查找
    // 4. 在默认主题目录查找
    std::vector<FilePath> themeDirList;
    if (!GetCurrentColorThemeInfo().m_themePath.IsEmpty()) {
        themeDirList.push_back(GetCurrentColorThemeInfo().m_themePath);
    }
    if (!GetCurrentIconThemeInfo().m_themePath.IsEmpty()) {
        themeDirList.push_back(GetCurrentIconThemeInfo().m_themePath);
    }
    if (!GetDefaultThemePath().IsEmpty()) {
        themeDirList.push_back(GetDefaultThemePath());
    }

    FilePath tempPath;
    const FilePath themeRootPath = GetThemeRootPath();
    if (!windowResPath.IsEmpty()) {
        for (const FilePath& themeDir : themeDirList) {
            tempPath = themeRootPath;
            tempPath /= themeDir;
            tempPath /= windowResPath;
            resFileSearchPathList.push_back(tempPath);
        }
    }
    for (const FilePath& themeDir : themeDirList) {
        tempPath = themeRootPath;
        tempPath /= themeDir;
        resFileSearchPathList.push_back(tempPath);
    }
    auto iter = resFileSearchPathList.begin();
    while (iter != resFileSearchPathList.end()) {
        if (iter->IsExistsDirectory()) {
            ++iter;
        }
        else {
            iter = resFileSearchPathList.erase(iter);
        }
    }
}

void ThemeManager::GetResFileSearchPathEx(const FilePath& windowResPath,
                                          const FilePath& windowXmlPath,
                                          bool bPublicFirst,
                                          std::vector<FilePath>& resFileSearchPathList) const
{
    std::vector<FilePath> themeDirList;
    if (!GetCurrentColorThemeInfo().m_themePath.IsEmpty()) {
        themeDirList.push_back(GetCurrentColorThemeInfo().m_themePath);
    }
    if (!GetCurrentIconThemeInfo().m_themePath.IsEmpty()) {
        themeDirList.push_back(GetCurrentIconThemeInfo().m_themePath);
    }
    if (!GetDefaultThemePath().IsEmpty()) {
        themeDirList.push_back(GetDefaultThemePath());
    }
    FilePath tempPath;
    const FilePath themeRootPath = GetThemeRootPath();

    if (bPublicFirst) {
        // 资源查找顺序：
        // 1. 在颜色主题目录查找
        // 2. 在图标主题目录查找
        // 3. 在默认主题目录查找
        // 4. 首先在窗口对应的资源目录下查找（windowResPath目录）
        // 5. 在窗口XML对应的资源目录下查找（windowXmlPath目录）
        for (const FilePath& themeDir : themeDirList) {
            tempPath = themeRootPath;
            tempPath /= themeDir;
            resFileSearchPathList.push_back(tempPath);
        }

        if (!windowResPath.IsEmpty()) {
            for (const FilePath& themeDir : themeDirList) {
                tempPath = themeRootPath;
                tempPath /= themeDir;
                tempPath /= windowResPath;
                resFileSearchPathList.push_back(tempPath);
            }
        }
        if (!windowXmlPath.IsEmpty()) {
            for (const FilePath& themeDir : themeDirList) {
                tempPath = themeRootPath;
                tempPath /= themeDir;
                tempPath /= windowXmlPath;
                resFileSearchPathList.push_back(tempPath);
            }
        }
    }
    else {
        // 资源查找顺序：
        // 1. 在窗口对应的资源目录下查找（windowResPath目录）
        // 2. 在窗口XML对应的资源目录下查找（windowXmlPath目录）
        // 3. 在颜色主题目录查找
        // 4. 在图标主题目录查找
        // 5. 在默认主题目录查找
        if (!windowResPath.IsEmpty()) {
            for (const FilePath& themeDir : themeDirList) {
                tempPath = themeRootPath;
                tempPath /= themeDir;
                tempPath /= windowResPath;
                resFileSearchPathList.push_back(tempPath);
            }
        }
        if (!windowXmlPath.IsEmpty()) {
            for (const FilePath& themeDir : themeDirList) {
                tempPath = themeRootPath;
                tempPath /= themeDir;
                tempPath /= windowXmlPath;
                resFileSearchPathList.push_back(tempPath);
            }
        }
        for (const FilePath& themeDir : themeDirList) {
            tempPath = themeRootPath;
            tempPath /= themeDir;
            resFileSearchPathList.push_back(tempPath);
        }
    }

    auto iter = resFileSearchPathList.begin();
    while (iter != resFileSearchPathList.end()) {
        if (iter->IsExistsDirectory()) {
            ++iter;
        }
        else {
            iter = resFileSearchPathList.erase(iter);
        }
    }
}

FilePath ThemeManager::FindExistsResFullPath(const FilePath& windowResPath,
                                             const FilePath& windowXmlPath,
                                             const FilePath& resPath,
                                             bool& bLocalPath,
                                             bool& bResPath) const
{
    bLocalPath = true;
    bResPath = true;
    ASSERT(!resPath.IsEmpty());
    if (resPath.IsEmpty()) {
        return resPath;
    }
    FilePath imageFullPath;
#ifdef DUILIB_BUILD_FOR_WIN
    const bool bOSWindows = true;
#else
    const bool bOSWindows = false;
#endif

    bool bWindows = bOSWindows;//避免编译警告
    if (bWindows && resPath.IsAbsolutePath()) {
        //Windows平台的绝对路径: 外部文件
        imageFullPath = resPath;
        imageFullPath.NormalizeFilePath();
        if (imageFullPath.IsExistsFile()) {
            bLocalPath = true;
            bResPath = false;
        }
        else {
            //如果文件不存在，返回空
            imageFullPath.Clear();
        }
    }
    else {
        const bool bResInPublic = IsResInPublicPath(resPath);
        std::vector<FilePath> resFileSearchPathList;
        GetResFileSearchPathEx(windowResPath, windowXmlPath, bResInPublic, resFileSearchPathList);
        for (const FilePath& resFileSearchPath : resFileSearchPathList) {
            imageFullPath = FilePathUtil::JoinFilePath(resFileSearchPath, resPath);
            CheckImagePath(imageFullPath, bLocalPath);            
            if (!imageFullPath.IsEmpty()) {
                break;
            }
        }        
    }
    if (!bWindows && imageFullPath.IsEmpty() && resPath.IsAbsolutePath()) {
        //注意：非Windows的绝对路径与相对路径形式相同，都是以'/'开头，所以放在最后判断
        imageFullPath = resPath;
        imageFullPath.NormalizeFilePath();
        if (imageFullPath.IsExistsFile()) {
            bLocalPath = true;
            bResPath = false;
        }
        else {
            //如果文件不存在，返回空
            imageFullPath.Clear();
        }
    }
    return imageFullPath;
}

void ThemeManager::CheckImagePath(FilePath& imageFullPath, bool& bLocalPath) const
{
    imageFullPath.NormalizeFilePath();
    if (GlobalManager::Instance().Zip().IsZipResExist(imageFullPath)) {
        bLocalPath = false;
    }
    else if (imageFullPath.IsExistsFile()) {
        bLocalPath = true;
    }
    else {
        //如果文件不存在，返回空
        imageFullPath.Clear();
    }
}

bool ThemeManager::IsResInPublicPath(const FilePath& resPath) const
{
    DString resPathString = resPath.ToString();
    StringUtil::ReplaceAll(_T("\\"), _T("/"), resPathString);
    if ((resPathString.find(_T("public/")) == 0) || ((resPathString.find(_T("/public/")) == 0))) {
        return true;
    }
    return false;
}

const FilePath& ThemeManager::GetThemeRootPath() const
{
    return m_themeRootPath;
}

const FilePath& ThemeManager::GetDefaultThemePath() const
{
    return m_defaultThemePath;
}

const DString& ThemeManager::GetGlobalXmlFileName() const
{
    return m_globalXmlFileName;
}

void ThemeManager::SetThemeChangeCallback(ThemeChangedCallback callback)
{
    m_themeChangedCallback = callback;
}

ThemeChangedCallback ThemeManager::GetThemeChangeCallback() const
{
    return m_themeChangedCallback;
}

void ThemeManager::Clear()
{
    m_themeChangedCallback = nullptr;
}

} // namespace ui
