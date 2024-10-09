#include "NativeWindow_SDL.h"
#include "MessageLoop_SDL.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Image/ImageDecoder.h"
#include "duilib/Image/ImageLoadAttribute.h"
#include "duilib/Image/ImageInfo.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include <SDL3/SDL.h>

namespace ui {

//窗口指针与SDL窗口ID的映射关系，用于转接消息
std::unordered_map<SDL_WindowID, NativeWindow_SDL*> NativeWindow_SDL::s_windowIDMap;

void NativeWindow_SDL::SetWindowFromID(SDL_WindowID id, NativeWindow_SDL* pNativeWindow)
{
    ASSERT(id != 0);
    if (id == 0) {
        return;
    }
    ASSERT(pNativeWindow != nullptr);
    if (pNativeWindow == nullptr) {
        return;
    }
    ASSERT(pNativeWindow->m_sdlWindow != nullptr);
    if (pNativeWindow->m_sdlWindow == nullptr) {
        return;
    }
    ASSERT(SDL_GetWindowFromID(id) == pNativeWindow->m_sdlWindow);
    if (SDL_GetWindowFromID(id) != pNativeWindow->m_sdlWindow) {
        return;
    }
    s_windowIDMap[id] = pNativeWindow;
}

void NativeWindow_SDL::ClearWindowFromID(SDL_WindowID id, NativeWindow_SDL* pNativeWindow)
{
    ASSERT(id != 0);
    NativeWindow_SDL* pOldNativeWindow = nullptr;
    auto iter = s_windowIDMap.find(id);
    if (iter != s_windowIDMap.end()) {
        pOldNativeWindow = iter->second;
        s_windowIDMap.erase(iter);
    }
    if (pNativeWindow != nullptr) {
        ASSERT(pNativeWindow == pOldNativeWindow);
    }
}

uint32_t NativeWindow_SDL::GetModifiers(SDL_Keymod keymod)
{
    uint32_t modifierKey = 0;
    if (keymod & SDL_KMOD_CTRL) {
        modifierKey |= ModifierKey::kControl;
    }
    if (keymod & SDL_KMOD_SHIFT) {
        modifierKey |= ModifierKey::kShift;
    }
    if (keymod & SDL_KMOD_ALT) {
        modifierKey |= ModifierKey::kAlt;
    }
    if (keymod & SDL_KMOD_GUI) {
        modifierKey |= ModifierKey::kWin;
    }
    return modifierKey;
}

NativeWindow_SDL* NativeWindow_SDL::GetWindowFromID(SDL_WindowID id)
{
    ASSERT(id != 0);
    auto iter = s_windowIDMap.find(id);
    if (iter != s_windowIDMap.end()) {
        return iter->second;
    }
    return nullptr;
}

SDL_WindowID NativeWindow_SDL::GetWindowIdFromEvent(const SDL_Event& sdlEvent)
{
    SDL_Window* sdlWindow = SDL_GetWindowFromEvent(&sdlEvent);
    if (sdlWindow != nullptr) {
        //优先使用SDL的API获取窗口ID
        return SDL_GetWindowID(sdlWindow);
    }

    if ((sdlEvent.type >= SDL_EVENT_WINDOW_FIRST) && (sdlEvent.type <= SDL_EVENT_WINDOW_LAST)) {
        //窗口事件
        return sdlEvent.window.windowID;
    }

    SDL_WindowID windowID = 0;
    switch (sdlEvent.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        //键盘事件
        windowID = sdlEvent.key.windowID;
        break;
    case SDL_EVENT_TEXT_EDITING:
        //键盘事件
        windowID = sdlEvent.edit.windowID;
        break;
    case SDL_EVENT_TEXT_INPUT:
        //键盘事件
        windowID = sdlEvent.text.windowID;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        //鼠标事件
        windowID = sdlEvent.motion.windowID;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        //鼠标事件
        windowID = sdlEvent.button.windowID;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        //鼠标事件
        windowID = sdlEvent.wheel.windowID;
        break;
    default:
        if ((sdlEvent.type > SDL_EVENT_USER) && (sdlEvent.type < SDL_EVENT_LAST)) {
            //用户自定义消息
            windowID = sdlEvent.user.windowID;
        }
        break;
    }

    return windowID;
}

/** 绘制的辅助类
*/
class NativeWindowRenderPaint :
    public IRenderPaint
{
public:
    NativeWindow_SDL* m_pNativeWindow = nullptr;
    INativeWindow* m_pOwner = nullptr;
    NativeMsg m_nativeMsg;
    bool m_bHandled = false;

public:
    /** 通过回调接口，完成绘制
    * @param [in] rcPaint 需要绘制的区域（客户区坐标）
    */
    virtual bool DoPaint(const UiRect& rcPaint) override
    {
        if (m_pOwner != nullptr) {
            m_pOwner->OnNativePaintMsg(rcPaint, m_nativeMsg, m_bHandled);
            return true;
        }
        return false;
    }

    /** 回调接口，获取当前窗口的透明度值
    */
    virtual uint8_t GetLayeredWindowAlpha() override
    {
        return m_pNativeWindow->GetLayeredWindowAlpha();
    }
};

bool NativeWindow_SDL::OnSDLWindowEvent(const SDL_Event& sdlEvent)
{
    INativeWindow* pOwner = m_pOwner;
    ASSERT(pOwner != nullptr);
    if (pOwner == nullptr) {
        return false;
    }
    //接口的生命周期标志
    std::weak_ptr<WeakFlag> ownerFlag = pOwner->GetWeakFlag();

    //消息首先转给过滤器(全部消息)
    bool bHandled = false;
    LRESULT lResult = pOwner->OnNativeWindowMessage(sdlEvent.type, (WPARAM)&sdlEvent, 0, bHandled);
    if (bHandled || ownerFlag.expired()) {
        return true;
    }

    if (sdlEvent.type != SDL_EVENT_WINDOW_DESTROYED) {
        if (m_sdlWindow == nullptr) {
            return false;
        }
    }

    //派发消息到处理函数
    switch (sdlEvent.type) {
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        {
            UiSize newWindowSize;
            SDL_GetWindowSize(m_sdlWindow, &newWindowSize.cx, &newWindowSize.cy);
            ASSERT(sdlEvent.window.data1 == newWindowSize.cx);
            ASSERT(sdlEvent.window.data2 == newWindowSize.cy);
        }
        break;
    case SDL_EVENT_WINDOW_RESIZED:
        {
            //窗口大小改变
            WindowSizeType sizeType = WindowSizeType::kSIZE_RESTORED;
            SDL_WindowFlags nFlags = SDL_GetWindowFlags(m_sdlWindow);
            if (nFlags & SDL_WINDOW_FULLSCREEN) {
                sizeType = WindowSizeType::kSIZE_MAXIMIZED;
            }
            else if (nFlags & SDL_WINDOW_MAXIMIZED) {
                sizeType = WindowSizeType::kSIZE_MAXIMIZED;
            }
            else if (nFlags & SDL_WINDOW_MINIMIZED) {
                sizeType = WindowSizeType::kSIZE_MINIMIZED;
            }
            UiSize newWindowSize;
            newWindowSize.cx = sdlEvent.window.data1;
            newWindowSize.cy = sdlEvent.window.data2;
            lResult = pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(SDL_EVENT_WINDOW_RESIZED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_MINIMIZED:
        {
            //窗口最小化
            WindowSizeType sizeType = WindowSizeType::kSIZE_MINIMIZED;
            UiSize newWindowSize;
            SDL_GetWindowSize(m_sdlWindow, &newWindowSize.cx, &newWindowSize.cy);            
            lResult = pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(SDL_EVENT_WINDOW_MINIMIZED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_MAXIMIZED:
        {
            //窗口最大化
            WindowSizeType sizeType = WindowSizeType::kSIZE_MAXIMIZED;
            UiSize newWindowSize;
            SDL_GetWindowSize(m_sdlWindow, &newWindowSize.cx, &newWindowSize.cy);
            lResult = pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(SDL_EVENT_WINDOW_MAXIMIZED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_RESTORED:
        {
            //窗口还原
            WindowSizeType sizeType = WindowSizeType::kSIZE_RESTORED;
            UiSize newWindowSize;
            SDL_GetWindowSize(m_sdlWindow, &newWindowSize.cx, &newWindowSize.cy);
            lResult = pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(SDL_EVENT_WINDOW_RESTORED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_MOVED:
        {
            //窗口移动
            UiPoint ptTopLeft;
            ptTopLeft.x = sdlEvent.window.data1;
            ptTopLeft.y = sdlEvent.window.data2;
            lResult = pOwner->OnNativeMoveMsg(ptTopLeft, NativeMsg(SDL_EVENT_WINDOW_MOVED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
        {
            //窗口绘制
            bool bPaint = pOwner->OnNativePreparePaint();
            if (bPaint && !ownerFlag.expired()) {
                IRender* pRender = pOwner->OnNativeGetRender();
                ASSERT(pRender != nullptr);
                if (pRender != nullptr) {
                    NativeWindowRenderPaint renderPaint;
                    renderPaint.m_pNativeWindow = this;
                    renderPaint.m_pOwner = pOwner;
                    renderPaint.m_nativeMsg = NativeMsg(SDL_EVENT_WINDOW_EXPOSED, 0, 0);
                    renderPaint.m_bHandled = bHandled;
                    bPaint = pRender->PaintAndSwapBuffers(&renderPaint);
                    bHandled = renderPaint.m_bHandled;
                }
            }
        }
        break;
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
        //不需要处理，Windows没有这个消息
        break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        {
            //鼠标离开窗口
            lResult = pOwner->OnNativeMouseLeaveMsg(NativeMsg(SDL_EVENT_WINDOW_MOUSE_LEAVE, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_MOUSE_MOTION:
        {
            UiPoint pt;
            pt.x = (int32_t)sdlEvent.motion.x;
            pt.y = (int32_t)sdlEvent.motion.y;
            uint32_t modifierKey = GetModifiers(SDL_GetModState());
            lResult = pOwner->OnNativeMouseMoveMsg(pt, modifierKey, false, NativeMsg(SDL_EVENT_MOUSE_MOTION, 0, 0), bHandled);

            //由于SDL没有WM_SETCURSOR消息，在鼠标移动的时候，触发设置光标消息
            if (!ownerFlag.expired()) {
                bool bNativeHandled = false;
                pOwner->OnNativeSetCursorMsg(NativeMsg(SDL_EVENT_MOUSE_MOTION, 0, 0), bNativeHandled);
            }
        }
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        {
#ifdef DUILIB_BUILD_FOR_WIN
            #define WHEEL_DELTA     120
#else
            //TODO: 确认其他平台应该时什么值
            #define WHEEL_DELTA     120
#endif
            int32_t wheelDelta = (int32_t)(sdlEvent.wheel.y * WHEEL_DELTA);
            UiPoint pt;
            pt.x = (int32_t)sdlEvent.wheel.mouse_x;
            pt.y = (int32_t)sdlEvent.wheel.mouse_y;
            uint32_t modifierKey = GetModifiers(SDL_GetModState());
            lResult = pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_WHEEL, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            UiPoint pt;
            pt.x = (int32_t)sdlEvent.button.x;
            pt.y = (int32_t)sdlEvent.button.y;
            uint32_t modifierKey = GetModifiers(SDL_GetModState());
            if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
                //鼠标左键
                lResult = pOwner->OnNativeMouseLButtonDownMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_DOWN, 0, 0), bHandled);
            }
            else if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
                //鼠标右键
                lResult = pOwner->OnNativeMouseRButtonDownMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_DOWN, 0, 0), bHandled);
            }
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            UiPoint pt;
            pt.x = (int32_t)sdlEvent.button.x;
            pt.y = (int32_t)sdlEvent.button.y;
            uint32_t modifierKey = GetModifiers(SDL_GetModState());
            bool bDoubleClick = (sdlEvent.button.clicks == 2) ? true : false;//是否为双击
            if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
                //鼠标左键: 先触发左键弹起消息，然后再触发左键双击消息，避免左键弹起消息丢失的现象
                lResult = pOwner->OnNativeMouseLButtonUpMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                if (!bHandled && bDoubleClick) {
                    lResult = pOwner->OnNativeMouseLButtonDbClickMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
            }
            else if (sdlEvent.button.button == SDL_BUTTON_RIGHT) {
                //鼠标右键: 先触发右键弹起消息，然后再触发右键双击消息，避免右键弹起消息丢失的现象
                lResult = pOwner->OnNativeMouseRButtonUpMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                if (!bHandled && !ownerFlag.expired() && (sdlEvent.button.clicks == 1)) {
                    //模拟Windows，触发一次Context Menu事件
                    bool bMenuHandled = false;
                    pOwner->OnNativeContextMenuMsg(pt, NativeMsg(0, 0, 0), bMenuHandled);
                }
                if (!bHandled && bDoubleClick && !ownerFlag.expired()) {
                    lResult = pOwner->OnNativeMouseRButtonDbClickMsg(pt, modifierKey, NativeMsg(SDL_EVENT_MOUSE_BUTTON_UP, 0, 0), bHandled);
                }
            }
        }
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        {
            //获取键盘输入焦点
            INativeWindow* pLostFocusWindow = nullptr;//此参数得不到（目前无影响，代码中目前没有使用这个参数的）
            lResult = pOwner->OnNativeSetFocusMsg(pLostFocusWindow, NativeMsg(SDL_EVENT_WINDOW_FOCUS_GAINED, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        {
            INativeWindow* pSetFocusWindow = nullptr;//此参数得不到，只能间接获取（这个参数代码中使用的较多，需要获取到）
            SDL_Window* pKeyboardFocus = SDL_GetKeyboardFocus();
            if (pKeyboardFocus != nullptr) {
                SDL_WindowID id = SDL_GetWindowID(pKeyboardFocus);
                if (id != 0) {
                    NativeWindow_SDL* pNativeWindow = GetWindowFromID(id);
                    if ((pNativeWindow != nullptr) && (pNativeWindow != this)) {
                        pSetFocusWindow = pNativeWindow->m_pOwner;
                    }
                }
            }
            lResult = pOwner->OnNativeKillFocusMsg(pSetFocusWindow, NativeMsg(SDL_EVENT_WINDOW_FOCUS_LOST, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_KEY_DOWN:
        {            
            VirtualKeyCode vkCode = Keycode::GetVirtualKeyCode(sdlEvent.key.key);
            uint32_t modifierKey = GetModifiers(sdlEvent.key.mod);
            lResult = pOwner->OnNativeKeyDownMsg(vkCode, modifierKey, NativeMsg(SDL_EVENT_KEY_DOWN, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_KEY_UP:
        {
            VirtualKeyCode vkCode = Keycode::GetVirtualKeyCode(sdlEvent.key.key);
            uint32_t modifierKey = GetModifiers(sdlEvent.key.mod);
            lResult = pOwner->OnNativeKeyUpMsg(vkCode, modifierKey, NativeMsg(SDL_EVENT_KEY_UP, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_TEXT_INPUT:
        {
            //相当于Windows下的WM_CHAR消息
            if (sdlEvent.text.text != nullptr) {
                //该文本为UTF-8编码的
                DStringW textW = StringConvert::UTF8ToUTF16(sdlEvent.text.text);
                if (!textW.empty()) {
                    ASSERT(textW.size() == 1);
                    if (textW.size() == 1) { //这里可能是2，一个字（比如汉字），可能占1-2个Unicode字符
                        //转换成WM_CHAR事件
                        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(textW.front());
                        uint32_t modifierKey = GetModifiers(SDL_GetModState());
                        lResult = m_pOwner->OnNativeCharMsg(vkCode, modifierKey, NativeMsg(SDL_EVENT_TEXT_INPUT, 0, 0), bHandled);
                    }
                }
            }
        }
        break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        {
            //回调关闭事件
            bHandled = false;
            pOwner->OnNativeWindowCloseMsg((uint32_t)m_closeParam, NativeMsg(SDL_EVENT_WINDOW_CLOSE_REQUESTED, 0, 0), bHandled);
            if (bHandled && !ownerFlag.expired()) {
                //拦截了关闭事件, 恢复关闭前的状态
                m_bCloseing = false;
            }
            else if(!ownerFlag.expired()) {
                //PreClose事件
                pOwner->OnNativePreCloseWindow();

                //关闭窗口
                if (!ownerFlag.expired()) {
                    ClearNativeWindow();
                }
            }
        }
        break;
    case SDL_EVENT_WINDOW_DESTROYED:
        {
            //窗口已经销毁
            SDL_WindowID windowID = NativeWindow_SDL::GetWindowIdFromEvent(sdlEvent);
            ClearWindowFromID(windowID, this);

            //窗口已经关闭（关闭后）
            pOwner->OnNativePostCloseWindow();
            if (!ownerFlag.expired()) {
                OnFinalMessage();
            }
        }
        break;
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        {
            //窗口显示的DPI变化
            float fNewDpiScale = SDL_GetWindowDisplayScale(m_sdlWindow);
            if (fNewDpiScale > 0.01f) {
                uint32_t nNewDPI = (uint32_t)(fNewDpiScale * 96);//新的DPI值
                if (nNewDPI != pOwner->OnNativeGetDpi().GetDPI()) {
                    //DPI发生变化
                    uint32_t nOldDpiScale = pOwner->OnNativeGetDpi().GetScale();
                    pOwner->OnNativeProcessDpiChangedMsg(nNewDPI, UiRect());
                    if (!ownerFlag.expired() && (nOldDpiScale != pOwner->OnNativeGetDpi().GetScale())) {
                        m_ptLastMousePos = pOwner->OnNativeGetDpi().GetScalePoint(m_ptLastMousePos, nOldDpiScale);
                    }
                }
            }            
        }
        break;
    case SDL_EVENT_WINDOW_SHOWN:
        {
            lResult = pOwner->OnNativeShowWindowMsg(true, NativeMsg(SDL_EVENT_WINDOW_SHOWN, 0, 0), bHandled);
        }
        break;
    case SDL_EVENT_WINDOW_HIDDEN:
        {
            lResult = pOwner->OnNativeShowWindowMsg(false, NativeMsg(SDL_EVENT_WINDOW_HIDDEN, 0, 0), bHandled);
        }
        break;
    default:
        break;
    }
    return true;
}

NativeWindow_SDL::NativeWindow_SDL(INativeWindow* pOwner):
    m_pOwner(pOwner),
    m_sdlWindow(nullptr),
    m_sdlRenderer(nullptr),
    m_bIsLayeredWindow(false),
    m_nLayeredWindowAlpha(255),
    m_nLayeredWindowOpacity(255),
    m_bUseSystemCaption(false),
    m_bMouseCapture(false),
    m_bCloseing(false),
    m_closeParam(kWindowCloseNormal),
    m_bFakeModal(false),
    m_bDoModal(false),
    m_bFullScreen(false),
    m_ptLastMousePos(-1, -1)
{
    ASSERT(m_pOwner != nullptr);    
}

NativeWindow_SDL::~NativeWindow_SDL()
{
    ASSERT(m_sdlWindow == nullptr);
    ClearNativeWindow();
}

bool NativeWindow_SDL::CreateWnd(NativeWindow_SDL* pParentWindow,
                                 const WindowCreateParam& createParam,
                                 const WindowCreateAttributes& createAttributes)
{
    //获取DiplayMode，避免在底层出现错误时，出现卡死现象
    {
        SDL_DisplayID id = SDL_GetPrimaryDisplay();
        const SDL_DisplayMode* mode1 = SDL_GetDesktopDisplayMode(id);
        const SDL_DisplayMode* mode2 = SDL_GetCurrentDisplayMode(id);
        if (mode1 == mode2) {
            id = 0;
        }
    }

    ASSERT(m_sdlWindow == nullptr);
    if (m_sdlWindow != nullptr) {
        return false;
    }
    ASSERT(m_sdlRenderer == nullptr);
    if (m_sdlRenderer != nullptr) {
        return false;
    }
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
            return false;
        }
    }

    //保存参数
    m_createParam = createParam;

    //设置默认风格
    if (m_createParam.m_dwStyle == 0) {
        m_createParam.m_dwStyle = kWS_OVERLAPPEDWINDOW;
    }

    //同步XML文件中Window的属性，在创建窗口的时候带着这些属性
    SyncCreateWindowAttributes(createAttributes);

    //创建属性
    SDL_PropertiesID props = SDL_CreateProperties();
    SetCreateWindowProperties(props, pParentWindow, createAttributes);
    m_sdlWindow = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return false;
    }

    if (createAttributes.m_bSizeBoxDefined && !createAttributes.m_rcSizeBox.IsZero()) {
        SDL_SetWindowResizable(m_sdlWindow, true);
    }

    m_sdlRenderer = CreateSdlRenderer();
    ASSERT(m_sdlRenderer != nullptr);
    if (m_sdlRenderer == nullptr) {
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
        return false;
    }

    //初始化
    InitNativeWindow();

    if (m_pOwner != nullptr) {
        bool bHandled = false;
        m_pOwner->OnNativeCreateWndMsg(false, NativeMsg(0, 0, 0), bHandled);

        bool bMinimizeBox = false;
        bool bMaximizeBox = false;
        if (m_pOwner->OnNativeHasMinMaxBox(bMinimizeBox, bMaximizeBox)) {
            //如果有最大化按钮，设置可调整窗口大小的属性
            SDL_SetWindowResizable(m_sdlWindow, true);
        }
    }
    return true;
}

int32_t NativeWindow_SDL::DoModal(NativeWindow_SDL* pParentWindow,
                                  const WindowCreateParam& createParam,
                                  const WindowCreateAttributes& createAttributes,
                                  bool bCenterWindow, bool bCloseByEsc, bool bCloseByEnter)
{
    ASSERT(m_sdlWindow == nullptr);
    if (m_sdlWindow != nullptr) {
        return -1;
    }
    ASSERT(m_sdlRenderer == nullptr);
    if (m_sdlRenderer != nullptr) {
        return -1;
    }
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
            return -1;
        }
    }

    //保存参数
    m_createParam = createParam;

    //设置默认风格
    if (m_createParam.m_dwStyle == 0) {
        m_createParam.m_dwStyle = kWS_OVERLAPPEDWINDOW;
    }

    //同步XML文件中Window的属性，在创建窗口的时候带着这些属性
    SyncCreateWindowAttributes(createAttributes);

    //创建属性
    SDL_PropertiesID props = SDL_CreateProperties();
    SetCreateWindowProperties(props, pParentWindow, createAttributes);
    m_sdlWindow = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return -1;
    }

    if (createAttributes.m_bSizeBoxDefined && !createAttributes.m_rcSizeBox.IsZero()) {
        SDL_SetWindowResizable(m_sdlWindow, true);
    }

    m_sdlRenderer = CreateSdlRenderer();
    ASSERT(m_sdlRenderer != nullptr);
    if (m_sdlRenderer == nullptr) {
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
        return -1;
    }

    //初始化
    InitNativeWindow();

    //标记为模式对话框状态
    m_bDoModal = true;

    if (m_pOwner != nullptr) {
        bool bHandled = false;
        m_pOwner->OnNativeCreateWndMsg(true, NativeMsg(0, 0, 0), bHandled);

        bool bMinimizeBox = false;
        bool bMaximizeBox = false;
        if (m_pOwner->OnNativeHasMinMaxBox(bMinimizeBox, bMaximizeBox)) {
            //如果有最大化按钮，设置可调整窗口大小的属性
            SDL_SetWindowResizable(m_sdlWindow, true);
        }
    }

    SDL_WindowID currentWindowId = SDL_GetWindowID(m_sdlWindow);
    if (currentWindowId == 0) {
        m_bDoModal = false;
        return 0;
    }

    //设置未模态对话框
    if (pParentWindow != nullptr) {
        SDL_SetWindowModal(m_sdlWindow, true);
    }

    //设置窗口居中
    if (bCenterWindow) {
        CenterWindow();
    }

    //显示窗口
    m_bCloseing = false;
    m_closeParam = kWindowCloseNormal;
    SDL_ShowWindow(m_sdlWindow);

    //进入内部消息循环
    MessageLoop_SDL messageLoop;
    messageLoop.RunDoModal(*this, bCloseByEsc, bCloseByEnter);

    m_bDoModal = false;
    return m_closeParam;
}

SDL_Renderer* NativeWindow_SDL::CreateSdlRenderer() const
{
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return nullptr;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    //备注：当前支持透明的（属性：SDL_WINDOW_TRANSPARENT）有："direct3d11", "opengl"
    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(m_sdlWindow, "direct3d11");
    if (sdlRenderer == nullptr) {
        sdlRenderer = SDL_CreateRenderer(m_sdlWindow, "opengl");
    }
    if (sdlRenderer == nullptr) {
        //如果创建失败，则使用默认的Render引擎
        sdlRenderer = SDL_CreateRenderer(m_sdlWindow, nullptr);
    }
#else
    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(m_sdlWindow, "opengl");
    if (sdlRenderer == nullptr) {
        //如果创建失败，则使用默认的Render引擎
        sdlRenderer = SDL_CreateRenderer(m_sdlWindow, nullptr);
    }
#endif

    ASSERT(sdlRenderer != nullptr);
    return sdlRenderer;
}

void NativeWindow_SDL::SyncCreateWindowAttributes(const WindowCreateAttributes& createAttributes)
{
    m_bUseSystemCaption = false;
    if (createAttributes.m_bUseSystemCaptionDefined && createAttributes.m_bUseSystemCaption) {
        //使用系统标题栏
        m_bUseSystemCaption = true;
    }
    //由于目前Linux系统尚不支持透明，所以强制使用系统标题栏
    m_bUseSystemCaption = true;

    if (m_bUseSystemCaption) {
        //使用系统标题栏
        if (m_createParam.m_dwStyle & kWS_POPUP) {
            //弹出式窗口
            m_createParam.m_dwStyle |= (kWS_CAPTION | kWS_SYSMENU);
        }
        else {
            m_createParam.m_dwStyle |= (kWS_CAPTION | kWS_SYSMENU | kWS_MINIMIZEBOX | kWS_MAXIMIZEBOX);
        }
    }

    //初始化层窗口属性
    m_bIsLayeredWindow = false;
    if (createAttributes.m_bIsLayeredWindowDefined) {
        if (createAttributes.m_bIsLayeredWindow) {
            m_bIsLayeredWindow = true;
            m_createParam.m_dwExStyle |= kWS_EX_LAYERED;
        }
        else {
            m_createParam.m_dwExStyle &= ~kWS_EX_LAYERED;
        }
    }
    else if (m_createParam.m_dwExStyle & kWS_EX_LAYERED) {
        m_bIsLayeredWindow = true;
    }

    //如果使用系统标题栏，关闭层窗口
    if (m_bUseSystemCaption) {
        m_bIsLayeredWindow = false;
        m_createParam.m_dwExStyle &= ~kWS_EX_LAYERED;
    }

    //如果设置了不透明度，则设置为层窗口
    if (createAttributes.m_bLayeredWindowOpacityDefined && (createAttributes.m_nLayeredWindowOpacity != 255)) {
        m_createParam.m_dwExStyle |= kWS_EX_LAYERED;
        m_bIsLayeredWindow = true;
    }
    if (createAttributes.m_bLayeredWindowAlphaDefined && (createAttributes.m_nLayeredWindowAlpha != 255)) {
        m_createParam.m_dwExStyle |= kWS_EX_LAYERED;
        m_bIsLayeredWindow = true;
    }

    if (createAttributes.m_bInitSizeDefined) {
        if (createAttributes.m_szInitSize.cx > 0) {
            m_createParam.m_nWidth = createAttributes.m_szInitSize.cx;
        }
        if (createAttributes.m_szInitSize.cy > 0) {
            m_createParam.m_nHeight = createAttributes.m_szInitSize.cy;
        }
    }
}

void NativeWindow_SDL::SetCreateWindowProperties(SDL_PropertiesID props, NativeWindow_SDL* pParentWindow,
                                                 const WindowCreateAttributes& createAttributes)
{
    //设置关闭窗口的时候，不自动退出消息循环
    SDL_SetHint(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE, "false");

    //需要提前设置这个属性，在无边框的情况下，可以保持调整窗口大小的功能
    SDL_SetHint("SDL_BORDERLESS_RESIZABLE_STYLE", "true");

    if (!m_createParam.m_windowTitle.empty()) {
        std::string windowTitle = StringConvert::TToUTF8(m_createParam.m_windowTitle);
        SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, windowTitle.c_str());
    }

    //窗口的位置和大小
    int32_t x = kCW_USEDEFAULT;
    int32_t y = kCW_USEDEFAULT;
    int32_t cx = kCW_USEDEFAULT;
    int32_t cy = kCW_USEDEFAULT;

    if ((m_createParam.m_nX != kCW_USEDEFAULT) && (m_createParam.m_nY != kCW_USEDEFAULT)) {
        x = m_createParam.m_nX;
        y = m_createParam.m_nY;
    }
    if ((m_createParam.m_nWidth != kCW_USEDEFAULT) && (m_createParam.m_nHeight != kCW_USEDEFAULT)) {
        cx = m_createParam.m_nWidth;
        cy = m_createParam.m_nHeight;
    }
    if (x != kCW_USEDEFAULT) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, x);
    }
    if (y != kCW_USEDEFAULT) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, y);
    }
    if (cx != kCW_USEDEFAULT) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, cx);
    }
    if (cy != kCW_USEDEFAULT) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, cy);
    }

    //父窗口
    if ((pParentWindow != nullptr) && (pParentWindow->m_sdlWindow != nullptr)) {
        SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_PARENT_POINTER, pParentWindow->m_sdlWindow);
    }

    //窗口属性
    SDL_WindowFlags windowFlags = SDL_WINDOW_INPUT_FOCUS;
    if (createAttributes.m_bSizeBoxDefined && !createAttributes.m_rcSizeBox.IsZero()) {
        windowFlags |= SDL_WINDOW_RESIZABLE;
    }

    //创建的时候，窗口保持隐藏状态，需要调用API显示窗口，避免创建窗口的时候闪烁
    windowFlags |= SDL_WINDOW_HIDDEN;

    if (!m_bUseSystemCaption && m_bIsLayeredWindow) {
        //设置透明属性，这个属性必须在创建窗口时传入，窗口创建完成后，不支持修改
        windowFlags |= SDL_WINDOW_TRANSPARENT;
    }

    //如果是弹出窗口，并且无阴影和标题栏，则默认为无边框
    const bool bPopupWindow = m_createParam.m_dwStyle & kWS_POPUP;
    if (!m_bUseSystemCaption) {
        //只要没有使用系统标题栏，就需要设置此属性，否则窗口就会带系统标题栏
        windowFlags |= SDL_WINDOW_BORDERLESS;
    }

    if (bPopupWindow && (m_createParam.m_dwExStyle & kWS_EX_TOOLWINDOW)) {
        windowFlags |= SDL_WINDOW_UTILITY;
    }
    else if (bPopupWindow && (pParentWindow != nullptr)) {
        //SDL窗口的父子窗口关系：在系统层面，并未设置父子关系，所以弹出窗口需要设置这个属性，避免任务栏出现多个窗口的现象
        //但由于SDL未使用系统的父子窗口关系，弹出窗口后，主窗口便失去焦点，任务栏上显示的是非激活状态，体验不佳
        windowFlags |= SDL_WINDOW_UTILITY;
    }
    if (bPopupWindow && (m_createParam.m_dwExStyle & kWS_EX_NOACTIVATE)) {
        windowFlags |= SDL_WINDOW_NOT_FOCUSABLE;
    }
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, windowFlags);
}

SDL_HitTestResult SDLCALL NativeWindow_SDL_HitTest(SDL_Window* win,
                                                   const SDL_Point* area,
                                                   void* data)
{
    NativeWindow_SDL* pWindow = (NativeWindow_SDL*)data;
    if (pWindow == nullptr) {
        return SDL_HITTEST_NORMAL;
    }
    return (SDL_HitTestResult)pWindow->SDL_HitTest(win, area, data);
}

int32_t NativeWindow_SDL::SDL_HitTest(SDL_Window* win, const SDL_Point* area, void* data)
{
    if (((NativeWindow_SDL*)data != this) ||
        (area == nullptr) || (win != GetWindowHandle()) ||
        IsUseSystemCaption() || (m_pOwner == nullptr)) {
        return SDL_HITTEST_NORMAL;
    }

    //全屏模式, 只返回客户区域属性
    if (IsWindowFullScreen()) {
        return SDL_HITTEST_NORMAL;
    }

    UiPoint pt;
    pt.x = area->x;
    pt.y = area->y;

    UiRect rcClient;
    GetClientRect(rcClient);

    //客户区域，排除掉阴影部分区域
    UiPadding rcCorner;
    m_pOwner->OnNativeGetShadowCorner(rcCorner);
    rcClient.Deflate(rcCorner);

    if (!IsWindowMaximized()) {
        //非最大化状态
        UiRect rcSizeBox = m_pOwner->OnNativeGetSizeBox();
        if (pt.y < rcClient.top + rcSizeBox.top) {
            if (pt.y >= rcClient.top) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return SDL_HITTEST_RESIZE_TOPLEFT;//在窗口边框的左上角。
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return SDL_HITTEST_RESIZE_TOPRIGHT;//在窗口边框的右上角
                }
                else {
                    return SDL_HITTEST_RESIZE_TOP;//在窗口的上水平边框中
                }
            }
            else {
                return SDL_HITTEST_NORMAL;//在工作区中
            }
        }
        else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
            if (pt.y <= rcClient.bottom) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return SDL_HITTEST_RESIZE_BOTTOMLEFT;//在窗口边框的左下角
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return SDL_HITTEST_RESIZE_BOTTOMRIGHT;//在窗口边框的右下角
                }
                else {
                    return SDL_HITTEST_RESIZE_BOTTOM;//在窗口的下水平边框中
                }
            }
            else {
                return SDL_HITTEST_NORMAL;//在工作区中
            }
        }

