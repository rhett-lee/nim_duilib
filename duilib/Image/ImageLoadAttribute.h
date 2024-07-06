#ifndef UI_IMAGE_IMAGE_LOAD_ATTRIBUTE_H_
#define UI_IMAGE_IMAGE_LOAD_ATTRIBUTE_H_

#include "duilib/Core/UiTypes.h"

namespace ui 
{
class Control;

/** 图片加载属性，用于加载一个图片
*/
class UILIB_API ImageLoadAttribute
{
public:
    ImageLoadAttribute(DString srcWidth,
                       DString srcHeight,
                       bool srcDpiScale,
                       bool bHasSrcDpiScale,
                       uint32_t iconSize);

    /** 设置图片路径（本地绝对路径或者压缩包内的相对路径）
    */
    void SetImageFullPath(const DString& imageFullPath);

    /** 获取图片路径（UTF8或者UTF16编码，本地绝对路径或者压缩包内的相对路径）
    */
    DString GetImageFullPath() const;

    /** 是否含有图片路径
    */
    bool HasImageFullPath() const;

    /** 获取加载图片的缓存KEY
    *   完整的格式是：<图片完整路径>@<界面缩放百分比>@<宽度>:<高度>
    *          举例: "C:\Test.jpg@200@80:40"
    * @param [in] nDpiScale 请求图片对应的DPI缩放百分比
    */
    DString GetCacheKey(uint32_t nDpiScale) const;

    /** 设置加载图片时，是否需要按照DPI缩放图片大小
    */
    void SetNeedDpiScale(bool bNeedDpiScale);

    /** 获取加载图片时，是否需要按照DPI缩放图片大小
    */
    bool NeedDpiScale() const;

    /** 获取加载图片时，是否设置了DPI自适应属性（配置XML文件中，可以通过设置："file='test.png' dpi_scale='false'"）
    */
    bool HasSrcDpiScale() const;

    /** 获取图片加载后应当缩放的宽度和高度（如果只设置了宽度或者高度，那么会按锁定纵横比的方式对整个图片进行缩放）
    * @param [in,out] nImageWidth 传入原始图片的宽度，返回计算后的宽度
    * @param [in,out] nImageHeight 传入原始图片的高度，返回计算后的高度
    * @return 返回true表示图片大小有缩放，返回false表示图片大小无缩放
    */
    bool CalcImageLoadSize(uint32_t& nImageWidth, uint32_t& nImageHeight) const;

    /** 如果是ICO文件，用于指定需要加载的ICO图片的大小
    */
    uint32_t GetIconSize() const;

private:
    /** 获取设置的缩放后的大小
    * @return 返回缩放后的大小，如果失败则返回0
    */
    uint32_t GetScacledSize(const DString& srcSize, uint32_t nImageSize) const;

private:
    //(属性名称："file")本地绝对路径或者压缩包内的相对路径，不包含属性
    UiString m_srcImageFullPath;

    //设置图片宽度(属性名称："width")，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
    UiString m_srcWidth;

    //设置图片高度(属性名称："height")，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
    UiString m_srcHeight;

    //加载图片时，按照DPI缩放图片大小(属性名称："dpi_scale")
    bool m_srcDpiScale;

    //加载图片时，是否设置了DPI自适应属性
    bool m_bHasSrcDpiScale;

    //如果是ICO文件，用于指定需要加载的ICO图片的大小（属性名称："icon_size"）
    //(ICO文件中包含很多个不同大小的图片，常见的有256，48，32，16，并且每个大小都有32位真彩、256色、16色之分）
    //目前ICO文件在加载时，只会选择一个大小的ICO图片进行加载，加载后为单张图片
    uint32_t m_iconSize;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_LOAD_ATTRIBUTE_H_
