#ifndef UI_IMAGE_IMAGE_INFO_H_
#define UI_IMAGE_IMAGE_INFO_H_

#include "duilib/Render/IRender.h"
#include "duilib/Core/UiTypes.h"

namespace ui 
{
    class IRender;
    class Control;

/** 图片信息
*/
class UILIB_API ImageInfo
{
public:
    ImageInfo();
    ~ImageInfo();

    ImageInfo(const ImageInfo&) = delete;
    ImageInfo& operator = (const ImageInfo&) = delete;

public:
    /** 设置该图片的大小是否已经做过适应DPI处理
    *  （这个属性值影响：图片的"source"和"corner"属性的DPI缩放操作）
    */
    void SetBitmapSizeDpiScaled(bool isDpiScaled) { m_bDpiScaled = isDpiScaled; }

    /** 判断该图片的大小是否已经做过适应DPI处理
    */
    bool IsBitmapSizeDpiScaled() const { return m_bDpiScaled; }

    /** 设置图片的宽和高
    */
    void SetImageSize(int32_t nWidth, int32_t nHeight);

    /** 获取图片宽度
    */
    int32_t GetWidth() const { return m_nWidth; }

    /** 获取图片高度
    */
    int32_t GetHeight() const { return m_nHeight; }

    /** 添加一个图片帧数据, 添加后该资源由该类内部托管
    */
    void SetFrameBitmap(const std::vector<IBitmap*>& frameBitmaps);

    /** 获取一个图片帧数据
    */
    IBitmap* GetBitmap(uint32_t nIndex) const;

    /** 设置图片的多帧播放事件间隔（毫秒为单位 ）
    */
    void SetFrameInterval(const std::vector<int32_t>& frameIntervals);

    /** 获取图片帧对应的播放时间间隔（毫秒为单位 ）
    */
    int32_t GetFrameInterval(uint32_t nIndex) const;

    /** 获取图片的帧数
    */
    uint32_t GetFrameCount() const;

    /** 是否位多帧图片(比如GIF等)
    */
    bool IsMultiFrameImage() const;

    /** 设置循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
    */
    void SetPlayCount(int32_t nPlayCount);

    /** 获取循环播放次数
    *@return 返回值：-1 表示未设置
    *               0  表示动画是一致循环播放的
    *              > 0 表示动画循环播放的具体次数
    */
    int32_t GetPlayCount() const;

    /** 设置图片的加载KEY, 用于图片的生命周期管理
    * @param [in] loadKey 图片加载时的KEY
    */
    void SetLoadKey(const DString& loadKey);

    /** 获取图片的加载KEY
    */
    DString GetLoadKey() const;

    /** 设置生成图片加载时的DPI界面缩放比
    */
    void SetLoadDpiScale(uint32_t dpiScale);

    /** 获取生成图片加载时的DPI界面缩放比
    */
    uint32_t GetLoadDpiScale() const;

    /** 设置实际图片的KEY, 用于图片的生命周期管理
    * @param [in] imageKey 实际图片时的KEY
    */
    void SetImageKey(const DString& imageKey);

    /** 获取实际图片的KEY
    */
    DString GetImageKey() const;

public:
    /** 与另外一个图片数据交换数据
    */
    bool SwapImageData(ImageInfo& r);

private:
    //该图片的大小是否已经做过适应DPI处理（这个属性值影响：图片的"source"和"corner"属性的DPI缩放操作）
    bool m_bDpiScaled;

    //图片的宽度
    int32_t m_nWidth;
    
    //图片的高度
    int32_t m_nHeight;

    //图片帧对应的播放时间间隔（毫秒为单位 ）
    std::vector<int32_t>* m_pFrameIntervals;

    //图片帧数据
    IBitmap** m_pFrameBitmaps;

    //图片帧数量
    uint32_t m_nFrameCount;

    //循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
    int32_t m_nPlayCount;

    /** 图片的加载KEY, 用于图片的生命周期管理
    */
    UiString m_loadKey;

    /** 生成图片加载时的DPI界面缩放比
    */
    uint32_t m_loadDpiScale;

    /** 实际图片的KEY, 用于图片的生命周期管理（多个DPI的图片，实际可能指向同一个文件）
    */
    UiString m_imageKey;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_INFO_H_
