#ifndef DUILIB_UTILS_TOAST_WND_H_
#define DUILIB_UTILS_TOAST_WND_H_

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Control/Label.h"
#include <vector>
#include <memory>

namespace ui
{

class AnimationPlayer;

/** 自绘皮肤的非模态通知窗口（Toast）
*   功能：
*   1. 非模态、不抢父窗口焦点，显示期间父窗口仍可正常操作
*   2. 支持 4 种类型：信息/成功/警告/错误（对应不同的彩色图标）
*   3. 到时自动关闭（默认 3000ms，可自定义；为 0 表示不自动关闭），支持淡入淡出与上移动画
*   4. 多条通知从锚点位置开始垂直堆叠排列，关闭后其余通知平滑补齐
*   5. 鼠标悬停时暂停倒计时，移开后继续；点击通知立即关闭
*   6. 消息文本支持多语言ID（bTextId=true），切换语言后实时刷新
*
*   使用方式（静态接口，fire-and-forget，窗口对象在关闭后自动销毁）：
*   @code
*       ui::ToastWnd::Show(this, _T("保存成功"), ui::ToastWnd::kTypeSuccess);
*       ui::ToastWnd::Show(this, _T("STRID_NETWORK_ERROR"), ui::ToastWnd::kTypeError,
*                          5000, ui::ToastWnd::kPosTop, true);
*   @endcode
*/
class DUILIB_API ToastWnd : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;

public:
    /** 通知类型，决定左侧图标的样式
    */
    enum ToastType : uint32_t
    {
        kTypeInfo    = 0,  //信息（蓝色 i）
        kTypeSuccess = 1,  //成功（绿色对勾）
        kTypeWarning = 2,  //警告（黄色 !）
        kTypeError   = 3   //错误（红色 X）
    };

    /** 通知出现的位置（相对父窗口客户区；父窗口为空时相对父窗口所在显示器的工作区）
    */
    enum ToastPosition : uint32_t
    {
        kPosTop         = 0,  //顶部居中（默认）
        kPosCenter      = 1,  //垂直居中
        kPosBottom      = 2,  //底部居中
        kPosTopRight    = 3,  //右上角
        kPosBottomRight = 4   //右下角（类系统通知）
    };

public:
    /** 显示一条 Toast 通知（非模态，不阻塞调用线程与父窗口）
    * @param [in] pParentWindow 父窗口，通知在其客户区附近显示；可为 nullptr（按显示器工作区定位）
    * @param [in] text 通知内容，支持 _T('\n') 换行，长文本自动换行
    * @param [in] type 通知类型，参见 enum ToastType
    * @param [in] nDurationMs 自动关闭的停留时长（毫秒），默认 3000；传 0 表示不自动关闭
    *                         （鼠标悬停时暂停计时，移开后按剩余时间继续）
    * @param [in] position 显示位置，参见 enum ToastPosition
    * @param [in] bTextId 为 true 时 text 按多语言 ID 解析，切换语言后实时刷新
    */
    static void Show(ui::Window* pParentWindow,
                     const DString& text,
                     ToastType type = kTypeInfo,
                     int32_t nDurationMs = 3000,
                     ToastPosition position = kPosTop,
                     bool bTextId = false);

private:
    ToastWnd();
    virtual ~ToastWnd() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 窗口创建完成后：填充内容、绑定交互、计算位置并播放入场动画、启动自动关闭计时
    */
    virtual void OnInitWindow() override;

    /** 窗口消息循环结束后：从活动列表中注销（非模态窗口基类随后会 delete this）
    */
    virtual void OnFinalMessage() override;

    /** 鼠标离开窗口客户区（WM_MOUSELEAVE）：框架不会把该消息派发给控件，
    *   这里用于在鼠标直接移出通知窗口时恢复自动关闭倒计时
    */
    virtual LRESULT OnMouseLeaveMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

private:
    /** 填充文本与图标
    */
    void InitContent();

    /** 测量窗口大小（含阴影），并计算最终的显示位置
    */
    void MeasureAndPosition();

    /** 绑定根容器的点击关闭、悬停暂停/恢复事件
    */
    void InitInteraction();

