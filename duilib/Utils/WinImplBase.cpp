#include "WinImplBase.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Box.h"
#include "duilib/Control/Menu.h"
#include "duilib/Utils/FilePath.h"

// 窗口标题栏按钮：最大化、最小化、关闭、还原、全屏窗口的名字(旧的名称，保留以保持兼容性)
#define  DUI_CTR_CAPTION_BAR                     (_T("window_caption_bar"))
#define  DUI_CTR_BUTTON_CLOSE                    (_T("closebtn"))
#define  DUI_CTR_BUTTON_MIN                      (_T("minbtn"))
#define  DUI_CTR_BUTTON_MAX                      (_T("maxbtn"))
#define  DUI_CTR_BUTTON_RESTORE                  (_T("restorebtn"))
#define  DUI_CTR_BUTTON_FULLSCREEN               (_T("fullscreenbtn"))

namespace ui
{

WindowImplBase::WindowImplBase():
    m_pMaxButton(nullptr),
    m_pMinButton(nullptr),
    m_pRestoreButton(nullptr)
{
}

WindowImplBase::~WindowImplBase()
{
}

void WindowImplBase::PreInitWindow()
{
    BaseClass::PreInitWindow();

    //标题栏控件
    m_pTitleBar = GetWindowTitleBar();

    //关闭按钮
    Control* pControl = GetBtnWindowClose();
    if (pControl) {
        ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
        ControlPtr pButton(pControl);
        pControl->AttachClick([this, pButton](const ui::EventArgs& /*args*/) {
            if ((pButton != nullptr) && !IsUseSystemCaption()) {
                CloseWnd();
            }
            return true;
            });
    }

    //最小化按钮
    pControl = GetBtnWindowMin();
    if (pControl) {
        ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
        m_pMinButton = pControl;
        ControlPtr pButton(pControl);
        pControl->AttachClick([this, pButton](const ui::EventArgs& /*args*/) {
            if ((pButton != nullptr) && !IsUseSystemCaption()) {
                ShowWindow(kSW_MINIMIZE);
            }
            return true;
            });
    }

    //最大化按钮
    pControl = GetBtnWindowMax();
    if (pControl) {
        ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
        m_pMaxButton = pControl;
        ControlPtr pButton(pControl);
        pControl->AttachClick([this, pButton](const ui::EventArgs& /*args*/) {
            if ((pButton != nullptr) && !IsUseSystemCaption()) {    
                ShowWindow(kSW_SHOW_MAXIMIZED);
            }
            return true;
            });
    }

    //还原按钮
    pControl = GetBtnWindowRestore();
    if (pControl) {
        ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
        m_pRestoreButton = pControl;
        ControlPtr pButton(pControl);
        pControl->AttachClick([this, pButton](const ui::EventArgs& /*args*/) {
            if ((pButton != nullptr) && !IsUseSystemCaption()) {      
                ShowWindow(kSW_RESTORE);
            }
            return true;
            });
    }
        
    //全屏按钮
    pControl = GetBtnWindowFullscreen();
    if (pControl) {
        ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
        ControlPtr pButton(pControl);
        pControl->AttachClick([this, pButton](const ui::EventArgs& /*args*/) {
            if ((pButton != nullptr) && !IsUseSystemCaption()) {
                EnterFullscreen();
            }
            return true;
            });
    }

    //选择语言按钮
    pControl = GetBtnSelectLanguage();
    if (pControl) {
        ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
        ControlPtr pButton(pControl);
        pControl->AttachClick([this, pButton](const ui::EventArgs& /*args*/) {
            if ((pButton != nullptr) && !IsUseSystemCaption()) {
                OnSelectLanguage(pButton.get());
            }
            return true;
            });
    }

    //选择主题按钮
    pControl = GetBtnSelectTheme();
    if (pControl) {
        ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
        ControlPtr pButton(pControl);
        pControl->AttachClick([this, pButton](const ui::EventArgs& /*args*/) {
            if ((pButton != nullptr) && !IsUseSystemCaption()) {
                OnSelectTheme(pButton.get());
            }
            return true;
            });
    }

#if defined (DUILIB_BUILD_FOR_SDL) && !defined (DUILIB_BUILD_FOR_WIN)
    //标题栏: 由于使用SDL时，不支持双击标题栏最大化/还原窗口，所以自己实现此逻辑（非Windows平台）
    pControl = GetWindowTitleBar();
    if (pControl) {
        pControl->AttachBubbledEvent(ui::kEventMouseDoubleClick, UiBind(&WindowImplBase::OnTitleBarDoubleClick, this, std::placeholders::_1), 0);
    }
#endif
}

DString WindowImplBase::GetSkinFolder()
{
    return BaseClass::GetSkinFolder();
}

DString WindowImplBase::GetSkinFile()
{
    return BaseClass::GetSkinFile();
}

Control* WindowImplBase::CreateControl(const DString& strClass)
{
    return BaseClass::CreateControl(strClass);
}

void WindowImplBase::OnInitWindow()
{
    BaseClass::OnInitWindow();
}

void WindowImplBase::OnInitLayout()
{
    BaseClass::OnInitLayout();
}

void WindowImplBase::OnPreCloseWindow()
{
    BaseClass::OnPreCloseWindow();
}

void WindowImplBase::OnCloseWindow()
{
    BaseClass::OnCloseWindow();
}

void WindowImplBase::OnFinalMessage()
{
    BaseClass::OnFinalMessage();
}

bool WindowImplBase::OnTitleBarDoubleClick(const EventArgs& /*param*/)
{
    Control* pControl = GetBtnWindowMax();
    if ((pControl != nullptr) && pControl->IsVisible()) {
        //最大化按钮
        if (!IsWindowMaximized()) {
            ShowWindow(kSW_SHOW_MAXIMIZED);
        }
    }
    else {
        //还原按钮
        pControl = GetBtnWindowRestore();
        if ((pControl != nullptr) && pControl->IsVisible()) {
            if (IsWindowMaximized()) {
                ShowWindow(kSW_RESTORE);
            }            
        }
    }
    return true;
}

void WindowImplBase::OnSelectLanguage(Control* pBtnSelectLanguage)
{
    ASSERT(pBtnSelectLanguage != nullptr);
    if (pBtnSelectLanguage == nullptr) {
        return;
    }
    ui::UiRect rect = pBtnSelectLanguage->GetPos();
    ui::UiPoint point;
    point.x = rect.left;
    point.y = rect.bottom;
    ClientToScreen(point);

    //显示选择语言菜单
    ui::Menu* menu = new ui::Menu(this);//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    //设置菜单xml所在的目录
    menu->SetSkinFolder(DString(DUILIB_PUBLIC_RES_DIR) + _T("/menu/"));
    DString xml(_T("lang_menu.xml"));
    menu->ShowMenu(xml, point);

    //当前语言文件
    DString currentLangFileName = GlobalManager::Instance().GetLanguageFileName();

    //可用语言文件列表和显示名称
    std::vector<std::pair<DString, DString>> languageList;
    GlobalManager::Instance().GetLanguageList(languageList);
    if (languageList.empty()) {
        languageList.push_back({ currentLangFileName , _T("") });
    }

    //动态添加菜单项
    for (auto& lang : languageList) {
        const DString fileName = lang.first;
        DString& displayName = lang.second;

        MenuItem* pMenuItem = new MenuItem(menu);
        pMenuItem->SetClass(_T("menu_element"));
        CheckBox* pCheckBox = new CheckBox(menu);
        pCheckBox->SetClass(_T("menu_language_select_class"));
        pCheckBox->SetText(!displayName.empty() ? displayName : fileName);
        pMenuItem->AddItem(pCheckBox);
        menu->AddMenuItem(pMenuItem);

        if (ui::StringUtil::IsEqualNoCase(fileName, currentLangFileName)) {
            pCheckBox->Selected(true);
        }

        //挂载选择语言事件
        pMenuItem->AttachClick([fileName](const EventArgs& /*args*/) {
            //切换语言
            ui::GlobalManager& globalManager = ui::GlobalManager::Instance();
            if (globalManager.GetLanguageFileName() != fileName) {
                globalManager.ReloadLanguage(ui::FilePath(), fileName, true);
            }
            return true;
            });
    }
}

void WindowImplBase::OnSelectTheme(Control* pBtnSelectTheme)
{
    ASSERT(pBtnSelectTheme != nullptr);
    if (pBtnSelectTheme == nullptr) {
        return;
    }
    ui::UiRect rect = pBtnSelectTheme->GetPos();
    ui::UiPoint point;
    point.x = rect.left;
    point.y = rect.bottom;
    ClientToScreen(point);

    //显示选择主题菜单
    ui::Menu* menu = new ui::Menu(this);//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    //设置菜单xml所在的目录
    menu->SetSkinFolder(DString(DUILIB_PUBLIC_RES_DIR) + _T("/menu/"));
    DString xml(_T("theme_menu.xml"));
    menu->ShowMenu(xml, point);

    //可选的主题列表
    std::vector<FilePath> themePathList; //支持的主题列表
    std::vector<ThemeInfo> themeInfoList;
    ThemeManager& theme = GlobalManager::Instance().Theme();
    theme.GetAllThemes(themePathList, themeInfoList);

    //动态添加菜单项
    for (const auto& themeInfo : themeInfoList) {
        MenuItem* pMenuItem = new MenuItem(menu);
        pMenuItem->SetClass(_T("menu_element"));
        CheckBox* pCheckBox = new CheckBox(menu);
        pCheckBox->SetClass(_T("menu_theme_select_class"));
        pCheckBox->SetText(themeInfo.m_themeName);
        pMenuItem->AddItem(pCheckBox);
        menu->AddMenuItem(pMenuItem);

        if (themeInfo.m_bSelectedTheme) {
            pCheckBox->Selected(true);
        }
        //挂载选择主题事件
        pMenuItem->AttachClick([themeInfo](const EventArgs& /*args*/) {
            //切换主题
            GlobalManager& globalManager = ui::GlobalManager::Instance();
            globalManager.Theme().SwitchColorTheme(themeInfo.m_themePath);
            return true;
            });
    }
}

LRESULT WindowImplBase::OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled)
{
    std::weak_ptr<WeakFlag> windowFlag = GetWeakFlag();
    LRESULT lResult = BaseClass::OnSizeMsg(sizeType, newWindowSize, nativeMsg, bHandled);
    if (windowFlag.expired()) {
        return lResult;
    }
    if (sizeType == WindowSizeType::kSIZE_MAXIMIZED) {
        //窗口最大化
        if (!IsWindowFullscreen()) {
            OnWindowMaximized();
        }
    }
    else if (sizeType == WindowSizeType::kSIZE_RESTORED) {
        //窗口还原
        if (!IsWindowFullscreen()) {
            OnWindowRestored();
        }
    }
    else if (sizeType == WindowSizeType::kSIZE_MINIMIZED) {
        //窗口最小化
        if (!IsWindowFullscreen()) {
            OnWindowMinimized();
        }
    }
    return lResult;
}

