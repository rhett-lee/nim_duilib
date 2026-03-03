#ifndef UI_CONTROL_BITMAP_CONTROL_H_
#define UI_CONTROL_BITMAP_CONTROL_H_

#include "duilib/Core/Box.h"
#include <mutex>

namespace ui
{
class IBitmap;

/** 用于显示位图数据的控件，适合显示基于内存的位图数据，并且支持图片资源的各种属性
 *  位图的数据格式为32位的位图(外部处理)：
 *    Windows平台为BGRA: struct BGRA { uint8_t b, g, r, a; };
 *    非Windows平台RGBA: struct RGBA { uint8_t r, g, b, a; };
 *  位图的alpha type为kPremul_SkAlphaType(外部预乘)
 *  线程模型：
 *     （1）对于位图数据操作相关的函数：支持多线程，可以在子线程中操作位图数据，位图的绘制是在UI线程中进行的
 *     （2）对于非位图数据操作相关的函数：只能在UI线程中调用，不支持多线程
 */
class BitmapControl : public Box
{
    typedef Box BaseClass;
public:
    explicit BitmapControl(Window* pWindow);
    virtual ~BitmapControl() override;
    BitmapControl(const BitmapControl&) = delete;
    BitmapControl& operator=(const BitmapControl&) = delete;

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 计算图片区域大小（宽和高）
     *  @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
     *  @param [in] estImageType 估算图片的类型
     *  @return 控件的图片估算大小，包含内边距(Box)，不包含外边距
     */
    virtual UiSize EstimateImage(UiSize szAvailable, EstimateImageType estImageType) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

public:
    /** 设置位图数据，数据会被复制一份保存
     * @param [in] nWidth 宽度
     * @param [in] nHeight 高度
     * @param [in] pPixelBits 位图数据
     * @param [in] nPixelBitsSize 位图数据的长度（按字节）
     */
    bool SetBitmapData(int32_t nWidth, int32_t nHeight, const uint8_t* pPixelBits, int32_t nPixelBitsSize);

    /** 设置位图数据，数据会被复制一份保存
     * @param [in] pBitmap 位图数据接口
     */
    bool SetBitmapDataWithCopy(IBitmap* pBitmap);

    /** 清除位图数据
    */
    void ClearBitmapData();

    /** 获取当前是否含有位图数据
    */
    bool HasBitmapData();

public:
    /** 设置位图图片绘制时水平对齐方式
    */
    void SetBitmapHAlignType(HorAlignType hAlignType);

    /** 获取位图图片绘制时水平对齐方式
    */
    HorAlignType GetBitmapHAlignType() const;

    /** 设置位图图片绘制时垂直对齐方式
    */
    void SetBitmapVAlignType(VerAlignType vAlignType);

    /** 获取位图图片绘制时垂直对齐方式
    */
    VerAlignType GetBitmapVAlignType() const;

    /** 设置绘制图片时的透明度（0 - 255）
    */
    void SetBitmapAlpha(uint8_t nBitmapAlpha);

    /** 获取绘制图片时的透明度（0 - 255）
    */
    uint8_t GetBitmapAlpha() const;

    /** 设置绘制目标区域位置和大小(相对于控件区域的位置)
    * @param [in] rcDest 绘制目标区域位置和大小
    * @param [in] bNeedDpiScale 是否需要DPI缩放
    */
    void SetBitmapDest(UiRect rcDest, bool bNeedDpiScale);

    /** 获取绘制目标区域位置和大小(相对于控件区域的位置)
    * @return 返回绘制目标区域位置和大小，已做过DPI缩放
    */
    UiRect GetBitmapDest() const;

    /** 是否设置了绘制目标区域位置和大小(相对于控件区域的位置)
    */
    bool HasBitmapDest() const;

    /** 设置绘制源区域位置和大小
    * @param [in] rcSource 绘制源区域位置和大小
    * @param [in] bNeedDpiScale 是否需要DPI缩放
    */
    void SetBitmapSource(UiRect rcSource, bool bNeedDpiScale);

