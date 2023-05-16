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

class UILIB_API ImageInfo
{
public:
	ImageInfo();
	~ImageInfo();

	ImageInfo(const ImageInfo&) = delete;
	ImageInfo& operator = (const ImageInfo&) = delete;

	void SetAlpha(bool bAlphaChannel) {	m_bAlphaChannel = bAlphaChannel; }
	bool IsAlpha() { return m_bAlphaChannel; }
	void SetCached(bool bCached) { m_bCached = bCached; }
	bool IsCached() { return m_bCached; }
	void SetSvg(bool svg) { m_bSvg = svg; }
	bool IsSvg() { return m_bSvg; }

	void SetPropertyItem(Gdiplus::PropertyItem* pPropertyItem);

	void PushBackHBitmap(HBITMAP hBitmap);
	HBITMAP GetHBitmap(int nIndex);
	int GetFrameCount();
	bool IsGif();
	int GetInterval(int nIndex); //毫秒为单位 
	 
	static std::unique_ptr<ImageInfo> LoadImage(const std::wstring& strImageFullPath);
	static std::unique_ptr<ImageInfo> LoadImage(std::vector<unsigned char>& file_data, const std::wstring& strImageFullPath);

private:
	static std::unique_ptr<ImageInfo> LoadImageByBitmap(std::unique_ptr<Gdiplus::Bitmap>& pGdiplusBitmap, const std::wstring& imageFullPath);

public:
	int nX = 0;
	int nY = 0;
	std::wstring sImageFullPath;

private:
	bool m_bAlphaChannel = false;
	bool m_bCached = false;
	bool m_bSvg = false;
	std::unique_ptr<Gdiplus::PropertyItem> m_propertyItem;
	std::vector<HBITMAP> m_vecBitmap;
};

struct UILIB_API ImageAttribute
{
public:
	ImageAttribute();

	void Init();
	void SetImageString(const std::wstring& strImageString);
	static void ModifyAttribute(ImageAttribute& imageAttribute, const std::wstring& strImageString);

	std::wstring simageString;
	std::wstring sImageName;
	UiRect rcDest;
	UiRect rcSource;
	UiRect rcCorner;
	BYTE bFade;
	bool bTiledX;
	bool bFullTiledX;
	bool bTiledY;
	bool bFullTiledY;
	int nTiledMargin;
	int nPlayCount;//如果是GIF可以指定播放次数 -1 ：一直播放，缺省值。
};

class UILIB_API Image
{
public:
	Image();

	bool IsValid() { return (m_imageCache.get() != nullptr); }
	bool IsPlaying() { return m_bPlaying; }
	void SetPlaying(bool bPlaying) { m_bPlaying = bPlaying; }

	void SetImageString(const std::wstring& strImageString);
	void ClearCache();

	bool IncrementCurrentFrame();
	void SetCurrentFrame(int nCurrentFrame);
	HBITMAP GetCurrentHBitmap();
	int GetCurrentInterval();
	int GetCurrentFrameIndex();
	int GetCycledCount();
	void ClearCycledCount();
	bool ContinuePlay();

	const ImageAttribute& GetImageAttribute() const;
	ImageAttribute& SetImageAttribute();

	const std::shared_ptr<ImageInfo>& GetImageCache() const;
	void SetImageCache(const std::shared_ptr<ImageInfo>& imageInfo);

private:
	int m_nCurrentFrame;
	bool m_bPlaying;
	int m_nCycledCount;//播放次数

	ImageAttribute m_imageAttribute;
	std::shared_ptr<ImageInfo> m_imageCache;
};

/** 控件状态与图片的映射
*/
class UILIB_API StateImage
{
public:
	StateImage();

	void SetControl(Control* control) {	m_pControl = control; }
	Image& operator[](ControlStateType stateType) {	return m_stateImageMap[stateType]; }

	bool HasHotImage();
	bool HasImage();
	bool PaintStatusImage(IRenderContext* pRender, ControlStateType stateType, const std::wstring& sImageModify = L"");
	Image* GetEstimateImage();
	void ClearCache();

private:
	Control* m_pControl;
	std::map<ControlStateType, Image> m_stateImageMap;
};

/** 控件图片类型与状态图片的映射
*/
class UILIB_API StateImageMap
{
public:
	StateImageMap()	{ }

	void SetControl(Control* control);

	void SetImage(StateImageType stateImageType, ControlStateType stateType, const std::wstring& strImagePath);
	std::wstring GetImagePath(StateImageType stateImageType, ControlStateType stateType);

	bool HasHotImage();
	bool HasImageType(StateImageType stateImageType);
	bool PaintStatusImage(IRenderContext* pRender, StateImageType stateImageType, ControlStateType stateType, const std::wstring& sImageModify = L"");
	Image* GetEstimateImage(StateImageType stateImageType);

	void ClearCache();

private:
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

	bool HasHotColor();
	bool HasColor();
	void PaintStatusColor(IRenderContext* pRender, UiRect rcPaint, ControlStateType stateType);

private:
	Control* m_pControl;
	std::map<ControlStateType, std::wstring> m_stateColorMap;
};

} // namespace ui

#endif // UI_CORE_IMAGEDECODE_H_
