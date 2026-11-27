#ifndef EXAMPLES_DISK_UTILS_WINDOWS_H_
#define EXAMPLES_DISK_UTILS_WINDOWS_H_

#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_WIN
#include <string>
#include <vector>

namespace ui
{

/** 磁盘分区信息辅助类（字符串编码类型：UTF-16或者UTF-8）
*   提供本地磁盘枚举、分区信息查询与剩余空间计算，仅在 Windows 平台可用。
*/
class DUILIB_API DiskUtils
{
public:

    /** 磁盘分区的详细信息
    */
    struct DiskInfo
    { 
        /** 分区卷标（可能为空字符串）
        */
        DString m_volumeName;
        
        /** 分区类型名称，如"本地磁盘"、"可移动磁盘"等
        */
        DString m_volumeType;
        
        /** 文件系统类型，如NTFS等；无文件系统时为空字符串
        */
        DString m_fileSystem;
        
        /** 簇大小（单位：字节）；获取失败时为0
        */
        uint32_t m_clusterBytes = 0;
        
        /** 总大小（单位：字节）
        */
        uint64_t m_totalBytes = 0;
        
        /** 可用空间大小（单位：字节）
        */
        uint64_t m_freeBytes = 0;
        
        /** 是否成功获取到文件系统信息（false 时簇大小与容量信息均为0）
        */
        bool m_hasFileSystem = false;
    };
    
public:
    
    /** 获取本地磁盘列表
    * @param [out] driveList 返回本地磁盘的列表，磁盘格式如："C:\\"等；结果追加到列表中，不清理原有内容
    * @return 成功返回true，否则返回false
    */
    static bool GetLogicalDriveList(std::vector<DString>& driveList);
    
    /** 获取分区信息
    * @param [in] driveString 驱动器名称，格式如："C:\\"
    * @param [out] diskInfo 返回磁盘对应信息；查询失败时不会被修改
    * @return 成功返回true，否则返回false
    */
    static bool GetLogicalDriveInfo(const DString& driveString, DiskInfo& diskInfo);

    /** 根据路径得到所在磁盘的盘符
    * @param [in] path 当前路径
    * @return 盘符名称，如 "C:\\"；路径不以盘符开头（长度不足或第二个字符不是':'）时返回空字符串
    */
    static DString GetDriveFromDirectoryPath(const DString& path);

    /** 获取剩余空间最大的本地磁盘的盘符（只统计固定磁盘）
    * @return 返回磁盘的盘符，比如: "C:\\"；没有可用本地磁盘时返回空字符串
    */
    static DString GetMaxFreeSpaceLocalDisk();

    /** 获取指定目录的剩余磁盘空间
    * @param [in] fullDirectory 文件或目录的完整路径
    * @return 剩余磁盘空间大小（单位：字节）；获取失败返回0
    */
    static uint64_t GetFreeDiskSpace(const DString& fullDirectory);
};

}//namespace ui

#endif //DUILIB_BUILD_FOR_WIN

#endif //EXAMPLES_DISK_UTILS_WINDOWS_H_
