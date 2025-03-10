#ifndef UI_CORE_RESOURCE_PARAM_H_
#define UI_CORE_RESOURCE_PARAM_H_

#include "duilib/Utils/FilePath.h"

namespace ui 
{
/** 资源的类型
*/
enum class ResourceType
{
    /** 本地文件的形式，所有资源都已本地文件的形式存在
    */
    kLocalFiles,

    /** 资源文件打包为zip压缩包，然后以本地文件的形式存在
    */
    kZipFile,

    /** 资源文件打包为zip压缩包，然后放在exe/dll的资源文件中
    */
    kResZipFile
};

/** 加载全局资源所需的基本参数（基类，使用时需要使用子类，见下方定义）
*/
class UILIB_API ResourceParam
{
protected:
    explicit ResourceParam(ResourceType resourceType): resType(resourceType) { }
    ~ResourceParam() { }

protected:
    /** 资源的类型
    */
    ResourceType resType;

public:
    /** 获取资源类型
    */
    ResourceType GetResType() const { return resType; }

    /** 资源路径的所在位置
    *   如果 resType == kLocalFiles，需要设置资源所在的本地路径（绝对路径）
    *   如果 resType == kZipFile 或者 resType == kResZipFile，设置资源所在的起始目录（相对路径），比如：_T("resources\\")
    */
    FilePath resourcePath;

    /** 平台相关数据（可选参数，如不填写则使用默认值：nullptr）
    *   Windows平台：是资源所在模块句柄（HMODULE），如果为nullptr，则使用所在exe的句柄（可选参数）
    */
    void* platformData = nullptr;

public:
    /** 主题路径名称（相对路径）
    */
#if defined (DUILIB_BUILD_FOR_WIN)
    FilePath themePath = FilePath(_T("themes\\default"));
#else
    FilePath themePath = FilePath(_T("themes/default"));
#endif

    /** 外部字体文件所在路径
    */
    FilePath fontFilePath = FilePath(_T("fonts"));

    /** 语言文件所在路径，可以是相对路径或者是绝对路径（多语言版时，所有的语言文件都放在这个目录中）
    *   如果是绝对路径，则在这个绝对路径中查找语言文件
    *   如果是相对路径，则根据resType和resourcePath决定的资源路径下，按相对路径查找资源文件
    */
    FilePath languagePath = FilePath(_T("lang"));

    /** 当前使用语言文件的文件名（不含路径）
    */
    DString languageFileName = _T("zh_CN.txt");

    /** 全局资源描述XML文件的文件名，默认为："global.xml"
    */
    DString globalXmlFileName = _T("global.xml");
};

/** 加载全局资源所需的参数（本地文件形式，对应资源类型：kLocalFiles）
*/
class UILIB_API LocalFilesResParam: public ResourceParam
{
public:
    LocalFilesResParam() : ResourceParam(ResourceType::kLocalFiles)
    {
    }

    /** 构造一个本地文件形式的参数
    * @param [in] resPath 本地资源文件所在的路径（绝对路径）
    */
    explicit LocalFilesResParam(const FilePath& resPath) :
        ResourceParam(ResourceType::kLocalFiles)
    {
        resourcePath = resPath;
    }
};

/** 加载全局资源所需的参数（Zip压缩包文件形式，对应资源类型：kZipFile）
*/
class UILIB_API ZipFileResParam : public ResourceParam
{
public:
    ZipFileResParam() : ResourceParam(ResourceType::kZipFile)
    {
    }

    /** 压缩包的本地路径（绝对路径）
    */
    FilePath zipFilePath;

    /** 可选参数：可以设置压缩包的解压密码；如果压缩包无密码，不需要设置
    *   注意事项：需要使用"ZIP legacy encryption"模式的密码，否则无法解压
    */
    DString zipPassword;
};

#ifdef DUILIB_BUILD_FOR_WIN

/** 加载全局资源所需的参数（资源文件打包为zip压缩包，然后放在exe/dll的资源文件中，对应资源类型：kResZipFile）
*/
class UILIB_API ResZipFileResParam : public ResourceParam
{
public:
    ResZipFileResParam() : ResourceParam(ResourceType::kResZipFile)
    {
    }

    /** Zip压缩包所在资源的模块句柄，可以为nullptr
    */
    HMODULE hResModule = nullptr;

    /** Zip压缩包的资源名称，由 MAKEINTRESOURCE 宏生成，参见FindResource函数
    */
    LPCTSTR resourceName = _T("");

    /** Zip压缩包的资源类型，自定义导入的资源类型名称，参见FindResource函数
    */
    LPCTSTR resourceType = _T("");

    /** 可选参数：可以设置压缩包的解压密码；如果压缩包无密码，不需要设置
    *   注意事项：需要使用"ZIP legacy encryption"模式的密码，否则无法解压
    */
    DString zipPassword;
};

#endif

} // namespace ui

#endif // UI_CORE_RESOURCE_PARAM_H_
