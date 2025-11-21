#ifndef UI_IMAGE_IMAGE_ATTRIBUTE_H_
#define UI_IMAGE_IMAGE_ATTRIBUTE_H_

#include "duilib/Core/UiTypes.h"

namespace ui 
{
class DpiManager;

/** 图片属性
*/
class UILIB_API ImageAttribute
{
public:
    ImageAttribute();
    ~ImageAttribute();
    ImageAttribute(const ImageAttribute&);
    ImageAttribute& operator=(const ImageAttribute&);

    /** 对数据成员进行初始化
    */
    void Init();

    /** 根据图片参数进行初始化(先调用Init初始化成员变量，再按照传入参数进行更新部分属性)
    * @param [in] strImageString 图片参数字符串
    * @param [in] dpi DPI缩放接口
    */
    void InitByImageString(const DString& strImageString, const DpiManager& dpi);

    /** 根据图片参数修改属性值（仅更新新设置的图片属性, 未包含的属性不进行更新）
    * @param [in] strImageString 图片参数字符串
    * @param [in] dpi DPI缩放接口
    */
    void ModifyAttribute(const DString& strImageString, const DpiManager& dpi);

public:
    /** 判断rcDest区域是否含有有效值
    * @param [in] rcDest 需要判断的区域
    */
    static bool HasValidImageRect(const UiRect& rcDest);

    /** 计算保持比例的自适应绘制区域
     * @param nImageWidth 原始图片宽度
     * @param nImageHeight 原始图片高度
     * @param targetRect 目标矩形区域(left, top, right, bottom)
     * @param hAlign 横向对齐方式 (left, center, right)
     * @param vAlign 纵向对齐方式 (top, center, bottom)
     * @return 自适应后的绘制区域(left, top, right, bottom)
     */
    static UiRect CalculateAdaptiveRect(int32_t nImageWidth, int32_t nImageHeight,
                                        const UiRect& targetRect,
                                        const DString& hAlign,
                                        const DString& vAlign);

public:
    /** 获取rcSource(未进行DPI缩放)
    */
    UiRect GetImageSourceRect() const;

    /** 获取rcCorner(未进行DPI缩放)
    */
    UiRect GetImageCorner() const;

    /** 是否含有rcCorner属性
    */
    bool HasImageCorner() const;

    /** 获取rcDest(按配置决定是否进行DPI缩放)
    * @param [in] imageWidth 图像的宽度
    * @param [in] imageHeight 图像的高度
    * @param [in] dpi DPI缩放接口
    */
    UiRect GetImageDestRect(int32_t imageWidth, int32_t imageHeight, const DpiManager& dpi) const;

    /** 是否含有rcDest属性
    */
    bool HasDestRect() const;

    /** 获取图片属性的外边距
    * @param [in] dpi DPI缩放管理器
    * @return 返回按照传入DPI缩放管理器适应的内边距数据
    */
    UiMargin GetImageMargin(const DpiManager& dpi) const;

    /** 设置图片属性的外边距
    * @param [in] newMargin 需要设置的内边距
    * @param [in] bNeedDpiScale 是否需要对newMargin进行DPI缩放
    * @param [in] dpi 与newPadding数据关联的DPI管理器
    */
    void SetImageMargin(const UiMargin& neMargin, bool bNeedDpiScale, const DpiManager& dpi);

    /** 判断图片是否平铺绘制
    */
    bool IsTiledDraw() const;

    /** 获取图片平铺绘制的属性
    */
    TiledDrawParam GetTiledDrawParam(const DpiManager& dpi) const;

    /** 是否启用图片加载失败时的断言错误
    */
    bool IsAssertEnabled() const;

    /** 获取图片的名称（可作为图片的唯一ID）
    */
    DString GetImageName() const;

public:
    //图片文件属性字符串
    UiString m_sImageString;

    //图片文件文件名，含相对路径，不包含属性
    UiString m_sImagePath;

    //图片名称（控件内唯一字符串，用于标识图片资源）
    UiString m_sImageName;

    //设置图片宽度，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
    UiString m_srcWidth;

    //设置图片高度，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
    UiString m_srcHeight;

    //在绘制目标区域中横向对齐方式(如果指定了rcDest值，则此选项无效)
    UiString m_hAlign;

    //在绘制目标区域中纵向对齐方式(如果指定了rcDest值，则此选项无效)
    UiString m_vAlign;

    //平铺绘制相关参数
    std::unique_ptr<TiledDrawParam> m_pTiledDrawParam;

    //透明度（0 - 255）
    uint8_t m_bFade;

    //加载图片时是否支持DPI自适应，即按照DPI缩放图片大小（"dpi_scale"）
    bool m_bImageDpiScaleEnabled;

    //rcDest属性是否支持DPI自适应，即按照DPI缩放（"dest_scale"）
    bool m_bDestDpiScaleEnabled;

    //九宫格绘制时，不绘制中间部分（比如窗口阴影，只需要绘制边框，不需要绘制中间部分，以避免不必要的绘制动作）
    bool m_bWindowShadowMode;

    //是否自动适应目标区域（等比例缩放图片）
    bool m_bAdaptiveDestRect;

    //如果是动画图片，是否自动播放
    bool m_bAutoPlay;

    //该图片是否支持异步加载（即放在子线程中加载图片数据，避免主界面卡顿）
    bool m_bAsyncLoad;

    //如果是动画图片，可以指定播放次数
    //    - 1: 表示一直播放
    //    0  : 表示无有效的播放次数，使用图片的默认值(或者预设值)
    //    > 0: 具体的播放次数，达到播放次数后，停止播放
    int32_t m_nPlayCount;

    //如果是PAG文件，用于指定动画的帧率，默认为30.0f
    float m_fPagMaxFrameRate;

    //如果是ICO文件，用于指定需要加载的ICO图片的大小
    uint32_t m_nIconSize;

    //如果是ICO文件，当按多帧图片显示时，每帧播放的时间间隔，毫秒
    //仅当m_bIconAsAnimation为true时有效
    int32_t m_nIconFrameDelayMs;

    //如果是ICO文件，指定是否按多帧图片加载（按动画图片显示）
    bool m_bIconAsAnimation;

    //可绘制标志：true表示允许绘制，false表示禁止绘制
    bool m_bPaintEnabled;

private:
    //图片加载失败时，代码断言的设置（debug编译时启用，用于排查图片加载过程中的错误，尤其时图片数据错误导致加载失败的问题）
    bool m_bAssertEnabled;

    //rcMargin对应的DPI缩放百分比
    uint16_t m_rcMarginScale;

    //绘制目标区域位置和大小(相对于控件区域的位置, 未进行DPI缩放)
    UiRect* m_rcDest;

    //在绘制目标区域中的外边距(如果指定了rcDest值，则此选项无效)
    UiMargin16* m_rcMargin;

    //图片源区域位置和大小(未进行DPI缩放)
    UiRect* m_rcSource;

    //圆角属性(未进行DPI缩放)
    UiRect* m_rcCorner;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_ATTRIBUTE_H_
