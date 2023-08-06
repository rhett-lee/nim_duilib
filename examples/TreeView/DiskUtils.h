#ifndef __DiskUtils_h_
#define __DiskUtils_h_

#include "duilib/duilib_config.h"
#include <string>
#include <vector>

/** 磁盘分区信息辅助类
*/
class DiskUtils
{
public:

    /** 磁盘分区的详细信息
    */
    struct DiskInfo
    { 
        /** 分区卷标
        */
        std::wstring m_volumeName;
        
        /** 分区类型
        */
        std::wstring m_volumeType;
        
        /** 分区类型，如NTFS等
        */
        std::wstring m_fileSystem;
        
        /** 簇大小
        */
        uint32_t m_clusterBytes;
        
        /** 总大小
        */
        uint64_t m_totalBytes;
        
        /** 可用空间大小
        */
        uint64_t m_freeBytes;
        
        /** 是否含有文件系统
        */
        bool m_hasFileSystem;
    };
    
public:
    
    /** 获取本地磁盘列表
    @param [out] 返回本地磁盘的列表，磁盘格式如："C:\\"等
    @return 成功返回true，否则返回false
    */
    static bool GetLogicalDriveList(std::vector<std::wstring>& driveList);
    
    /** 获取分区信息
    @param [in] driveString 驱动器名称，格式如："C:\\"
    @param [out] diskInfo 返回磁盘对应信息
    @return 成功返回true，否则返回false
    */
    static bool GetLogicalDriveInfo(const std::wstring& driveString, DiskInfo& diskInfo);

    /** 根据路径得到当前的盘符
    @param [in] path 当前路径
    @return 盘符名称
    */
    static std::wstring GetDriveFromDirectoryPath(std::wstring path);

    /** 获取剩余空间最大的本地磁盘的盘符
    @return 返回磁盘的盘符， 比如: "C:\\"
    */
    static std::wstring GetMaxFreeSpaceLocalDisk();

    /** 获取指定目录的剩余磁盘空间
    @param[in] fullDirectory 文件目录
    @return 剩余磁盘空间大小(BYTE)
    */
    static uint64_t GetFreeDiskSpace(const std::wstring& fullDirectory);
};

#endif //__DiskUtils_h_
