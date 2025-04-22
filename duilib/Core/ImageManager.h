#ifndef UI_CORE_IMAGEMANAGER_H_
#define UI_CORE_IMAGEMANAGER_H_

#include "duilib/Core/Callback.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace ui 
{
class ImageInfo;
class ImageLoadAttribute;
class DpiManager;
class Window;

/** 图片管理器
 */
class UILIB_API ImageManager
{
public:
    ImageManager();
    ~ImageManager();
    ImageManager(const ImageManager&) = delete;
    ImageManager& operator = (const ImageManager&) = delete;

public:
    /** 加载图片 ImageInfo 对象
     * @param [in] pWindow 图片关联的窗口（用于DPI缩放、HICON绘制等）
     * @param [in] loadAtrribute 图片的加载属性，包含图片路径等信息
     * @param [in] asyncLoadCallback 多帧图片异步加载完成后的回调函数（用来更新界面，显示动画图片）
     * @return 返回图片 ImageInfo 对象的智能指针
     */
    std::shared_ptr<ImageInfo> GetImage(const Window* pWindow,
                                        const ImageLoadAttribute& loadAtrribute,
                                        StdClosure asyncLoadCallback);

    /** 从缓存中删除所有图片
     */
    void RemoveAllImages();

    /** 设置是否默认对所有图片在加载时根据DPI进行缩放，这个是全局属性，默认为true，应用于所有图片
       （设置为true后，也可以通过在xml中，使用"dpiscale='false'"属性关闭某个图片的DPI自动缩放）
    */
    void SetDpiScaleAllImages(bool bEnable);

    /** 判断是否默认对所有图片在加载时根据DPI进行缩放
    */
    bool IsDpiScaleAllImages() const;

    /** 设置是否智能匹配临近的缩放百分比图片
    *   比如当dpiScale为120的时候，如果无图片匹配，但存在缩放百分比为125的图片，会自动匹配到
    *   这个功能可用减少各个DPI下的图片，降低适配DPI的工作量
    */
    void SetAutoMatchScaleImage(bool bAutoMatchScaleImage);

    /** 获取是否智能匹配临近的缩放百分比图片
    */
    bool IsAutoMatchScaleImage() const;

private:
    /** 保存图片数据到缓存
    * @param [in] pImageInfo 图片数据指针
    * @param [in] loadKey 图片的加载KEY
    * @param [in] nWindowDpiScale 对应窗口的DPI缩放百分比
    * @param [in] isDpiScaledImageFile 该图片是否为DPI自适应的图片（不是DPI为96的原始图片）
    * @return 返回生成该图片数据的智能指针
    */
    std::shared_ptr<ImageInfo> SaveImageInfo(ImageInfo* pImageInfo, const DString& loadKey, uint32_t nWindowDpiScale, bool isDpiScaledImageFile);

    /** 更新图片数据到缓存（异步加载的多帧图片数据）
    */
    bool UpdateImageInfo(std::shared_ptr<ImageInfo> spNewSharedImage, const DString& loadKey, const DString& imageKey,
                         uint32_t nWindowDpiScale, bool isDpiScaledImageFile);

    /** 图片被销毁的回调函数，用于释放图片资源
     * @param[in] pImageInfo 图片对应的 ImageInfo 对象
     */
    static void OnImageInfoDestroy(ImageInfo* pImageInfo);

    /** 查找指定DPI缩放百分比下的图片，可以每个DPI设置一个图片，以提高不同DPI下的图片质量
    *   举例：DPI缩放百分比为120（即放大到120%）的图片："image.png" 对应于 "image@120.png"
    * @param [in] dpiScale 需要查找的DPI缩放百分比
    * @param [in] bIsUseZip 是否使用zip压缩包资源
    * @param [in] imageFullPath 图片资源的完整路径
    * @param [out] dpiImageFullPath 返回指定DPI下的图片资源路径，如果没找到，则返回空串
    * @param [out] nImageDpiScale 图片对应的DPI缩放百分比
    */
    bool GetDpiScaleImageFullPath(uint32_t dpiScale, 
                                  bool bIsUseZip,
                                  const DString& imageFullPath,
                                  DString& dpiImageFullPath,
                                  uint32_t& nImageDpiScale) const;

    /** 查找指定DPI缩放百分比下的图片，可以每个DPI设置一个图片，以提高不同DPI下的图片质量
    *   举例：DPI缩放百分比为120（即放大到120%）的图片："image.png" 对应于 "image@120.png"
    * @param [in] dpiScale 需要查找的DPI缩放百分比
    * @param [in] bIsUseZip 是否使用zip压缩包资源
    * @param [in] imageFullPath 图片资源的完整路径
    * @param [out] dpiImageFullPath 返回指定DPI下的图片资源路径，如果没找到，则返回空串
    */
    bool FindDpiScaleImageFullPath(uint32_t dpiScale, 
                                  bool bIsUseZip,
                                  const DString& imageFullPath,
                                  DString& dpiImageFullPath) const;

    /** 获取指定DPI缩放百分比下的图片资源路径
    *   举例：DPI缩放百分比为120（即放大到120%）的图片："image.png" 对应于 "image@120.png"
    * @param [in] dpiScale 需要查找的DPI缩放百分比
    * @param [in] imageFullPath 图片资源的完整路径
    * @return 返回指定DPI下的图片资源路径, 如果失败则返回空串
    */
    DString GetDpiScaledPath(uint32_t dpiScale, const DString& imageFullPath) const;

    /** 从ICON数据加载一个图片
    */
    void LoadIconData(const Window* pWindow,
                      const ImageLoadAttribute& loadAtrribute,
                      std::unique_ptr<ImageInfo>& imageInfo) const;

private:
    /** 是否默认对所有图片在加载时根据DPI进行缩放，这个是全局属性，默认为true，应用于所有图片
       （设置为true后，也可以通过在xml中，使用"dpiscale='false'"属性关闭某个图片的DPI自动缩放）
    */
    bool m_bDpiScaleAllImages;

    /** 是否智能匹配临近的缩放百分比图片
    */
    bool m_bAutoMatchScaleImage;

    /** 图片资源映射表（图片的Key与图片数据）
    */
    std::unordered_map<DString, std::weak_ptr<ImageInfo>> m_imageMap;

    /** 图片资源Key映射表（图片的加载Key与图片Key）
    */
    std::unordered_map <DString, DString> m_loadKeyMap;
};

}
#endif //UI_CORE_IMAGEMANAGER_H_
