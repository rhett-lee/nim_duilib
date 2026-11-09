#ifndef EXAMPLES_CONTROLS_FORM_H_
#define EXAMPLES_CONTROLS_FORM_H_

// duilib
#include "duilib/duilib.h"

class ControlForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    ControlForm();
    virtual ~ControlForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口已经被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

private:
    /** 以非模态方式打开子窗口
    * @param [in] pForm 子窗口对象，由框架接管生命周期
    */
    void ShowChildForm(ui::WindowImplBase* pForm);

    /** 显示菜单
    * @param [in] point 显示位置坐标，屏幕坐标
    */
    void ShowPopupMenu(const ui::UiPoint& point, ui::Control* pRelatedControl);

    /** 注册编辑框的拖放事件
    */
    void AttachRichEditEvents(ui::Control* edit);

    /** 动态更新进度条接口
    */
    void OnProgressValueChagned(float value);

    /** 显示拾色器窗口
    * @param [in] bDoModal 是否使用模态对话框
    */
    void ShowColorPicker(bool bDoModal);

    /** 显示模态对话框
    */
    void ShowDoModalDlg();

private:
    /** 快捷键消息（WM_HOTKEY）
    */
    virtual LRESULT OnHotKeyMsg(int32_t hotkeyId, ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

private:
    /** 显示/隐藏托盘图标
    */
    void ShowTrayIcon(bool bShow);

    /** 托盘图标消息回调函数
     */
    void OnTrayIconMessage(ui::TrayIconMessageType msgType, int32_t x, int32_t y);

    /** 显示托盘图标的菜单
    */
    void ShowTrayMenu(int32_t x, int32_t y);

private:
    /** 托盘图标的接口
    */
    std::unique_ptr<ui::TrayIcon> m_pTrayIcon;
};

#endif //EXAMPLES_CONTROLS_FORM_H_
