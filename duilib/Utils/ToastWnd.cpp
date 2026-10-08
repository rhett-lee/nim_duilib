#include "duilib/Utils/ToastWnd.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/Callback.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Control/Button.h"

#include <chrono>

namespace ui
{
    //静态成员初始化
    std::vector<ToastWnd*> ToastWnd::s_toasts;
    const int32_t ToastWnd::kToastGap          = 12;  //通知之间的垂直间距
    const int32_t ToastWnd::kAnchorMargin      = 24;  //距父窗口客户区顶/底边的边距
    const int32_t ToastWnd::kRightMargin       = 24;  //距父窗口客户区右边的边距
    const int32_t ToastWnd::kEnterAnimMs       = 200; //淡入动画时长
    const int32_t ToastWnd::kExitAnimMs        = 180; //淡出动画时长
    const int32_t ToastWnd::kEnterSlideOffset  = 12;  //入场时向上滑动的距离
    const size_t ToastWnd::kMaxToastCount      = 5;   //同屏最多显示的通知数量

    ToastWnd::ToastWnd() :
        m_pParentWindow(nullptr),
        m_type(kTypeInfo),
        m_durationMs(3000),
        m_remainingMs(3000),
        m_position(kPosTop),
        m_bTextId(false),
        m_pActionBox(nullptr),
        m_nAutoCloseTaskId(0),
        m_nExpireTick(0),
        m_bClosing(false),
        m_pRootBox(nullptr),
        m_pTextLabel(nullptr),
        m_pIconControl(nullptr),
        m_nWindowX(0),
        m_nWindowY(0),
        m_nWindowWidth(0),
        m_nWindowHeight(0),
        m_nClientWidth(0),
        m_nClientHeight(0)
    {
    }

    ToastWnd::~ToastWnd()
    {
        CancelAutoCloseTimer();
    }

    DString ToastWnd::GetSkinFolder()
    {
        return _T("public/toast");
    }

    DString ToastWnd::GetSkinFile()
    {
        return _T("toast.xml");
    }

    void ToastWnd::Show(ui::Window* pParentWindow,
                        const DString& text,
                        ToastType type,
                        int32_t nDurationMs,
                        ToastPosition position,
                        bool bTextId)
    {
        Show(pParentWindow, text, type, {}, nDurationMs, position, bTextId);
    }

    void ToastWnd::Show(ui::Window* pParentWindow,
                        const DString& text,
                        ToastType type,
                        const std::vector<ToastAction>& actions,
                        int32_t nDurationMs,
                        ToastPosition position,
                        bool bTextId)
    {
        ASSERT(nDurationMs >= 0);
        if (nDurationMs < 0) {
            nDurationMs = 3000;
        }

        //数量超过上限时，让最早出现的通知先关闭（关闭流程会自动从列表摘除）
        if (s_toasts.size() >= kMaxToastCount) {
            s_toasts.front()->RequestClose();
        }

        ToastWnd* pToastWnd = new ToastWnd();
        pToastWnd->m_pParentWindow = pParentWindow;
        pToastWnd->m_text = text;
        pToastWnd->m_type = type;
        pToastWnd->m_durationMs = nDurationMs;
        pToastWnd->m_remainingMs = nDurationMs;
        pToastWnd->m_position = position;
        pToastWnd->m_bTextId = bTextId;
        pToastWnd->m_actions = actions;

        WindowCreateParam createWndParam;
        createWndParam.m_dwStyle = kWS_POPUP;
        createWndParam.m_dwExStyle = kWS_EX_TOPMOST | kWS_EX_LAYERED | kWS_EX_NOACTIVATE;
        if (pParentWindow == nullptr) {
            //没有父窗口时作为工具窗口，避免出现在任务栏和 Alt+Tab 列表中
            createWndParam.m_dwExStyle |= kWS_EX_TOOLWINDOW;
        }

        if (!pToastWnd->CreateWnd(pParentWindow, createWndParam)) {
            delete pToastWnd;
            return;
        }

        //创建完成后（OnInitWindow 中已完成内容填充与尺寸测量），加入活动列表并按堆叠序号定位
        s_toasts.push_back(pToastWnd);
        int32_t nX = 0;
        int32_t nY = 0;
        CalcTargetWindowPos(pToastWnd, nX, nY);
        pToastWnd->m_nWindowX = nX;
        pToastWnd->m_nWindowY = nY;

        //先以全透明状态移动到最终位置，避免首次出现时闪烁
        pToastWnd->SetLayeredWindowAlpha(0);
        pToastWnd->MoveToWindowPos(nX, nY);
        pToastWnd->ShowWindow(kSW_SHOW_NA);

        pToastWnd->PlayEnterAnimation();
        pToastWnd->StartAutoCloseTimer(nDurationMs);
    }