        if (pt.x < rcClient.left + rcSizeBox.left) {
            if (pt.x >= rcClient.left) {
                return SDL_HITTEST_RESIZE_LEFT;//在窗口的左边框
            }
            else {
                return SDL_HITTEST_NORMAL;//在工作区中
            }
        }
        if (pt.x > rcClient.right - rcSizeBox.right) {
            if (pt.x <= rcClient.right) {
                return SDL_HITTEST_RESIZE_RIGHT;//在窗口的右边框中
            }
            else {
                return SDL_HITTEST_NORMAL;//在工作区中
            }
        }
    }

    UiRect rcCaption;
    m_pOwner->OnNativeGetCaptionRect(rcCaption);
    //标题栏区域的矩形范围
    UiRect rcCaptionRect;
    rcCaptionRect.left = rcClient.left + rcCaption.left;
    rcCaptionRect.right = rcClient.right - rcCaption.right;
    rcCaptionRect.top = rcClient.top + rcCaption.top;
    rcCaptionRect.bottom = rcClient.top + rcCaption.bottom;
    if (rcCaptionRect.ContainsPt(pt)) {
        //在标题栏范围内（SDL不支持HTSYSMENU和HTMAXBUTTON，此处实现与Windows默认的不同）
        if (m_pOwner->OnNativeIsPtInCaptionBarControl(pt)) {
            return SDL_HITTEST_NORMAL;//在工作区中（放在标题栏上的控件，视为工作区）
        }
        else {
            return SDL_HITTEST_DRAGGABLE;//在标题栏中
        }
    }
    //其他，在工作区中
    return SDL_HITTEST_NORMAL;
}

