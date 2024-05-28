#ifndef UI_CORE_ZIPMANAGER_H_
#define UI_CORE_ZIPMANAGER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <string>
#include <vector>
#include <unordered_set>

namespace ui 
{
/** ZIP压缩包管理器
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
    bool OpenResZip(HMODULE hModule, LPCTSTR resourceName, LPCTSTR resourceType, const std::string& password);
#endif

    /** 打开一个本地文件压缩包资源
     * @param[in] path 压缩包文件路径
     * @param[in] password 压缩包密码
     */
    bool OpenZipFile(const std::wstring& path, const std::string& password);

    /** 获取压缩包中的内容到内存
     * @param[in] path 要获取的文件的路径
     * @param[out] file_data 要获取的文件的路径
     */
    bool GetZipData(const std::wstring& path, std::vector<unsigned char>& file_data) const;

    /** 获取文件在压缩包中的位置
     * @param[in] path 要获取的文件路径
     * @return 返回在压缩包中的文件位置
     */
    std::wstring GetZipFilePath(const std::wstring& path) const;

    /** 判断资源是否存在zip当中
     * @param[in] path 要判断的资源路径
     */
    bool IsZipResExist(const std::wstring& path) const;

    /** 获取指定目录下的文件名称列表
    * @param [in] path 目录名称
    * @param [out] fileList 返回该目录下的文件列表
    */
    bool GetZipFileList(const std::wstring& path, std::vector<std::wstring>& fileList) const;

    /** 关闭压缩包
    */
    void CloseResZip();

private:
    
    /** 打开的压缩包句柄
    */
    void* m_hzip;

    /** 路径缓存
    */
    mutable std::unordered_set<std::wstring> m_zipPathCache;
};

}
#endif //UI_CORE_ZIPMANAGER_H_
