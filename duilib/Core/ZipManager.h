#ifndef UI_CORE_ZIPMANAGER_H_
#define UI_CORE_ZIPMANAGER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <string>
#include <vector>

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
	bool IsUseZip();

	/** 打开一个内存压缩包资源
	 * @param[in] resourceName 资源名称，由 MAKEINTRESOURCE 宏生成
	 * @param[in] resourceType 资源类型，自定义导入的资源类型名称
	 * @param[in] password 压缩包密码
	 */
	bool OpenResZip(LPCTSTR resourceName, LPCTSTR resourceType, const std::string& password);

	/** 打开一个本地文件压缩包资源
	 * @param[in] path 压缩包文件路径
	 * @param[in] password 压缩包密码
	 */
	bool OpenResZip(const std::wstring& path, const std::string& password);

	/** 获取压缩包中的内容到内存
	 * @param[in] path 要获取的文件的路径
	 * @param[out] file_data 要获取的文件的路径
	 */
	bool GetZipData(const std::wstring& path, std::vector<unsigned char>& file_data);

	/** 获取文件在压缩包中的位置
	 * @param[in] path 要获取的文件路径
	 * @return 返回在压缩包中的文件位置
	 */
	std::wstring GetZipFilePath(const std::wstring& path);

	/** 判断资源是否存在zip当中
	 * @param[in] path 要判断的资源路径
	 */
	bool IsZipResExist(const std::wstring& path);

	/** 关闭压缩包
	*/
	void CloseResZip();

private:
	
	/** 打开的压缩包句柄
	*/
	void* m_hzip;
};

}
#endif //UI_CORE_ZIPMANAGER_H_
