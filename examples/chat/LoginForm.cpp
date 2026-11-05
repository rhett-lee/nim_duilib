#include "LoginForm.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ThemeManager.h"
#include "duilib/Utils/ToastWnd.h"

namespace
{
    /** 登录窗口的背景图（深浅色主题各一张）
    */
    const DString::value_type* kLoginSceneLight = _T("file='glass_login_scene.svg'");
    const DString::value_type* kLoginSceneDark = _T("file='glass_login_scene_dark.svg'");
}

LoginForm::LoginForm() :
    m_loginCallback(nullptr)
{
}

LoginForm::~LoginForm()
{
}

DString LoginForm::GetSkinFolder()
{
    return _T("chat");
}

DString LoginForm::GetSkinFile()
{
    return _T("login.xml");
}

void LoginForm::OnInitWindow()
{
    //登录按钮
    ui::Button* pLoginBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_login")));
    if (pLoginBtn != nullptr) {
        pLoginBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            OnLoginClicked();
            return true;
            });
    }

    //注册链接：仅给出提示，不做真实注册
    ui::Button* pRegisterBtn = dynamic_cast<ui::Button*>(FindControl(_T("register_account")));
    if (pRegisterBtn != nullptr) {
        pRegisterBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::ToastWnd::Show(this, ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_REGISTER_TIP")),
                               ui::ToastWnd::kTypeInfo, 2500, ui::ToastWnd::kPosBottom);
            return true;
            });
    }

    //回车即登录
    ui::RichEdit2* pPassword = dynamic_cast<ui::RichEdit2*>(FindControl(_T("password")));
    if (pPassword != nullptr) {
        pPassword->AttachReturn([this](const ui::EventArgs& /*args*/) {
            OnLoginClicked();
            return true;
            });
    }

    //背景图跟随主题（启动时也应用一次，避免与上一次运行的主题不一致）
    UpdateBackgroundImage();
}

bool LoginForm::OnThemeChanged()
{
    const bool bRet = ui::WindowImplBase::OnThemeChanged();
    UpdateBackgroundImage();
    return bRet;
}

void LoginForm::UpdateBackgroundImage()
{
    ui::Box* pRoot = dynamic_cast<ui::Box*>(FindControl(_T("login_root")));
    if (pRoot == nullptr) {
        return;
    }
    //登录窗口的背景是一张整幅图片（不是半透明装饰），深色主题下必须换成深色版本，
    //否则浅色背景上叠加深色主题的白色文字会看不清
    const ui::ThemeInfo& themeInfo = ui::GlobalManager::Instance().Theme().GetCurrentColorThemeInfo();
    const bool bDark = (themeInfo.m_themeStyle == ui::ThemeStyle::kDark);
    pRoot->SetBkImage(bDark ? kLoginSceneDark : kLoginSceneLight);
}

void LoginForm::SetLoginCallback(const std::function<void(const DString&)>& callback)
{
    m_loginCallback = callback;
}

void LoginForm::OnLoginClicked()
{
    ui::RichEdit2* pUserName = dynamic_cast<ui::RichEdit2*>(FindControl(_T("username")));
    ui::RichEdit2* pPassword = dynamic_cast<ui::RichEdit2*>(FindControl(_T("password")));
    const DString strUser = (pUserName != nullptr) ? pUserName->GetText().c_str() : _T("");
    const DString strPassword = (pPassword != nullptr) ? pPassword->GetText().c_str() : _T("");

    if (strUser.empty() || strPassword.empty()) {
        ShowLoginError(ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_CHAT_LOGIN_EMPTY")));
        return;
    }

    //模拟登录成功：通知外部切换到主界面
    if (m_loginCallback) {
        m_loginCallback(strUser);
    }
}

void LoginForm::ShowLoginError(const DString& strText)
{
    ui::ToastWnd::Show(this, strText, ui::ToastWnd::kTypeWarning, 2500, ui::ToastWnd::kPosBottom);
}