    void ToastWnd::OnInitWindow()
    {
        BaseClass::OnInitWindow();

        m_pRootBox = GetRoot();
        ASSERT(m_pRootBox != nullptr);
        if (m_pRootBox == nullptr) {
            return;
        }

        InitContent();
        InitInteraction();
        InitActions();
        MeasureAndPosition();
    }

    void ToastWnd::OnFinalMessage()
    {
        //窗口被外部销毁（如父窗口关闭、程序退出）时，从活动列表摘除并重排其余通知。
        //正常关闭流程在 RequestClose 中已经摘除，这里再次摘除是空操作。
        auto it = std::find(s_toasts.begin(), s_toasts.end(), this);
        if (it != s_toasts.end()) {
            s_toasts.erase(it);
            LayoutAllToasts(true);
        }

        //非模态窗口基类在 OnFinalMessage 返回后会自动 delete this
        BaseClass::OnFinalMessage();
    }

    LRESULT ToastWnd::OnMouseLeaveMsg(const NativeMsg& nativeMsg, bool& bHandled)
    {
        //鼠标直接移出通知窗口时，控件级 kEventMouseLeave 不会被派发，需在此恢复倒计时
        ResumeAutoCloseTimer();
        return BaseClass::OnMouseLeaveMsg(nativeMsg, bHandled);
    }

    void ToastWnd::InitContent()
    {
        ASSERT(m_pRootBox != nullptr);
        if (m_pRootBox == nullptr) {
            return;
        }

        //图标
        m_pIconControl = FindControl(_T("toast_icon"));
        if (m_pIconControl != nullptr) {
            DString sIconFile;
            switch (m_type) {
            case kTypeSuccess:
                sIconFile = _T("file='toast_success.svg'");
                break;
            case kTypeWarning:
                sIconFile = _T("file='toast_warning.svg'");
                break;
            case kTypeError:
                sIconFile = _T("file='toast_error.svg'");
                break;
            case kTypeInfo:
            default:
                sIconFile = _T("file='toast_info.svg'");
                break;
            }
            m_pIconControl->SetBkImage(sIconFile);
            m_pIconControl->SetVisible(true);
        }

        //文本（支持多语言ID，切换语言后控件自动刷新）
        m_pTextLabel = dynamic_cast<Label*>(FindControl(_T("toast_text")));
        if (m_pTextLabel != nullptr) {
            if (m_bTextId) {
                m_pTextLabel->SetTextId(m_text);
            }
            else {
                m_pTextLabel->SetText(m_text);
            }
        }
    }

    void ToastWnd::InitInteraction()
    {
        //注意：GetRoot() 返回的是阴影容器（已被框架设置为不接收鼠标消息），
        //事件必须绑定在 XML 根容器（即实际可见的通知条）上。
        Box* pContentBox = GetXmlRoot();
        ASSERT(pContentBox != nullptr);
        if (pContentBox == nullptr) {
            return;
        }

        //点击通知任意位置立即关闭（普通容器不会派发 kEventClick，使用鼠标左键弹起事件）
        pContentBox->AttachButtonUp([this](const EventArgs& /*args*/) {
            RequestClose();
            return true;
        });

        //鼠标进入通知区域：暂停自动关闭倒计时
        pContentBox->AttachMouseEnter([this](const EventArgs& /*args*/) {
            PauseAutoCloseTimer();
            return true;
        });

        //鼠标离开通知区域：按剩余时间继续倒计时
        pContentBox->AttachMouseLeave([this](const EventArgs& /*args*/) {
            ResumeAutoCloseTimer();
            return true;
        });
    }

