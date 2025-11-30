#include "ToolTip.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Control/Label.h"

#ifdef DUILIB_BUILD_FOR_SDL

namespace ui
{
class ToolTipWindow: public Window
{
public:
    ToolTipWindow()
    {
        InitSkin(_T("public/tooltip/"), _T("tooltip.xml"));
    }

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override
    {
        m_pToolTipText = dynamic_cast<Label*>(FindControl(_T("tooltip_text")));
        if (!m_text.empty() && (m_pToolTipText != nullptr)) {
            m_pToolTipText->SetText(m_text);
        }
        if (m_pToolTipText != nullptr) {
            m_pToolTipText->SetMaxWidth(m_nMaxWidth, false);
        }
    }

    /** 设置文本
    */
    void SetToolTipText(const DString& text)
    {
        m_text = text;
        if (m_pToolTipText != nullptr) {
            m_pToolTipText->SetText(text);
        }
    }

    /** 设置文本的最大宽度
    */
    void SetToolTipMaxWidth(int32_t nMaxWidth)
    {
        if (nMaxWidth <= 0) {
            m_nMaxWidth = INT32_MAX;
        }
        else {
            m_nMaxWidth = nMaxWidth;
        }
        if (m_pToolTipText != nullptr) {
            m_pToolTipText->SetMaxWidth(m_nMaxWidth, false);
        }
    }

private:
    /** 文本控件
    */
    ControlPtrT<Label> m_pToolTipText;

    /** Tooltip文本内容
    */
    DString m_text;

    /** 文本的最大宽度
    */
    int32_t m_nMaxWidth = INT32_MAX;
};

class ToolTip::TImpl
{
public:
    TImpl();
    ~TImpl();

public:
    /** 设置鼠标跟踪状态
    * @param [in] pParentWnd 父窗口
    * @param [in] bTracking 是否跟踪鼠标状态
    */
    void SetMouseTracking(WindowBase* pParentWnd, bool bTracking);

    /**@brief 显示ToolTip信息
    * @param [in] pParentWnd 父窗口
    * @param [in] rect Tooltip显示区域
    * @param [in] maxWidth Tooltip显示最大宽度
    * @param [in] trackPos 跟踪的位置
    * @param [in] text Tooltip显示内容
    */
    void ShowToolTip(WindowBase* pParentWnd,
                     const UiRect& rect, 
                     uint32_t maxWidth,
                     const UiPoint& trackPos,
                     const DString& text);

    /**@brief 隐藏ToolTip信息
    */
    void HideToolTip();

    /**@brief 清除鼠标跟踪状态
    */
    void ClearMouseTracking();

    /** 销毁ToolTip窗口
    */
    void DestroyToolTip();

    //停止定时器
    void StopHoverTimer();

public:
    //Tooltip关联的窗口
    ControlPtrT<WindowBase> m_pParentWnd;

    //定时器的有效性保证
    std::weak_ptr<WeakFlag> m_hoverFlag;

    //Tooltip自身的窗口
    ControlPtrT<ToolTipWindow> m_pTooltipWnd;

    //鼠标跟踪状态
    bool m_bMouseTracking;

    //定时器ID
    size_t m_nTimerId;

