#ifndef UI_CORE_IMAGEDECODE_H_
#define UI_CORE_IMAGEDECODE_H_

#pragma once

#include "duilib/Render/IRender.h"
#include "duilib/Core/UiTypes.h"
#include <memory>
#include <string>
#include <map>
#include <vector>

namespace ui 
{
	class IRender;
	class Control;

/** 图片信息
*/
class UILIB_API ImageInfo
{
public:
	ImageInfo();
	~ImageInfo();

	ImageInfo(const ImageInfo&) = delete;
	ImageInfo& operator = (const ImageInfo&) = delete;

public:
	/** 设置图片路径
	*/
	void SetImageFullPath(const std::wstring& path);

	/** 获取图片路径
	*/
	std::wstring GetImageFullPath() const;

	/** 设置该图片的大小是否已经做过适应DPI处理
	*/
	void SetBitmapSizeDpiScaled(bool isDpiScaled) { m_bDpiScaled = isDpiScaled; }

	/** 判断该图片的大小是否已经做过适应DPI处理
	*/
	bool IsBitmapSizeDpiScaled() const { return m_bDpiScaled; }

	/** 设置图片的宽和高
	*/
	void SetImageSize(int32_t nWidth, int32_t nHeight);

	/** 获取图片宽度
	*/
	int32_t GetWidth() const { return m_nWidth; }

	/** 获取图片高度
	*/
	int32_t GetHeight() const { return m_nHeight; }

	/** 添加一个图片帧数据, 添加后该资源由该类内部托管
	*/
	void SetFrameBitmap(const std::vector<IBitmap*>& frameBitmaps);

	/** 获取一个图片帧数据
	*/
	IBitmap* GetBitmap(uint32_t nIndex) const;

	/** 设置图片的多帧播放事件间隔（毫秒为单位 ）
	*/
	void SetFrameInterval(const std::vector<int32_t>& frameIntervals);

	/** 获取图片帧对应的播放时间间隔（毫秒为单位 ）
	*/
	int32_t GetFrameInterval(uint32_t nIndex) const;

	/** 获取图片的帧数
	*/
	uint32_t GetFrameCount() const;

	/** 是否位多帧图片(比如GIF等)
	*/
	bool IsMultiFrameImage() const;

	/** 设置循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
	*/
	void SetPlayCount(int32_t nPlayCount);

	/** 获取循环播放次数
	*@return 返回值：-1 表示未设置
	*               0  表示动画是一致循环播放的
	*              > 0 表示动画循环播放的具体次数
	*/
	int32_t GetPlayCount() const;

	/** 设置图片的缓存KEY, 用于图片的生命周期管理
	*/
	void SetCacheKey(const std::wstring& cacheKey);

	/** 获取图片的缓存KEY
	*/
	std::wstring GetCacheKey() const;

private:
	//图片的完整路径
	UiString m_imageFullPath;

	//该图片的大小是否已经做过适应DPI处理
	bool m_bDpiScaled;

	//图片的宽度
	int32_t m_nWidth;
	
	//图片的高度
	int32_t m_nHeight;

	//图片帧对应的播放时间间隔（毫秒为单位 ）
	std::vector<int32_t>* m_pFrameIntervals;

	//图片帧数据
	IBitmap** m_pFrameBitmaps;

	//图片帧数量
	uint32_t m_nFrameCount;

	//循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
	int32_t m_nPlayCount;

	/** 图片的缓存KEY, 用于图片的生命周期管理
	*/
	UiString m_cacheKey;
};

/** 图片属性
*/
class UILIB_API ImageAttribute
{
public:
	ImageAttribute();
	~ImageAttribute();
	ImageAttribute(const ImageAttribute&);
	ImageAttribute& operator=(const ImageAttribute&);

	/** 对数据成员进行初始化
	*/
	void Init();

	/** 根据图片参数进行初始化(先调用Init初始化成员变量，再按照传入参数进行更新部分属性)
	* @param [in] strImageString 图片参数字符串
	*/
	void InitByImageString(const std::wstring& strImageString);