void NativeWindow_SDL::InitNativeWindow()
{
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return;
    }

    SDL_WindowID id = SDL_GetWindowID(m_sdlWindow);
    SetWindowFromID(id, this);

    //设置Hit Test函数
    if (!IsUseSystemCaption()) {
        bool nRet = SDL_SetWindowHitTest(m_sdlWindow, NativeWindow_SDL_HitTest, this);
        ASSERT_UNUSED_VARIABLE(nRet);
    }
    else {
        bool nRet = SDL_SetWindowHitTest(m_sdlWindow, nullptr, nullptr);
        ASSERT_UNUSED_VARIABLE(nRet);
    }

    if (!m_createParam.m_windowTitle.empty()) {
        std::string windowTitle = StringConvert::TToUTF8(m_createParam.m_windowTitle);
        bool nRet = SDL_SetWindowTitle(m_sdlWindow, windowTitle.c_str());
        ASSERT_UNUSED_VARIABLE(nRet);
    }    
}

void NativeWindow_SDL::ClearNativeWindow()
{
    if (m_sdlWindow != nullptr) {    
        SDL_SetWindowHitTest(m_sdlWindow, nullptr, nullptr);
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
    }
    if (m_sdlRenderer != nullptr) {
        SDL_DestroyRenderer(m_sdlRenderer);
        m_sdlRenderer = nullptr;
    }
}