void WindowImplBase::OnWindowEnterFullscreen()
{
    if (!IsUseSystemCaption()) {
        Control* pTitleBar = GetWindowTitleBar();
        if (pTitleBar != nullptr) {
            pTitleBar->SetVisible(false);
        }
    }
}

void WindowImplBase::OnWindowExitFullscreen()
{
    if (!IsUseSystemCaption()) {
        Control* pTitleBar = GetWindowTitleBar();
        if (pTitleBar != nullptr) {
            pTitleBar->SetVisible(true);
        }
        ProcessMaxRestoreStatus();
    }
}

void WindowImplBase::OnWindowMaximized()
{
    ProcessMaxRestoreStatus();
}

void WindowImplBase::OnWindowRestored()
{
    ProcessMaxRestoreStatus();
}

void WindowImplBase::OnWindowMinimized()
{
}

void WindowImplBase::OnUseSystemCaptionBarChanged()
{
    BaseClass::OnUseSystemCaptionBarChanged();
    if (GetRoot() == nullptr) {
        return;
    }
    Control* pTitleBar = GetWindowTitleBar();
    if (pTitleBar != nullptr) {
        pTitleBar->SetVisible(!IsUseSystemCaption());
        if (!IsUseSystemCaption()) {
            ProcessMaxRestoreStatus();
        }
    }
}