    /** 启动/暂停/恢复自动关闭计时
    */
    void StartAutoCloseTimer(int32_t nDelayMs);
    void PauseAutoCloseTimer();
    void ResumeAutoCloseTimer();
    void CancelAutoCloseTimer();

    /** 播放入场动画（淡入 + 向上轻微滑动）
    */
    void PlayEnterAnimation();

    /** 播放退场动画（淡出），动画结束后关闭窗口
    */
    void PlayExitAnimation();

    /** 播放位置移动动画（其他通知关闭后，本通知平滑上移补齐）
    * @param [in] nFromY 起始窗口 Y（客户区坐标）
    * @param [in] nToY 目标窗口 Y（客户区坐标）
    */
    void PlayMoveAnimation(int32_t nFromY, int32_t nToY);

    /** 请求关闭：从活动列表摘除并让其余通知补齐，然后播放退场动画
    */
    void RequestClose();

    /** 按当前活动列表重新排列所有通知
    * @param [in] bAnimate true=位置有变化时平滑动效；false=直接移动（新通知首次定位时）
    */
    static void LayoutAllToasts(bool bAnimate);

    /** 计算指定通知的目标窗口位置（窗口坐标，包含阴影偏移）
    * @param [in] pToast 待定位的通知（必须已经完成 MeasureAndPosition 的大小测量）
    * @param [out] nX 窗口左上角 X（客户区坐标）
    * @param [out] nY 窗口左上角 Y（客户区坐标）
    */
    static void CalcTargetWindowPos(const ToastWnd* pToast, int32_t& nX, int32_t& nY);

    /** 把窗口移动到目标位置（内部完成 DPI 转换）
    */
    void MoveToWindowPos(int32_t nX, int32_t nY);

private:
    //父窗口（不持有引用，仅用于定位；父窗口销毁时本窗口作为其 owned window 一并销毁）
    ui::Window* m_pParentWindow;

    //通知内容
    DString m_text;

    //通知类型
    ToastType m_type;

    //自动关闭停留时长（毫秒），0 表示不自动关闭
    int32_t m_durationMs;

    //剩余停留时长（毫秒），悬停暂停时使用
    int32_t m_remainingMs;

    //显示位置
    ToastPosition m_position;

    //文本是否为多语言ID
    bool m_bTextId;

    //自动关闭延迟任务的ID（0 表示无任务）
    size_t m_nAutoCloseTaskId;

    //倒计时到期时间点（steady_clock 毫秒时间戳），0 表示计时未运行
    int64_t m_nExpireTick;

    //是否正在播放退场动画（防止点击/定时器重复触发关闭）
    bool m_bClosing;

private:
    //根容器（用于绑定点击与悬停事件）
    ui::Control* m_pRootBox;

    //文本控件
    ui::Label* m_pTextLabel;

    //图标控件
    ui::Control* m_pIconControl;

    //窗口目标大小与位置（窗口坐标，包含阴影）
    int32_t m_nWindowX;
    int32_t m_nWindowY;
    int32_t m_nWindowWidth;
    int32_t m_nWindowHeight;

    //视觉有效区域大小（客户区坐标，不含阴影）
    int32_t m_nClientWidth;
    int32_t m_nClientHeight;

    //入场/退场/位置补齐动画播放器（随窗口对象析构而停止）
    std::unique_ptr<AnimationPlayer> m_pEnterPlayer;
    std::unique_ptr<AnimationPlayer> m_pExitPlayer;
    std::unique_ptr<AnimationPlayer> m_pMovePlayer;

private:
    //当前所有活动中的通知（按创建顺序，仅在 UI 线程访问）
    static std::vector<ToastWnd*> s_toasts;

    //通知之间的垂直间距，以及距锚点边缘的边距（客户区坐标）
    static const int32_t kToastGap;
    static const int32_t kAnchorMargin;
    static const int32_t kRightMargin;

    //入场/退场动画时长（毫秒），入场时向上滑动的距离（客户区坐标）
    static const int32_t kEnterAnimMs;
    static const int32_t kExitAnimMs;
    static const int32_t kEnterSlideOffset;

    //同一时刻允许显示的最大通知数量，超出后最早的通知先退场
    static const size_t kMaxToastCount;
};

} //namespace ui

#endif //DUILIB_UTILS_TOAST_WND_H_