void* NativeWindow_SDL::GetWindowHandle() const
{
    return m_sdlWindow;
}

bool NativeWindow_SDL::IsWindow() const
{
    return (m_sdlWindow != nullptr);
}

#ifdef DUILIB_BUILD_FOR_WIN

HWND NativeWindow_SDL::GetHWND() const
{
    if (!IsWindow()) {
        return nullptr;
    }
    SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
    HWND hWnd = (HWND)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    if (!::IsWindow(hWnd)) {
        hWnd = nullptr;
    }
    return hWnd;
}

HMODULE NativeWindow_SDL::GetResModuleHandle() const
{
    if (!IsWindow()) {
        return nullptr;
    }
    SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
    HMODULE hModule = (HMODULE)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr);
    ASSERT(hModule != nullptr);
    if (hModule == nullptr) {
        hModule = ::GetModuleHandle(nullptr);
    }
    return hModule;
}

HDC NativeWindow_SDL::GetPaintDC() const
{
    if (!IsWindow()) {
        return nullptr;
    }
    SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
    HDC hDC = (HDC)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HDC_POINTER, nullptr);
    ASSERT(hDC != nullptr);
    return hDC;
}

#endif //DUILIB_BUILD_FOR_WIN

void NativeWindow_SDL::CloseWnd(int32_t nRet)
{
    m_closeParam = nRet;
    m_bCloseing = true;

    //异步关闭
    if (m_sdlWindow != nullptr) {
        SDL_Event sdlEvent;
        sdlEvent.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
        sdlEvent.common.timestamp = 0;
        sdlEvent.window.data1 = 0;
        sdlEvent.window.data2 = 0;
        sdlEvent.window.windowID = SDL_GetWindowID(m_sdlWindow);
        bool nRetE = SDL_PushEvent(&sdlEvent);
        ASSERT_UNUSED_VARIABLE(nRetE);
    }
}

