#ifndef UI_CORE_ZIPMANAGER_H_
#define UI_CORE_ZIPMANAGER_H_

#include "duilib/Utils/FilePath.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>

namespace ui 
{
class ZipStreamIO;

/**ZIP压缩包管理器
 * 说明：
 * （1）Zip压缩包支持的压缩算法是：Deflate算法，其他算法均不支持(也不支持Deflate64算法)
 * （2）使用7-Zip做压缩包的时候，如果自定义参数：cu=on，可以制作出文件名编码为UTF-8的压缩包；若不设置，默认文件名编码是本机编码
 * （3）如果设置了密码，需要使用传统的密码加密算法，否则无法解压。（使用"ZIP legacy encryption"模式 或者 "ZipCrypto"算法的密码）
 */
class UILIB_API ZipManager
{
public:
    ZipManager();
    ~ZipManager();
    ZipManager(const ZipManager&) = delete;
    ZipManager& operator = (const ZipManager&) = delete;

public:
    /** 判断当前是否使用了 zip 压缩包
     * @return 返回 true 表示使用了 zip 压缩包作为资源，false 为普通目录模式
     */
    bool IsUseZip() const;

#ifdef DUILIB_BUILD_FOR_WIN
    /** 打开一个内存压缩包资源
     * @param[in] hModule 资源句柄，如果为nullptr，代表当前可执行程序的模块句柄
     * @param[in] resourceName 资源名称，由 MAKEINTRESOURCE 宏生成
     * @param[in] resourceType 资源类型，自定义导入的资源类型名称
     * @param[in] password 压缩包密码
     */
    bool OpenResZip(HMODULE hModule, LPCTSTR resourceName, LPCTSTR resourceType, const DString& password);
#endif

    /** 打开一个本地文件压缩包资源
     * @param[in] path 压缩包文件路径
     * @param[in] password 压缩包密码
     */
    bool OpenZipFile(const FilePath& path, const DString& password);

    /** 获取压缩包中的内容到内存
     * @param [in] path 要获取的文件的路径(压缩包内路径)
     * @param [out] fileData 要获取的文件的路径
     */
    bool GetZipData(const FilePath& path, std::vector<unsigned char>& fileData) const;

    /** 判断资源是否存在zip当中
     * @param[in] path 要判断的资源路径(压缩包内路径)
     */
    bool IsZipResExist(const FilePath& path) const;

    /** 获取指定目录下的文件名称列表
    * @param [in] dirPath 目录名称(压缩包内路径)
    * @param [out] fileList 返回该目录下的文件列表
    */
    bool GetZipFileList(const FilePath& dirPath, std::vector<DString>& fileList) const;

    /** 关闭压缩包
    */
    void CloseResZip();

private:
    /** 对Zip格式的路径进行规范化处理（'\\'替换成'/'）
    */
    void NormalizeZipFilePath(std::string& innerFilePath) const;
    void NormalizeZipFilePath(std::wstring& innerFilePath) const;

    /** 在压缩包中定位文件
    * @param [in] normalizePath 文件路径
    * @param [out] filePathA 返回压缩包内的文件路径（MBCS或者UTF8编码）
    * @return 定位成功返回true，否则返回false
    */
    bool LocateFile(const FilePath& normalizePath, std::string& filePathA) const;

    /** 获取压缩包内的路径(转换字符串编码)
    * @param [in] szInZipFilePath 要获取的文件路径(压缩包内路径)
    * @param [in] bUtf8 true表示UTF8编码，否则为Ansi编码
    */
    DString GetZipFilePath(const char* szInZipFilePath, bool bUtf8) const;

private:
    
    /** 打开的压缩包句柄
    */
    void* m_hzip;

    /** 压缩包的解压密码
    */
    DString m_password;

    /** 内存流的读取接口
    */
    std::unique_ptr<ZipStreamIO> m_pZipStreamIO;

    /** 路径缓存
    */
    mutable std::unordered_set<DStringW> m_zipPathCache;
};

}
#endif //UI_CORE_ZIPMANAGER_H_