	/** 根据图片参数修改属性值（仅更新新设置的图片属性, 未包含的属性不进行更新）
	* @param [in] strImageString 图片参数字符串
	*/
	void ModifyAttribute(const std::wstring& strImageString);

public:
	/** 判断rcDest区域是否含有有效值
	* @param [in] rcDest 需要判断的区域
	*/
	static bool HasValidImageRect(const UiRect& rcDest);

	/** 对图片的源区域、目标区域、圆角大小进行校验修正和DPI自适应
	* @param [in] imageWidth 图片的宽度
	* @param [in] imageHeight 图片的高度
	* @param [in] bImageDpiScaled 图片是否做过DPI自适应操作
	* @param [out] rcDestCorners 绘制目标区域的圆角信息，传出参数，内部根据rcImageCorners来设置，然后传出
	* @param [in/out] rcSource 图片区域
	* @param [in/out] rcSourceCorners 图片区域的圆角信息
	*/
	static void ScaleImageRect(uint32_t imageWidth, uint32_t imageHeight, bool bImageDpiScaled,
		                       UiRect& rcDestCorners,
		                       UiRect& rcSource, UiRect& rcSourceCorners);

public:
	/** 获取rcSource
	*/
	UiRect GetSourceRect() const;

	/** 获取rcDest
	*/
	UiRect GetDestRect() const;

	/** rcPadding;
	*/
	UiPadding GetPadding() const;

	/** 获取rcCorner;
	*/
	UiRect GetCorner() const;

	/** 设置图片属性的内边距(内部不做DPI自适应)
	*/
	void SetPadding(const UiPadding& newPadding);

public:
	//图片文件属性字符串
	UiString sImageString;

	//图片文件文件名，含相对路径，不包含属性
	UiString sImagePath;

	//设置图片宽度，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
	UiString srcWidth;

	//设置图片高度，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
	UiString srcHeight;

	//加载图片时，DPI自适应属性，即按照DPI缩放图片大小
	bool srcDpiScale;

	//加载图片时，是否设置了DPI自适应属性
	bool bHasSrcDpiScale;

	//在绘制目标区域中横向对齐方式(如果指定了rcDest值，则此选项无效)
	UiString hAlign;

	//在绘制目标区域中纵向对齐方式(如果指定了rcDest值，则此选项无效)
	UiString vAlign;

	//透明度（0 - 255）
	uint8_t bFade;

	//横向平铺
	bool bTiledX;

	//横向完全平铺，仅当bTiledX为true时有效
	bool bFullTiledX;

	//纵向平铺
	bool bTiledY;

	//纵向完全平铺，仅当bTiledY为true时有效
	bool bFullTiledY;

	//平铺时的边距（仅当bTiledX为true或者bTiledY为true时有效）
	int32_t nTiledMargin;

	//如果是GIF等动画图片，可以指定播放次数 -1 ：一直播放，缺省值。
	int32_t nPlayCount;	

	//如果是ICO文件，用于指定需要加载的ICO图片的大小
	//(ICO文件中包含很多个不同大小的图片，常见的有256，48，32，16，并且每个大小都有32位真彩、256色、16色之分）
	//目前ICO文件在加载时，只会选择一个大小的ICO图片进行加载，加载后为单张图片
	uint32_t iconSize;

	//可绘制标志：true表示允许绘制，false表示禁止绘制
	bool bPaintEnabled;

private:
	//绘制目标区域位置和大小（相对于控件区域的位置）
	UiRect* rcDest;

	//在绘制目标区域中的内边距(如果指定了rcDest值，则此选项无效)
	UiPadding16* rcPadding;

	//图片源区域位置和大小
	UiRect* rcSource;

	//圆角属性
	UiRect* rcCorner;
};

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

/** 图片相关封装，支持的文件格式：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO
*/
class UILIB_API Image
{
public:
	Image();
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

	/** 初始化图片属性
	*/
	void InitImageAttribute();

	/** 设置并初始化图片属性
	*@param [in] strImageString 图片属性字符串
	*/
	void SetImageString(const std::wstring& strImageString);

	/** 获取图片属性（含文件名，和图片设置属性等）
	*/
	std::wstring GetImageString() const;

	/** 获取图片文件名（含相对路径，不含图片属性）
	*/
	std::wstring GetImagePath() const;

	/** 设置图片属性的内边距
	*/
	void SetImagePadding(const UiPadding& newPadding);