    /** 获取绘制源区域位置和大小
    * @return 返回绘制目标区域位置和大小，已做过DPI缩放
    */
    UiRect GetBitmapSource() const;

    /** 是否设置了绘制源区域位置和大小
    */
    bool HasBitmapSource() const;

    /** 设置绘制目标区域中的外边距(如果指定了rcDest值，此值无效)
    * @param [in] rcMargin 绘制源区域位置和大小
    * @param [in] bNeedDpiScale 是否需要DPI缩放
    */
    void SetBitmapMargin(UiMargin rcMargin, bool bNeedDpiScale);

    /** 获取绘制目标区域中的外边距(如果指定了rcDest值，此值无效)
    * @return 返回绘制目标区域位置和大小，已做过DPI缩放
    */
    UiMargin GetBitmapMargin() const;

    /** 是否设置了绘制目标区域中的外边距(如果指定了rcDest值，此值无效)
    */
    bool HasBitmapMargin() const;

    /** 设置绘制时是否自动适应目标区域（等比例缩放图片）
    * @param [in] bAdaptiveDestRect true表示绘制时自动适应目标区域（等比例缩放图片）
    */
    void SetAdaptiveDestRect(bool bAdaptiveDestRect);

    /** 获取绘制时是否自动适应目标区域（等比例缩放图片）
    */
    bool IsAdaptiveDestRect() const;

    /** 设置绘制时是否拉伸绘制图片（与IsAdaptiveDestRect()互斥，优先级低于IsAdaptiveDestRect()）
    * @param [in] bStretchedDrawing true表示绘制时拉伸绘制图片，false表示绘制时不拉伸绘制图片
    */
    void SetStretchedDrawing(bool bStretchedDrawing);

    /** 获取绘制时绘制时是否拉伸绘制图片
    */
    bool IsStretchedDrawing() const;

    /** 设置是否支持多线程操作位图数据（如果无调用，则默认为true，默认是支持多线程操作位图数据的）
    * @param [in] bSupportMultiThread true表示支持多线程操作位图数据，false表示不支持多线程操作位图数据
    */
    void SetSupportMultiThread(bool bSupportMultiThread);

    /** 获取是否支持多线程操作位图数据
    */
    bool IsSupportMultiThread() const;

protected:
    /** 重写父控件绘制函数
    */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

private:
    /** 获取位图图片的宽度和高度
    */
    void GetBitmapSize(int32_t& nImageWidth, int32_t& nImageHeight);

    /** 绘制图片
    */
    void PaintBitmap(IRender* pRender, const UiRect& rcPaint);

    /** 加载指定的图片
    */
    void CheckLoadBitmapFile();

private:
    // 位图数据锁，支持在子线程中操作位图数据
    std::mutex m_bitmapMutex;

    //用于绘制的位图数据
    std::unique_ptr<IBitmap> m_pBitmap;

    //关联的图片文件：主要用于测试
    UiString m_bitmapFile;

    //绘制目标区域位置和大小(相对于控件区域的位置)
    std::unique_ptr<UiRect> m_rcDest;

    //图片源区域位置和大小
    std::unique_ptr<UiRect> m_rcSource;

    //在绘制目标区域中的外边距(如果指定了rcDest值，此值无效)
    std::unique_ptr<UiMargin> m_rcMargin;

    //图片的水平对齐方式
    HorAlignType m_hAlignType;

    //图片的垂直对齐方式
    VerAlignType m_vAlignType;

    //透明度（0 - 255）
    uint8_t m_nBitmapAlpha;

    //是否自动适应目标区域（等比例缩放图片）
    bool m_bAdaptiveDestRect;

    //是否拉伸绘制图片
    bool m_bStretchedDrawing;

    //是否支持多线程操作位图数据
    bool m_bSupportMultiThread;
};

}//namespace ui

#endif //UI_CONTROL_BITMAP_CONTROL_H_
