#ifndef UI_CORE_ZIPMANAGER_H_
#define UI_CORE_ZIPMANAGER_H_

#pragma once

#include "duilib/duilib_defs.h"
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

#ifdef DUILIB_PLATFORM_WIN
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
    bool OpenZipFile(const DString& path, const DString& password);

    /** 获取压缩包中的内容到内存
     * @param [in] path 要获取的文件的路径
     * @param [out] fileData 要获取的文件的路径
     */
    bool GetZipData(const DString& path, std::vector<unsigned char>& fileData) const;

    /** 判断资源是否存在zip当中
     * @param[in] path 要判断的资源路径
     */
    bool IsZipResExist(const DString& path) const;

    /** 获取指定目录下的文件名称列表
    * @param [in] path 目录名称
    * @param [out] fileList 返回该目录下的文件列表
    */
    bool GetZipFileList(const DString& path, std::vector<DString>& fileList) const;

    /** 关闭压缩包
    */
    void CloseResZip();

private:
    /** 获取文件在压缩包中的位置
     * @param [in] path 要获取的文件路径
     * @return 返回在压缩包中的文件位置
     */
    DString GetZipFilePath(const DString& path) const;

    /** 获取压缩包内的路径(包内编码格式的)
    */
    std::string GetZipFilePathA(const DString& path, bool bUtf8) const;

    /** 获取压缩包内的路径(转换字符串编码)
    */
    DString GetZipFilePath(const char* szInZipFilePath, bool bUtf8) const;

    /** 打开压缩包后初始化文件名的编码格式
    */
    void InitUtf8();

private:
    
    /** 打开的压缩包句柄
    */
    void* m_hzip;

    /** 压缩包的解压密码
    */
    DString m_password;

    /** 压缩包的文件名编码是否为UTF8格式的
    */
    bool m_bUtf8;

    /** 内存流的读取接口
    */
    std::unique_ptr<ZipStreamIO> m_pZipStreamIO;

    /** 路径缓存
    */
    mutable std::unordered_set<std::string> m_zipPathCache;
};

}
#endif //UI_CORE_ZIPMANAGER_H_