void NativeWindow_SDL::Close()
{
    //同步关闭
    m_bCloseing = true;
    if (m_pOwner == nullptr) {
        return;
    }

    //回调关闭事件
    bool bHandled = false;
    m_pOwner->OnNativeWindowCloseMsg(0, NativeMsg(0, 0, 0), bHandled);
    if (bHandled) {
        //拦截了关闭事件
        m_bCloseing = false;
        return;
    }

    //PreClose事件
    if (m_pOwner) {
        m_pOwner->OnNativePreCloseWindow();
    }

    //关闭窗口
    ClearNativeWindow();
}

bool NativeWindow_SDL::IsClosingWnd() const
{
    return m_bCloseing;
}

int32_t NativeWindow_SDL::GetCloseParam() const
{
    return m_closeParam;
}

bool NativeWindow_SDL::IsLayeredWindow() const
{
    return m_bIsLayeredWindow;
}

void NativeWindow_SDL::SetLayeredWindowAlpha(int32_t nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    m_nLayeredWindowAlpha = static_cast<uint8_t>(nAlpha);
}

uint8_t NativeWindow_SDL::GetLayeredWindowAlpha() const
{
    return m_nLayeredWindowAlpha;
}

void NativeWindow_SDL::SetLayeredWindowOpacity(int32_t nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    m_nLayeredWindowOpacity = static_cast<uint8_t>(nAlpha);
    float opacity = 1.0f;//完全不透明
    if (m_nLayeredWindowOpacity != 255) {
        //支持不透明度设置
        opacity = 1.0f * nAlpha / 255.0f;
    }
    bool nRet = SDL_SetWindowOpacity(m_sdlWindow, opacity);
    ASSERT_UNUSED_VARIABLE(nRet);
}

uint8_t NativeWindow_SDL::GetLayeredWindowOpacity() const
{
    return m_nLayeredWindowOpacity;
}

void NativeWindow_SDL::SetUseSystemCaption(bool bUseSystemCaption)
{
    m_bUseSystemCaption = bUseSystemCaption;

    //由于目前Linux系统尚不支持透明，所以强制使用系统标题栏
    m_bUseSystemCaption = true;

    if (IsUseSystemCaption()) {
        //使用系统默认标题栏, 需要增加标题栏风格
        if (IsWindow()) {
            bool nRet = SDL_SetWindowBordered(m_sdlWindow, true);
            ASSERT_UNUSED_VARIABLE(nRet);
        }
        //关闭层窗口
        if (IsLayeredWindow()) {
            SetLayeredWindow(false, false);
        }

        //设置Hit Test函数为默认
        SDL_SetWindowHitTest(m_sdlWindow, nullptr, nullptr);
    }
    else {
        //需要提前设置这个属性，在无边框的情况下，可以保持调整窗口大小的功能
        SDL_SetHint("SDL_BORDERLESS_RESIZABLE_STYLE", "true"); 
        bool nRet = SDL_SetWindowBordered(m_sdlWindow, false);
        ASSERT_UNUSED_VARIABLE(nRet);

        //设置HitTest函数
        nRet = SDL_SetWindowHitTest(m_sdlWindow, NativeWindow_SDL_HitTest, this);
        ASSERT_UNUSED_VARIABLE(nRet);
    }
    m_pOwner->OnNativeUseSystemCaptionBarChanged();
}

bool NativeWindow_SDL::IsUseSystemCaption() const
{
    return m_bUseSystemCaption;
}

bool NativeWindow_SDL::ShowWindow(ShowWindowCommands nCmdShow)
{
    ASSERT(IsWindow());
    if (m_sdlWindow == nullptr) {
        return false;
    }
    bool nRet = false;
    switch(nCmdShow)
    {
    case kSW_HIDE:
        nRet = SDL_HideWindow(m_sdlWindow);
        break;
    case kSW_SHOW:
    case kSW_SHOW_NORMAL:
        {
            const bool bActivate = SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, true);
            if (!bActivate) {
                SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "true");
                ASSERT(SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, true) == true);
            }
            nRet = SDL_ShowWindow(m_sdlWindow);
            if (!bActivate) {
                SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "false");
                ASSERT(SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, true) == false);
            }
            //调整窗口的Z-Order，避免有时候窗口不显示的问题
            SDL_RaiseWindow(m_sdlWindow);
        }        
        break;
    case kSW_SHOW_NA:
    case kSW_SHOW_NOACTIVATE:
        {
            const bool bActivate = SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, true);
            if (bActivate) {
                SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "false");
                ASSERT(SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, true) == false);
            }
            nRet = SDL_ShowWindow(m_sdlWindow);
            if (bActivate) {
                SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "true");
                ASSERT(SDL_GetHintBoolean(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, true) == true);
            }
        }
        break;
    case kSW_MINIMIZE:
    case kSW_SHOW_MINIMIZED:
        nRet = SDL_MinimizeWindow(m_sdlWindow);
        break;
    case kSW_SHOW_MAXIMIZED:
        nRet = SDL_MaximizeWindow(m_sdlWindow);
        break;
    case kSW_SHOW_MIN_NOACTIVE:
        //不支持No Active
        nRet = SDL_MinimizeWindow(m_sdlWindow);
        break;
    case kSW_RESTORE:
        nRet = SDL_RestoreWindow(m_sdlWindow);
        break;
    default:
        ASSERT(false);
        break;
    }
    return nRet;
}

void NativeWindow_SDL::ShowModalFake(NativeWindow_SDL* pParentWindow)
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    if (!pParentWindow->IsWindow()) {
        pParentWindow = nullptr;
    }
    if (pParentWindow != nullptr) {
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(false);
        }
    }
    ShowWindow(kSW_SHOW_NORMAL);
    if (pParentWindow != nullptr) {
        bool nRet = SDL_SetWindowParent(m_sdlWindow, pParentWindow->m_sdlWindow);
        ASSERT_UNUSED_VARIABLE(nRet);

        nRet = SDL_SetWindowModal(m_sdlWindow, true);
        ASSERT_UNUSED_VARIABLE(nRet);
        m_bFakeModal = true;
    }    
}

void NativeWindow_SDL::OnCloseModalFake(NativeWindow_SDL* pParentWindow)
{
    if (IsFakeModal()) {
        bool nRet = SDL_SetWindowModal(m_sdlWindow, false);
        ASSERT_UNUSED_VARIABLE(nRet);

        nRet = SDL_SetWindowParent(m_sdlWindow, nullptr);
        ASSERT_UNUSED_VARIABLE(nRet);

        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(true);
            pParentWindow->SetWindowFocus();
        }
        m_bFakeModal = false;
    }
}

bool NativeWindow_SDL::IsFakeModal() const
{
    return m_bFakeModal;
}

bool NativeWindow_SDL::IsDoModal() const
{
    return m_bDoModal;
}

void NativeWindow_SDL::CenterWindow()
{  
    ASSERT(IsWindow());
    UiRect rcDlg;
    GetWindowRect(rcDlg);
    UiRect rcArea;
    UiRect rcCenter;

    SDL_Window* pCenterWindow = SDL_GetWindowParent(m_sdlWindow);

    // 处理多显示器模式下屏幕居中
    UiRect rcMonitor;
    GetMonitorRect(m_sdlWindow, rcMonitor, rcArea);
    if (pCenterWindow == nullptr) {
        rcCenter = rcArea;
    }
    else if (SDL_GetWindowFlags(pCenterWindow) & SDL_WINDOW_MINIMIZED) {
        rcCenter = rcArea;
    }
    else {
        GetWindowRect(pCenterWindow, rcCenter);
    }

    // Find dialog's upper left based on rcCenter
    int xLeft = rcCenter.CenterX() - rcDlg.Width() / 2;
    int yTop = rcCenter.CenterY() - rcDlg.Height() / 2;

    // The dialog is outside the screen, move it inside
    if (xLeft < rcArea.left) {
        xLeft = rcArea.left;
    }
    else if (xLeft + rcDlg.Width() > rcArea.right) {
        xLeft = rcArea.right - rcDlg.Width();
    }
    if (yTop < rcArea.top) {
        yTop = rcArea.top;
    }
    else if (yTop + rcDlg.Height() > rcArea.bottom) {
        yTop = rcArea.bottom - rcDlg.Height();
    }
    SetWindowPos(nullptr, InsertAfterFlag(), xLeft, yTop, -1, -1, kSWP_NOSIZE | kSWP_NOZORDER | kSWP_NOACTIVATE);
}

void NativeWindow_SDL::SetWindowAlwaysOnTop(bool bOnTop)
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    bool nRet = SDL_SetWindowAlwaysOnTop(m_sdlWindow, bOnTop ? true : false);
    ASSERT_UNUSED_VARIABLE(nRet);
}

bool NativeWindow_SDL::IsWindowAlwaysOnTop() const
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    SDL_WindowFlags nFlags = SDL_GetWindowFlags(m_sdlWindow);
    return (nFlags & SDL_WINDOW_ALWAYS_ON_TOP) ? true : false;
}

bool NativeWindow_SDL::SetWindowForeground()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    bool nRet = SDL_RaiseWindow(m_sdlWindow);
    ASSERT(nRet);
//#ifdef _DEBUG
//    备注：这里有时候会出现条件不成立的情况，暂时未发现影响功能
//    auto pKeyboardFocus = SDL_GetKeyboardFocus();
//    ASSERT(pKeyboardFocus == m_sdlWindow);
//#endif
    return nRet;
}

bool NativeWindow_SDL::IsWindowForeground() const
{
    if (!IsWindow()) {
        return false;
    }
    return (SDL_GetKeyboardFocus() == m_sdlWindow) ? true : false;
}

bool NativeWindow_SDL::SetWindowFocus()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    SDL_Window* pKeyboardFocus = SDL_GetKeyboardFocus();
    if (pKeyboardFocus != m_sdlWindow) {
        SetWindowForeground();
        pKeyboardFocus = SDL_GetKeyboardFocus();
    }
    return (pKeyboardFocus == m_sdlWindow) ? true : false;
}

