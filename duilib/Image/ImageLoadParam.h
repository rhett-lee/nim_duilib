#ifndef UI_IMAGE_IMAGE_LOAD_PARAM_H_
#define UI_IMAGE_IMAGE_LOAD_PARAM_H_

#include "duilib/Core/UiTypes.h"

namespace ui 
{
class Control;

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
    * @param [in] nIconSize 加载图标的大小值（仅限ico格式，每个ico包含了各种大小的图标，这个参数可以指定加载哪个尺寸的图标）
    * @param [in] fPagMaxFrameRate PAG格式默认播放的最大帧率（仅限PAG格式）
    */
    ImageLoadParam(DString srcWidth,
                   DString srcHeight,
                   DpiScaleOption nDpiScaleOption,
                   uint32_t nLoadDpiScale = 100,
                   uint32_t nIconSize = 32,
                   float fPagMaxFrameRate = 30.0f);

    /** 拷贝构造和复制
    */
    ImageLoadParam(const ImageLoadParam& r) = default;
    ImageLoadParam& operator= (const ImageLoadParam& r) = default;

public:
    /** 设置图片路径（本地绝对路径或者压缩包内的相对路径）
    */
    void SetImageFullPath(const DString& imageFullPath);

    /** 获取图片路径（UTF8或者UTF16编码，本地绝对路径或者压缩包内的相对路径）
    */
    DString GetImageFullPath() const;

    /** 是否含有图片路径
    */
    bool HasImageFullPath() const;

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
    /** 如果是ICO文件，用于指定需要加载的ICO图片的大小（仅限ico格式）
    */
    uint32_t GetIconSize() const;

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

    /** 获取图片加载后应当缩放的宽度和高度
       （如果只设置了宽度或者高度，那么会按锁定纵横比的方式对整个图片进行缩放）
    * @param [in,out] nImageWidth 传入原始图片的宽度，返回计算后的宽度（返回的值，已根据配置进行DPI缩放）
    * @param [in,out] nImageHeight 传入原始图片的高度，返回计算后的高度（返回的值，已根据配置进行DPI缩放）
    * @param [in] bNeedDpiScale 是否做DPI缩放，如果设置了DPI，图片的宽度和高度会根据配置进行DPI缩放
    * @return 返回true表示图片大小有缩放，返回false表示图片大小无缩放
    */
    bool CalcImageLoadSize(uint32_t& nImageWidth, uint32_t& nImageHeight, bool bNeedDpiScale) const;

private:
    /** 获取设置的缩放后的大小(无DPI缩放)
    * @return 返回缩放后的大小，如果失败则返回0
    */
    uint32_t GetScacledSize(const DString& srcSize, uint32_t nImageSize, bool bImageSizeDpiScaled) const;

private:
    //(属性名称："file")本地绝对路径或者压缩包内的相对路径，不包含属性
    UiString m_srcImageFullPath;

    //设置图片宽度(属性名称："width")，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
    UiString m_srcWidth;

    //设置图片高度(属性名称："height")，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
    UiString m_srcHeight;

    //图片大小的DPI缩放选项
    DpiScaleOption m_nDpiScaleOption;

    //绘制目标的DPI缩放百分比（举例：100代表缩放百分比为100%，无缩放）
    uint32_t m_nLoadDpiScale;

    //如果是ICO文件，用于指定需要加载的ICO图片的大小（属性名称："icon_size"）
    //(ICO文件中包含很多个不同大小的图片，常见的有256，48，32，16，并且每个大小都有32位真彩、256色、16色之分）
    //目前ICO文件在加载时，只会选择一个大小的ICO图片进行加载，加载后为单张图片
    uint32_t m_nIconSize;

    //PAG格式默认播放的最大帧率（仅限PAG格式）
    float m_fPagMaxFrameRate;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_LOAD_PARAM_H_