void WindowImplBase::OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)
{
    BaseClass::OnWindowDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);
}

void WindowImplBase::ProcessMaxRestoreStatus()
{
    if (IsUseSystemCaption() || (GetRoot() == nullptr)) {
        return;
    }
    Control* pMaxButton = GetBtnWindowMax();
    Control* pRestoreButton = GetBtnWindowRestore();
    bool bWindowMax = IsWindowMaximized();
    if (pMaxButton != nullptr) {
        pMaxButton->SetFadeVisible(bWindowMax ? false : true);
    }
    if (pRestoreButton != nullptr) {
        pRestoreButton->SetFadeVisible(bWindowMax ? true : false);
    }
}

bool WindowImplBase::HasMinMaxBox(bool& bMinimizeBox, bool& bMaximizeBox) const
{
    bMinimizeBox = false;
    bMaximizeBox = false;
    if (m_pMinButton != nullptr) {
        bMinimizeBox = true;
    }
    if (m_pMaxButton != nullptr) {
        bMaximizeBox = true;
    }
    return true;
}

bool WindowImplBase::IsPtInMaximizeRestoreButton(const UiPoint& pt) const
{
    bool bInButton = false;
    if ((m_pMaxButton != nullptr) && m_pMaxButton->IsVisible()) {
        bInButton = m_pMaxButton->GetRect().ContainsPt(pt);
    }
    else if ((m_pRestoreButton != nullptr) && m_pRestoreButton->IsVisible()) {
        bInButton = m_pRestoreButton->GetRect().ContainsPt(pt);
    }
    return bInButton;
}

