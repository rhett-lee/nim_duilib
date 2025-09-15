#ifndef UI_IMAGE_IMAGE_LOAD_PARAM_H_
#define UI_IMAGE_IMAGE_LOAD_PARAM_H_

#include "duilib/Core/UiTypes.h"

namespace ui 
{
class Control;

/** 图片加载的路径信息
*/
enum class UILIB_API ImageLoadPathType
{
    kUnknownPath,   //未知类型路径
    kLocalPath,     //本地绝对路径(非资源路径)
    kLocalResPath,  //本地绝对路径(资源路径内)
    kZipResPath,    //压缩包内相对路径
    kVirtualPath    //虚拟路径，比如:"icon:1"这种
};

/** 图片加载的路径信息
*/
struct UILIB_API ImageLoadPath
{
    //图片路径（本地绝对路径或者压缩包内的相对路径）
    DString m_imageFullPath;

    //图片路径类型
    ImageLoadPathType m_pathType = ImageLoadPathType::kUnknownPath;
};

/** 图片加载参数，用于加载一个图片
*/
class UILIB_API ImageLoadParam
{
public:
    /** DPI缩放选项
    */
    enum class DpiScaleOption
    {
        kDefault,   //默认：未设置dpi_scale图片属性
        kOn,        //开启：图片属性：dpi_scale="true"
        kOff        //关闭：图片属性：dpi_scale="false"
    };
public:
    /** 构造函数
    * @param [in] srcWidth 指定的图片宽度，像素值或者百分比值，比如"300"，或者"30%"，也可以为空
    * @param [in] srcHeight 指定的图片高度，像素值或者百分比值，比如"300"，或者"30%"，也可以为空
    * @param [in] nDpiScaleOption 图片大小DPI缩放的设置
    * @param [in] nLoadDpiScale 绘制目标的DPI缩放百分比（举例：100代表缩放百分比为100%，无缩放）
    * @param [in] bIconAsAnimation 如果是ICO文件，指定是否按多帧图片加载（按动画图片显示）
    * @param [in] nIconFrameDelayMs 如果是ICO文件，当按多帧图片显示时，每帧播放的时间间隔，毫秒（仅当m_bIconAsAnimation为true时有效）
    * @param [in] nIconSize 如果是ICO文件，用于指定需要加载的ICO图片的大小
    * @param [in] fPagMaxFrameRate PAG格式默认播放的最大帧率（仅限PAG格式）
    */
    ImageLoadParam(DString srcWidth,
                   DString srcHeight,
                   DpiScaleOption nDpiScaleOption,
                   uint32_t nLoadDpiScale = 100,
                   bool bIconAsAnimation = false,
                   int32_t nIconFrameDelayMs = 1000,
                   uint32_t nIconSize = 32,
                   float fPagMaxFrameRate = 30.0f);

    /** 拷贝构造和复制
    */
    ImageLoadParam(const ImageLoadParam& r) = default;
    ImageLoadParam& operator= (const ImageLoadParam& r) = default;

public:
    /** 设置图片路径（本地绝对路径或者压缩包内的相对路径）
    */
    void SetImageLoadPath(const ImageLoadPath& imageLoadPath);

    /** 获取图片路径（UTF8或者UTF16编码，本地绝对路径或者压缩包内的相对路径）
    */
    const ImageLoadPath& GetImageLoadPath() const;

public:
    /** 设置图片大小的DPI缩放选项
    */
    void SetDpiScaleOption(DpiScaleOption nDpiScaleOption);

    /** 获取图片大小的DPI缩放选项
    */
    DpiScaleOption GetDpiScaleOption() const;

    /** 设置绘制目标的DPI缩放百分比（举例：100代表缩放百分比为100% ，无缩放）
    */
    void SetLoadDpiScale(uint32_t nLoadDpiScale);

    /** 获取绘制目标的DPI缩放百分比（举例：100代表缩放百分比为100% ，无缩放）
    */
    uint32_t GetLoadDpiScale() const;

public:
    /** 如果是ICO文件，指定是否按多帧图片加载（按动画图片显示）
    */
    bool IsIconAsAnimation() const;

    /** 如果是ICO文件，用于指定需要加载的ICO图片的大小
    */
    uint32_t GetIconSize() const;

    /** 如果是ICO文件，当按多帧图片显示时，每帧播放的时间间隔，毫秒
    *   仅当IsIconAsAnimation()为true时有效
    */
    int32_t GetIconFrameDelayMs() const;

    /** PAG格式默认播放的最大帧率（仅限PAG格式）
    */
    float GetPagMaxFrameRate() const;

public:
    /** 获取加载图片的缓存KEY
    *   完整的格式是：<图片完整路径>@<界面缩放百分比>@<宽度>:<高度>
    *          举例: "C:\Test.jpg@200@80:40"
    * @param [in] nLoadDpiScale 请求图片对应的DPI缩放百分比
    */
    DString GetLoadKey(uint32_t nLoadDpiScale) const;

    /** 获取图片加载的固定设置大小
    * @param [out] nImageWidth 图片设置的宽度，如果返回0则无数据，比如：width='300'
    * @param [out] nImageHeight 图片设置的高度，如果返回0则无数据，比如：height='300'
    */
    bool GetImageFixedSize(uint32_t& nImageWidth, uint32_t& nImageHeight, bool bNeedDpiScale) const;

    /** 获取图片加载的固定百分比设置大小
    * @param [out] fImageWidthPercent 图片设置的宽度，如果返回1.0f则无数据，比如：width='300%'
    * @param [out] fImageHeightPercent 图片设置的高度，如果返回1.0f则无数据，比如：height='300%'
    */
    bool GetImageFixedPercent(float& fImageWidthPercent, float& fImageHeightPercent, bool bNeedDpiScale) const;

private:
    /** 获取图片加载的固定设置大小
    */
    bool GetScaledFixedSize(const DString& srcSize, uint32_t& nScaledSize, bool bNeedDpiScale) const;

    /** 获取图片加载的百分比设置大小
    */
    bool GetScaledFixedPercent(const DString& srcSize, float& fScaledPercent, bool bNeedDpiScale) const;

private:
    //(属性名称："file")本地绝对路径或者压缩包内的相对路径，不包含属性
    ImageLoadPath m_srcImageLoadPath;

    //设置图片宽度(属性名称："width")，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
    UiString m_srcWidth;

    //设置图片高度(属性名称："height")，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
    UiString m_srcHeight;

    //图片大小的DPI缩放选项
    DpiScaleOption m_nDpiScaleOption;

    //绘制目标的DPI缩放百分比（举例：100代表缩放百分比为100%，无缩放）
    uint32_t m_nLoadDpiScale;

    //PAG格式默认播放的最大帧率（仅限PAG格式）
    float m_fPagMaxFrameRate;

    //如果是ICO文件，用于指定需要加载的ICO图片的大小
    uint32_t m_nIconSize;

    //如果是ICO文件，当按多帧图片显示时，每帧播放的时间间隔，毫秒
    //仅当m_bIconAsAnimation为true时有效
    int32_t m_nIconFrameDelayMs;

    //如果是ICO文件，指定是否按多帧图片加载（按动画图片显示）
    bool m_bIconAsAnimation;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_LOAD_PARAM_H_
