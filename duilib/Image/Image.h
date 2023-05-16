#ifndef UI_CORE_IMAGEDECODE_H_
#define UI_CORE_IMAGEDECODE_H_

#pragma once

#include "duilib/Image/GdiPlusDefs.h"
#include "duilib/Core/Define.h"
#include "duilib/duilib_defs.h"
#include <memory>
#include <string>
#include <map>
#include <vector>

namespace ui 
{
	class IRenderContext;

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
	const std::wstring& GetImageFullPath() const;

	/** 设置是否有Alpha通道
	*/
	void SetAlpha(bool bAlphaChannel) {	m_bAlphaChannel = bAlphaChannel; }

	/** 判断是否有Alpha通道
	*/
	bool IsAlpha() const { return m_bAlphaChannel; }
	
	/** 设置是否保存在缓存中
	*/
	void SetCached(bool bCached) { m_bCached = bCached; }

	/** 判断是否保存在缓存中
	*/
	bool IsCached() const { return m_bCached; }

	/** 设置该图片的大小是否已经做过适应DPI处理
	*/
	void SetBitmapSizeDpiScaled(bool isDpiScaled) { m_bDpiScaled = isDpiScaled; }

	/** 判断该图片的大小是否已经做过适应DPI处理
	*/
	bool IsBitmapSizeDpiScaled() const { return m_bDpiScaled; }

	/** 添加一个图片帧数据
	*/
	void PushBackHBitmap(HBITMAP hBitmap);

	/** 获取一个图片帧数据
	*/
	HBITMAP GetHBitmap(size_t nIndex) const;

	/** 设置图片的宽和高
	*/
	void SetImageSize(int nWidth, int nHeight);

	/** 获取图片宽度
	*/
	int GetWidth() const { return m_nWidth; }

	/** 获取图片高度
	*/
	int GetHeight() const { return m_nHeight; }

	/** 获取图片的帧数
	*/
	size_t GetFrameCount() const;

	/** 是否位多帧图片(比如GIF等)
	*/
	bool IsMultiFrameImage() const;

	/** 设置图片的多帧播放事件间隔（毫秒为单位 ）
	*/
	void SetFrameInterval(const std::vector<int>& frameIntervals);

	/** 获取图片帧对应的播放时间间隔（毫秒为单位 ）
	*/
	int GetFrameInterval(size_t nIndex);

private:
	//图片的完整路径
	std::wstring m_imageFullPath;

	//是否包含Alpha通道
	bool m_bAlphaChannel;

	//是否保存在缓存中
	bool m_bCached;

	//该图片的大小是否已经做过适应DPI处理
	bool m_bDpiScaled;

	//图片的宽度
	int m_nWidth;
	
	//图片的高度
	int m_nHeight;

	//图片帧对应的播放时间间隔（毫秒为单位 ）
	std::vector<int> m_frameIntervals;

	//图片帧数据
	std::vector<HBITMAP> m_frameBitmaps;
};

/** 图片属性
*/
class UILIB_API ImageAttribute
{
public:
	ImageAttribute();

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
	//图片文件属性字符串
	std::wstring simageString;

	//图片文件名，不包含属性
	std::wstring sImageName;

	//绘制目标区域大小位置
	UiRect rcDest;

	//图片源区域大小和位置
	UiRect rcSource;

	//圆角属性
	UiRect rcCorner;

	//透明度（0 - 255）
	BYTE bFade;

	//横向平铺
	bool bTiledX;

	//横向完全平铺，未用到
	bool bFullTiledX;

	//纵向平铺
	bool bTiledY;

	//横向完全平铺
	bool bFullTiledY;

	//平铺时的边距
	int nTiledMargin;

	//如果是GIF等动画图片，可以指定播放次数 -1 ：一直播放，缺省值。
	int nPlayCount;	
};

/** 图片相关封装
*/
class UILIB_API Image
{
public:
	Image();

