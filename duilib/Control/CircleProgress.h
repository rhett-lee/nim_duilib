/** @file CircleProgress.h
* @brief 环型进度条控件，圆环中间可以有文本（如85%）
* @copyright (c) 2019-2022, NetEase Inc. All rights reserved
* @author Xuhuajie
* @date 2019/8/14
*/

#ifndef UI_CONTROL_CIRCLEPROGRESS_H_
#define UI_CONTROL_CIRCLEPROGRESS_H_

#include "duilib/Control/Progress.h"
#include <functional>

namespace ui
{

/** 中心内容区绘制回调
*   @param pRender渲染接口
*   @param rcCenter 中心内容区矩形（控件内相对坐标，已按 center_ratio 收缩）
*   @param fValue 当前进度值
*   @return 返回 true 表示已自行绘制（跳过默认文本绘制）
*/
typedef std::function<bool(IRender* pRender, const UiRect& rcCenter, double fValue)> CircleCenterCallback;

class DUILIB_API CircleProgress : public Progress
{
    typedef Progress BaseClass;
public:
    explicit CircleProgress(Window* pWindow);
    virtual ~CircleProgress() override;

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

    /** 设置环型进度条，默认为普通进度条
    * @param [in] bCircular 为 true 时设置为环型进度条，false 时设置为父级进度条样式，默认为 true
    */
    void SetCircular(bool bCircular = true);

    /** 设置递增方向
    * @param [in] bClockwise 为 true 时设置为顺时针，false 时设置为逆时针，默认为 true
    */
    void SetClockwiseRotation(bool bClockwise = true);

    /** 设置圆环宽度
    * @param [in] fCircleWidth 宽度数值
    * @param [in] bNeedDpiScale 是否支持DPI缩放
    */
    void SetCircleWidth(float fCircleWidth, bool bNeedDpiScale);

    /** 获取圆环宽度
    */
    float GetCircleWidth() const;

    /** 设置进度条背景颜色
    * @param [in] strColor要设置的背景颜色字符串，该字符串必须在 global.xml 中存在
    */
    void SetBackgroundColor(const DString& strColor);

    /** 设置进度条前景颜色
    * @param [in] strColor要设置的前景颜色字符串，该字符串必须在 global.xml 中存在
    */
    void SetForegroundColor(const DString& strColor);

    /** 设置进度条前景渐变颜色，与 SetForegroundColor 同时使用，可以不设置,则无渐变效果
    * @param [in] strColor要设置的前景渐变颜色字符串，该字符串必须在 global.xml 中存在
    */
    void SetCircleGradientColor(const DString& strColor);

    /** 设置进度指示移动图标
    * @param [in] sIndicatorImage 要设置的图片
    */
    void SetIndicator(const DString& sIndicatorImage);

    /** 设置是否为不确定进度（indeterminate，忽略 value，圆环一段弧持续旋转）
    * @param [in] bIndeterminate 为 true 时启用不确定进度模式
    */
    void SetIndeterminate(bool bIndeterminate);

    /** 获取是否为不确定进度
    */
    bool IsIndeterminate() const { return m_bIndeterminate; }

    /** 设置弧的起始角度（度）
    * @param [in] fAngle 起始角度，0 表示 3 点钟方向，顺时针增加；默认 270 表示 12 点方向
    */
    void SetStartAngle(float fAngle);

    /** 获取弧的起始角度（度）
    */
    float GetStartAngle() const { return m_fStartAngle; }

    /** 设置弧的总跨度角度（度），即 value=min 与 value=max 之间扫过的角度
    * @param [in] fSpan 跨度角度，取值范围 (0, 360]；360 为整圆，180 为半圆表盘
    */
    void SetSpanAngle(float fSpan);

    /** 获取弧的总跨度角度（度）
    */
    float GetSpanAngle() const { return m_fSpanAngle; }

    /** 设置不确定进度模式下旋转弧的角度
    * @param [in] fArc 弧长（度），取值范围 (0, 360]，默认 90
    */
    void SetIndeterminateArc(float fArc);

    /** 获取不确定进度模式下旋转弧的角度
    */
    float GetIndeterminateArc() const { return m_fIndeterminateArc; }

    /** 设置不确定进度模式下每帧旋转的角度
    * @param [in] fSpeed 每帧角度（度），默认 6
    */
    void SetIndeterminateSpeed(float fSpeed);

    /** 获取不确定进度模式下每帧旋转的角度
    */
    float GetIndeterminateSpeed() const { return m_fIndeterminateSpeed; }

    /** 设置不确定进度动画的刷新间隔（毫秒，默认 16）
    */
    void SetIndeterminateInterval(int32_t nInterval);

    /** 获取不确定进度动画的刷新间隔（毫秒）
    */
    int32_t GetIndeterminateInterval() const { return m_nIndeterminateInterval; }

    /** 设置中心内容区占控件尺寸的比例（0~1，默认 1.0 即整个控件内切圆）
    * @param [in] fRatio 比例，用于给环留出边距、放置仪表盘指针等
    */
    void SetCenterRatio(float fRatio);

    /** 获取中心内容区占控件尺寸的比例
    */
    float GetCenterRatio() const { return m_fCenterRatio; }

    /** 设置中心内容区内边距（像素，DPI 缩放），格式 "left,top,right,bottom"
    */
    void SetCenterPadding(const UiPadding& padding, bool bNeedDpiScale = true);

    /** 获取中心内容区内边距
    */
    const UiPadding& GetCenterPadding() const { return m_rcCenterPadding; }

    /** 设置中心内容区自定义绘制回调（可绘制图标、图片、指针等）
    * @param [in] callback 绘制回调，置空则恢复默认的文本绘制
    */
    void SetCenterCallback(const CircleCenterCallback& callback);

    /** 获取中心内容区矩形（控件内相对坐标）
    */
    UiRect GetCenterRect() const;

private:
    /** 不确定进度定时回调（旋转弧）
    */
    void OnIndeterminateTimer();

    /** 绘制中心内容（文本或自定义回调）
    */
    void PaintCenterContent(IRender* pRender, double fValue);

protected:
    bool m_bCircular;
    bool m_bClockwise;
    float m_fCircleWidth;
    UiColor m_dwBackgroundColor;
    UiColor m_dwForegroundColor;
    UiColor m_dwGradientColor;
    Image* m_pIndicatorImage;

    /** 是否不确定进度
    */
    bool m_bIndeterminate;

    /** 不确定进度的当前旋转角度（度）
    */
    float m_fIndeterminateAngle;

    /** 不确定进度定时器取消机制
    */
    WeakCallbackFlag m_indeterminateTimer;

    /** 弧的起始角度（度），默认 270（12 点方向）
    */
    float m_fStartAngle;

    /** 弧的总跨度角度（度），默认 360（整圆）
    */
    float m_fSpanAngle;

    /** 不确定进度的弧长（度），默认 90
    */
    float m_fIndeterminateArc;

    /** 不确定进度每帧旋转角度（度），默认 6
    */
    float m_fIndeterminateSpeed;

    /** 不确定进度的刷新间隔（毫秒），默认 16
    */
    int32_t m_nIndeterminateInterval;

    /** 中心内容区占控件尺寸的比例
    */
    float m_fCenterRatio;

    /** 中心内容区内边距
    */
    UiPadding m_rcCenterPadding;

    /** 中心内容区自定义绘制回调
    */
    CircleCenterCallback m_centerCallback;
};

} // namespace ui

#endif // UI_CONTROL_CIRCLEPROGRESS_H_