bool NativeWindow_SDL::IsWindowFocused() const
{
    if (!IsWindow()) {
        return false;
    }
    return (SDL_GetKeyboardFocus() == m_sdlWindow) ? true : false;
}

void NativeWindow_SDL::CheckSetWindowFocus()
{
    if (!IsWindowFocused()) {
        SetWindowFocus();
    }
}

LRESULT NativeWindow_SDL::PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return -1;
    }

    ASSERT((uMsg > SDL_EVENT_USER) && (uMsg < SDL_EVENT_LAST));
    if ((uMsg <= SDL_EVENT_USER) || (uMsg >= SDL_EVENT_LAST)) {
        return -1;
    }
    
    SDL_Event sdlEvent;
    sdlEvent.type = uMsg;
    sdlEvent.common.timestamp = 0;
    sdlEvent.user.type = uMsg;
    sdlEvent.user.reserved = 0;
    sdlEvent.user.timestamp = 0;
    sdlEvent.user.code = uMsg;
    sdlEvent.user.data1 = (void*)wParam;
    sdlEvent.user.data2 = (void*)lParam;
    sdlEvent.user.windowID = SDL_GetWindowID(m_sdlWindow);
    bool nRet = SDL_PushEvent(&sdlEvent);
    ASSERT(nRet);
    return nRet ? 0 : -1;
}

void NativeWindow_SDL::PostQuitMsg(int32_t /*nExitCode*/)
{
    SDL_Event sdlEvent;
    sdlEvent.type = SDL_EVENT_QUIT;
    sdlEvent.common.timestamp = 0;
    bool nRet = SDL_PushEvent(&sdlEvent);
    ASSERT_UNUSED_VARIABLE(nRet);
}

bool NativeWindow_SDL::EnterFullScreen()
{
    ASSERT(IsWindow());
    if (m_sdlWindow == nullptr) {
        return false;
    }
    if (IsWindowMinimized()) {
        //最小化的时候，不允许激活全屏
        return false;
    }
    if (m_bFullScreen) {
        return true;
    }
    m_bFullScreen = true;

    bool nRet = SDL_SetWindowFullscreen(m_sdlWindow, true);
    ASSERT_UNUSED_VARIABLE(nRet);

    m_lastWindowFlags = ::SDL_GetWindowFlags(m_sdlWindow);
    if (m_lastWindowFlags & SDL_WINDOW_RESIZABLE) {
        //需要去掉可调整窗口大小的属性
        SDL_SetWindowResizable(m_sdlWindow, false);
    }
    
    m_pOwner->OnNativeWindowEnterFullScreen();
    return true;
}

bool NativeWindow_SDL::ExitFullScreen()
{
    ASSERT(IsWindow());
    if (m_sdlWindow == nullptr) {
        return false;
    }
    if (!m_bFullScreen) {
        return false;
    }

    bool nRet = SDL_SetWindowFullscreen(m_sdlWindow, false);
    ASSERT_UNUSED_VARIABLE(nRet);

    if (m_lastWindowFlags & SDL_WINDOW_RESIZABLE) {
        //需要恢复可调整窗口大小的属性
        SDL_SetWindowResizable(m_sdlWindow, true);
    }

    m_bFullScreen = false;
    m_pOwner->OnNativeWindowExitFullScreen();
    return true;
}

bool NativeWindow_SDL::IsWindowMaximized() const
{
    ASSERT(IsWindow());
    bool bWindowMaximized = false;
    if (m_sdlWindow != nullptr) {
        SDL_WindowFlags nFlags = SDL_GetWindowFlags(m_sdlWindow);
        bWindowMaximized = nFlags & SDL_WINDOW_MAXIMIZED;
    }
    return bWindowMaximized;
}

bool NativeWindow_SDL::IsWindowMinimized() const
{
    ASSERT(IsWindow());
    bool bWindowMinimized = false;
    if (m_sdlWindow != nullptr) {
        SDL_WindowFlags nFlags = SDL_GetWindowFlags(m_sdlWindow);
        bWindowMinimized = nFlags & SDL_WINDOW_MINIMIZED;
    }
    return bWindowMinimized;
}

bool NativeWindow_SDL::IsWindowFullScreen() const
{
    ASSERT(IsWindow());
    bool bFullScreen = false;
    if (m_sdlWindow != nullptr) {
        SDL_WindowFlags nFlags = SDL_GetWindowFlags(m_sdlWindow);
        bFullScreen = nFlags & SDL_WINDOW_FULLSCREEN;
    }
    return bFullScreen;
}

bool NativeWindow_SDL::IsWindowVisible() const
{
    if (!IsWindow()) {
        return false;
    }
    SDL_WindowFlags windowFlags = SDL_GetWindowFlags(m_sdlWindow);
    return (windowFlags & SDL_WINDOW_HIDDEN) ? false : true;
}

bool NativeWindow_SDL::SetWindowPos(const NativeWindow_SDL* /*pInsertAfterWindow*/,
                                   InsertAfterFlag /*insertAfterFlag*/,
                                   int32_t X, int32_t Y, int32_t cx, int32_t cy,
                                   uint32_t uFlags)
{
    bool bRet = true;
    ASSERT(IsWindow());
    if (!(uFlags & kSWP_NOMOVE)) {
        bool nRet = SDL_SetWindowPosition(m_sdlWindow, X, Y);
        ASSERT_UNUSED_VARIABLE(nRet);
        if (!nRet) {
            bRet = false;
        }
    }
    if (!(uFlags & kSWP_NOSIZE)) {
        bool nRet = SDL_SetWindowSize(m_sdlWindow, cx, cy);
        ASSERT_UNUSED_VARIABLE(nRet);
        if (!nRet) {
            bRet = false;
        }
    }

    if (uFlags & kSWP_HIDEWINDOW) {
        if (!ShowWindow(ShowWindowCommands::kSW_HIDE)) {
            bRet = false;
        }
    }
    if (uFlags & kSWP_SHOWWINDOW) {
        ShowWindowCommands showCommand = ShowWindowCommands::kSW_SHOW;
        if (uFlags & kSWP_NOACTIVATE) {
            showCommand = ShowWindowCommands::kSW_SHOW_NA;
        }
        if (!ShowWindow(showCommand)) {
            bRet = false;
        }
    }
    if (bRet) {
        SDL_SyncWindow(m_sdlWindow);
    }    
    return bRet;
}

bool NativeWindow_SDL::MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool /*bRepaint*/)
{
    ASSERT(IsWindow());
    bool bRet = SDL_SetWindowPosition(m_sdlWindow, X, Y);
    ASSERT(bRet);
    bool nRet = SDL_SetWindowSize(m_sdlWindow, nWidth, nHeight);
    ASSERT(nRet);
    if (!nRet) {
        bRet = false;
    }    
    SDL_SyncWindow(m_sdlWindow);
    return nRet;
}

void NativeWindow_SDL::SetText(const DString& strText)
{
    ASSERT(IsWindow());
    //转为UTF-8编码
    DStringA utf8Text = StringConvert::TToUTF8(strText);
    bool nRet = SDL_SetWindowTitle(m_sdlWindow, utf8Text.c_str());
    ASSERT_UNUSED_VARIABLE(nRet);
}

void NativeWindow_SDL::SetWindowMaximumSize(const UiSize& szMaxWindow)
{
    m_szMaxWindow = szMaxWindow;
    if (m_szMaxWindow.cx < 0) {
        m_szMaxWindow.cx = 0;
    }
    if (m_szMaxWindow.cy < 0) {
        m_szMaxWindow.cy = 0;
    }
    ASSERT(IsWindow());
    if (m_sdlWindow != nullptr) {
        SDL_SetWindowMaximumSize(m_sdlWindow, m_szMaxWindow.cx, m_szMaxWindow.cy);
    }
}

const UiSize& NativeWindow_SDL::GetWindowMaximumSize() const
{
    return m_szMaxWindow;
}

void NativeWindow_SDL::SetWindowMinimumSize(const UiSize& szMinWindow)
{
    m_szMinWindow = szMinWindow;
    if (m_szMinWindow.cx < 0) {
        m_szMinWindow.cx = 0;
    }
    if (m_szMinWindow.cy < 0) {
        m_szMinWindow.cy = 0;
    }
    ASSERT(IsWindow());
    if (m_sdlWindow != nullptr) {
        SDL_SetWindowMinimumSize(m_sdlWindow, m_szMinWindow.cx, m_szMinWindow.cy);
    }
}

const UiSize& NativeWindow_SDL::GetWindowMinimumSize() const
{
    return m_szMinWindow;
}

void NativeWindow_SDL::SetCapture()
{
    //ASSERT(SDL_GetMouseFocus() == m_sdlWindow);
    if (SDL_GetMouseFocus() == m_sdlWindow) {
        bool nRet = SDL_CaptureMouse(true);
        ASSERT_UNUSED_VARIABLE(nRet);
        if (nRet) {
            m_bMouseCapture = true;
        }
    }
}

void NativeWindow_SDL::ReleaseCapture()
{
    if (m_bMouseCapture) {
        ASSERT(SDL_GetMouseFocus() == m_sdlWindow);
        if (SDL_GetMouseFocus() == m_sdlWindow) {
            bool nRet = SDL_CaptureMouse(false);
            ASSERT_UNUSED_VARIABLE(nRet);
        }
        m_bMouseCapture = false;
    }
}

bool NativeWindow_SDL::IsCaptured() const
{
    return m_bMouseCapture;
}

struct NativeWindowExposedEvent
{
    SDL_Window* m_sdlWindow = nullptr;
    bool m_bFoundExposedEvent = false;
};

