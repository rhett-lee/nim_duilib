#ifndef UI_RENDER_SKIA_RENDER_H_
#define UI_RENDER_SKIA_RENDER_H_

#include "duilib/Render/IRender.h"
#include "duilib/Core/Callback.h"

//Skia相关类的前置声明
class SkSurface;
class SkCanvas;
struct SkPoint;
class SkPaint;
enum class SkTextEncoding;

namespace ui 
{

class Bitmap_Skia;

class UILIB_API Render_Skia : public IRender
{
public:
    Render_Skia();
    Render_Skia(const Render_Skia& r) = delete;
    Render_Skia& operator = (const Render_Skia& r) = delete;
    virtual ~Render_Skia() override;

    /** 获取Render实现类型
    */
    virtual RenderType GetRenderType() const override;

    virtual void Clear(const UiColor& uiColor) override;
    virtual void ClearRect(const UiRect& rcDirty, const UiColor& uiColor) override;

    /** 分离位图
    *@return 返回位图接口，返回后由调用方管理资源（包括释放资源等）
    */
    virtual IBitmap* MakeImageSnapshot() override;

    virtual void ClearAlpha(const UiRect& rcDirty, uint8_t alpha = 0) override;
    virtual void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) override;
    virtual void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding = UiPadding()) override;

    virtual UiPoint OffsetWindowOrg(UiPoint ptOffset) override;
    virtual UiPoint SetWindowOrg(UiPoint ptOffset) override;
    virtual UiPoint GetWindowOrg() const override;

    virtual void SaveClip(int32_t& nState) override;
    virtual void RestoreClip(int32_t nState) override;
    virtual void SetClip(const UiRect& rc, bool bIntersect = true) override;
    virtual void SetRoundClip(const UiRect& rc, int32_t width, int32_t height, bool bIntersect = true) override;
    virtual void ClearClip() override;

    virtual bool BitBlt(int32_t x, int32_t y, int32_t cx, int32_t cy, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, RopMode rop) override;
    virtual bool StretchBlt(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc, RopMode rop) override;
    virtual bool AlphaBlend(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc, uint8_t alpha = 255) override;

    virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap, 
                           const UiRect& rcDest, const UiRect& rcDestCorners,
                           const UiRect& rcSource, const UiRect& rcSourceCorners,
                           uint8_t uFade = 255, bool xtiled = false, bool ytiled = false, 
                           bool fullxtiled = true,  bool fullytiled = true, int32_t nTiledMargin = 0) override;
    virtual void DrawImage(const UiRect& rcPaint, IBitmap* pBitmap, 
                           const UiRect& rcDest,  const UiRect& rcSource, 
                           uint8_t uFade = 255, bool xtiled = false, bool ytiled = false, 
                           bool fullxtiled = true, bool fullytiled = true, int32_t nTiledMargin = 0) override;
    virtual void DrawImageRect(const UiRect& rcPaint, IBitmap* pBitmap,
                               const UiRect& rcDest, const UiRect& rcSource,
                               uint8_t uFade = 255, IMatrix* pMatrix = nullptr) override;

    virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int32_t nWidth) override;
    virtual void DrawLine(const UiPointF& pt1, const UiPointF& pt2, UiColor penColor, float fWidth) override;
    virtual void DrawLine(const UiPoint& pt1, const UiPoint& pt2, IPen* pen) override;

    virtual void DrawRect(const UiRect& rc, UiColor penColor, int32_t nWidth, bool bLineInRect) override;
    virtual void FillRect(const UiRect& rc, UiColor dwColor, uint8_t uFade = 255) override;
    virtual void FillRect(const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) override;

    virtual void DrawRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor penColor, int32_t nWidth) override;
    virtual void FillRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor dwColor, uint8_t uFade = 255) override;
    virtual void FillRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade = 255) override;

    virtual void DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, int32_t nWidth) override;
    virtual void FillCircle(const UiPoint& centerPt, int32_t radius, UiColor dwColor, uint8_t uFade = 255) override;

    virtual void DrawArc(const UiRect& rc, float startAngle, float sweepAngle, bool useCenter, 
                         const IPen* pen, 
                         UiColor* gradientColor = nullptr, const UiRect* gradientRect = nullptr) override;

    virtual void DrawPath(const IPath* path, const IPen* pen) override;
    virtual void FillPath(const IPath* path, const IBrush* brush) override;
    virtual void FillPath(const IPath* path, const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) override;

    virtual UiRect MeasureString(const DString& strText, IFont* pFont,
                                 uint32_t uFormat, int32_t width = DUI_NOSET_VALUE) override;


    virtual void DrawString(const UiRect& rc, const DString& strText,
                            UiColor dwTextColor, IFont* pFont,
                            uint32_t uFormat, uint8_t uFade = 255) override;

    virtual void DrawRichText(const UiRect& rc,
                              IRenderFactory* pRenderFactory, 
                              std::vector<RichTextData>& richTextData,
                              uint32_t uFormat = 0,
                              bool bMeasureOnly = false,
                              uint8_t uFade = 255) override;

    void DrawBoxShadow(const UiRect& rc, const UiSize& roundSize, const UiPoint& cpOffset, int32_t nBlurRadius, int32_t nSpreadRadius, UiColor dwColor) override;

    virtual bool ReadPixels(const UiRect& rc, void* dstPixels, size_t dstPixelsLen) override;
    virtual bool WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc) override;
    virtual bool WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc, const UiRect& rcPaint) override;
    virtual RenderClipType GetClipInfo(std::vector<UiRect>& clipRects) override;
    virtual bool IsClipEmpty() const override;
    virtual bool IsEmpty() const override;
    virtual void SetRenderDpi(const IRenderDpiPtr& spRenderDpi) override;

public:
    /** 获取SkSurface接口
    */
    virtual SkSurface* GetSkSurface() const = 0;

    /** 获取SkCanvas接口
    */
    virtual SkCanvas* GetSkCanvas() const = 0;

protected:
    /** 视图的原点坐标
    */
     SkPoint& GetPointOrg() const;

    /** 获取Render使用的DPI转换接口
    */
    IRenderDpiPtr GetRenderDpi() const;

private:
    /** 获取GDI的光栅操作代码
    */
    void SetRopMode(SkPaint& skPaint, RopMode rop) const;

    /** 根据Pen接口，初始化Paint对象
    */
    void SetPaintByPen(SkPaint& skPaint, const IPen* pen);

    /** 按设置的属性，绘制文字
    */
    void DrawTextString(const UiRect& rc, const DString& strText, uint32_t uFormat,
                        const SkPaint& skPaint, IFont* pFont) const;

    /** 设置颜色渐变的绘制属性
    */
    void InitGradientColor(SkPaint& skPaint, const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) const;

    /** 获取文本编码
    */
    SkTextEncoding GetTextEncoding() const;

    /** 获取位图数据
    * @return 返回位图数据的地址, 数据长度为: 高度*宽度*sizeof(uint32_t)
    */
    void* GetPixelBits() const;

    /** 整数的DPI转换
    */
    int32_t GetScaleInt(int32_t iValue) const;

private:
    /** Canval保存的状态
    */
    int32_t m_saveCount;

    /** 绘制属性
    */
    SkPaint* m_pSkPaint;

    /** 视图的原点坐标
    */
    SkPoint* m_pSkPointOrg;

    /** DPI转换辅助接口
    */
    IRenderDpiPtr m_spRenderDpi;
};

} // namespace ui

#endif // UI_RENDER_SKIA_RENDER_H_
