#ifndef UI_CORE_IMAGEMANAGER_H_
#define UI_CORE_IMAGEMANAGER_H_

#include "duilib/Core/Callback.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Image/ImageDecoder.h"
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <chrono>

namespace ui 
{
class ImageInfo;
class ImageLoadParam;
class DpiManager;
class Window;
class Control;
class Image;

/** 延迟释放图片的回调函数类型
 * @param [in] pImageData 原图的图像数据接口
 * @param [in] imageFullPath 该图片的完整路径
 * @return 返回true表示允许放入延迟释放队列，返回false表示组织放入延迟释放队列
 */
using ReleaseImageCallback = std::function<bool (const std::shared_ptr<ui::IImage>& pImageData,
                                                 const DString& imageFullPath)>;

/** 图片管理器（对于图片资源的释放：延迟释放，内部有个原图图片队列，如果需要立即释放图片，则需要ReleaseImageCallback回调函数阻止放入延迟释放队列）
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
     * @param [in] loadParam 图片的加载属性，包含图片路径等信息
     * @param [out] bImageDataFromCache 返回true表示从缓存获取的ImageData共享原图数据，否则表示重新加载的
     * @return 返回图片 ImageInfo 对象的智能指针
     */
    std::shared_ptr<ImageInfo> GetImage(const ImageLoadParam& loadParam, bool& bImageDataFromCache);

    /** 从缓存中删除所有图片
     */
    void RemoveAllImages();

    /** 从缓存中释放一个原图图片（延迟释放）
    * @param [in] pImageData 原图的图像数据接口
    * @param [in] imageFullPath 该图片的完整路径
    */
    void ReleaseImage(const std::shared_ptr<IImage>& pImageData, const DString& imageFullPath);

    /** 取消释放原图图片
    */
    void CancelReleaseImage(const std::shared_ptr<IImage>& pImageData);

    /** 设置延迟释放图片的回调函数，可以用来阻止图片资源放入延迟释放队列，立即释放图片资源
     *   备注：如果图片资源是在虚表的子项中使用，立即释放原图资源会导致性能降低，因为虚表的元素是每次刷新都重新填充
     * @param [in] callback 延迟释放图片的回调函数
     */
    void SetReleaseImageCallback(ReleaseImageCallback callback);

public:
    /** 设置是否智能匹配临近的缩放百分比图片
    *   比如当dpiScale为120的时候，如果无图片匹配，但存在缩放百分比为125的图片，会自动匹配到
    *   这个功能可用减少各个DPI下的图片，降低适配DPI的工作量
    */
    void SetAutoMatchScaleImage(bool bAutoMatchScaleImage);

    /** 获取是否智能匹配临近的缩放百分比图片
    */
    bool IsAutoMatchScaleImage() const;

    /** 设置默认是否启用图片数据的多线程异步加载
    */
    void SetImageAsyncLoad(bool bImageAsyncLoad);

    /** 获取默认是否启用图片数据的多线程异步加载
    */
    bool IsImageAsyncLoad() const;

public:
    /** 添加到延迟绘制列表
    * @param [in] pControl 图片关联的控件
    * @param [in] pImage 图片接口
    * @param [in] imageKey 图片资源的KEY
    */
    void AddDelayPaintData(Control* pControl, Image* pImage, const DString& imageKey);

    /** 从延迟绘制列表中移除图片关联的数据
    * @param [in] pControl 图片关联的控件
    * @param [in] pImage 图片接口
    */
    void RemoveDelayPaintData(Control* pControl);
    void RemoveDelayPaintData(Image* pImage);

    /** 执行延迟绘制（当图片资源在子线程加载完成时调用）
    * @param [in] imageKey 图片资源的KEY
    */
    void DelayPaintImage(const DString& imageKey);

private:
    /** 图片信息被销毁的回调函数，用于释放图片资源
    * @param[in] pImageInfo 图片对应的 ImageInfo 对象
    */
    static void CallImageInfoDestroy(ImageInfo* pImageInfo);

