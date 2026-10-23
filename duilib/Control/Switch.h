#ifndef UI_CONTROL_SWITCH_H_
#define UI_CONTROL_SWITCH_H_

#include "duilib/Control/CheckBox.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Image/Image.h"

namespace ui {

/** Switch 滑块开关控件（继承CheckBox）
 *  圆角轨道 + 圆形滑块，选中状态切换时带有滑块滑动与颜色/图片淡入淡出过渡动画。
 *  轨道和滑块均支持 SVG 图片皮肤：配置图片后，off/on 两张图片按动画进度交叉淡入淡出，
 *  滑块图片同时沿轨道平移；未配置图片时使用语义色自绘（FillRoundRect + 颜色插值）。
 *  默认皮肤类 switch（iOS 风格）、switch_fluent（Windows 11 Fluent 风格）、
 *  switch_material（Material Design 3 风格）。
 *  选中状态与事件与CheckBox一致（kEventSelect / kEventUnSelect）。
 */
class DUILIB_API Switch : public CheckBox
{
    typedef CheckBox BaseClass;
public:
    explicit Switch(Window* pWindow);
    Switch(const Switch& r) = delete;
    Switch& operator=(const Switch& r) = delete;
    ~Switch() override;

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintStateColors(IRender* pRender) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void PaintText(IRender* pRender) override;
    virtual void Activate(const EventArgs* pMsg) override;
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

public:
    /** 设置切换动画时长（毫秒），0表示不使用动画
    */
    void SetAnimationDuration(int32_t nMs);

    /** 获取切换动画时长（毫秒）
    */
    int32_t GetAnimationDuration() const { return m_nAnimationMs; }

    /** 设置/获取轨道颜色（未选中状态）
    */
    void SetTrackOffColor(const DString& strColor);
    DString GetTrackOffColor() const { return m_strTrackOffColor; }

    /** 设置/获取轨道颜色（选中状态）
    */
    void SetTrackOnColor(const DString& strColor);
    DString GetTrackOnColor() const { return m_strTrackOnColor; }

    /** 设置/获取滑块颜色（未选中状态）
    */
    void SetThumbOffColor(const DString& strColor);
    DString GetThumbOffColor() const { return m_strThumbOffColor; }

    /** 设置/获取滑块颜色（选中状态）
    */
    void SetThumbOnColor(const DString& strColor);
    DString GetThumbOnColor() const { return m_strThumbOnColor; }

    /** 设置轨道图片（未选中状态），strImage 为空表示清除，改用颜色自绘
    */
    void SetTrackOffImage(const DString& strImage);
    DString GetTrackOffImage() const { return m_strTrackOffImage; }

    /** 设置轨道图片（选中状态），strImage 为空表示清除，改用颜色自绘
    */
    void SetTrackOnImage(const DString& strImage);
    DString GetTrackOnImage() const { return m_strTrackOnImage; }

    /** 设置滑块图片（未选中状态），strImage 为空表示清除，改用颜色自绘
    */
    void SetThumbOffImage(const DString& strImage);
    DString GetThumbOffImage() const { return m_strThumbOffImage; }

    /** 设置滑块图片（选中状态），strImage 为空表示清除，改用颜色自绘
    */
    void SetThumbOnImage(const DString& strImage);
    DString GetThumbOnImage() const { return m_strThumbOnImage; }

    /** 设置滑块图片方形框与轨道边缘的距离（会做DPI自适应），默认0
    *   备注：滑块图片方形框边长等于轨道高度，阴影等留白请绘制在图片内部
    */
    void SetThumbPadding(int32_t nPadding, bool bNeedDpiScale = true);
    int32_t GetThumbPadding() const { return m_nThumbPadding; }

    /** 设置/获取轨道内文字（选中状态）
    */
    void SetTextOn(const DString& strText);
    DString GetTextOn() const { return m_strTextOn; }

    /** 设置/获取轨道内文字（未选中状态）
    */
    void SetTextOff(const DString& strText);
    DString GetTextOff() const { return m_strTextOff; }

    /** 设置/获取轨道内文字颜色（选中状态）
    */
    void SetTextOnColor(const DString& strColor);
    DString GetTextOnColor() const { return m_strTextOnColor; }

    /** 设置/获取轨道内文字颜色（未选中状态）
    */
    void SetTextOffColor(const DString& strColor);
    DString GetTextOffColor() const { return m_strTextOffColor; }

    /** 设置轨道内图标（选中状态），strImage 为空表示清除
    */
    void SetImageOn(const DString& strImage);
    DString GetImageOn() const { return m_strImageOn; }

    /** 设置轨道内图标（未选中状态），strImage 为空表示清除
    */
    void SetImageOff(const DString& strImage);
    DString GetImageOff() const { return m_strImageOff; }

    /** 设置/获取加载中状态（加载中时在滑块内绘制旋转指示器，并禁止用户点击切换）
    */
    void SetLoading(bool bLoading);
    bool IsLoading() const { return m_bLoading; }

