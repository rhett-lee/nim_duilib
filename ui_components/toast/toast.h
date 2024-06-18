#ifndef UI_COMPONENTS_TOAST_H_
#define UI_COMPONENTS_TOAST_H_

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Button.h"

namespace nim_comp {
/** @class Toast
 * @brief 提供一个简易的提示窗体，相当于MessageBox的简化版
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2016/8/24
 */
class Toast : public ui::WindowImplBase
{
public:
    /**
     * 在屏幕中间显示一个提示框
     * @param[in] content 提示内容
     * @param[in] duration 持续时间(毫秒),0代表一直显示
     * @param[in] pParentWindow 父窗口，如果提供的话toast会显示在父窗口的中心
     * @return void 无返回值
     */
    static void ShowToast(const DString& content, int duration = 0, Window* pParentWindow = nullptr);
public:
    Toast(){};
    virtual ~Toast() override = default;

    //覆盖虚函数
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual DString GetWindowId() const /*override*/;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /**
     * 拦截并处理底层窗体消息
     * @param[in] uMsg 消息类型
     * @param[in] wParam 附加参数
     * @param[in] lParam 附加参数
     * @return LRESULT 处理结果
     */
    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    /**
     * 设置窗体的显示时长
     * @param[in] duration 持续时间，单位毫秒
     * @return void    无返回值
     */
    void SetDuration(int duration);

private:
    /**
     * 处理所有控件单击消息
     * @param[in] msg 消息的相关信息
     * @return bool true 继续传递控件消息，false 停止传递控件消息
     */
    bool OnClicked(const ui::EventArgs& msg);

    /**
     * 设置提示内容
     * @param[in] str 提示内容
     * @return void    无返回值
     */
    void SetContent(const DString& str);

    public:
        static const DString kClassName;
    private:
        ui::RichEdit* m_content = nullptr;
        ui::Button* m_close_button = nullptr;
    };
}

#endif //UI_COMPONENTS_TOAST_H_