    // 显示Tooltip的时间间隔，即定时器的触发时间间隔（毫秒）
    const uint32_t m_hoverMillSeconds = 320;
};

ToolTip::TImpl::TImpl():
    m_nTimerId(0),
    m_bMouseTracking(false)
{
}

ToolTip::TImpl::~TImpl()
{
    StopHoverTimer();
}

void ToolTip::TImpl::StopHoverTimer()
{
    m_hoverFlag.reset();
    if (m_nTimerId != 0) {        
        GlobalManager::Instance().Timer().RemoveTimer(m_nTimerId);
        m_nTimerId = 0;        
    }
    m_bMouseTracking = false;
}

void ToolTip::TImpl::SetMouseTracking(WindowBase* pParentWnd, bool bTracking)
{
    ASSERT(pParentWnd != nullptr);
    if (pParentWnd == nullptr) {
        return;
    }
    if (bTracking && !m_bMouseTracking) {
        //启动定时器，跟踪状态
        m_hoverFlag = pParentWnd->GetWeakFlag();
        ControlPtrT<WindowBase> spParentWnd(pParentWnd);
        auto hoverCallback = [this, pParentWnd]() {
                if (pParentWnd != nullptr) {
                    pParentWnd->PostMsg(NativeWindow_SDL::GetHoverMsgId());
                    m_nTimerId = 0;
                    m_bMouseTracking = false;
                    m_hoverFlag.reset();
                }                
            };
        m_nTimerId = GlobalManager::Instance().Timer().AddTimer(m_hoverFlag, hoverCallback, m_hoverMillSeconds, 1);
    }
    m_pParentWnd = pParentWnd;
    m_bMouseTracking = bTracking;
    if (!bTracking && (m_nTimerId != 0)) {
        //停止定时器
        StopHoverTimer();
    }
}

void ToolTip::TImpl::ShowToolTip(WindowBase* pParentWnd,
                                 const UiRect& /*rect*/,
                                 uint32_t maxWidth,
                                 const UiPoint& trackPos,
                                 const DString& text)
{
    ASSERT(pParentWnd != nullptr);
    if (pParentWnd == nullptr) {
        return;
    }
    if (m_pParentWnd != pParentWnd) {
        return;
    }
    if (text.empty()) {
        return;
    }
    
    //窗口的初始位置(客户区坐标)
    UiPoint windowPos = trackPos;
    if ((m_pTooltipWnd == nullptr) || m_pTooltipWnd->IsClosingWnd()) {
        m_pTooltipWnd = new ToolTipWindow;
    }    
    DString skinFolder = m_pTooltipWnd->GetSkinFolder();
    DString skinFile = m_pTooltipWnd->GetSkinFile();
    FilePath xmlPath(skinFolder);
    xmlPath.NormalizeDirectoryPath();
    xmlPath += skinFile;

    UiPadding rcShadowCorner;
    WindowBuilder windowBuilder;
    if (windowBuilder.ParseXmlFile(xmlPath)) {
        WindowCreateAttributes createAttributes;
        if (windowBuilder.ParseWindowCreateAttributes(createAttributes)) {
            rcShadowCorner = createAttributes.m_rcShadowCorner;
        }
    }

    WindowCreateParam createParam;
    createParam.m_nX = windowPos.x + pParentWnd->Dpi().GetScaleInt(10); //在鼠标点，向右的偏移
    createParam.m_nY = windowPos.y + pParentWnd->Dpi().GetScaleInt(10); //在鼠标点，向下的偏移
    //阴影的偏移
    pParentWnd->Dpi().ScaleInt(rcShadowCorner.left);
    pParentWnd->Dpi().ScaleInt(rcShadowCorner.top);
    createParam.m_nX -= rcShadowCorner.left;
    createParam.m_nY -= rcShadowCorner.top;
    
    //SDL内部，对Popup窗口的坐标，调整窗口位置时，使用的是客户区坐标，但其偏移值用的是窗口大小值(主要影响macOS, 高分屏的逻辑)
    pParentWnd->Dpi().UnscaleInt(createParam.m_nX);
    pParentWnd->Dpi().UnscaleInt(createParam.m_nY);
    pParentWnd->Dpi().ScaleWindowSize(createParam.m_nX);
    pParentWnd->Dpi().ScaleWindowSize(createParam.m_nY);

    if (!m_pTooltipWnd->IsWindow()) {
        createParam.m_nWidth = pParentWnd->Dpi().GetScaleWindowSize(200); //窗口创建或者显示后，会自动设置宽度和高度
        createParam.m_nHeight = pParentWnd->Dpi().GetScaleWindowSize(80);
        createParam.m_dwStyle = kWS_POPUPWINDOW;
        createParam.m_dwExStyle = kWS_EX_NOACTIVATE | kWS_EX_TRANSPARENT | kWS_EX_LAYERED | kWS_EX_TOOLTIP_WINDOW;
        m_pTooltipWnd->CreateWnd(pParentWnd, createParam);
    }

    //设置窗口位置
    m_pTooltipWnd->SetWindowPos(InsertAfterWnd(), createParam.m_nX, createParam.m_nY, 0, 0, kSWP_NOSIZE | kSWP_NOZORDER);

    m_pTooltipWnd->SetToolTipMaxWidth((int32_t)maxWidth);
    m_pTooltipWnd->SetToolTipText(text);
    m_pTooltipWnd->ShowWindow(kSW_SHOW_NOACTIVATE);
}

void ToolTip::TImpl::HideToolTip()
{
    //停止定时器
    StopHoverTimer();

    if (m_pTooltipWnd != nullptr) {
        m_pTooltipWnd->ShowWindow(ShowWindowCommands::kSW_HIDE);
    }
}

void ToolTip::TImpl::ClearMouseTracking()
{
    //停止定时器
    StopHoverTimer();
    m_pParentWnd = nullptr;    
}

void ToolTip::TImpl::DestroyToolTip()
{
    if (m_pTooltipWnd != nullptr) {
        m_pTooltipWnd->CloseWnd();
    }
}

ToolTip::ToolTip()
{
    m_impl = new TImpl;
}

ToolTip::~ToolTip()
{
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

void ToolTip::SetMouseTracking(WindowBase* pParentWnd, bool bTracking)
{
    m_impl->SetMouseTracking(pParentWnd, bTracking);
}

void ToolTip::ShowToolTip(WindowBase* pParentWnd,
                          const UiRect& rect,
                          uint32_t maxWidth,
                          const UiPoint& trackPos,
                          const DString& text)
{
    m_impl->ShowToolTip(pParentWnd, rect, maxWidth, trackPos, text);
}

void ToolTip::HideToolTip()
{
    m_impl->HideToolTip();
}

void ToolTip::ClearMouseTracking()
{
    m_impl->ClearMouseTracking();
}

void ToolTip::DestroyToolTip()
{
    m_impl->DestroyToolTip();
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