    void ToastWnd::InitActions()
    {
        if (m_actions.empty()) {
            return;
        }
        Box* pActions = dynamic_cast<Box*>(FindControl(_T("toast_actions")));
        if (pActions == nullptr) {
            return;
        }
        m_pActionBox = pActions;

        for (const ToastAction& action : m_actions) {
            ui::Button* pBtn = new ui::Button(this);
            if (action.bTextId) {
                pBtn->SetTextId(action.text);
            }
            else {
                pBtn->SetText(action.text);
            }
            //样式：与日历 today/clear 按钮同款（圆角边框 + hover/按下反馈），语义色适配深浅色
            pBtn->SetAttribute(_T("width"), _T("auto"));
            pBtn->SetAttribute(_T("height"), _T("28"));
            pBtn->SetAttribute(_T("margin"), _T("4,0,4,0"));
            pBtn->SetAttribute(_T("min_width"), _T("64"));
            pBtn->SetAttribute(_T("text_padding"), _T("12,0,12,0"));
            pBtn->SetAttribute(_T("font"), _T("system_regular_12"));
            pBtn->SetAttribute(_T("text_color"), _T("color_accent"));
            pBtn->SetAttribute(_T("border_size"), _T("1"));
            pBtn->SetAttribute(_T("border_round"), _T("4,4"));
            pBtn->SetAttribute(_T("normal_border_color"), _T("border_control_normal"));
            pBtn->SetAttribute(_T("hovered_border_color"), _T("border_btn_hovered"));
            pBtn->SetAttribute(_T("hovered_color"), _T("bg_btn_hovered"));
            pBtn->SetAttribute(_T("pressed_color"), _T("bg_btn_pressed"));
            //点击：先执行回调，再关闭通知（按钮 mouse_enabled=true，不会冒泡到根容器的整条关闭）
            pBtn->AttachClick([this, action](const ui::EventArgs& /*args*/) {
                if (action.callback) {
                    action.callback();
                }
                RequestClose();
                return true;
            });
            pActions->AddItem(pBtn);
        }

        pActions->SetVisible(true);
    }

    void ToastWnd::MeasureAndPosition()
    {
        ASSERT(m_pRootBox != nullptr);
        if (m_pRootBox == nullptr) {
            return;
        }

        //以显示器工作区大小作为可用空间估算最终尺寸（返回值包含窗口阴影）
        UiRect rcWork;
        GetMonitorWorkRect(rcWork);
        Dpi().WindowSizeToClientSize(rcWork);
        UiSize szAvailable(rcWork.Width(), rcWork.Height());

        UiEstSize estSize = m_pRootBox->EstimateSize(szAvailable);
        UiSize szWindow(szAvailable.cx, szAvailable.cy);
        if (estSize.cx.IsInt32()) {
            szWindow.cx = estSize.cx.GetInt32();
        }
        if (estSize.cy.IsInt32()) {
            szWindow.cy = estSize.cy.GetInt32();
        }

        m_nWindowWidth = szWindow.cx;
        m_nWindowHeight = szWindow.cy;

        //根容器 padding 即阴影在四周所占的区域（参考 Menu::ResizeMenu）
        UiPadding rcShadow = m_pRootBox->GetPadding();
        m_nClientWidth = szWindow.cx - rcShadow.left - rcShadow.right;
        m_nClientHeight = szWindow.cy - rcShadow.top - rcShadow.bottom;
        if (m_nClientWidth < 10) {
            m_nClientWidth = 10;
        }
        if (m_nClientHeight < 10) {
            m_nClientHeight = 10;
        }
    }

