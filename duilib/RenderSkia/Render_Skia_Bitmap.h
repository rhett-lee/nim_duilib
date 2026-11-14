#ifndef UI_RENDER_SKIA_RENDER_BITMAP_H_
#define UI_RENDER_SKIA_RENDER_BITMAP_H_

#include "duilib/RenderSkia/Render_Skia.h"
#include "include/core/SkRefCnt.h"

class SkSurface;
class SkCanvas;

namespace ui 
{
class IBitmap;

/** 位图渲染实现：直接绘制到指定的位图上（零拷贝）
*
* 绘制内容直接写入位图的像素内存，不进行拷贝；
* 本对象不拥有位图的所有权，位图的生命周期必须长于本对象。
*/
class Render_Skia_Bitmap : public Render_Skia
{
public:
    /** 构造函数
    * @param [in] pBitmap 绘制目标的位图
    */
    explicit Render_Skia_Bitmap(IBitmap* pBitmap);
    Render_Skia_Bitmap(const Render_Skia_Bitmap& r) = delete;
    Render_Skia_Bitmap& operator = (const Render_Skia_Bitmap& r) = delete;
    virtual ~Render_Skia_Bitmap() override;

public:
    /** 获取后台渲染的类型
    */
    virtual RenderBackendType GetRenderBackendType() const override;

    /** 调整画布大小（目标位图的尺寸不可变，仅尺寸相同时返回true）
    */
    virtual bool Resize(int32_t width, int32_t height) override;

    /** 获取宽度
    */
    virtual int32_t GetWidth() const override;

    /** 获取高度
    */
    virtual int32_t GetHeight() const override;

    /** 复制一个新的渲染对象（新对象为独立的离屏画布，不写回位图）
    */
    virtual std::unique_ptr<IRender> Clone() const override;

    /** 获取SkSurface接口
    */
    virtual SkSurface* GetSkSurface() const override;

    /** 获取SkCanvas接口
    */
    virtual SkCanvas* GetSkCanvas() const override;

    /** 判断是否已成功关联到有效的位图
    */
    bool IsValid() const;

private:
    /** 绘制目标的位图（不持有所有权）
    */
    IBitmap* m_pBitmap;

    /** 直接包装位图像素数据的画布（零拷贝）
    */
    sk_sp<SkSurface> m_skSurface;

    /** 位图的Alpha类型
    */
    AlphaType m_alphaType;
};

} // namespace ui

#endif // UI_RENDER_SKIA_RENDER_BITMAP_H_
