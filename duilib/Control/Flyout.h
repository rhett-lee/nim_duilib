#ifndef UI_CONTROL_FLYOUT_H_
#define UI_CONTROL_FLYOUT_H_

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Core/ControlPtrT.h"
#include <functional>
#include <vector>

namespace ui
{

/** 通用浮层窗口（Flyout/Popup）
*   功能：
*   1. 锚定宿主控件周围弹出，支持 8 个方位；主方位空间不足时自动翻转到对侧，并夹持在显示器工作区内
*   2. 默认不抢焦点（WS_EX_NOACTIVATE），浮层显示期间父窗口保持激活，可继续操作
*   3. 默认点击浮层外部、按 Esc 自动关闭（可通过 SetAutoDismiss 关闭该行为）
*   4. 可承载任意 Box 内容（由 XML 描述），按主题色自绘，支持窗口阴影
*   5. 非模态窗口：对象在窗口关闭后由框架自动 delete，调用方不需要手动释放
*
*   使用方式：
*   @code
*       ui::Flyout* pFlyout = new ui::Flyout(this);
*       pFlyout->SetSkinFolder(GetResourcePath().ToString());
*       pFlyout->ShowAt(pAnchorButton, _T("my_flyout.xml"), ui::Flyout::Placement::Bottom);
*       ui::Button* pBtn = dynamic_cast<ui::Button*>(pFlyout->FindControl(_T("action")));
*   @endcode
*/
class DUILIB_API Flyout : public WindowImplBase
{
    typedef WindowImplBase BaseClass;

public:
    /** 浮层相对锚点控件的弹出方位
    */
    enum class Placement : uint32_t
    {
        Bottom      = 0,    //锚点下方，左边缘对齐
        BottomEnd   = 1,    //锚点下方，右边缘对齐
        Top         = 2,    //锚点上方，左边缘对齐
        TopEnd      = 3,    //锚点上方，右边缘对齐
        Right       = 4,    //锚点右侧，上边缘对齐
        RightEnd    = 5,    //锚点右侧，下边缘对齐
        Left        = 6,    //锚点左侧，上边缘对齐
        LeftEnd     = 7     //锚点左侧，下边缘对齐
    };

    /** 浮层关闭原因
    */
    enum class CloseReason : uint32_t
    {
        kManual         = 0,    //调用 Dismiss 主动关闭
        kClickOutside   = 1,    //点击浮层/锚点外部，或浮层失去焦点
        kEscape         = 2,    //按下 Esc 键
        kAnchorLost     = 3     //锚点控件或父窗口失效（销毁/最小化/隐藏/DPI 变化）
    };

    /** 浮层打开/关闭的回调函数原型
    * @param [in] reason 关闭原因（打开回调固定为 CloseReason::kManual）
    */
    typedef std::function<void(CloseReason reason)> FlyoutEvent;

public:
    /** 构造函数
    * @param [in] pParentWindow 父窗口（锚点控件必须属于该窗口），浮层作为其 owned window 随其一起销毁
    */
    explicit Flyout(Window* pParentWindow);
    virtual ~Flyout() override;

    /** 设置内容 XML 资源加载文件夹（相对资源根目录的路径）
    *   不设置时默认使用父窗口的资源路径
    */
    void SetSkinFolder(const DString& skinFolder);

    /** 在锚点控件周围显示浮层
    * @param [in] pAnchor 锚点控件
    * @param [in] xmlFile 内容 XML 文件名（相对 SkinFolder 解析）；也支持以 '<' 字符开头的 XML 文本
    * @param [in] placement 期望弹出方位，空间不足时会自动翻转到对侧
    * @return 显示成功返回 true；创建失败时浮层对象会被自动销毁并返回 false，此时不可再访问该对象
    */
    bool ShowAt(Control* pAnchor, const DString& xmlFile, Placement placement = Placement::Bottom);

    /** 关闭浮层（关闭原因记为 kManual）
    */
    void Dismiss();

    /** 设置点击外部/按 Esc 是否自动关闭，默认 true
    * @note 非 Windows 平台：不抢焦点模式（SetNoFocus(true)，默认）下无法检测外部点击，
    *       此时外部点击自动关闭不生效；如需自动关闭，请用 SetNoFocus(false) 走焦点模式
    *       （通过失去焦点感知外部点击）。
    */
    void SetAutoDismiss(bool bAutoDismiss);
    bool IsAutoDismiss() const;