Control* WindowImplBase::GetBtnWindowByName(const DString& newCtrlName, const DString& oldCtrlName) const
{
    Control* pControl = nullptr;
    if (m_pTitleBar != nullptr) {
        //如果有标题栏，则在标题栏范围内查找
        pControl = m_pTitleBar->FindControl(newCtrlName);
        if ((pControl == nullptr) && !oldCtrlName.empty()) {
            pControl = m_pTitleBar->FindControl(oldCtrlName);
        }
    }
    else {
        pControl = FindControl(newCtrlName);
        if ((pControl == nullptr) && !oldCtrlName.empty()) {
            pControl = FindControl(oldCtrlName);
        }
    }
    return pControl;
}

Control* WindowImplBase::GetWindowTitleBar() const
{
    Control* pControl = FindControl(DUI_CTR_WINDOW_TITLE_BAR);
    if ((pControl == nullptr)) {
        pControl = FindControl(DUI_CTR_CAPTION_BAR);
    }
    return pControl;
}

Control* WindowImplBase::GetBtnWindowMax() const
{
    return GetBtnWindowByName(DUI_CTR_WINDOW_BUTTON_MAX, DUI_CTR_BUTTON_MAX);
}

Control* WindowImplBase::GetBtnWindowRestore() const
{
    return GetBtnWindowByName(DUI_CTR_WINDOW_BUTTON_RESTORE, DUI_CTR_BUTTON_RESTORE);
}

Control* WindowImplBase::GetBtnWindowMin() const
{
    return GetBtnWindowByName(DUI_CTR_WINDOW_BUTTON_MIN, DUI_CTR_BUTTON_MIN);
}

Control* WindowImplBase::GetBtnWindowClose() const
{
    return GetBtnWindowByName(DUI_CTR_WINDOW_BUTTON_CLOSE, DUI_CTR_BUTTON_CLOSE);
}

Control* WindowImplBase::GetBtnWindowFullscreen() const
{
    return GetBtnWindowByName(DUI_CTR_WINDOW_BUTTON_FULLSCREEN, DUI_CTR_BUTTON_FULLSCREEN);
}

Control* WindowImplBase::GetBtnSelectLanguage() const
{
    return GetBtnWindowByName(DUI_CTR_WINDOW_BUTTON_LANGUAGE, DString());
}

Control* WindowImplBase::GetBtnSelectTheme() const
{
    return GetBtnWindowByName(DUI_CTR_WINDOW_BUTTON_THEME, DString());
}

} // namespace ui
