#ifndef UI_RENDER_SKIA_RENDER_H_
#define UI_RENDER_SKIA_RENDER_H_

#pragma once

#include "duilib/Render/IRender.h"

//Skia相关类的前置声明
class SkCanvas;
struct SkPoint;
class SkPaint;

namespace ui 
{

class Bitmap_Skia;

class UILIB_API Render_Skia : public IRender
{
public:
	explicit Render_Skia(IRenderFactory* pRenderFactory);
	virtual ~Render_Skia();

	/** 获取Render实现类型
	*/
	virtual RenderType GetRenderType() const override;

	virtual bool Resize(int width, int height) override;
	virtual void Clear() override;
	virtual std::unique_ptr<IRender> Clone() override;

	/** 分离位图
	*@return 返回位图接口，返回后由调用方管理资源（包括释放资源等）
	*/
	virtual IBitmap* DetachBitmap();

	virtual int	GetWidth() override;
	virtual int GetHeight() override;
	virtual void ClearAlpha(const UiRect& rcDirty, uint8_t alpha = 0) override;
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) override;
	virtual void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding = UiPadding()) override;

	virtual bool IsRenderTransparent() const override;
	virtual bool SetRenderTransparent(bool bTransparent) override;

	virtual UiPoint OffsetWindowOrg(UiPoint ptOffset) override;
	virtual UiPoint SetWindowOrg(UiPoint ptOffset) override;
	virtual UiPoint GetWindowOrg() const override;

	virtual void SaveClip(int& nState) override;
	virtual void RestoreClip(int nState) override;
	virtual void SetClip(const UiRect& rc, bool bIntersect = true) override;
	virtual void SetRoundClip(const UiRect& rc, int width, int height, bool bIntersect = true) override;
	virtual void ClearClip() override;

	virtual bool BitBlt(int x, int y, int cx, int cy, IBitmap* pSrcBitmap, int xSrc, int ySrc, RopMode rop) override;
	virtual bool BitBlt(int x, int y, int cx, int cy, IRender* pSrcRender, int xSrc, int ySrc, RopMode rop) override;
	virtual bool StretchBlt(int xDest, int yDest, int widthDest, int heightDest, IRender* pSrcRender, int xSrc, int ySrc, int widthSrc, int heightSrc, RopMode rop) override;
	virtual bool AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, IRender* pSrcRender, int xSrc, int ySrc, int widthSrc, int heightSrc, uint8_t alpha = 255) override;

	virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap, 
						   const UiRect& rcDest, const UiRect& rcDestCorners,
						   const UiRect& rcSource, const UiRect& rcSourceCorners,
						   uint8_t uFade = 255, bool xtiled = false, bool ytiled = false, 
						   bool fullxtiled = true,  bool fullytiled = true, int nTiledMargin = 0) override;
	virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap, 
						   const UiRect& rcDest,  const UiRect& rcSource, 
						   uint8_t uFade = 255, bool xtiled = false, bool ytiled = false, 
						   bool fullxtiled = true, bool fullytiled = true, int nTiledMargin = 0) override;
	virtual void DrawImageRect(const UiRect& rcPaint, IBitmap* pBitmap,
						       const UiRect& rcDest, const UiRect& rcSource,
						       uint8_t uFade = 255, IMatrix* pMatrix = nullptr);

	virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int32_t nWidth) override;
	virtual void DrawLine(const UiPointF& pt1, const UiPointF& pt2, UiColor penColor, float fWidth) override;
	virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, IPen* pen) override;

	virtual void DrawRect(const UiRect& rc, UiColor penColor, int32_t nWidth, bool bLineInRect) override;
	virtual void FillRect(const UiRect& rc, UiColor dwColor, uint8_t uFade = 255) override;

	virtual void DrawRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor penColor, int nWidth) override;
	virtual void FillRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor dwColor, uint8_t uFade = 255) override;

	virtual void DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, int nWidth) override;
	virtual void FillCircle(const UiPoint& centerPt, int32_t radius, UiColor dwColor, uint8_t uFade = 255) override;

	virtual void DrawArc(const UiRect& rc, float startAngle, float sweepAngle, bool useCenter, 
						 const IPen* pen, 
		                 UiColor* gradientColor = nullptr, const UiRect* gradientRect = nullptr) override;

	virtual void DrawPath(const IPath* path, const IPen* pen) override;
	virtual void FillPath(const IPath* path, const IBrush* brush) override;

	virtual UiRect MeasureString(const std::wstring& strText,
		                         const std::wstring& strFontId, 
								 uint32_t uFormat, int width = DUI_NOSET_VALUE) override;


	virtual void DrawString(const UiRect& rc, const std::wstring& strText,
		                    UiColor dwTextColor, const std::wstring& strFontId, 
							uint32_t uFormat, uint8_t uFade = 255) override;

	virtual void DrawRichText(const UiRect& rc,
							  std::vector<RichTextData>& richTextData,
							  uint32_t uFormat = 0,
							  bool bMeasureOnly = false,
							  uint8_t uFade = 255) override;

	void DrawBoxShadow(const UiRect& rc, const UiSize& roundSize, const UiPoint& cpOffset, int nBlurRadius, int nSpreadRadius, UiColor dwColor) override;

#ifdef UILIB_IMPL_WINSDK
	/** 获取DC句柄，当不使用后，需要调用ReleaseDC接口释放资源
	*/
	virtual HDC GetDC() override;

	/** 释放DC资源
	* @param [in] hdc 需要释放的DC句柄
	*/
	virtual void ReleaseDC(HDC hdc) override;
#endif

private:
	/** 位图绘制封装
	*/
	bool BitBlt(int x, int y, int cx, int cy, Bitmap_Skia* pSrcBitmap, int xSrc, int ySrc, RopMode rop);

	/** 获取GDI的光栅操作代码
	*/
	void SetRopMode(SkPaint& skPaint, RopMode rop) const;

	/** 根据Pen接口，初始化Paint对象
	*/
	void SetPaintByPen(SkPaint& skPaint, const IPen* pen);

	/** 按设置的属性，绘制文字
	*/
	void DrawTextString(const UiRect& rc, const std::wstring& strText, uint32_t uFormat,
					    const SkPaint& skPaint, IFont* pFont) const;

private:

	/** 位图对象
	*/
	std::unique_ptr<Bitmap_Skia> m_pBitmapSkia;

	/** Canvas 接口
	*/
	SkCanvas* m_pSkCanvas;

	/** Canval保存的状态
	*/
	int m_saveCount;

	/** 绘制属性
	*/
	SkPaint* m_pSkPaint;

	/** 视图的原点坐标
	*/
	SkPoint* m_pSkPointOrg;

	/** 是否透明
	*/
	bool m_bTransparent;

	/** 关联的DC句柄
	*/
	HDC m_hDC;

	/** Render工厂接口，用于创建字体
	*/
	IRenderFactory* m_pRenderFactory;
};

} // namespace ui

#endif // UI_RENDER_SKIA_RENDER_H_