bool SDLCALL FilterNativeWindowExposedEvent(void* userdata, SDL_Event* event)
{
    if ((userdata != nullptr) && (event != nullptr)) {
        NativeWindowExposedEvent* data = (NativeWindowExposedEvent*)userdata;
        if (!data->m_bFoundExposedEvent && (event->type == SDL_EVENT_WINDOW_EXPOSED)) {
            if (data->m_sdlWindow == SDL_GetWindowFromEvent(event)) {
                data->m_bFoundExposedEvent = true;
            }
        }
    }
    return true;
}

void NativeWindow_SDL::Invalidate(const UiRect& rcItem)
{
#ifdef DUILIB_BUILD_FOR_WIN
    HWND hWnd = GetHWND();
    RECT rc = { rcItem.left, rcItem.top, rcItem.right, rcItem.bottom };
    ::InvalidateRect(hWnd, &rc, FALSE);
#else
    (void)rcItem;
    //暂时没有此功能, 只能发送一个绘制消息，触发界面绘制
    if (m_sdlWindow != nullptr) {
        NativeWindowExposedEvent data;
        data.m_bFoundExposedEvent = false;
        data.m_sdlWindow = m_sdlWindow;
        SDL_FilterEvents(FilterNativeWindowExposedEvent, &data);
        if (!data.m_bFoundExposedEvent) {
            //如果队列中没有该窗口的绘制消息，则添加一个；但如果有的话，就不重复添加，避免重复绘制而影响性能
            SDL_Event sdlEvent;
            sdlEvent.type = SDL_EVENT_WINDOW_EXPOSED;
            sdlEvent.common.timestamp = 0;
            sdlEvent.window.data1 = 0;
            sdlEvent.window.data2 = 0;
            sdlEvent.window.windowID = SDL_GetWindowID(m_sdlWindow);
            bool nRet = SDL_PushEvent(&sdlEvent);
            ASSERT_UNUSED_VARIABLE(nRet);
        }
    }
#endif
}

void NativeWindow_SDL::GetClientRect(UiRect& rcClient) const
{
    rcClient.Clear();
    ASSERT(IsWindow());
    int nWidth = 0;
    int nHeight = 0;
    bool nRet = SDL_GetWindowSize(m_sdlWindow, &nWidth, &nHeight);
    ASSERT(nRet);
    if (nRet) {
        rcClient.left = 0;
        rcClient.top = 0;
        rcClient.right = rcClient.left + nWidth;
        rcClient.bottom = rcClient.top + nHeight;
    }
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    {
        HWND hWnd = GetHWND();        
        if (!::IsIconic(hWnd) && ::IsWindowVisible(hWnd)) {
            //最小化的时候，或者隐藏的时候，不比对，两者不同
            RECT rect = { 0, };
            ::GetClientRect(hWnd, &rect);
            ASSERT(rcClient.left == rect.left);
            ASSERT(rcClient.top == rect.top);
            ASSERT(rcClient.right == rect.right);
            ASSERT(rcClient.bottom == rect.bottom);
        }
    }
#endif
}

void NativeWindow_SDL::GetWindowRect(UiRect& rcWindow) const
{
    GetWindowRect(m_sdlWindow, rcWindow);
}

void NativeWindow_SDL::GetWindowRect(SDL_Window* sdlWindow, UiRect& rcWindow) const
{
    rcWindow.Clear();
    ASSERT(sdlWindow != nullptr);
    if (sdlWindow == nullptr) {
        return;
    }

    //窗口的左上角坐标值（屏幕坐标）
    int nXPos = 0;
    int nYPos = 0;
    SDL_GetWindowPosition(m_sdlWindow, &nXPos, &nYPos);

    //边框大小
    int nTopBorder = 0;
    int nLeftBorder = 0;
    int nBottomBorder = 0;
    int nRightBorder = 0;
    SDL_GetWindowBordersSize(m_sdlWindow, &nTopBorder, &nLeftBorder, &nBottomBorder, &nRightBorder);

    //客户区大小
    int nWidth = 0;
    int nHeight = 0;
    SDL_GetWindowSize(m_sdlWindow, &nWidth, &nHeight);

    rcWindow.left = nXPos - nLeftBorder;
    rcWindow.top = nYPos - nTopBorder;
    rcWindow.right = rcWindow.left + nWidth + nLeftBorder + nRightBorder;
    rcWindow.bottom = rcWindow.top + nHeight + nTopBorder + nBottomBorder;

#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    {
        HWND hWnd = GetHWND();
        if (!::IsIconic(hWnd) && ::IsWindowVisible(hWnd)) {
            //最小化的时候，或者隐藏的时候，不比对，两者不同
            RECT rect = { 0, };
            ::GetWindowRect(hWnd, &rect);
            if (rect.left != -32000) {
                ASSERT(rcWindow.left == rect.left);
                ASSERT(rcWindow.top == rect.top);
                ASSERT(rcWindow.right == rect.right);
                ASSERT(rcWindow.bottom == rect.bottom);
            }            
        }
    }
#endif
}

void NativeWindow_SDL::ScreenToClient(UiPoint& pt) const
{
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    POINT ptWnd = { pt.x, pt.y };
#endif
    int nXPos = 0;
    int nYPos = 0;
    bool nRet = SDL_GetWindowPosition(m_sdlWindow, &nXPos, &nYPos);
    ASSERT(nRet);
    if (nRet) {
        pt.x -= nXPos;
        pt.y -= nYPos;
    }
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    {
        HWND hWnd = GetHWND();
        ::ScreenToClient(hWnd, &ptWnd);
        ASSERT(ptWnd.x == pt.x);
        ASSERT(ptWnd.y == pt.y);
    }
#endif
}

void NativeWindow_SDL::ClientToScreen(UiPoint& pt) const
{
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    POINT ptWnd = { pt.x, pt.y };
#endif
    int nXPos = 0;
    int nYPos = 0;
    bool nRet = SDL_GetWindowPosition(m_sdlWindow, &nXPos, &nYPos);
    ASSERT(nRet);
    if (nRet) {
        pt.x += nXPos;
        pt.y += nYPos;
    }
#if defined (DUILIB_BUILD_FOR_WIN) && defined (_DEBUG)
    {
        HWND hWnd = GetHWND();
        ::ClientToScreen(hWnd, &ptWnd);
        ASSERT(ptWnd.x == pt.x);
        ASSERT(ptWnd.y == pt.y);
    }
#endif
}

void NativeWindow_SDL::GetCursorPos(UiPoint& pt) const
{
    float x = 0;
    float y = 0;
    SDL_GetGlobalMouseState(&x, &y);
    pt.x = (int32_t)x;
    pt.y = (int32_t)y;
}

bool NativeWindow_SDL::GetMonitorRect(UiRect& rcMonitor) const
{
    UiRect rcWork;
    return GetMonitorRect(m_sdlWindow, rcMonitor, rcWork);
}

bool NativeWindow_SDL::GetMonitorRect(SDL_Window* sdlWindow, UiRect& rcMonitor, UiRect& rcWork) const
{
    rcMonitor.Clear();
    rcWork.Clear();

    SDL_DisplayID displayID = 0;
    if (sdlWindow != nullptr) {
        displayID = SDL_GetDisplayForWindow(sdlWindow);
    }
    if (displayID == 0) {
        displayID = SDL_GetPrimaryDisplay();
    }
    ASSERT(displayID != 0);
    if (displayID == 0) {
        return false;
    }

    SDL_Rect rect = {0, };
    bool nRet = SDL_GetDisplayBounds(displayID, &rect);
    ASSERT(nRet);
    if (nRet) {
        rcMonitor.left = rect.x;
        rcMonitor.top = rect.y;
        rcMonitor.right = rcMonitor.left + rect.w;
        rcMonitor.bottom = rcMonitor.top + rect.h;
    }

    nRet = SDL_GetDisplayUsableBounds(displayID, &rect);
    ASSERT(nRet);
    if (nRet) {
        rcWork.left = rect.x;
        rcWork.top = rect.y;
        rcWork.right = rcWork.left + rect.w;
        rcWork.bottom = rcWork.top + rect.h;
    }
    return !rcMonitor.IsEmpty() && !rcWork.IsEmpty();
}

bool NativeWindow_SDL::GetMonitorWorkRect(UiRect& rcWork) const
{
    UiRect rcMonitor;
    return GetMonitorRect(m_sdlWindow, rcMonitor, rcWork);
}

bool NativeWindow_SDL::GetPrimaryMonitorWorkRect(UiRect& rcWork)
{
    rcWork.Clear();
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
            return false;
        }
    }

    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    ASSERT(displayID != 0);
    if (displayID == 0) {
        return false;
    }
    SDL_Rect rect = { 0, };
    bool nRet = SDL_GetDisplayUsableBounds(displayID, &rect);
    ASSERT(nRet);
    if (nRet) {
        rcWork.left = rect.x;
        rcWork.top = rect.y;
        rcWork.right = rcWork.left + rect.w;
        rcWork.bottom = rcWork.top + rect.h;
    }
    return !rcWork.IsEmpty();
}

bool NativeWindow_SDL::GetMonitorWorkRect(const UiPoint& pt, UiRect& rcWork) const
{
    rcWork.Clear();
    SDL_Point point = {pt.x, pt.y};
    SDL_DisplayID displayID = SDL_GetDisplayForPoint(&point);
    if (displayID == 0) {
        displayID = SDL_GetPrimaryDisplay();
    }
    ASSERT(displayID != 0);
    if (displayID == 0) {
        return false;
    }

    SDL_Rect rect = { 0, };
    bool nRet = SDL_GetDisplayUsableBounds(displayID, &rect);
    ASSERT(nRet);
    if (nRet) {
        rcWork.left = rect.x;
        rcWork.top = rect.y;
        rcWork.right = rcWork.left + rect.w;
        rcWork.bottom = rcWork.top + rect.h;
    }
    return !rcWork.IsEmpty();
}

const UiPoint& NativeWindow_SDL::GetLastMousePos() const
{
    return m_ptLastMousePos;
}

