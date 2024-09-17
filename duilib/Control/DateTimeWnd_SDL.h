#ifndef _UI_CONTROL_DATETIME_WND_SDL_H_
#define _UI_CONTROL_DATETIME_WND_SDL_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Core/Callback.h"

#if defined (DUILIB_BUILD_FOR_SDL)

namespace ui
{
class DateTime;
class RichEdit;
class Control;
class VBox;

/** 日期时间选择控件的实现（SDL）
*/
class DateTimeWnd : public virtual SupportWeakCallback
{
public:
    explicit DateTimeWnd(DateTime* pOwner);
    ~DateTimeWnd();

public:
    //初始化
    bool Init(DateTime* pOwner);

    //更新窗口的位置
    void UpdateWndPos();

    //显示窗口
    void ShowWindow();

private:
    /** 清除与控件的关联
    */
    void Clear();

    /** 创建年月日时分秒的编辑框
    */
    RichEdit* CreateEditYear() const;
    RichEdit* CreateEditMon() const;
    RichEdit* CreateEditMDay() const;
    RichEdit* CreateEditHour() const;
    RichEdit* CreateEditMin() const;
    RichEdit* CreateEditSec() const;

    /** 创建一个文本控件
    */
    Control* CreateLabel(const DString& text) const;

    /** 控制日的范围（闰年等情况）
    */
    void UpdateDayRange();

    /** 设置Spin功能的Class名称
    */
    bool SetSpinClass(const DString& spinClass);

    /** 调整文本数字值
    */
    void AdjustTextNumber(int32_t nDelta);

    /** 开始启动调整文本数字值的定时器
    */
    void StartAutoAdjustTextNumberTimer(int32_t nDelta);

    /** 开始自动调整文本数字值
    */
    void StartAutoAdjustTextNumber(int32_t nDelta);

    /** 结束自动调整文本数字值
    */
    void StopAutoAdjustTextNumber();

    /** 结束编辑
    */
    void EndEditDateTime();

    /** 编辑框的焦点转移事件
    */
    void OnKillFocusEvent(Control* pControl, Control* pNewFocus);

private:
    /** 关联的控件
    */
    DateTime* m_pOwner;

    /** 是否已经初始化
    */
    bool m_bInited;

    /** 当前初始化的日期编辑格式
    */
    int8_t m_editFormat;

private:
    /** 年月日时分秒
    */
    enum EditType
    {
        YEAR  = 0,
        MON   = 1,
        MDAY  = 2,
        HOUR  = 3,
        MIN   = 4,
        SEC   = 5,
        COUNT = 6
    };
    /** 编辑框列表
    */
    std::vector<RichEdit*> m_editList;

    /** Spin功能的容器
    */
    VBox* m_pSpinBox;

    /** 自动调整文本数字值的定时器生命周期管理
    */
    WeakCallbackFlag m_flagAdjustTextNumber;
};

} //namespace ui

#endif // DUILIB_BUILD_FOR_SDL

#endif // _UI_CONTROL_DATETIME_WND_SDL_H_