    /** 设置弹出后是否不抢焦点，默认 true
    *   设为 false 时浮层会获取焦点，此时通过失去焦点来感知外部点击
    * @note 非 Windows 平台：不抢焦点模式（true）下外部点击检测暂不支持（见 StartDetectTimer），
    *       浮层无法在点击外部时自动关闭；如需该能力，请改用 false（焦点模式）。
    */
    void SetNoFocus(bool bNoFocus);
    bool IsNoFocus() const;

    /** 设置浮层与锚点之间的间距（未经 DPI 缩放的逻辑值），默认 6
    */
    void SetGap(int32_t nGap);
    int32_t GetGap() const;

    /** 设置主方位空间不足时是否允许自动翻转到对侧，默认 true
    */
    void SetAllowFlip(bool bAllowFlip);
    bool IsAllowFlip() const;

    /** 获取锚点控件（浮层关闭或锚点销毁后可能为 nullptr）
    */
    Control* GetAnchor() const;

    /** 获取期望的弹出方位
    */
    Placement GetPlacement() const;

    /** 浮层是否处于打开状态
    */
    bool IsOpen() const;

    /** 注册浮层打开后的回调
    */
    void AttachOpened(const FlyoutEvent& callback);

    /** 注册浮层关闭时的回调（在窗口实际关闭前触发）
    */
    void AttachClosed(const FlyoutEvent& callback);

    /** 获取当前活动中的浮层（同一时刻只保留一个），没有时返回 nullptr
    */
    static Flyout* GetActiveFlyout();

    /** 关闭当前活动浮层（没有活动浮层时为空操作）
    */
    static void DismissActive();

public:
    //WindowImplBase 接口
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual void OnInitWindow() override;
    virtual void OnFinalMessage() override;

    /** 抢焦点模式下：失去焦点即关闭；Esc 键关闭
    */
    virtual LRESULT OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override;
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** DPI 变化时直接关闭浮层，避免跨屏后锚点坐标与尺寸错位
    */
    virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor) override;

private:
    /** 按锚点与方位计算窗口位置并显示
    */
    void PositionAroundAnchor();

    /** 启动/停止外部输入检测（不抢焦点模式下通过定时器轮询全局鼠标与 Esc 状态）
    */
    void StartDetectTimer();
    void StopDetectTimer();
    void OnDetectTick();

    /** 执行关闭流程（只生效一次）
    */
    void DoClose(CloseReason reason);

    /** 计算锚点控件在屏幕坐标系下的矩形
    * @param [in] bPhysical true 返回物理像素（与 GetCursorPos/GetWindowRect 同坐标系）；
    *                       false 返回 DIP 逻辑像素（用于布局计算）
    */
    bool GetAnchorScreenRect(UiRect& rcAnchor, bool bPhysical) const;

    /** 判断屏幕坐标点是否落在锚点控件上
    */
    bool IsPointInAnchor(const UiPoint& ptScreen) const;

private:
    //父窗口（不持有引用，仅用于定位；父窗口销毁时本窗口作为其 owned window 一并销毁）
    Window* m_pParentWindow;

    //锚点控件（弱引用）
    ControlPtrT<Control> m_pAnchor;

    //内容 XML 文件名或 XML 文本
    UiString m_xml;

    //资源加载文件夹
    UiString m_skinFolder;

    //期望弹出方位
    Placement m_placement;

    //实际弹出方位（自动翻转后可能与期望不同）
    Placement m_actualPlacement;

    //是否自动关闭
    bool m_bAutoDismiss;

    //是否不抢焦点
    bool m_bNoFocus;

    //与锚点的间距（未经 DPI 缩放）
    int32_t m_nGap;

    //是否允许自动翻转
    bool m_bAllowFlip;

    //是否已经打开/是否正在关闭
    bool m_bOpen;
    bool m_bClosing;

    //外部输入检测任务 ID（0 表示无任务）
    size_t m_nDetectTaskId;

    //上一次轮询时鼠标按键/Esc 是否按下（用于边沿检测）
    bool m_bPointerDownLast;
    bool m_bEscapeDownLast;

    //打开/关闭回调
    std::vector<FlyoutEvent> m_openedCallbacks;
    std::vector<FlyoutEvent> m_closedCallbacks;

private:
    //当前活动中的浮层（同一时刻只保留一个）
    static Flyout* s_pActiveFlyout;

    //外部输入检测轮询周期（毫秒）
    static const int32_t kDetectIntervalMs;
};

} // namespace ui

#endif // UI_CONTROL_FLYOUT_H_
