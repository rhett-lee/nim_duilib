#ifndef EXAMPLES_CHAT_LOGINFORM_H_
#define EXAMPLES_CHAT_LOGINFORM_H_

// duilib
#include "duilib/duilib.h"

#include <functional>

/** 登录窗口
 *
 *  点击"登录"模拟登录成功，随后切换到聊天主界面（不退出程序）；
 *  未登录时可直接关闭本窗口退出程序。
*/
class LoginForm : public ui::WindowImplBase
{
public:
    LoginForm();
    virtual ~LoginForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 窗口创建完成后初始化
    */
    virtual void OnInitWindow() override;

    /** 主题切换：背景图需要跟着深浅色主题更换（否则深色主题下文字看不清）
    */
    virtual bool OnThemeChanged() override;

public:
    /** 登录成功回调（参数为登录的用户名）
    */
    void SetLoginCallback(const std::function<void(const DString&)>& callback);

private:
    /** 按当前主题（深色/浅色）切换登录窗口的背景图
    */
    void UpdateBackgroundImage();

private:
    /** 处理登录按钮点击
    */
    void OnLoginClicked();

    /** 显示登录错误提示（账号或密码为空）
    */
    void ShowLoginError(const DString& strText);

private:
    std::function<void(const DString&)> m_loginCallback;
};

#endif // EXAMPLES_CHAT_LOGINFORM_H_
