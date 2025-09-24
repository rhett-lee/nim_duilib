#ifndef UI_IMAGE_IMAGE_H_
#define UI_IMAGE_IMAGE_H_

#include "duilib/Image/ImageInfo.h"
#include "duilib/Image/ImageAttribute.h"
#include "duilib/Image/ImageLoadParam.h"
#include "duilib/Image/StateImageMap.h"
#include "duilib/Utils/Delegate.h"
#include <memory>

namespace ui 
{
class Control;
class ImagePlayer;
class DpiManager;

/** 图片相关封装，支持的文件格式：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO
*/
class UILIB_API Image : public virtual SupportWeakCallback
{
public:
    Image();
    ~Image();
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

public:
    /** @name 图片属性
    * @{
    */
    /** 初始化图片属性
    */
    void InitImageAttribute();

    /** 设置并初始化图片属性
    * @param [in] strImageString 图片属性字符串
    * @param [in] dpi DPI缩放接口
    */
    void SetImageString(const DString& strImageString, const DpiManager& dpi);

    /** 更新图片属性
    * @param [in] strImageString 图片属性字符串
    * @param [in] dpi DPI缩放接口
    */
    void UpdateImageAttribute(const DString& strImageString, const DpiManager& dpi);

    /** 获取图片属性（含文件名，和图片设置属性等）
    */
    DString GetImageString() const;

    /** 判断图片属性是否相等
    */
    bool EqualToImageString(const DString& imageString) const;

    /** 获取图片文件名（含相对路径，不含图片属性）
    */
    DString GetImagePath() const;

    /** 获取图片属性的内边距
    * @param [in] dpi DPI缩放管理器
    * @return 返回按照传入DPI缩放管理器适应的内边距数据
    */
    UiPadding GetImagePadding(const DpiManager& dpi) const;

    /** 设置图片属性的内边距(内部不做DPI自适应)
    * @param [in] newPadding 需要设置的内边距
    * @param [in] bNeedDpiScale 是否需要对newPadding进行DPI缩放
    * @param [in] dpi 与newPadding数据关联的DPI管理器
    */
    void SetImagePadding(const UiPadding& newPadding, bool bNeedDpiScale, const DpiManager& dpi);

    /** 判断是否禁用图片绘制
    */
    bool IsImagePaintEnabled() const;

    /** 设置是否禁止背景图片绘制
    */
    void SetImagePaintEnabled(bool bEnable);

    /** 设置图片属性：播放次数（仅当多帧图片时）
    */
    void SetImagePlayCount(int32_t nPlayCount);

    /** 设置图片属性：透明度（仅当多帧图片时）
    */
    void SetImageFade(uint8_t nFade);

    /** 获取图片属性（只读）
    */
    const ImageAttribute& GetImageAttribute() const;

    /** 获取图片加载属性
    */
    ImageLoadParam GetImageLoadParam() const;

    /** @} */

public:
    /** @name 图片数据（由外部加载图片数据）
    * @{
    */
    /** 获取图片信息接口
    */
    const std::shared_ptr<ImageInfo>& GetImageInfo() const;

    /** 设置图片信息接口
    */
    void SetImageInfo(const std::shared_ptr<ImageInfo>& imageInfo);

    /** 清除图片信息缓存数据, 释放资源
    */
    void ClearImageCache();

public:
    /** 是否位多帧图片(比如GIF等)
    */
    bool IsMultiFrameImage() const;

    /** 设置当前图片帧（仅当多帧图片时）
    */
    void SetCurrentFrameIndex(uint32_t nCurrentFrame);

    /** 获取当前图片帧索引（仅当多帧图片时）
    */
    uint32_t GetCurrentFrameIndex() const;

    /** 获取图片的帧数
    */
    uint32_t GetFrameCount() const;

    /** 获取循环播放次数（TODO：未使用，需要修改播放的实现源码）
    *@return 返回值：-1 表示动画是一致循环播放的
    *              >= 0 表示动画循环播放的具体次数
    */
    int32_t GetLoopCount() const;