    /** 设置/获取加载指示器颜色
    */
    void SetLoadingColor(const DString& strColor);
    DString GetLoadingColor() const { return m_strLoadingColor; }

    /** 设置/获取禁用状态的轨道颜色
    */
    void SetTrackDisabledColor(const DString& strColor);
    DString GetTrackDisabledColor() const { return m_strTrackDisabledColor; }

    /** 设置/获取禁用状态的滑块颜色
    */
    void SetThumbDisabledColor(const DString& strColor);
    DString GetThumbDisabledColor() const { return m_strThumbDisabledColor; }

protected:
    /** 选中状态变化时触发（来自CheckBox），用于启动切换动画
    */
    virtual void OnPrivateSetSelected() override;

private:
    /** 计算滑块图片方形框（按当前动画进度沿轨道平移，垂直居中）
    */
    UiRect GetThumbRect() const;

    /** 获取有效动画进度：禁用时直接返回目标值（0 或 100）
    */
    int32_t GetEffectiveAnimValue() const;

    /** 绘制轨道内文字/图标（可用区域内居中，图标与文字可组合）
    * @param [in] rcContent 可用区域
    * @param [in] strText   文字内容
    * @param [in] pImage    图标（可为空）
    * @param [in] textColor 文字颜色
    * @param [in] nAlpha    透明度[0, 255]
    */
    void PaintSwitchContent(IRender* pRender, const UiRect& rcContent, const DString& strText,
                            Image* pImage, UiColor textColor, uint8_t nAlpha) const;

    /** 绘制加载中的旋转指示器
    */
    void PaintLoadingIndicator(IRender* pRender, const UiRect& rcThumb) const;

    /** 加载动画定时器回调
    */
    void OnLoadingTimer();

    /** 两颜色按进度插值
    * @param [in] t 进度值，范围[0, 1]
    */
    static UiColor LerpColor(UiColor colorFrom, UiColor colorTo, double t);

    /** 设置/更新一张图片（strImage 为空时释放）
    */
    void UpdateSwitchImage(std::unique_ptr<Image>& pImage, const DString& strImage);

    /** 按指定透明度把图片绘制到目标矩形
    * @param [in] nAlpha 图片透明度，范围[0, 255]
    */
    void DrawSwitchImage(IRender* pRender, Image* pImage, const UiRect& rcDest, uint8_t nAlpha) const;

    /** 轨道是否配置了一对可用图片（off/on 必须成对配置）
    */
    bool HasTrackImages() const;

    /** 滑块是否配置了一对可用图片（off/on 必须成对配置）
    */
    bool HasThumbImages() const;

private:
    /** 动画时长（毫秒）
    */
    int32_t m_nAnimationMs;

    /** 当前动画进度（0:未选中, 100:选中）
    */
    int32_t m_nAnimValue;

    DString m_strTrackOffColor;     //轨道颜色（未选中）
    DString m_strTrackOnColor;      //轨道颜色（选中）
    DString m_strThumbOffColor;     //滑块颜色（未选中）
    DString m_strThumbOnColor;      //滑块颜色（选中）

    DString m_strTrackOffImage;     //轨道图片属性（未选中）
    DString m_strTrackOnImage;      //轨道图片属性（选中）
    DString m_strThumbOffImage;     //滑块图片属性（未选中）
    DString m_strThumbOnImage;      //滑块图片属性（选中）

    DString m_strTextOn;            //轨道内文字（选中）
    DString m_strTextOff;           //轨道内文字（未选中）
    DString m_strTextOnColor;       //轨道内文字颜色（选中）
    DString m_strTextOffColor;      //轨道内文字颜色（未选中）
    DString m_strImageOn;           //轨道内图标属性（选中）
    DString m_strImageOff;          //轨道内图标属性（未选中）
    DString m_strTrackDisabledColor; //禁用状态的轨道颜色
    DString m_strThumbDisabledColor; //禁用状态的滑块颜色
    DString m_strLoadingColor;       //加载指示器颜色

    /** 滑块图片方形框与轨道边缘的距离（DPI缩放后的物理像素）
    */
    int32_t m_nThumbPadding;

    std::unique_ptr<Image> m_pTrackOffImage;
    std::unique_ptr<Image> m_pTrackOnImage;
    std::unique_ptr<Image> m_pThumbOffImage;
    std::unique_ptr<Image> m_pThumbOnImage;
    std::unique_ptr<Image> m_pImageOn;
    std::unique_ptr<Image> m_pImageOff;

    /** 加载中状态
    */
    bool m_bLoading;

    /** 加载指示器的旋转角度（度）
    */
    int32_t m_nLoadingAngle;

    /** 加载指示器定时器
    */
    WeakCallbackFlag m_loadingTimer;

    /** 切换动画播放器
    */
    std::unique_ptr<AnimationPlayer> m_pAnimationPlayer;
};

} // namespace ui

#endif // UI_CONTROL_SWITCH_H_
