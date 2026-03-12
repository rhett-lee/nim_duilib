#ifndef UI_CORE_THEME_MANAGER_H_
#define UI_CORE_THEME_MANAGER_H_

#include "duilib/Utils/FilePath.h"
#include "duilib/Core/Callback.h"

namespace ui 
{
/** 主题风格
*/
enum class ThemeStyle
{
    kUnknown,  //未知主题
    kLight,    //浅色主题
    kDark      //深色主题
};

/** 主题类型
*/
enum class ThemeType
{
    kUnknown,  //未知主题
    kColor,    //颜色主题
    kIcon,     //图标主题
    kCombined  //组合主题（颜色+图标）
};

/** 主题元数据
*/
struct ThemeInfo
{
    /** 主题显示名称
    */
    DString m_themeName;

    /** 主题的颜色风格
    */
    ThemeStyle m_themeStyle;

    /** 主题的类型
    */
    ThemeType m_themeType;

    /** 主题资源路径
    */
    FilePath m_themePath;

    /** 是否为默认主题
    */
    bool m_bDefaultTheme;

    /** 是否为已选择主题
    */
    bool m_bSelectedTheme;
};

/** 主题变化回调函数
*/
typedef std::function<void(const ThemeInfo& themeInfo)> ThemeChangedCallback;

/** 主题管理器
*/
class UILIB_API ThemeManager
{
public:
    ThemeManager();

public:
    /** 初始化主题管理器
    * @param [in] themeRootFullPath 主题资源所在的根路径
    *                           当使用zip文件时，为相对路径，格式示例：resources/themes
    *                           当使用本地文件时，为绝对路径，格式示例：<程序所在目录的绝对路径>/resources/themes
    * @param [in] defaultThemePath 主题的默认资源路径
    * @param [in] globalXmlFileName 全局资源描述XML文件的文件名，默认为："global.xml"
    */
    bool InitTheme(const FilePath& themeRootFullPath,
                   const FilePath& defaultThemePath,
                   const DString& globalXmlFileName);

    /** 获取主题资源的根目录
    */
    const FilePath& GetThemeRootPath() const;

    /** 获取默认资源路径
    */
    const FilePath& GetDefaultThemePath() const;

    /** 获取默认资源路径
    */
    const DString& GetGlobalXmlFileName() const;

public:
    /** 设置主题切换回调函数（供UI控件刷新使用）
    * @param [in] callback 主题变化回调函数
    */
    void SetThemeChangeCallback(ThemeChangedCallback callback);

    /** 获取主题切换回调函数
    */
    ThemeChangedCallback GetThemeChangeCallback() const;

    /** 获取主题资源信息列表
    * @param [in] themePathList 需要获取的主题路径（比如"color_dark"）列表，如果为空则获取主题根目录下的所有可用主题列表
    * @param [out] themeInfoList 返回所有的主题资源信息列表
    */
    bool GetAllThemes(const std::vector<FilePath>& themePathList,
                      std::vector<ThemeInfo>& themeInfoList) const;

    /** 切换颜色主题（立即生效），该主题的类型应该是Color或者Combined
    * @param [in] themePath 主题资源所在的路径
    */
    bool SwitchColorTheme(const FilePath& themePath);

    /** 切换图标主题（立即生效），该主题的类型应该是Icon或者Combined
    * @param [in] themePath 主题资源所在的路径
    */
    bool SwitchIconTheme(const FilePath& themePath);

    /** 当前是否正在切换主题
    */
    bool IsSwitchingTheme() const;

public:
    /** 获取当前主题的颜色风格
    */
    ThemeStyle GetCurrentThemeStyle() const;

    /** 当前默认主题信息
    */
    const ThemeInfo& GetDefaultThemeInfo() const;

    /** 获取当前选择的颜色主题信息
    */
    const ThemeInfo& GetCurrentColorThemeInfo() const;

    /** 获取当前选择的图标主题信息
    */
    const ThemeInfo& GetCurrentIconThemeInfo() const;

public:
    /** 获取资源文件文件路径或者数据（当使用zip资源压缩包时返回文件数据）
    * @param [in] resFilePath 资源文件路径（配置路径）
    * @param [in] windowResPath 窗口资源子目录, 用于查找资源文件（当不指定文件路径时）
    * @param [out] resFileFullPath 返回该资源资源的完整文件路径
    * @param [out] resFileData 返回资源文件的数据
    */
    bool GetResFile(const FilePath& resFilePath,
                    const FilePath& windowResPath,
                    FilePath& resFileFullPath,
                    std::vector<uint8_t>& resFileData) const;

    /** 判断是否存在资源文件
    * @param [in] resFilePath 资源资源的文件路径（配置路径）
    * @param [in] windowResPath 窗口资源子目录, 用于查找资源文件（当不指定文件路径时）
    */
    bool IsResFileExists(const FilePath& resFilePath, const FilePath& windowResPath) const;

