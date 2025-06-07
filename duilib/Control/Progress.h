#ifndef UI_CONTROL_PROGRESS_H_
#define UI_CONTROL_PROGRESS_H_

#include "duilib/Control/Label.h"
#include "duilib/Image/Image.h"

namespace ui
{

/** 进度条控件
*/
class UILIB_API Progress : public LabelTemplate<Control>
{
    typedef LabelTemplate<Control> BaseClass;
public:
    explicit Progress(Window* pWindow);
    virtual ~Progress() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void ClearImageCache() override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 判断是否是水平进度条
     * @return 返回 true 是水平进度条，false 为垂直进度条
     */
    bool IsHorizontal() const;

    /** 设置水平或垂直进度条
     * @param [in] bHorizontal 为 true 时设置为水平进度条，false 时设置为垂直进度条，默认为 true
     */
    void SetHorizontal(bool bHorizontal = true);

    /** 获取进度条最小值
     * @return 返回进度条最小值
     */
    int32_t GetMinValue() const;

    /** 设置进度条最小值
     * @param [in] nMin 最小值数值
     */
    void SetMinValue(int32_t nMin);

    /** 获取进度条最大值
     * @return 返回进度条最大值
     */
    int32_t GetMaxValue() const;

    /** 设置进度条最大值
     * @param [in] nMax 要设置的最大值
     */
    void SetMaxValue(int32_t nMax);

    /** 获取当前进度百分比
     * @return 返回当前进度百分比
     */
    double GetValue() const;

    /** 设置当前进度百分比
     * @param[in] fValue 要设置的百分比数值
     */
    void SetValue(double fValue);

    /** 进度条前景图片是否缩放显示
     * @return 返回 true 为缩放显示，false 为不缩放显示
     */
    bool IsStretchForeImage();

    /** 设置进度条前景图片是否缩放显示
     * @param [in] bStretchForeImage true 为缩放显示，false 为不缩放显示
     */
    void SetStretchForeImage(bool bStretchForeImage = true);

    /** 获取滚动条背景图片
     * @return 返回背景图片位置
     */
    DString GetProgressImage() const;

    /** 设置进度条背景图片
     * @param [in] strImage 图片地址
     */
    void SetProgressImage(const DString& strImage);

    /** 获取进度条背景颜色
     * @return 返回背景颜色的字符串值，对应 global.xml 中的指定色值
     */
    DString GetProgressColor() const;

    /** 设置进度条背景颜色
     * @param [in] 要设置的背景颜色字符串，该字符串必须在 global.xml 中存在
     */
    void SetProgressColor(const DString& strProgressColor);

    /** 获取进度条位置
    * @return 返回进度条区域坐标（以GetRect()为整个区域的相对坐标，左上角为<0,0>）
    */
    virtual UiRect GetProgressPos();

    /** 播放Marquee
     */
    virtual void Play();

    /** 绘制Marquee
     */
    void PaintMarquee(IRender* pRender);

    /** 是否是Marquee
     */
    bool IsMarquee() const;

    /** 设置Marquee
     */
    void SetMarquee(bool bMarquee);

    /** 获取Marquee宽度
     */
    int32_t GetMarqueeWidth() const;

    /** 设置Marquee宽度
     */
    void SetMarqueeWidth(int32_t nMarqueeWidth, bool bNeedDpiScale);

    /** 获取Marquee步长
     */
    int32_t GetMarqueeStep() const;

    /** 设置Marquee步长
     */
    void SetMarqueeStep(int32_t nMarqueeStep, bool bNeedDpiScale);

    /** 获取Marquee持续时间
     */
    int32_t GetMarqueeElapsed() const;

    /** 设置Marquee持续时间
     */
    void SetMarqueeElapsed(int32_t nMarqueeElapsed);

public:
    /** 设置是否反向填充进度条
    */
    void SetReverse(bool bReverse);

    /** 是否反向填充进度条
    */
    bool IsReverse() const;

private:
    //进度条是水平或垂直: true为水平，false为垂直
    bool m_bHorizontal;

    //指定进度条前景图片是否缩放显示
    bool m_bStretchForeImage;

    //进度最大值（默认为100）
    int32_t m_nMaxValue;

    //进度最小值（默认为0）
    int32_t m_nMinValue;

    //当前进度值
    double m_fCurrentValue;

    //进度条前景图片
    Image* m_pProgressImage;

    //进度条前景图片属性
    UiString m_sProgressImageModify;

    //进度条前景颜色,不指定则使用默认颜色
    UiString m_sProgressColor;

    //滚动效果
    bool m_bMarquee;
    int32_t m_nMarqueeWidth;
    int32_t m_nMarqueeStep;
    int32_t m_nMarqueeElapsed;
    int32_t m_nMarqueePos;

private:
    //是否倒数（进度从100 到 0）
    bool m_bReverse;

    //定时器取消机制
    WeakCallbackFlag m_timer;
};

} // namespace ui

#endif // UI_CONTROL_PROGRESS_H_
