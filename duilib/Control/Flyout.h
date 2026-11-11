#ifndef UI_CONTROL_FLYOUT_H_
#define UI_CONTROL_FLYOUT_H_

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Core/INativeWindow.h"
#include <functional>
#include <memory>
#include <vector>

namespace ui
{

class AnimationPlayer;

/** 通用浮层窗口（Flyout/Popup）
*   功能：
*   1. 锚定宿主控件周围弹出，支持 8 个方位；主方位空间不足时自动翻转到对侧，并夹持在显示器工作区内
*   2. 默认不抢焦点（WS_EX_NOACTIVATE），浮层显示期间父窗口保持激活，可继续操作
*   3. 默认点击浮层外部、按 Esc 自动关闭（可通过 SetAutoDismiss / SetClickOutsideDismiss 分别控制）
*   4. 可承载任意 Box 内容（由 XML 描述），按主题色自绘，支持窗口阴影
*   5. 非模态窗口：对象在窗口关闭后由框架自动 delete，调用方不需要手动释放
*   6. 窗口阴影不拦截鼠标：阴影区域上的鼠标操作会穿透到浮层下面的窗口（Windows 平台），
*      阴影也不参与"鼠标是否在浮层内"的判定（见 GetVisualRect）
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
        LeftEnd     = 7,    //锚点左侧，下边缘对齐
        BottomCenter= 8,    //锚点下方，水平居中对齐
        TopCenter   = 9,    //锚点上方，水平居中对齐
        RightCenter = 10,   //锚点右侧，垂直居中对齐
        LeftCenter  = 11    //锚点左侧，垂直居中对齐
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

    /** 浮层布局完成后的回调函数原型（在首次绘制前触发，此时各个控件的实际布局矩形已确定）
    */
    typedef std::function<void()> FlyoutLayoutEvent;

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

    /** 设置"点击浮层外部"时是否自动关闭，默认 true
    * @note 该选项与 SetAutoDismiss 的区别：它只控制"外部点击"这一种关闭方式，
    *       按 Esc 关闭仍由 SetAutoDismiss 控制。
    *       用于受控场景（例如浮层由外部按钮控制显隐）时，避免鼠标按在外部按钮上的
    *       那一刻浮层就被关闭，导致该按钮的 Click 逻辑读到"浮层已关闭"的错误状态；
    *       关闭该选项后，外部点击（含阴影区域）不会关闭浮层，Esc 仍然有效。
    */
    void SetClickOutsideDismiss(bool bDismiss);
    bool IsClickOutsideDismiss() const;

    /** 设置鼠标落在窗口阴影区域时是否穿透到浮层下面的窗口（仅 Windows 平台），默认 true
    * @note 浮层窗口的阴影区域本身不显示内容，却会拦截鼠标消息，导致其下方控件无法悬停、
    *       无法点击。开启后阴影区域上的鼠标消息交由下层窗口处理（与"点击浮层外部"判定一致）。
    */
    void SetShadowClickThrough(bool bClickThrough);
    bool IsShadowClickThrough() const;

    /** 获取浮层的可视区域（屏幕坐标、物理像素；已扣除窗口阴影占据的边缘）
    * @param [out] rcVisual 返回可视区域的屏幕矩形；窗口无效时为空矩形
    */
    void GetVisualRect(UiRect& rcVisual) const;

    /** 判断屏幕坐标点（物理像素）是否落在浮层的可视区域内（不含阴影）
    */
    bool IsPointInVisualArea(const UiPoint& ptScreen) const;

    /** 设置浮层是否跟随锚点移动（锚点因滚动/布局变化而位移时，浮层自动重新定位），默认 false
    * @note 开启后浮层按外部输入检测的周期（约 50 毫秒）检查锚点位置，发现位移即重新定位；
    *       重新定位时保持打开时的方位、不做翻转（浮层内容例如尖角箭头与阴影是按该方位生成的），
    *       该侧空间不足时由定位逻辑把窗口夹持在显示器工作区内；
    *       锚点被滚动出窗口客户区或滚动容器（ScrollBox 及其派生类）的可视区域时，
    *       浮层窗口暂时隐藏（浮层对象与打开状态保留，不触发关闭回调），锚点重新可见后
    *       自动按新位置恢复显示；因此滚动往返后浮层不会丢失
    */
    void SetFollowAnchor(bool bFollow);
    bool IsFollowAnchor() const;