    /** 根据资源加载方式，返回对应的资源路径
     * @param [in] windowResPath 窗口对应的资源相对目录，比如："controls\\"
     * @param [in] windowXmlPath 窗口对应XML所在的相对目录，比如："controls\\menu\\"
     * @param [in] resPath 资源文件路径，比如："../public/button/btn_wnd_gray_min_hovered.png"
     * @param [out] bLocalPath 返回true表示文件为本地路径，返回false表示文件为zip压缩包内路径
     * @param [out] bResPath 返回true表示文件在程序资源路径内，返回false表示文件不在程序资源路径内
     * @return 返回可用的完整的资源路径，如果资源路径不存在，则返回空
               返回有效的路径格式如下：
              （1）如果是使用ZIP压缩包，返回："resources\themes\default\public\button\btn_wnd_gray_min_hovered.png"
              （2）如果未使用ZIP压缩包，返回："<程序所在目录>\resources\themes\default\public\button\btn_wnd_gray_min_hovered.png"
     */
    FilePath FindExistsResFullPath(const FilePath& windowResPath, const FilePath& windowXmlPath,
                                   const FilePath& resPath, bool& bLocalPath, bool& bResPath) const;

public:
    /** 判断一个路径是否在public子目录中
    */
    bool IsResInPublicPath(const FilePath& resPath) const;

    /** 获取资源搜索路径
    * @param [in] windowResPath 窗口的资源路径，比如basic示例程序的窗口资源目录名是"basic"
    * @param [out] resFileSearchPathList 返回资源搜索路径，按优先级排序
    */
    void GetResFileSearchPath(const FilePath& windowResPath, std::vector<FilePath>& resFileSearchPathList) const;

    /** 获取资源搜索路径
    * @param [in] windowResPath 窗口的资源路径，比如basic示例程序的窗口资源目录名是"controls"
    * @param [in] windowXmlPath 窗口对应XML所在的相对目录，比如："controls\\menu\\"
    * @param [in] bPublicFirst 优先在公共目录查找，窗口目录作为低优先级
    * @param [out] resFileSearchPathList 返回资源搜索路径，按优先级排序
    */
    void GetResFileSearchPathEx(const FilePath& windowResPath,
                                const FilePath& windowXmlPath,
                                bool bPublicFirst,
                                std::vector<FilePath>& resFileSearchPathList) const;

    /** 清空
    */
    void Clear();

private:
    /** 检查图片文件路径是否存在
    * @param [in,out] imageFullPath 如果不存在清空，如果存在保留
    * @param [out] bLocalPath 返回true表示文件为本地路径，返回false表示文件为zip压缩包内路径
    */
    void CheckImagePath(FilePath& imageFullPath, bool& bLocalPath) const;

    /** 获取资源文件路径或者数据（当使用zip资源压缩包时返回文件数据）
    * @param [in] resFilePath 资源的文件路径（配置路径）
    * @param [in] windowResPath 窗口资源子目录, 用于查找资源文件（当不指定文件路径时）
    * @param [out] pResFileFullPath 返回该资源资源的完整文件路径
    * @param [out] pResFileData 返回资源文件的数据
    */
    bool GetResFileData(const FilePath& ResFilePath,
                        const FilePath& windowResPath,
                        FilePath* pResFileFullPath,
                        std::vector<uint8_t>* pResFileData) const;

    /** 将字符串转换为主题类型
    */
    ThemeType GetThemeTypeValue(DString themeType) const;

    /** 将字符串转换为主题风格
    */
    ThemeStyle GetThemeStyleValue(DString themeStyle) const;

    /** 切换颜色主题或者图标主题（立即生效）
    * @param [in] themePath 主题资源所在的路径
    * @param [in] destThemeType 主题类型
    */
    bool SwitchTheme(const FilePath& themePath, ThemeType destThemeType);

    /** 获取本地文件系统中，主题根目录下的子目录
    */
    bool GetLocalThemePathList(const FilePath& themeRootPath,
                               const std::vector<FilePath>& themePathList,
                               std::vector<FilePath>& localThemePathList) const;

    /** 获取Zip压缩包中，主题根目录下的子目录
    */
    bool GetZipThemePathList(const FilePath& themeRootPath,
                             const std::vector<FilePath>& themePathList,
                             std::vector<FilePath>& localThemePathList) const;

    /** 解析出XML中定义的主题信息
    */
    bool ParseXmlThemeInfo(const FilePath& configXmlFile,
                           const std::vector<uint8_t>& configXmlData,
                           const FilePath& themePath,
                           ThemeInfo& themeInfo) const;

private:
    /** 主题资源的根目录
     *  当使用zip文件时，为相对路径，格式示例："resources/themes"
     *  当使用本地文件时，为绝对路径，格式示例："<程序所在目录的绝对路径>/resources/themes"
     */
    FilePath m_themeRootPath;

    /** 默认资源路径，格式示例："default"
    */
    FilePath m_defaultThemePath;

    /** 全局资源描述XML文件的文件名，默认为："global.xml"
    */
    DString m_globalXmlFileName;

    /** 主题切换回调函数
    */
    ThemeChangedCallback m_themeChangedCallback;

private:
    /** 当前默认主题信息
    */
    ThemeInfo m_defaultThemeInfo;

    /** 当前选择的颜色主题信息
    */
    ThemeInfo m_colorThemeInfo;

    /** 当前选择的图标主题信息
    */
    ThemeInfo m_iconThemeInfo;

    /** 当前正在切换主题
    */
    bool m_bSwitchingTheme;
};

} // namespace ui

#endif // UI_CORE_THEME_MANAGER_H_
