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

private:

    /** 显示菜单
    * @param [in] point 显示位置坐标，屏幕坐标
    */
    void ShowPopupMenu(const ui::UiPoint& point, ui::Control* pRelatedControl);

    /** 注册编辑框的事件
    */
    void AttachRichEditEvents(ui::RichEdit* edit);

    /**
     * 被投递到杂事线程读取 xml 数据的任务函数
     */
    void LoadRichEditData();

    /**
     * 用于在杂事线程读取 xml 完成后更新 UI 内容的接口
     */
    void OnResourceFileLoaded(const DString& xml);

    /**
     * 动态更新进度条接口
     */
    void OnProgressValueChagned(float value);

    /** 显示拾色器窗口
    * @param [in] bDoModal 是否使用模态对话框
    */
    void ShowColorPicker(bool bDoModal);

    /** 显示模态对话框
    */
    void ShowDoModalDlg();

    /** 显示控件测试窗口
    */
    void ShowTestWindow();

    /** 显示控件测试窗口
    */
    void ShowAnimationWindow();

private:
    /** 快捷键消息（WM_HOTKEY）
    * @param [in] hotkeyId 热键的ID
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kAlt, ModifierKey::kControl, ModifierKey::kShift, ModifierKey::kWin
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnHotKeyMsg(int32_t hotkeyId, ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;
};

#endif //EXAMPLES_CONTROLS_FORM_H_