	/** 获取图片属性的内边距
	*/
	UiPadding GetImagePadding() const;

	/** 判断是否禁用图片绘制
	*/
	bool IsImagePaintEnabled() const;

	/** 设置是否禁止背景图片绘制
	*/
	void SetImagePaintEnabled(bool bEnable);

	/** 获取图片属性（只读）
	*/
	const ImageAttribute& GetImageAttribute() const;

	/** 获取图片加载属性
	*/
	ImageLoadAttribute GetImageLoadAttribute() const;

public:
	/** 获取图片信息接口
	*/
	const std::shared_ptr<ImageInfo>& GetImageCache() const;

	/** 设置图片信息接口
	*/
	void SetImageCache(const std::shared_ptr<ImageInfo>& imageInfo);

	/** 清除图片信息缓存数据, 释放资源
	*/
	void ClearImageCache();

public:
	/** 设置图片属性：播放次数（仅当多帧图片时）
	*/
	void SetImagePlayCount(int32_t nPlayCount);

	/** 设置图片属性：透明度（仅当多帧图片时）
	*/
	void SetImageFade(uint8_t nFade);

	/** 是否正在播放中（仅当多帧图片时）
	*/
	bool IsPlaying() const { return m_bPlaying; }

	/** 设置是否正在播放中（仅当多帧图片时）
	*/
	void SetPlaying(bool bPlaying) { m_bPlaying = bPlaying; }

	/** 跳到下一帧（仅当多帧图片时）
	*/
	bool IncrementCurrentFrame();

	/** 设置当前图片帧（仅当多帧图片时）
	*/
	void SetCurrentFrame(uint32_t nCurrentFrame);

	/** 获取当前图片帧索引（仅当多帧图片时）
	*/
	uint32_t GetCurrentFrameIndex() const;

	/** 获取当前图片帧的图片数据
	*/
	IBitmap* GetCurrentBitmap() const;

	/** 获取当前图片帧播放的时间间隔（单位: 毫秒，仅当多帧图片时）
	*/
	int32_t GetCurrentInterval() const;

	/** 获取当前已循环播放的次数（仅当多帧图片时）
	*/
	int32_t GetCycledCount() const;

	/** 清空当前已循环播放的次数（仅当多帧图片时）
	*/
	void ClearCycledCount();

	/** 判断是否应该继续播放（仅当多帧图片时）
	*/
	bool ContinuePlay() const;

private:

	//当前正在播放的图片帧（仅当多帧图片时）
	uint32_t m_nCurrentFrame;

	//是否正在播放（仅当多帧图片时）
	bool m_bPlaying;

	//已播放次数（仅当多帧图片时）
	int32_t m_nCycledCount;

	//图片属性
	ImageAttribute m_imageAttribute;

	//图片信息
	std::shared_ptr<ImageInfo> m_imageCache;
};

/** 控件状态与图片的映射
*/
class UILIB_API StateImage
{
public:
	StateImage();

	/** 设置关联的控件接口
	*/
	void SetControl(Control* control) {	m_pControl = control; }

	/** 设置图片属性
	*@param [in] stateType 图片类型
	*@param [in] strImageString 图片属性字符串
	*/
	void SetImageString(ControlStateType stateType, const std::wstring& strImageString);

	/** 获取图片属性
	*@param [in] stateType 图片类型
	*/
	std::wstring GetImageString(ControlStateType stateType) const;

	/** 获取图片文件名
	*@param [in] stateType 图片类型
	*/
	std::wstring GetImagePath(ControlStateType stateType) const;

	/** 获取图片的源区域大小
	*@param [in] stateType 图片类型
	*/
	UiRect GetImageSourceRect(ControlStateType stateType) const;

	/** 获取图片的透明度
	*@param [in] stateType 图片类型
	*/
	int32_t GetImageFade(ControlStateType stateType) const;

	/** 获取图片接口(可读，可写)
	*/
	Image* GetStateImage(ControlStateType stateType);

public:
	/** 是否包含Hot状态的图片
	*/
	bool HasHotImage() const;

	/** 是否包含状态图片
	*/
	bool HasImage() const;

