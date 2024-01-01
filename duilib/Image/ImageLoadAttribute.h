#ifndef UI_IMAGE_IMAGE_LOAD_ATTRIBUTE_H_
#define UI_IMAGE_IMAGE_LOAD_ATTRIBUTE_H_

#pragma once

#include "duilib/Core/UiTypes.h"

namespace ui 
{
class Control;

/** 图片加载属性，用于加载一个图片
*/
class UILIB_API ImageLoadAttribute
{
public:
	ImageLoadAttribute(std::wstring srcWidth,
					   std::wstring srcHeight,
		               bool srcDpiScale,
		               bool bHasSrcDpiScale,
		               uint32_t iconSize);

	/** 设置图片路径（本地绝对路径或者压缩包内的相对路径）
	*/
	void SetImageFullPath(const std::wstring& imageFullPath);

	/** 获取图片路径（本地绝对路径或者压缩包内的相对路径）
	*/
	std::wstring GetImageFullPath() const;

	/** 获取加载图片的缓存KEY
	*/
	std::wstring GetCacheKey() const;

	/** 设置加载图片时，是否需要按照DPI缩放图片大小
	*/
	void SetNeedDpiScale(bool bNeedDpiScale);

	/** 获取加载图片时，是否需要按照DPI缩放图片大小
	*/
	bool NeedDpiScale() const;

	/** 获取加载图片时，是否设置了DPI自适应属性（配置XML文件中，可以通过设置："file='test.png' dpiscale='false'"）
	*/
	bool HasSrcDpiScale() const;

	/** 获取图片加载后应当缩放的宽度和高度
	* @param [in,out] nImageWidth 传入原始图片的宽度，返回计算后的宽度
	* @param [in,out] nImageHeight 原始图片的高度，返回计算后的高度
	* @return 返回true表示图片大小有缩放，返回false表示图片大小无缩放
	*/
	bool CalcImageLoadSize(uint32_t& nImageWidth, uint32_t& nImageHeight) const;

	/** 如果是ICO文件，用于指定需要加载的ICO图片的大小
	*/
	uint32_t GetIconSize() const;

private:
	/** 获取设置的缩放后的大小
	*/
	uint32_t GetScacledSize(const std::wstring& srcSize, uint32_t nImageSize) const;

private:
	//本地绝对路径或者压缩包内的相对路径，不包含属性
	UiString m_srcImageFullPath;

	//设置图片宽度，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
	UiString m_srcWidth;

	//设置图片高度，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
	UiString m_srcHeight;

	//加载图片时，按照DPI缩放图片大小
	bool m_srcDpiScale;

	//加载图片时，是否设置了DPI自适应属性
	bool m_bHasSrcDpiScale;

	//如果是ICO文件，用于指定需要加载的ICO图片的大小
	//(ICO文件中包含很多个不同大小的图片，常见的有256，48，32，16，并且每个大小都有32位真彩、256色、16色之分）
	//目前ICO文件在加载时，只会选择一个大小的ICO图片进行加载，加载后为单张图片
	uint32_t m_iconSize;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_LOAD_ATTRIBUTE_H_
