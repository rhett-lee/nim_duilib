#ifndef UI_CORE_GLOBALMANAGER_H_
#define UI_CORE_GLOBALMANAGER_H_

#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/ColorManager.h"
#include "duilib/Core/FontManager.h"
#include "duilib/Core/ImageManager.h"
#include "duilib/Core/ZipManager.h"
#include "duilib/Core/LangManager.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/TimerManager.h"
#include "duilib/Core/ThreadManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/CursorManager.h"
#include "duilib/Core/IconManager.h"
#include "duilib/Core/WindowManager.h"

#include <string>
#include <vector>
#include <map>
#include <thread>

namespace ui 
{
/** 渲染引擎工厂接口
*/
class IRenderFactory;

/** 全局属性管理工具类
*   用于管理一些全局属性的工具类，包含全局样式（global.xml）和语言设置等
*/
class UILIB_API GlobalManager
{
private:
    GlobalManager();
    ~GlobalManager();
    GlobalManager(const GlobalManager&) = delete;
    GlobalManager& operator = (const GlobalManager&) = delete;

public:
    /** 获取单例对象
    */
    static GlobalManager& Instance();

    /** 获取默认的资源目录
    * @param [in] bMacOsAppBundle MacOS平台是否使用App Bundle 
    */
    static FilePath GetDefaultResourcePath(bool bMacOsAppBundle);

public:
    /** 初始化全局设置函数
     * @param [in] resParam 资源相关的参数，根据资源类型不同，有以下可选项
     *                      1. 本地文件的形式，所有资源都已本地文件的形式存在
     *                         使用 LocalFilesResParam 类型作为参数
     *                      2. 资源文件打包为zip压缩包，然后以本地文件的形式存在
     *                         使用 ZipFileResParam 类型作为参数
     *                      3. 资源文件打包为zip压缩包，然后放在exe/dll的资源文件中
     *                         使用 ResZipFileResParam 类型作为参数
     * @param [in] dpiInitParam DPI感知模式和DPI值的设置参数
     * @param [in] callback 创建自定义控件时的全局回调函数
     */
    bool Startup(const ResourceParam& resParam,
                 DpiInitParam dpiInitParam = DpiInitParam(),
                 const CreateControlCallback& callback = nullptr);

    /** 释放全局资源
     */
    void Shutdown();

public:
    /** 设置皮肤资源所在路径
     *   如果 resType == kLocalFiles，需要设置资源所在的本地路径（绝对路径）
     *   如果 resType == kZipFile 或者 resType == kResZip，设置资源所在的起始目录（相对路径），比如：_T("resources\\")
     */
    void SetResourcePath(const FilePath& strPath);

    /** 获取当前资源所在路径
     */
    const FilePath& GetResourcePath() const;

    /** 重新加载皮肤资源（可通过此接口实现动态换肤功能）
    * @param [in] resParam 资源相关的参数，根据资源类型不同，有以下可选项
     *                      1. 本地文件的形式，所有资源都已本地文件的形式存在
     *                         使用 LocalFilesResParam 类型作为参数
     *                      2. 资源文件打包为zip压缩包，然后以本地文件的形式存在
     *                         使用 ZipFileResParam 类型作为参数
     *                      3. 资源文件打包为zip压缩包，然后放在exe/dll的资源文件中
     *                         使用 ResZipFileResParam 类型作为参数
     * @param [in] bInvalidate 是否刷新界面显示：true表示更新完语言文件后刷新界面显示，false表示不刷新界面显示
    */
    bool ReloadResource(const ResourceParam& resParam, bool bInvalidate = false);

    /** 获取平台相关数据（可选参数，如不填写则使用默认值：nullptr）
    *   Windows平台：是资源所在模块句柄（HMODULE），如果为nullptr，则使用所在exe的句柄（可选参数）
    */
    void* GetPlatformData() const;

    /** 设置字体文件所在路径
    */
    void SetFontFilePath(const FilePath& strPath);

    /** 获取字体文件所在路径
    */
    const FilePath& GetFontFilePath() const;

public:
    /** 设置语言文件所在路径，可以是相对路径或者是绝对路径（多语言版时，所有的语言文件都放在这个目录中）
    *   如果是绝对路径，则在这个绝对路径中查找语言文件
    *   如果是相对路径，则根据resType和resourcePath决定的资源路径下，按相对路径查找资源文件
    */
    void SetLanguagePath(const FilePath& strPath);

