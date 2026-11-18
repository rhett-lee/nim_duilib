#ifndef UI_RENDER_SKIA_BITMAP_H_
#define UI_RENDER_SKIA_BITMAP_H_

#include "duilib/Render/IRender.h"

//Skia 头文件（需要完整类型定义，sk_sp<SkImage> 不能只用前置声明）
#include "SkiaHeaderBegin.h"
#include "include/core/SkImage.h"
#include "SkiaHeaderEnd.h"

//Skia相关类的前置声明
class SkBitmap;

namespace ui
{
/** 位图的实现：Skia绘制引擎
*/
class Bitmap_Skia: public IBitmap
{
public:
    Bitmap_Skia();
    virtual ~Bitmap_Skia() override;

public:
    /** 从数据初始化（ARGB格式）
    @param [in] nWidth 宽度
    @param [in] nHeight 高度
    @param [in] pPixelBits 位图数据, 如果为nullptr表示窗口空位图，如果不为nullptr，其数据长度为：nHeight * nWidth * 4
    @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值
    @param [in] alphaType 位图的Alpha类型
    */
    virtual bool Init(uint32_t nWidth, uint32_t nHeight,
                      const void* pPixelBits, float fImageSizeScale = 1.0f,
                      AlphaType alphaType = AlphaType::kPremul) override;

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

    /** 判断位图是否有效（已成功初始化）
    */
    virtual bool IsValid() const override;

    /** 获取每行像素数据的字节数
    */
    virtual uint32_t GetRowBytes() const override;

    /** 获取位图的Alpha类型
    */
    virtual AlphaType GetAlphaType() const override;

    /** 获取初始化位图时使用的图片缩放比例
    */
    virtual float GetImageSizeScale() const override;

    /** 锁定位图数据（可读写），数据长度 = GetWidth() * GetHeight() * 4
    */
    virtual void* LockPixelBits() override;

    /** 获取位图数据的只读指针，数据长度 = GetWidth() * GetHeight() * 4
    */
    virtual const void* GetPixelBits() const override;

    /** 释放位图数据
    */
    virtual void UnLockPixelBits() override;

    /** 克隆生成新的的位图
    *@return 返回新生成的位图接口，由调用方释放资源
    */
    virtual std::unique_ptr<IBitmap> Clone() const override;

public:
    /** 获取Skia 位图
    */
    const SkBitmap& GetSkBitmap() const;

    /** 获取 Skia Image（懒创建并缓存，内容不变时后续调用直接复用）
    * 用 asImage() 从 SkBitmap 创建，内部克隆像素副本，保证 SkImage 不依赖 SkBitmap 的后续修改；
    * 因此在 SkBitmap 内容变更后需要调用 InvalidateSkImageCache() 让下次 DrawImage 时重建
    */
    sk_sp<SkImage> GetSkImage() const;

    /** 强制让 SkImage 缓存失效（SkBitmap 像素内容被修改后调用）
    */
    void InvalidateSkImageCache();

private:
    /** 更新图片的透明通道标志
    */
    void UpdateAlphaFlag(uint8_t* pPixelBits);

    /** 执行数据初始化
    */
    bool InitImage(uint32_t nWidth, uint32_t nHeight, const void* pPixelBits, AlphaType alphaType);

    /** 从 SkBitmap 懒创建 SkImage 缓存
    * 用 mutable 成员实现 const GetSkImage() 中的懒加载
    */
    mutable sk_sp<SkImage> m_pSkImageCache;

private:
    /** Skia 位图
    */
    std::unique_ptr<SkBitmap> m_pSkBitmap;

    /** 初始化位图时使用的图片缩放比例
    */
    float m_fImageSizeScale = 1.0f;
};

} // namespace ui

#endif // UI_RENDER_SKIA_BITMAP_H_