    /** 获取当前图片帧的图片数据（多帧图片）
    * @param [in] rcDest 绘制目标区域, 用于矢量图的缩放
    * @param [in,out] rcSource 图片源区域，如果缩放时，会同步修改此区域的相应大小
    * @param [in,out] rcSourceCorners 图片的九宫格圆角属性
    */
    AnimationFramePtr GetCurrentFrame(const UiRect& rcDest, UiRect& rcSource, UiRect& rcSourceCorners) const;

public:
    /** 获取图片数据(单帧图片，对于Svg等格式，支持矢量缩放图片)
    * @param [in] bImageStretch 绘制图片时是否会被拉伸
    * @param [in] rcDest 绘制目标区域, 用于矢量图的缩放
    * @param [in,out] rcSource 图片源区域，如果缩放时，会同步修改此区域的相应大小
    * @param [in,out] rcSourceCorners 图片的九宫格圆角属性
    */
    std::shared_ptr<IBitmap> GetCurrentBitmap(bool bImageStretch,
                                              const UiRect& rcDest,
                                              UiRect& rcSource,
                                              UiRect& rcSourceCorners) const;

    /** @} */

public:
    /** @name 图片的显示
    * @{
    */
    /** 设置关联的控件接口
    */
    void SetControl(Control* pControl);

    /** 设置图片的显示区域（在绘制前调用）
    */
    void SetDrawDestRect(const UiRect& rcImageRect);

    /** 获取图片的显示区域
    */
    const UiRect& GetDrawDestRect() const;

    /** 重绘图片（按其显示区域）
    */
    void RedrawImage();

    /** 播放动画
    */
    void CheckStartImageAnimation();

    /** 暂停播放动画
     */
    void PauseImageAnimation();

    /** 播放动画
     * @param [in] nStartFrame 从哪一帧开始播放，可设置第一帧、当前帧和最后一帧。请参考 AnimationImagePos 枚举
     * @param [in] nPlayCount 指定播放次数
                   -1: 表示一直播放
                    0: 表示无有效的播放次数，使用图片的默认值(或者预设值)
                   >0: 具体的播放次数，达到播放次数后，停止播放
     */
    bool StartImageAnimation(AnimationImagePos nStartFrame = AnimationImagePos::kFrameFirst,
                             int32_t nPlayCount = 0);

    /** 停止播放动画     
     * @param [in] nStopFrame 播放结束停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 AnimationImagePos 枚举
     * @param [in] bTriggerEvent 是否将停止事件通知给订阅者，参考 AttachImageAnimationStop 方法
     */
    void StopImageAnimation(AnimationImagePos nStopFrame = AnimationImagePos::kFrameCurrent,
                            bool bTriggerEvent = false);
    /** @} */

private:
    /** 初始化动画播放器
    */
    ImagePlayer* InitImagePlayer();

    /** 获取当前图片帧的图片数据（单帧图片）
    * @param [in,out] rcSource 图片源区域，如果缩放时，会同步修改此区域的相应大小
    * @param [in,out] rcSourceCorners 图片的九宫格圆角属性
    */
    std::shared_ptr<IBitmap> GetBitmapData(UiRect& rcSource, UiRect& rcSourceCorners) const;

    /** 根据位图大小，调整绘制源区域
    * @param [in] pBitmap 位图接口
    * @param [in,out] rcSource 图片源区域，如果缩放时，会同步修改此区域的相应大小
    * @param [in,out] rcSourceCorners 图片的九宫格圆角属性
    */
    void AdjustImageSourceRect(const std::shared_ptr<IBitmap>& pBitmap, UiRect& rcSource, UiRect& rcSourceCorners) const;

private:

    /** 当前正在播放的图片帧（仅当多帧图片时）
    */
    uint32_t m_nCurrentFrame;

    /** 关联的控件接口
    */
    Control* m_pControl;

    /** 多帧图片播放实现接口
    */
    std::unique_ptr<ImagePlayer> m_pImagePlayer;

    /** 图片属性
    */
    ImageAttribute m_imageAttribute;

    /** 图片信息
    */
    std::shared_ptr<ImageInfo> m_imageInfo;

    /** 该图片绘制的目标区域
    */
    UiRect m_rcDrawDestRect;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_H_
