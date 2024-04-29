#ifndef UI_CORE_IMAGEMANAGER_H_
#define UI_CORE_IMAGEMANAGER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace ui 
{
class ImageInfo;
class ImageLoadAttribute;

/** 图片管理器
 */
class UILIB_API ImageManager
{
public:
	ImageManager();
	~ImageManager();
	ImageManager(const ImageManager&) = delete;
	ImageManager& operator = (const ImageManager&) = delete;

public:
	/** 检查指定图片是否已经被缓存
	 * @param[in] loadAtrribute 图片的加载属性，包含图片路径等信息
	 * @return 如果已经被缓存，则返回 ImageInfo 的智能指针对象
	 */
	std::shared_ptr<ImageInfo> GetCachedImage(const ImageLoadAttribute& loadAtrribute);

	/** 加载图片 ImageInfo 对象
	 * @param[in] loadAtrribute 图片的加载属性，包含图片路径等信息
	 * @return 返回图片 ImageInfo 对象的智能指针
	 */
	std::shared_ptr<ImageInfo> GetImage(const ImageLoadAttribute& loadAtrribute);

	/** 从缓存中删除所有图片
	 */
	void RemoveAllImages();

	/** 设置是否默认对所有图片在加载时根据DPI进行缩放，这个是全局属性，默认为true，应用于所有图片
	   （设置为true后，也可以通过在xml中，使用"dpiscale='false'"属性关闭某个图片的DPI自动缩放）
	*/
	void SetDpiScaleAllImages(bool bEnable);

	/** 判断是否默认对所有图片在加载时根据DPI进行缩放
	*/
	bool IsDpiScaleAllImages();

private:
	/** 图片被销毁的回调函数，用于释放图片资源
	 * @param[in] pImageInfo 图片对应的 ImageInfo 对象
	 */
	static void OnImageInfoDestroy(ImageInfo* pImageInfo);

	/** 查找对应DPI下的图片，可以每个DPI设置一个图片，以提高不同DPI下的图片质量
	*/
	std::wstring GetDpiImageFullPath(const std::wstring& strImageFullPath, bool bIsUseZip) const;

	/** 从文件加载一个图片
	*/
	void LoadImageData(const ImageLoadAttribute& loadAtrribute,
		               bool& isDpiScaledImageFile,
		               std::unique_ptr<ImageInfo>& imageInfo) const;

#ifdef UILIB_IMPL_WINSDK
	/** 从HICON句柄加载一个图片
	*/
	void LoadIconData(const ImageLoadAttribute& loadAtrribute,
		              std::unique_ptr<ImageInfo>& imageInfo) const;
#endif

private:
	/** 设置是否默认对所有图片在加载时根据DPI进行缩放，这个是全局属性，默认为true，应用于所有图片
	   （设置为true后，也可以通过在xml中，使用"dpiscale='false'"属性关闭某个图片的DPI自动缩放）
	*/
	bool m_bDpiScaleAllImages;

	/** 图片资源映射表
	*/
	std::unordered_map<std::wstring, std::weak_ptr<ImageInfo>> m_imageMap;
};

}
#endif //UI_CORE_IMAGEMANAGER_H_