    void ToastWnd::StartAutoCloseTimer(int32_t nDelayMs)
    {
        CancelAutoCloseTimer();
        m_remainingMs = nDelayMs;
        if (nDelayMs <= 0) {
            //0 表示不自动关闭
            return;
        }

        m_nExpireTick = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count() + nDelayMs;
        m_nAutoCloseTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
            kThreadUI,
            UiBind(this, [this]() {
                m_nAutoCloseTaskId = 0;
                m_nExpireTick = 0;
                RequestClose();
            }),
            nDelayMs);
    }

    void ToastWnd::PauseAutoCloseTimer()
    {
        if (m_bClosing || (m_durationMs <= 0) || (m_nExpireTick == 0)) {
            return;
        }
        int64_t nNow = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        int32_t nRemain = static_cast<int32_t>(m_nExpireTick - nNow);
        if (nRemain < 0) {
            nRemain = 0;
        }
        CancelAutoCloseTimer();
        m_remainingMs = nRemain;
    }

    void ToastWnd::ResumeAutoCloseTimer()
    {
        if (m_bClosing || (m_durationMs <= 0) || (m_nExpireTick != 0)) {
            return;
        }
        if (m_remainingMs > 0) {
            StartAutoCloseTimer(m_remainingMs);
        }
        else {
            //停留时间已经耗尽（暂停期间跨过了到期点），直接关闭
            RequestClose();
        }
    }

    void ToastWnd::CancelAutoCloseTimer()
    {
        if (m_nAutoCloseTaskId != 0) {
            GlobalManager::Instance().Thread().CancelTask(m_nAutoCloseTaskId);
            m_nAutoCloseTaskId = 0;
        }
        m_nExpireTick = 0;
    }

    void ToastWnd::PlayEnterAnimation()
    {
        std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
        if (m_pEnterPlayer != nullptr) {
            m_pEnterPlayer->Stop();
        }
        AnimationPlayer* pPlayer = new AnimationPlayer;
        m_pEnterPlayer.reset(pPlayer);
        pPlayer->SetAnimationType(AnimationType::kAnimationNone);
        pPlayer->SetStartValue(0);
        pPlayer->SetEndValue(100);
        pPlayer->SetTotalMillSeconds(kEnterAnimMs);
        pPlayer->SetFrameIntervalMillSeconds(1000 / 60);
        pPlayer->SetEasingFunctionType(EaseOutCubic);
        pPlayer->SetPlayCallback([this, weakFlag](int32_t nValue) {
            if (weakFlag.expired()) {
                return;
            }
            int32_t nAlpha = 255 * nValue / 100;
            int32_t nOffset = kEnterSlideOffset * (100 - nValue) / 100;
            SetLayeredWindowAlpha(nAlpha);
            MoveToWindowPos(m_nWindowX, m_nWindowY + nOffset);
        });
        pPlayer->SetCompleteCallback([this, weakFlag]() {
            if (weakFlag.expired()) {
                return;
            }
            SetLayeredWindowAlpha(255);
            MoveToWindowPos(m_nWindowX, m_nWindowY);
        });
        pPlayer->Start();
    }

    void ToastWnd::PlayExitAnimation()
    {
        std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
        if (m_pEnterPlayer != nullptr) {
            m_pEnterPlayer->Stop();
        }
        if (m_pMovePlayer != nullptr) {
            m_pMovePlayer->Stop();
        }
        if (m_pExitPlayer != nullptr) {
            m_pExitPlayer->Stop();
        }
        AnimationPlayer* pPlayer = new AnimationPlayer;
        m_pExitPlayer.reset(pPlayer);
        pPlayer->SetAnimationType(AnimationType::kAnimationNone);
        pPlayer->SetStartValue(0);
        pPlayer->SetEndValue(100);
        pPlayer->SetTotalMillSeconds(kExitAnimMs);
        pPlayer->SetFrameIntervalMillSeconds(1000 / 60);
        pPlayer->SetEasingFunctionType(EaseInCubic);
        pPlayer->SetPlayCallback([this, weakFlag](int32_t nValue) {
            if (weakFlag.expired()) {
                return;
            }
            int32_t nAlpha = 255 * (100 - nValue) / 100;
            SetLayeredWindowAlpha(nAlpha);
        });
        pPlayer->SetCompleteCallback([this, weakFlag]() {
            if (weakFlag.expired()) {
                return;
            }
            SetLayeredWindowAlpha(0);
            CloseWnd();
        });
        pPlayer->Start();
    }

    void ToastWnd::PlayMoveAnimation(int32_t nFromY, int32_t nToY)
    {
        std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
        if (m_pMovePlayer != nullptr) {
            m_pMovePlayer->Stop();
        }
        AnimationPlayer* pPlayer = new AnimationPlayer;
        m_pMovePlayer.reset(pPlayer);
        pPlayer->SetAnimationType(AnimationType::kAnimationNone);
        pPlayer->SetStartValue(0);
        pPlayer->SetEndValue(100);
        pPlayer->SetTotalMillSeconds(kEnterAnimMs);
        pPlayer->SetFrameIntervalMillSeconds(1000 / 60);
        pPlayer->SetEasingFunctionType(EaseOutCubic);
        pPlayer->SetPlayCallback([this, weakFlag, nFromY, nToY](int32_t nValue) {
            if (weakFlag.expired()) {
                return;
            }
            int32_t nY = nFromY + (nToY - nFromY) * nValue / 100;
            MoveToWindowPos(m_nWindowX, nY);
        });
        pPlayer->SetCompleteCallback([this, weakFlag, nToY]() {
            if (weakFlag.expired()) {
                return;
            }
            MoveToWindowPos(m_nWindowX, nToY);
        });
        pPlayer->Start();
    }

    void ToastWnd::RequestClose()
    {
        if (m_bClosing) {
            return;
        }
        m_bClosing = true;
        CancelAutoCloseTimer();

        //从活动列表摘除，其余通知向上补齐
        auto it = std::find(s_toasts.begin(), s_toasts.end(), this);
        if (it != s_toasts.end()) {
            s_toasts.erase(it);
        }
        LayoutAllToasts(true);

        PlayExitAnimation();
    }

    void ToastWnd::LayoutAllToasts(bool bAnimate)
    {
        for (ToastWnd* pToast : s_toasts) {
            if ((pToast == nullptr) || pToast->m_bClosing) {
                continue;
            }
            int32_t nNewX = 0;
            int32_t nNewY = 0;
            CalcTargetWindowPos(pToast, nNewX, nNewY);

            const int32_t nOldY = pToast->m_nWindowY;
            pToast->m_nWindowX = nNewX;
            pToast->m_nWindowY = nNewY;

            if (!bAnimate) {
                pToast->MoveToWindowPos(nNewX, nNewY);
                continue;
            }

            //正在播放入场动画的通知直接吸附到目标位置，避免两个动画互相覆盖
            bool bEntering = (pToast->m_pEnterPlayer != nullptr) &&
                             pToast->m_pEnterPlayer->IsPlaying();
            if ((nNewY == nOldY) || bEntering) {
                pToast->MoveToWindowPos(nNewX, nNewY);
                continue;
            }
            pToast->PlayMoveAnimation(nOldY, nNewY);
        }
    }

    void ToastWnd::CalcTargetWindowPos(const ToastWnd* pToast, int32_t& nX, int32_t& nY)
    {
        ASSERT(pToast != nullptr);
        if (pToast == nullptr) {
            nX = 0;
            nY = 0;
            return;
        }

        //1. 计算锚点区域（屏幕客户区坐标/DIP）：优先用父窗口客户区，否则用显示器工作区
        UiRect rcAnchor;
        UiRect rcWork;
        Window* pParent = pToast->m_pParentWindow;
        if (pParent != nullptr) {
            UiPoint ptOrigin(0, 0);
            pParent->ClientToScreen(ptOrigin);
            UiRect rcClient;
            pParent->GetClientRect(rcClient);
            rcAnchor = UiRect(ptOrigin.x, ptOrigin.y,
                              ptOrigin.x + rcClient.Width(), ptOrigin.y + rcClient.Height());
            pParent->Dpi().WindowSizeToClientSize(rcAnchor);

            pParent->GetMonitorWorkRect(rcWork);
            pParent->Dpi().WindowSizeToClientSize(rcWork);
        }
        else {
            UiPoint ptCursor;
            pToast->GetCursorPos(ptCursor);
            pToast->GetMonitorWorkRect(ptCursor, rcWork);
            pToast->Dpi().WindowSizeToClientSize(rcWork);
            rcAnchor = rcWork;
        }

        //2. 统计同一锚点上、排在本通知之前的通知总高度（堆叠偏移）
        int32_t nStackOffset = 0;
        for (ToastWnd* pOther : s_toasts) {
            if (pOther == pToast) {
                break;
            }
            if ((pOther != nullptr) &&
                (pOther->m_pParentWindow == pToast->m_pParentWindow) &&
                (pOther->m_position == pToast->m_position) &&
                !pOther->m_bClosing) {
                nStackOffset += pOther->m_nClientHeight + kToastGap;
            }
        }

        const int32_t nClientW = pToast->m_nClientWidth;
        const int32_t nClientH = pToast->m_nClientHeight;

        //3. 水平位置
        int32_t nClientX = 0;
        switch (pToast->m_position) {
        case kPosTopRight:
        case kPosBottomRight:
            nClientX = rcAnchor.right - kRightMargin - nClientW;
            break;
        case kPosTop:
        case kPosCenter:
        case kPosBottom:
        default:
            nClientX = rcAnchor.CenterX() - nClientW / 2;
            break;
        }

        //4. 垂直位置
        int32_t nClientY = 0;
        switch (pToast->m_position) {
        case kPosBottom:
        case kPosBottomRight:
            nClientY = rcAnchor.bottom - kAnchorMargin - nStackOffset - nClientH;
            break;
        case kPosCenter:
            nClientY = rcAnchor.CenterY() - nClientH / 2 + nStackOffset;
            break;
        case kPosTop:
        case kPosTopRight:
        default:
            nClientY = rcAnchor.top + kAnchorMargin + nStackOffset;
            break;
        }

        //5. 夹取到显示器工作区内
        if (nClientX < rcWork.left) {
            nClientX = rcWork.left;
        }
        if (nClientX + nClientW > rcWork.right) {
            nClientX = rcWork.right - nClientW;
        }
        if (nClientY < rcWork.top) {
            nClientY = rcWork.top;
        }
        if (nClientY + nClientH > rcWork.bottom) {
            nClientY = rcWork.bottom - nClientH;
        }

        //6. 转换为窗口坐标（包含阴影偏移）
        UiPadding rcShadow = pToast->m_pRootBox != nullptr ?
                             pToast->m_pRootBox->GetPadding() : UiPadding();
        nX = nClientX - rcShadow.left;
        nY = nClientY - rcShadow.top;
    }

    void ToastWnd::MoveToWindowPos(int32_t nX, int32_t nY)
    {
        int32_t nWindowX = nX;
        int32_t nWindowY = nY;
        UiSize szWindow(m_nWindowWidth, m_nWindowHeight);
        Dpi().ClientSizeToWindowSize(nWindowX);
        Dpi().ClientSizeToWindowSize(nWindowY);
        Dpi().ClientSizeToWindowSize(szWindow);
        SetWindowPos(InsertAfterWnd(InsertAfterFlag::kHWND_TOPMOST),
                     nWindowX, nWindowY, szWindow.cx, szWindow.cy,
                     kSWP_SHOWWINDOW | kSWP_NOACTIVATE);
    }

} //namespace ui
