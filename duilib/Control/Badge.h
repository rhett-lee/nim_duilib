#ifndef UI_CONTROL_BADGE_H_
#define UI_CONTROL_BADGE_H_

#include "duilib/Control/Label.h"

namespace ui {

/** Badge 角标控件（继承Label）
 *  用于 TabCtrl 标签页、按钮、图标等控件上的未读数/新消息数量/小红点展示。
 *  两种形态：
 *  1. 数字角标：圆角胶囊背景 + 数字文本，数量超过 max_count 时显示"max+"（如"99+"）；
 *  2. 红点角标（dot模式）：小圆点，不显示数字。
 *  数字角标数量 count <= 0 时自动隐藏（主流平台惯例：无未读时不显示角标）；
 *  红点角标不受 count 影响，显隐由 visible 属性控制。
 *  作为子控件放在任意 Box 容器内使用，悬浮定位时配合 float="true" 与 margin 属性。
 *  背景使用语义色自绘（圆角 = 高度的一半），自动适配深浅色主题。
 */
class DUILIB_API Badge : public Label
{
    typedef Label BaseClass;
public:
    explicit Badge(Window* pWindow);
    Badge(const Badge& r) = delete;
    Badge& operator=(const Badge& r) = delete;
    ~Badge() override;

    /** 获取控件类型
    */
    virtual DString GetType() const override;

    /** 设置XML属性
    */
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 绘制控件状态颜色（绘制角标的圆角胶囊/圆点背景）
    */
    virtual void PaintStateColors(IRender* pRender) override;

public:
    /** 设置角标数量：数字模式下大于0时显示角标，小于等于0时自动隐藏（红点模式不受影响）
    */
    void SetCount(int64_t nCount);

    /** 获取角标数量
    */
    int64_t GetCount() const { return m_nCount; }

    /** 设置数量上限：超过上限时显示"上限+"（如"99+"），默认99
    */
    void SetMaxCount(int64_t nMaxCount);

    /** 获取数量上限
    */
    int64_t GetMaxCount() const { return m_nMaxCount; }

    /** 设置是否为红点模式：true为纯小圆点，不显示数字
    */
    void SetDotMode(bool bDotMode);

    /** 获取是否为红点模式
    */
    bool IsDotMode() const { return m_bDotMode; }

    /** 设置角标背景颜色（语义色名或颜色值，默认为语义色 bg_badge）
    */
    void SetBadgeColor(const DString& strColor);

    /** 获取角标背景颜色
    */
    DString GetBadgeColor() const { return m_strBadgeColor; }

private:
    /** 根据当前数量/红点模式，更新显示文本与可见性
    */
    void UpdateBadge();

private:
    /** 角标数量（数字模式下小于等于0时自动隐藏；红点模式不受影响）
    */
    int64_t m_nCount;

    /** 数量上限，超过时显示"上限+"（如"99+"）
    */
    int64_t m_nMaxCount;

    /** 是否为红点模式（纯小圆点，不显示数字）
    */
    bool m_bDotMode;

    /** 角标背景颜色（语义色名或颜色值）
    */
    DString m_strBadgeColor;
};

} // namespace ui

#endif // UI_CONTROL_BADGE_H_