void NativeWindow_SDL::SetLastMousePos(const UiPoint& pt)
{
    m_ptLastMousePos = pt;
}

INativeWindow* NativeWindow_SDL::WindowBaseFromPoint(const UiPoint& pt)
{
    SDL_Window* pKeyboardFocus = SDL_GetKeyboardFocus();
    if (pKeyboardFocus != nullptr) {
        SDL_Window* sdlWindow = pKeyboardFocus;
        UiRect rcWindow;
        GetWindowRect(sdlWindow, rcWindow);
        if (rcWindow.ContainsPt(pt)) {
            NativeWindow_SDL* pWindow = GetWindowFromID(SDL_GetWindowID(sdlWindow));
            if (pWindow != nullptr) {
                return pWindow->m_pOwner;
            }
        }
    }
    SDL_Window* pMouseFocus = SDL_GetMouseFocus();
    if (pMouseFocus != nullptr) {
        SDL_Window* sdlWindow = pMouseFocus;
        UiRect rcWindow;
        GetWindowRect(sdlWindow, rcWindow);
        if (rcWindow.ContainsPt(pt)) {
            NativeWindow_SDL* pWindow = GetWindowFromID(SDL_GetWindowID(sdlWindow));
            if (pWindow != nullptr) {
                return pWindow->m_pOwner;
            }
        }
    }

    int nCount = 0;
    SDL_Window* const* pWindowList = SDL_GetWindows(&nCount);
    if ((nCount <= 0) || (pWindowList == nullptr)){
        return nullptr;
    }
    //TODO: 没考虑Z-Order的问题
    for (int nIndex = 0; nIndex < nCount; ++nIndex) {
        SDL_Window* sdlWindow = pWindowList[nIndex];
        if (sdlWindow != nullptr) {
            UiRect rcWindow;
            GetWindowRect(sdlWindow, rcWindow);
            if (rcWindow.ContainsPt(pt)) {
                NativeWindow_SDL* pWindow = GetWindowFromID(SDL_GetWindowID(sdlWindow));
                if (pWindow != nullptr) {
                    return pWindow->m_pOwner;
                }
            }
        }
    }
    return nullptr;
}

void NativeWindow_SDL::OnFinalMessage()
{
    if (m_pOwner) {
        m_pOwner->OnNativeFinalMessage();
    }
}

bool NativeWindow_SDL::SetWindowIcon(const FilePath& iconFilePath)
{
    std::vector<uint8_t> fileData;
    bool bRet = FileUtil::ReadFileData(iconFilePath, fileData);
    ASSERT(bRet);
    if (bRet) {
        bRet = SetWindowIcon(fileData, iconFilePath.ToString());
    }
    return bRet;
}

bool NativeWindow_SDL::SetWindowIcon(const std::vector<uint8_t>& iconFileData, const DString& iconFileName)
{
    ASSERT(!iconFileData.empty());
    if (iconFileData.empty()) {
        return false;
    }
    ASSERT(m_pOwner != nullptr);
    if (m_pOwner == nullptr) {
        return false;
    }
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    ImageLoadAttribute loadAttr = ImageLoadAttribute(DString(), DString(), false, false, 0);
    loadAttr.SetImageFullPath(iconFileName);
    ImageDecoder imageDecoder;
    std::vector<uint8_t> fileData(iconFileData);
    std::unique_ptr<ImageInfo> imageInfo = imageDecoder.LoadImageData(fileData, loadAttr, true, 100, m_pOwner->OnNativeGetDpi());
    ASSERT(imageInfo != nullptr);
    if (imageInfo == nullptr) {
        return false;
    }

    IBitmap* pBitmap = imageInfo->GetBitmap(0);
    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return false;
    }

    void* pPixelBits = pBitmap->LockPixelBits();
    ASSERT(pPixelBits != nullptr);
    if (pPixelBits == nullptr) {
        return false;
    }

    SDL_Surface* cursorSurface = SDL_CreateSurfaceFrom(pBitmap->GetWidth(), pBitmap->GetHeight(), SDL_PIXELFORMAT_BGRA32, pPixelBits, pBitmap->GetWidth() * sizeof(uint32_t));
    ASSERT(cursorSurface != nullptr);
    if (cursorSurface == nullptr) {
        return false;
    }

    bool nRet = SDL_SetWindowIcon(m_sdlWindow, cursorSurface);
    SDL_DestroySurface(cursorSurface);
    ASSERT(nRet);
    return nRet;
}

bool NativeWindow_SDL::SetLayeredWindow(bool bIsLayeredWindow, bool /*bRedraw*/)
{
    //不支持该功能
    ASSERT(bIsLayeredWindow == bIsLayeredWindow);
    //m_bIsLayeredWindow = bIsLayeredWindow;
    //SDL_WINDOW_TRANSPARENT 这个属性，不支持修改，所以此属性不支持修改，在创建窗口的时候已经设置正确的属性
    return true;
}

bool NativeWindow_SDL::KillWindowFocus()
{
    //不支持此功能
    return false;
}

bool NativeWindow_SDL::EnableWindow(bool /*bEnable*/)
{
    //未能提供此功能
    return true;
}

bool NativeWindow_SDL::IsWindowEnabled() const
{
    //未能提供此功能
    return true;
}

bool NativeWindow_SDL::SetWindowRoundRectRgn(const UiRect& /*rcWnd*/, const UiSize& /*szRoundCorner*/, bool /*bRedraw*/)
{
    //没有此功能
    return false;
}

void NativeWindow_SDL::ClearWindowRgn(bool /*bRedraw*/)
{
    //没有此功能
}

bool NativeWindow_SDL::UpdateWindow() const
{
    //没有此功能
    if (m_sdlWindow != nullptr) {
        SDL_SyncWindow(m_sdlWindow);
    }    
    return true;
}

void NativeWindow_SDL::KeepParentActive()
{
    //不支持此功能
}

int32_t NativeWindow_SDL::SetWindowHotKey(uint8_t /*wVirtualKeyCode*/, uint8_t /*wModifiers*/)
{
    //不支持此功能
    ASSERT(0);
    return -1;
}

bool NativeWindow_SDL::GetWindowHotKey(uint8_t& /*wVirtualKeyCode*/, uint8_t& /*wModifiers*/) const
{
    //不支持此功能
    ASSERT(0);
    return false;
}

bool NativeWindow_SDL::RegisterHotKey(uint8_t /*wVirtualKeyCode*/, uint8_t /*wModifiers*/, int32_t /*id*/)
{
    //不支持此功能
    ASSERT(0);
    return false;
}

bool NativeWindow_SDL::UnregisterHotKey(int32_t /*id*/)
{
    //不支持此功能
    ASSERT(0);
    return false;
}

LRESULT NativeWindow_SDL::CallDefaultWindowProc(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    //不支持此功能
    ASSERT(0);
    return -1;
}

void NativeWindow_SDL::SetEnableSnapLayoutMenu(bool /*bEnable*/)
{
    //不支持此功能
}

bool NativeWindow_SDL::IsEnableSnapLayoutMenu() const
{
    //不支持此功能
    return false;
}

void NativeWindow_SDL::SetEnableSysMenu(bool /*bEnable*/)
{
    //不支持此功能
}

bool NativeWindow_SDL::IsEnableSysMenu() const
{
    //不支持此功能
    return false;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL

//
//LRESULT NativeWindow_SDL::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
//{
//    LRESULT lResult = 0;
//    bHandled = false;
//    switch (uMsg)
//    {
//
//    case WM_IME_STARTCOMPOSITION://不支持
//    {
//        lResult = m_pOwner->OnNativeImeStartCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_IME_ENDCOMPOSITION://不支持
//    {
//        lResult = m_pOwner->OnNativeImeEndCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_SETCURSOR://不支持（需要处理）
//    {
//        if (LOWORD(lParam) == HTCLIENT) {
//            //只处理设置客户区的光标
//            lResult = m_pOwner->OnNativeSetCursorMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
//        }
//        break;
//    }
//    case WM_CONTEXTMENU://不支持
//    {
//        UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
//        if ((pt.x != -1) && (pt.y != -1)) {
//            ScreenToClient(pt);
//        }
//        lResult = m_pOwner->OnNativeContextMenuMsg(pt, NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_CHAR:
//    {
//        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
//        uint32_t modifierKey = 0;
//        GetModifiers(uMsg, wParam, lParam, modifierKey);
//        lResult = m_pOwner->OnNativeCharMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_HOTKEY://不支持
//    {
//        int32_t hotkeyId = (int32_t)wParam;
//        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>((int32_t)(int16_t)HIWORD(lParam));
//        uint32_t modifierKey = 0;
//        GetModifiers(uMsg, wParam, lParam, modifierKey);
//        lResult = m_pOwner->OnNativeHotKeyMsg(hotkeyId, vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    case WM_MOUSEHOVER://不支持
//    {
//        UiPoint pt;
//        pt.x = GET_X_LPARAM(lParam);
//        pt.y = GET_Y_LPARAM(lParam);
//        uint32_t modifierKey = 0;
//        GetModifiers(uMsg, wParam, lParam, modifierKey);
//        lResult = m_pOwner->OnNativeMouseHoverMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//
//    case WM_CAPTURECHANGED://不支持
//    {
//        lResult = m_pOwner->OnNativeCaptureChangedMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
//        break;
//    }
//    default:
//        break;
//    }//end of switch
//    return lResult;
//}


//SDL源码：
//if (style & WS_POPUP) {
//    window->flags |= SDL_WINDOW_BORDERLESS;
//}
//else {
//    window->flags &= ~SDL_WINDOW_BORDERLESS;
//}
//if (style & WS_THICKFRAME) {
//    window->flags |= SDL_WINDOW_RESIZABLE;
//}
//else {
//    window->flags &= ~SDL_WINDOW_RESIZABLE;
//}