	/** 绘制指定状态的图片
	* @param [in] pRender 绘制接口
	* @param [in] stateType 控件状态，用于选择绘制哪个图片
	* @param [in] sImageModify 图片的附加属性
	* @param [out] pDestRect 返回图片绘制的最终目标矩形区域
	* @return 绘制成功返回true, 否则返回false
	*/
	bool PaintStateImage(IRender* pRender, ControlStateType stateType, 
					     const std::wstring& sImageModify = L"",
		                 UiRect* pDestRect = nullptr);

	/** 获取用于估算Control控件大小（宽和高）的图片接口
	*/
	Image* GetEstimateImage();

	/** 获取所有图片接口
	*/
	void GetAllImages(std::vector<Image*>& allImages);

	/** 清空图片缓存，释放资源
	*/
	void ClearImageCache();

private:
	//关联的控件接口
	Control* m_pControl;

	//每个状态的图片接口
	std::map<ControlStateType, Image> m_stateImageMap;
};

/** 控件图片类型与状态图片的映射
*/
class UILIB_API StateImageMap
{
public:
	StateImageMap();

	/** 设置关联的控件接口
	*/
	void SetControl(Control* control);

	/** 设置图片属性
	*@param [in] stateImageType 图片类型，比如正常状态前景图片、背景图片；选择状态的前景图片、背景图片等
	*@param [in] stateType 图片状态，比如正常、焦点、按下、禁用状态等
	*@param [in] strImagePath 图片属性字符串
	*/
	void SetImageString(StateImageType stateImageType, ControlStateType stateType, const std::wstring& strImagePath);

	/** 获取图片属性
	*@param [in] stateImageType 图片类型，比如正常状态前景图片、背景图片；选择状态的前景图片、背景图片等
	*@param [in] stateType 图片状态，比如正常、焦点、按下、禁用状态等
	*/
	std::wstring GetImageString(StateImageType stateImageType, ControlStateType stateType) const;

	/** 是否含有Hot状态的图片
	*/
	bool HasHotImage() const;

	/** 是否状态图片
	*/
	bool HasStateImages(void) const;

	/** 是否含有指定类型的图片
	*/
	bool HasStateImage(StateImageType stateImageType) const;

	/** 绘制指定图片类型和状态的图片
	*/
	bool PaintStateImage(IRender* pRender, StateImageType stateImageType, ControlStateType stateType, const std::wstring& sImageModify = L"");
	
	/** 获取用于估算Control控件大小（宽和高）的图片接口
	*/
	Image* GetEstimateImage(StateImageType stateImageType);

	/** 获取指定图片类型和状态的图片接口
	*/
	Image* GetStateImage(StateImageType stateImageType, ControlStateType stateType);

	/** 获取所有图片接口
	*/
	void GetAllImages(std::vector<Image*>& allImages);

	/** 清除所有图片类型的缓存，释放资源
	*/
	void ClearImageCache();

private:
	//关联的控件接口
	Control* m_pControl;

	//每个图片类型的状态图片(正常状态前景图片、背景图片；选择状态的前景图片、背景图片)
	std::map<StateImageType, StateImage> m_stateImageMap;
};

/** 控件状态与颜色值的映射
*/
class UILIB_API StateColorMap
{
public:
	StateColorMap();

	/** 设置管理的控件接口
	*/
	void SetControl(Control* control);

	/** 获取颜色值，如果不包含此颜色，则返回空
	*/
	std::wstring GetStateColor(ControlStateType stateType) const;

	/** 设置颜色值
	*/
	void SetStateColor(ControlStateType stateType, const std::wstring& color);

	/** 是否包含Hot状态的颜色
	*/
	bool HasHotColor() const;

	/** 是否含有指定颜色值
	*/
	bool HasStateColor(ControlStateType stateType) const;

	/** 是否含有颜色值
	*/
	bool HasStateColors() const ;

	/** 绘制指定状态的颜色
	*/
	void PaintStateColor(IRender* pRender, UiRect rcPaint, ControlStateType stateType) const;

private:
	//关联的控件接口
	Control* m_pControl;

	//状态与颜色值的映射表
	std::map<ControlStateType, UiString> m_stateColorMap;
};

} // namespace ui

#endif // UI_CORE_IMAGEDECODE_H_
