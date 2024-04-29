#ifndef UI_RENDER_SKIA_BITMAP_H_
#define UI_RENDER_SKIA_BITMAP_H_

#pragma once

#include "duilib/Render/IRender.h"

//Skia相关类的前置声明
class SkBitmap;

namespace ui
{

/** 位图的实现：Skia绘制引擎
*/
class UILIB_API Bitmap_Skia: public IBitmap
{
public:
	Bitmap_Skia();
	Bitmap_Skia(HBITMAP hBitmap, bool flipHeight);
	virtual ~Bitmap_Skia();

public:
	/** 从数据初始化（ARGB格式）
	@param [in] nWidth 宽度
	@param [in] nHeight 高度
	@param [in] flipHeight 是否翻转位图，如果为true，创建位图的时候，以左上角为圆点，图像方向是从上到下的；
						   如果为false，则以左下角为圆点，图像方向是从下到上。
	@param [in] pPixelBits 位图数据, 如果为nullptr表示窗口空位图，如果不为nullptr，其数据长度为：nWidth*4*nHeight
	@param [in] alphaType 位图的Alpha类型，只有Skia引擎需要此参数
	*/
	virtual bool Init(uint32_t nWidth, uint32_t nHeight, bool flipHeight,
		         const void* pPixelBits, BitmapAlphaType alphaType = kPremul_SkAlphaType) override;

	/** 获取图片宽度
	*/
	virtual uint32_t GetWidth() const override;

	/** 获取图片高度
	*/
	virtual uint32_t GetHeight() const override;

	/** 获取图片大小
	@return 图片大小
	*/
	virtual UiSize GetSize() const override;

	/** 锁定位图数据，数据长度 = GetWidth() * GetHeight() * 4
	*/
	virtual void* LockPixelBits() override;

	/** 释放位图数据
	*/
	virtual void UnLockPixelBits() override;

	/** 该位图是否有透明属性(即透明通道中，含有不是255的数据)
	*/
	virtual bool IsAlphaBitmap() const override;

	/** 克隆生成新的的位图
	*@return 返回新生成的位图接口，由调用方释放资源
	*/
	virtual IBitmap* Clone() override;

public:
	/** 获取Skia 位图
	*/
	const SkBitmap& GetSkBitmap() const;

	/** 获取位图GDI句柄
	*/
	HBITMAP GetHBitmap() const;

	/* 解除与位图句柄的关联
	*/
	HBITMAP DetachHBitmap();

	/** 创建一个设备无关的位图
	*@return 返回位图句柄，由调用方释放位图资源
	*/
	static HBITMAP CreateBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits);

private:
	/** 更新图片的透明通道标志
	*/
	void UpdateAlphaFlag(const uint8_t* pPixelBits);

	/** 对图片数据进行垂直翻转
	*/
	void FlipPixelBits(const uint8_t* pPixelBits, uint32_t nWidth, uint32_t nHeight, std::vector<uint8_t>& flipBits);

private:
	//位图GDI句柄
	HBITMAP m_hBitmap;

	//位图的宽度
	uint32_t m_nWidth;
	
	//位图的宽度
	uint32_t m_nHeight;

	/**位图方向标志为：true表示位图方向：从上到下，左上角为圆点
	                 false表示：位图方向：从下到上，左下角为圆点
	*/
	bool m_bFlipHeight;

	/** 该位图是否有透明属性(即透明通道中，含有不是255的数据)
	*/
	bool m_bAlphaBitmap;

	/** 位图的Alpha类型
	*/
	BitmapAlphaType m_alphaType;

	/** Skia 位图
	*/
	std::unique_ptr<SkBitmap> m_pSkBitmap;
};

} // namespace ui

#endif // UI_RENDER_SKIA_BITMAP_H_