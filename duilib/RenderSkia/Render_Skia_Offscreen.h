#ifndef UI_RENDER_SKIA_RENDER_OFFSCREEN_H_
#define UI_RENDER_SKIA_RENDER_OFFSCREEN_H_

#include "duilib/RenderSkia/Render_Skia.h"
#include "include/core/SkRefCnt.h"

class SkSurface;
class GrDirectContext;

namespace ui 
{
/** 离屏渲染实现（不关联任何窗口，因此不实现IRenderWindow接口）
*/
class Render_Skia_Offscreen : public Render_Skia
{
public:
    /** 构造函数
    * @param [in] nWidth 画布宽度（物理像素），可以为0
    * @param [in] nHeight 画布高度（物理像素），可以为0
    * @param [in] backendType 后台绘制方式
    * @param [in] alphaType 画布的Alpha类型
    */
    Render_Skia_Offscreen(uint32_t nWidth, uint32_t nHeight,
                          RenderBackendType backendType = RenderBackendType::kRaster_BackendType,
                          AlphaType alphaType = AlphaType::kPremul);
    Render_Skia_Offscreen(const Render_Skia_Offscreen& r) = delete;
    Render_Skia_Offscreen& operator = (const Render_Skia_Offscreen& r) = delete;
    virtual ~Render_Skia_Offscreen() override;

public:
    /** 离屏画布支持的最大边长（像素）
    */
    static constexpr int32_t kMaxOffscreenSize = 16384;

    /** 获取后台渲染的类型
    */
    virtual RenderBackendType GetRenderBackendType() const override;

    /** 大小发生变化（会重建画布，原有内容不保留）
    * @param [in] width 画布宽度（物理像素），可以为0
    * @param [in] height 画布高度（物理像素），可以为0
    * @return 成功返回true；尺寸非法或创建失败时返回false
    */
    virtual bool Resize(int32_t width, int32_t height) override;

    /** 获取宽度
    */
    virtual int32_t GetWidth() const override;

    /** 获取高度
    */
    virtual int32_t GetHeight() const override;

    /** 复制一个新的渲染对象
    */
    virtual std::unique_ptr<IRender> Clone() const override;

    /** 获取SkSurface接口
    */
    virtual SkSurface* GetSkSurface() const override;

    /** 获取SkCanvas接口
    */
    virtual SkCanvas* GetSkCanvas() const override;

    /** 判断离屏画布是否已创建成功（宽高为0时返回false）
    */
    bool IsValid() const;

private:
    /** 创建/重建画布
    */
    bool CreateSurface(int32_t width, int32_t height);

private:
    /** 离屏画布
    */
    sk_sp<SkSurface> m_skSurface;

    /** 后台绘制方式
    */
    RenderBackendType m_backendType;

    /** 画布的Alpha类型
    */
    AlphaType m_alphaType;

    /** GPU后端使用的图形上下文（Raster后端为nullptr；生命周期由GL上下文管理器管理）
    */
    GrDirectContext* m_pGrContext;
};

} // namespace ui

#endif // UI_RENDER_SKIA_RENDER_OFFSCREEN_H_