    /** 获取语言文件所在路径
    */
    const FilePath& GetLanguagePath() const;

    /** 获取语言文件名，不含路径
    */
    const DString& GetLanguageFileName() const;

    /** 重新加载语言资源（通过此接口实现多语言动态切换功能）
     * @param [in] languagePath 语言文件所在路径
                   如果是绝对路径，则在此绝对路径的目录中查找语言文件；
                   如果为空，表示使用Startup时初始化的语言文件路径；
                   如果为相对路径，则对应于压缩包中的相对路径
     * @param [in] languageFileName 当前使用语言文件的文件名（不含路径）
     * @param [in] bInvalidate 是否刷新界面显示：true表示更新完语言文件后刷新界面显示，false表示不刷新界面显示
     */
    bool ReloadLanguage(const FilePath& languagePath = FilePath(),
                        const DString& languageFileName = _T("zh_CN.txt"),
                        bool bInvalidate = false);

    /** 获取语言文件列表和显示名称（以支持多语言切换）
    * @param [in] languageNameID 用于读取显示名称的字符串ID，如果为空则不读取显示名称
    * @param [out] languageList 返回语言文件和显示名称的列表
    */
    bool GetLanguageList(std::vector<std::pair<DString, DString>>& languageList,
                         const DString& languageNameID = _T("LANGUAGE_DISPLAY_NAME")) const;

public:
    /** 添加一个全局 Class 属性
     * @param[in] strClassName 全局 Class 名称
     * @param[in] strControlAttrList 属性列表，需要做 XML 转义
     */
    void AddClass(const DString& strClassName, const DString& strControlAttrList);

    /** 获取一个全局 class 属性的值
     * @param[in] strClassName 全局 class 名称
     * @return 返回字符串形式的 class 属性值
     */
    DString GetClassAttributes(const DString& strClassName) const;

    /** 从全局属性中删除所有 class 属性
     * @return 返回绘制区域对象
     */
    void RemoveAllClasss();

public:
    /** 获取绘制接口类对象
    */
    IRenderFactory* GetRenderFactory();

    /** 获取颜色管理器
    */
    ColorManager& Color();

    /** 获取字体管理器
    */
    FontManager& Font();

    /** 获取图片管理器
    */
    ImageManager& Image();

    /** 获取ICON资源管理器
    */
    IconManager& Icon();

    /** 获取Zip管理器
    */
    ZipManager& Zip();

    /** 获取DPI管理器
    */
    DpiManager& Dpi();

    /** 获取定时器管理器
    */
    TimerManager& Timer();

    /** 获取线程管理器
    */
    ThreadManager& Thread();

    /** 多语言支持管理器
    */
    LangManager& Lang();

    /** 光标管理器
    */
    CursorManager& Cursor();

    /** 窗口管理器
    */
    WindowManager& Windows();

public:
    /** 根据资源加载方式，返回对应的资源路径
     * @param[in] path 要获取的资源路径
     * @param [in] windowResPath 窗口对应的资源相对目录，比如："controls\\"
     * @param [in] windowXmlPath 窗口对应XML所在的相对目录，比如："controls\\menu\\"
     * @param [in] resPath 资源文件路径，比如："../public/button/btn_wnd_gray_min_hovered.png"
     * @return 返回可用的完整的资源路径，如果资源路径不存在，则返回空
               返回有效的路径格式如下：
              （1）如果是使用ZIP压缩包，返回："resources\themes\default\public\button\btn_wnd_gray_min_hovered.png"
              （2）如果未使用ZIP压缩包，返回："<程序所在目录>\resources\themes\default\public\button\btn_wnd_gray_min_hovered.png"
     */
    FilePath GetExistsResFullPath(const FilePath& windowResPath, const FilePath& windowXmlPath, const FilePath& resPath);

    /** 根据 XML 创建一个 Box
     * @param[in] strXmlPath XML 文件路径
     * @param[in] callback 自定义控件的回调处理函数
     * @return 指定布局模块的对象指针
     */
    Box* CreateBox(const FilePath& strXmlPath, CreateControlCallback callback = CreateControlCallback());