	/** 判断图片信息是否有效
	*/
	bool IsValid() const { return (m_imageCache.get() != nullptr); }

	/** 初始化图片属性
	*/
	void InitImageAttribute();

	/** 设置图片属性
	*@param [in] strImageString 图片属性字符串
	*/
	void SetImageString(const std::wstring& strImageString);

	/** 清除图片信息缓存数据
	*/
	void ClearCache();

	/** 获取图片属性（只读）
	*/
	const ImageAttribute& GetImageAttribute() const;

	/** 获取图片信息接口
	*/
	const std::shared_ptr<ImageInfo>& GetImageCache() const;

	/** 设置图片信息接口
	*/
	void SetImageCache(const std::shared_ptr<ImageInfo>& imageInfo);

public:
	/** 设置图片属性：播放次数（仅当多帧图片时）
	*/
	void SetImagePlayCount(int nPlayCount);

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
	void SetCurrentFrame(size_t nCurrentFrame);

	/** 获取当前图片帧索引（仅当多帧图片时）
	*/
	size_t GetCurrentFrameIndex() const;

	/** 获取当前图片帧的图片数据
	*/
	HBITMAP GetCurrentHBitmap() const;

	/** 获取当前图片帧播放的时间间隔（单位: 毫秒，仅当多帧图片时）
	*/
	int GetCurrentInterval() const;

	/** 获取当前已循环播放的次数（仅当多帧图片时）
	*/
	int GetCycledCount() const;

	/** 清空当前已循环播放的次数（仅当多帧图片时）
	*/
	void ClearCycledCount();

	/** 判断是否应该继续播放（仅当多帧图片时）
	*/
	bool ContinuePlay() const;

private:

	//当前正在播放的图片帧（仅当多帧图片时）
	size_t m_nCurrentFrame;

	//是否正在播放（仅当多帧图片时）
	bool m_bPlaying;

	//已播放次数（仅当多帧图片时）
	int m_nCycledCount;

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
	std::wstring GetImageFilePath(ControlStateType stateType) const;

	/** 获取图片的源区域大小
	*@param [in] stateType 图片类型
	*/
	UiRect GetImageSourceRect(ControlStateType stateType) const;

	/** 获取图片的透明度
	*@param [in] stateType 图片类型
	*/
	int GetImageFade(ControlStateType stateType) const;

	/** 获取图片接口(可读，可写)
	*/
	Image& GetStateImage(ControlStateType stateType) { return m_stateImageMap[stateType]; }

public:
	/** 是否包含Hot状态的图片
	*/
	bool HasHotImage() const;

	/** 是否包含状态图片
	*/
	bool HasImage() const;

	/** 绘制指定状态的图片
	*/
	bool PaintStateImage(IRenderContext* pRender, ControlStateType stateType, const std::wstring& sImageModify = L"");

	/** 获取用于估算的图片接口
	*/
	Image* GetEstimateImage() ;

	/** 清空图片缓存，释放资源
	*/
	void ClearCache();

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
	StateImageMap()	{ }

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

	/** 是否含有指定类型的图片
	*/
	bool HasImageType(StateImageType stateImageType) const;

	/** 绘制指定图片类型和状态的图片
	*/
	bool PaintStateImage(IRenderContext* pRender, StateImageType stateImageType, ControlStateType stateType, const std::wstring& sImageModify = L"");
	
	/** 获取用于估算的图片接口
	*/
	Image* GetEstimateImage(StateImageType stateImageType);

	/** 清除所有图片类型的缓存，释放资源
	*/
	void ClearCache();

private:
	//每个图片类型的状态图片
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
	void PaintStateColor(IRenderContext* pRender, UiRect rcPaint, ControlStateType stateType) const;

private:
	//关联的控件接口
	Control* m_pControl;

	//状态与颜色值的映射表
	std::map<ControlStateType, std::wstring> m_stateColorMap;
};

} // namespace ui

#endif // UI_CORE_IMAGEDECODE_H_