    /** 图片数据被销毁的回调函数，用于释放图片资源的数据
    * @param[in] pImage 图片数据接口
    */
    static void CallImageDataDestroy(IImage* pImage);

private:
    /** 图片信息被创建的回调函数
    * @param[in] pImageInfo 图片对应的 ImageInfo 对象
    */
    void OnImageInfoCreate(std::shared_ptr<ImageInfo>& pImageInfo);

    /** 图片信息被销毁的回调函数，用于释放图片资源
     * @param[in] pImageInfo 图片对应的 ImageInfo 对象
     */
    void OnImageInfoDestroy(ImageInfo* pImageInfo);

    /** 图片数据被创建的回调函数
     * @param[in] imageKey 图片的KEY
     * @param[in] pImage 图片数据接口
     * @param[in] fImageSizeScale 该图片的缩放比
     */
    void OnImageDataCreate(const DString& imageKey, std::shared_ptr<IImage>& pImage, float fImageSizeScale);

    /** 图片数据被销毁的回调函数，用于释放图片资源的数据
     * @param[in] pImage 图片数据接口
     */
    void OnImageDataDestroy(IImage* pImage);

private:
    /** 查找指定DPI缩放百分比下的图片，可以每个DPI设置一个图片，以提高不同DPI下的图片质量
    *   举例：DPI缩放百分比为120（即放大到120%）的图片："image.png" 对应于 "image@120.png"
    * @param [in] dpiScale 需要查找的DPI缩放百分比
    * @param [in] bIsUseZip 是否使用zip压缩包资源
    * @param [in] imageFullPath 图片资源的完整路径
    * @param [out] dpiImageFullPath 返回指定DPI下的图片资源路径，如果没找到，则返回空串
    * @param [out] nImageFileDpiScale 图片对应的DPI缩放百分比
    */
    bool GetDpiScaleImageFullPath(uint32_t dpiScale, 
                                  bool bIsUseZip,
                                  const DString& imageFullPath,
                                  DString& dpiImageFullPath,
                                  uint32_t& nImageFileDpiScale) const;

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

private:
    /** 是否智能匹配临近的缩放百分比图片
    */
    bool m_bAutoMatchScaleImage;

    /** 默认是否启用图片数据的多线程异步加载
    */
    bool m_bImageAsyncLoad;

    /** 图片资源映射表（图片加载Key与图片UI数据的映射表）
    *   KEY: 由 ImageLoadParam::GetLoadKey 函数获取
    */
    std::unordered_map<DString, std::weak_ptr<ImageInfo>> m_imageInfoMap;

    /** 图片的原图数据
    */
    struct TImageData
    {
        //构造函数
        TImageData() :
            m_fImageSizeScale(1.0f)
        {
        }
        TImageData(const std::shared_ptr<IImage>& pImage, float fImageSizeScale) :
            m_pImage(pImage),
            m_fImageSizeScale(fImageSizeScale)
        {
        }

        //释放的图片接口
        std::weak_ptr<IImage> m_pImage;

        //加载时输入的图片缩放比例
        float m_fImageSizeScale;
    };

    /** 图片资源映射表（原图数据Key与图片数据的映射表）
    *   KEY：由ImageManager::GetDpiScaleImageFullPath函数获取，参数：dpiImageFullPath
    */
    std::unordered_map<DString, TImageData> m_imageDataMap;

    /** 等待释放的原图数据
    */
    struct TReleaseImageData
    {
        //释放的图片接口
        std::shared_ptr<IImage> m_pImage;

        //数据释放的时间
        std::chrono::steady_clock::time_point m_releaseTime;
    };
    std::vector<TReleaseImageData> m_delayReleaseImageList;

    /** 延迟释放图片的回调函数
    */
    ReleaseImageCallback m_releaseImageCallback;

private:
    /** 图片延迟绘制相关数据（图片资源在子线程加载完成后，需要通知界面重新绘制该图片）
    */
    struct TImageDelayPaintData
    {
        ControlPtr m_pControl;          //关联的控件接口
        ControlPtrT<Image> m_pImage;    //关联的图片接口
        DString m_imageKey;             //图片资源的KEY
    };
    std::list<TImageDelayPaintData> m_delayPaintImageList;
};

}
#endif //UI_CORE_IMAGEMANAGER_H_