    /** 根据 XML 在缓存中查找指定 Box，如果没有则创建
     * @param[in] strXmlPath XML 文件路径
     * @param[in] callback 自定义控件的回调处理函数
     * @return 指定布局模块的对象指针
     */
    Box* CreateBoxWithCache(const FilePath& strXmlPath, CreateControlCallback callback = CreateControlCallback());

    /** 使用 XML 填充指定 Box
     * @param[in] pUserDefinedBox 要填充的 box 指针
     * @param[in] strXmlPath XML 文件路径
     * @param[in] callback 自定义控件的回调处理函数
     */
    void FillBox(Box* pUserDefinedBox, const FilePath& strXmlPath, CreateControlCallback callback = CreateControlCallback());

    /** 使用构建过的缓存填充指定 Box，如果没有则重新构建
     * @param[in] pUserDefinedBox 要填充的 box 指针
     * @param[in] strXmlPath XML 文件路径
     * @param[in] callback 自定义控件的回调处理函数
     */
    void FillBoxWithCache(Box* pUserDefinedBox, const FilePath& strXmlPath, CreateControlCallback callback = CreateControlCallback());

    /** 自定义控件创建后的全局回调函数
     * @param[in] strControlName 自定义控件名称
     * @return 返回一个自定义控件的对象指针
     */
    Control* CreateControl(const DString& strControlName);

    /** 添加控件创建函数，用于用户自定义控件的创建
    */
    void AddCreateControlCallback(const CreateControlCallback& pfnCreateControlCallback);

    /** 判断当前是否在UI线程
    */
    bool IsInUIThread() const;

    /** 如果该函数不是在UI线程中调用的，进行断言报错
    */
    void AssertUIThread() const;

    /** 添加一个退出时执行的函数
    */
    void AddAtExitFunction(std::function<void()> atExitFunction);

private:
    /** 从缓存中删除所有图片
     */
    void RemoveAllImages();

private:

    /** 渲染引擎管理接口
    */
    std::unique_ptr<IRenderFactory> m_renderFactory;

    /** 全局的资源路径，换肤的时候修改这个变量（绝对路径）
    */
    FilePath m_resourcePath;

    /** 平台相关数据（可选参数，如不填写则使用默认值：nullptr）
    *   Windows平台：是资源所在模块句柄（HMODULE），如果为nullptr，则使用所在exe的句柄（可选参数）
    */
    void* m_platformData;

    /** 全局字体文件路径（绝对路径）
    */
    FilePath m_fontFilePath;

    /** 全局语言文件路径（绝对路径）
    */
    FilePath m_languagePath;

    /** 全局语言文件名（不含路径）
    */
    DString m_languageFileName;
    
    /** 窗口构建管理接口，KEY是XML文件路径，VALUE是窗口构建管理接口（已经解析后的XML，可避免重复解析）
    */
    std::map<FilePath, std::unique_ptr<WindowBuilder>> m_builderMap;

    /** 控件创建函数，用于用户自定义控件的创建
    */
    std::vector<CreateControlCallback> m_pfnCreateControlCallbackList;

    /** 每个Class的名称(KEY)和属性列表(VALUE)（比如global.xml中定义的Class）
    */
    std::map<DString, DString> m_globalClass;

    /** 主线程ID
    */
    std::thread::id m_dwUiThreadId;

    /** 颜色管理器
    */
    ColorManager m_colorManager;

    /** 字体管理器
    */
    FontManager m_fontManager;

    /** 图片管理器
    */
    ImageManager m_imageManager;

    /** ZIP压缩包管理器
    */
    ZipManager m_zipManager;

    /** DPI管理器
    */
    DpiManager m_dpiManager;

    /** 定时器管理器
    */
    TimerManager m_timerManager;

    /** 线程管理器
    */
    ThreadManager m_threadManager;

    /** 多语言管理器
    */
    LangManager m_langManager;

    /** ICON资源管理器
    */
    IconManager m_iconManager;

    /** 光标管理器
    */
    CursorManager m_cursorManager;

    /** 窗口管理器
    */
    WindowManager m_windowManager;

    /** 退出时要执行的函数
    */
    std::vector<std::function<void()>> m_atExitFunctions;
};

} // namespace ui

#endif // UI_CORE_GLOBALMANAGER_H_