    /** 判断锚点控件当前是否可见（未被窗口客户区或滚动容器裁剪掉）
    * @return 锚点已不可见（例如被滚动出滚动区域）时返回 false
    */
    bool IsAnchorVisible() const;

    /** 按锚点当前位置重新定位浮层（锚点移动后调用，复用当前窗口，不重建）
    *   重新定位后会触发"布局完成"回调，供子类按新的窗口位置调整内容（例如尖角箭头）
    * @return 重新定位后的实际方位是否发生了变化
    */
    bool UpdateAnchorPosition();

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

    /** 获取实际的弹出方位（主方位空间不足、自动翻转到对侧后，该值可能与 GetPlacement() 不同）
    * @note 该值在 ShowAt 定位时更新；子类可据此判断是否需要调整内容（例如箭头朝向）
    */
    Placement GetActualPlacement() const;

    /** 浮层是否处于打开状态
    */
    bool IsOpen() const;

    /** 注册浮层打开后的回调
    */
    void AttachOpened(const FlyoutEvent& callback);

    /** 注册浮层关闭时的回调（在窗口实际关闭前触发）
    */
    void AttachClosed(const FlyoutEvent& callback);

    /** 注册浮层布局完成后的回调（在首次绘制前触发；跟随锚点重新定位后也会触发）
    *   用于按控件的实际布局矩形调整内容的位置（例如把指示尖角定位到锚点中心）；
    *   回调中设置的浮动控件（float="true" keep_float_pos="true"）位置在后续布局中会被保持
    */
    void AttachLayouted(const FlyoutLayoutEvent& callback);

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
    virtual void OnInitLayout() override;
    virtual void OnFinalMessage() override;

    /** 抢焦点模式下：失去焦点即关闭；Esc 键关闭
    */
    virtual LRESULT OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override;
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** DPI 变化时直接关闭浮层，避免跨屏后锚点坐标与尺寸错位
    */
    virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor) override;

protected:
    /** 按锚点与方位重新计算浮层窗口的大小与位置
    *   子类在就地更新内容尺寸（例如候选项行数变化导致高度变化）后调用，
    *   从而复用当前窗口，避免销毁后重建
    */
    void PositionAroundAnchor();

private:
    /** 启动/停止外部输入检测（不抢焦点模式下通过定时器轮询全局鼠标与 Esc 状态；
    *   跟随锚点时也依赖该定时器轮询锚点位置）
    */
    void StartDetectTimer();
    void StopDetectTimer();
    void OnDetectTick();

    /** 触发"布局完成"回调（首次布局完成后、以及跟随锚点重新定位后调用）
    */
    void InvokeLayoutCallbacks();

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

    /** 判断屏幕坐标点是否落在"锚点所在滚动容器的滚动条"上
    *   拖动/点击滚动条属于滚动操作，此时不应按"点击外部"关闭跟随锚点的浮层
    */
    bool IsPointOnAnchorScrollBar(const UiPoint& ptScreen) const;

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

    //点击浮层外部（含阴影区域）时是否关闭
    bool m_bClickOutsideDismiss;

    //鼠标落在窗口阴影区域时是否穿透到浮层下面的窗口
    bool m_bShadowClickThrough;

    //是否跟随锚点移动（锚点位移时重新定位，滚出可视区域时暂时隐藏窗口）
    bool m_bFollowAnchor;

    //浮层窗口是否因锚点不可见而暂时隐藏（锚点重新可见后恢复显示）
    bool m_bAnchorSuspended;

    //上一次记录的锚点屏幕矩形（物理像素），用于检测锚点是否发生位移
    UiRect m_rcAnchorScreenLast;

    //窗口阴影区域的命中测试过滤器（Windows 平台，把阴影上的鼠标消息交给下层窗口）
    std::unique_ptr<IUIMessageFilter> m_pShadowHitTestFilter;

    //窗口显示时的淡入动画播放器（受 PopupAnimation 全局开关控制）
    std::unique_ptr<AnimationPlayer> m_pFadeInPlayer;

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

    //布局完成回调
    std::vector<FlyoutLayoutEvent> m_layoutedCallbacks;

private:
    //当前活动中的浮层（同一时刻只保留一个）
    static Flyout* s_pActiveFlyout;

    //外部输入检测轮询周期（毫秒）
    static const int32_t kDetectIntervalMs;
};

} // namespace ui

#endif // UI_CONTROL_FLYOUT_H_
