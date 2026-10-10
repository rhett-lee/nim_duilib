#ifndef UI_CONTROL_MARQUEE_H_
#define UI_CONTROL_MARQUEE_H_

#include "duilib/Control/Label.h"
#include "duilib/Core/Callback.h"

namespace ui {

/** Marquee 文本跑马灯滚动方向
*/
enum class MarqueeDirection
{
    kLeft  = 0,  //横向文本，向左滚动
    kRight = 1,  //横向文本，向右滚动
    kUp    = 2,  //纵向文本，向上滚动
    kDown  = 3   //纵向文本，向下滚动
};

/** Marquee 文本跑马灯控件（继承 Label，复用 text/text_id/font/text_color 等全部文本属性）
 *
 *  用于在界面中横向/纵向循环滚动显示文本。当文本内容超出控件显示区域时自动循环滚动；
 *  未超出时静态显示（按 Label 的 text_align 对齐）。
 *
 *  特性：
 *  - 横向文本（向左/向右滚动）与纵向文本（向上/向下滚动）两种显示方式，由 scroll_direction 决定
 *  - 自定义滚动速度（scroll_speed，像素/秒，自动 DPI 缩放）
 *  - 暂停/恢复（Pause/Resume），鼠标悬停时自动暂停（hover_pause，默认开启）
 *  - 文本内容动态更新（SetText/SetTextId，更新后自动重置滚动位置）
 *  - 文本超出显示区域时无缝循环滚动（双份文本首尾相接）
 *
 *  XML 属性（继承 Label 的全部属性，另增）：
 *  - scroll_direction: "left"/"right"/"up"/"down"，默认 "left"
 *  - scroll_speed: 滚动速度（像素/秒，自动 DPI 缩放），默认 50
 *  - scroll_interval: 定时器帧间隔（毫秒），默认 30
 *  - hover_pause: 鼠标悬停是否暂停滚动，默认 true
 */
class DUILIB_API Marquee : public Label
{
    typedef Label BaseClass;

public:
    explicit Marquee(Window* pWindow);
    Marquee(const Marquee& r) = delete;
    Marquee& operator=(const Marquee& r) = delete;
    ~Marquee() override;

    /** 获取控件类型
    */
    DString GetType() const override;

    /** 设置XML属性
    */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 绘制文本（滚动绘制）
    */
    void PaintText(IRender* pRender) override;

    /** 文本内容更新（重置滚动位置）
    */
    void SetText(const DString& strText) override;

    /** 文本语言 ID 更新（重置滚动位置）
    */
    void SetTextId(const DString& strTextId) override;

    /** DPI 变化（清空文本测量缓存）
    */
    void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 窗口关联变化（清理定时器）
    */
    void SetWindow(Window* pWindow) override;

    // 鼠标交互：悬停暂停
    bool MouseEnter(const EventArgs& msg) override;
    bool MouseLeave(const EventArgs& msg) override;

public:
    /** 设置滚动方向
    */
    void SetScrollDirection(MarqueeDirection direction);

    /** 获取滚动方向
    */
    MarqueeDirection GetScrollDirection() const { return m_direction; }

    /** 设置滚动速度（像素/秒，自动 DPI 缩放）
    */
    void SetScrollSpeed(int32_t nSpeed);

    /** 获取滚动速度（像素/秒，DPI 缩放前的原始值）
    */
    int32_t GetScrollSpeed() const { return m_nScrollSpeed; }

    /** 设置定时器帧间隔（毫秒）
    */
    void SetScrollInterval(int32_t nIntervalMs);

    /** 获取定时器帧间隔（毫秒）
    */
    int32_t GetScrollInterval() const { return m_nScrollInterval; }

    /** 设置鼠标悬停是否暂停滚动
    */
    void SetHoverPause(bool bHoverPause);

    /** 获取鼠标悬停是否暂停滚动
    */
    bool IsHoverPause() const { return m_bHoverPause; }

    /** 暂停滚动（手动）
    */
    void Pause();

    /** 恢复滚动
    */
    void Resume();

    /** 设置暂停状态
    */
    void SetPaused(bool bPaused);

    /** 获取是否处于暂停状态
    */
    bool IsPaused() const { return m_bPaused; }

    /** 获取当前是否正在滚动（文本超出显示区域）
    */
    bool IsScrolling() const { return m_bScrolling; }

private:
    /** 启动滚动定时器
    */
    void StartTimer();

    /** 停止滚动定时器
    */
    void StopTimer();

    /** 重置滚动偏移（文本变化时）
    */
    void ResetScroll();

    /** 定时器回调：更新滚动偏移
    */
    void OnTimerTick();

    /** 判断当前是否应暂停（手动暂停或悬停暂停）
    */
    bool IsPausedEffective() const;

private:
    /** 滚动方向
    */
    MarqueeDirection m_direction;

    /** 滚动速度（像素/秒，DPI 缩放前原始值）
    */
    int32_t m_nScrollSpeed;

    /** 定时器帧间隔（毫秒）
    */
    int32_t m_nScrollInterval;

    /** 鼠标悬停是否暂停滚动
    */
    bool m_bHoverPause;

    /** 手动暂停标志
    */
    bool m_bPaused;

    /** 鼠标是否悬停
    */
    bool m_bHovering;

    /** 当前是否处于滚动状态（文本超出显示区域）
    */
    bool m_bScrolling;

    /** 定时器是否已启动
    */
    bool m_bTimerStarted;

    /** 定时器 ID（大于0表示已注册）
    */
    size_t m_nTimerId;

    /** 当前滚动偏移（像素，0 ~ m_fPeriod 循环）
    */
    float m_fOffset;

    /** 滚动周期（文本滚动方向尺寸 + 间隔），由 PaintText 测量后缓存
    */
    float m_fPeriod;

    /** 文本测量缓存：上次测量的文本（文本相同则复用测量结果，避免每帧 MeasureString）
    */
    DString m_strMeasuredText;

    /** 缓存的文本滚动方向尺寸（横向=宽度，纵向=高度）
    */
    float m_fTextMainSize;

    /** 缓存的文本交叉方向尺寸（横向=行高，纵向=列宽）
    */
    float m_fTextCrossSize;

    /** 定时器取消标志
    */
    WeakCallbackFlag m_timerFlag;
};

} // namespace ui

#endif // UI_CONTROL_MARQUEE_H_
