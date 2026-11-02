#ifndef UI_CONTROL_COLORPICKER_H_
#define UI_CONTROL_COLORPICKER_H_

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Control/Label.h"
#include "duilib/Core/ControlPtrT.h"

namespace ui
{

/** 拾色器，独立窗口
*/
class Control;
class ColorPickerRegular;
class ColorPickerStandard;
class ColorPickerStandardGray;
class ColorPickerCustom;
class DUILIB_API ColorPicker : public WindowImplBase
{
    typedef WindowImplBase BaseClass;
public:
    ColorPicker();
    virtual ~ColorPicker() override;

    /** 以下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
     * GetSkinFolder        接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile            接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

public:
    /** 设置所选颜色
    * @param[in] color 需要显示的颜色
    * 说明：该接口既可以在窗口创建之前调用，也可以在窗口创建之后调用；
    *       在窗口创建之前调用时，颜色值会暂存，在窗口创建完成(OnInitWindow)后再应用。
    */
    void SetSelectedColor(const UiColor& color);

    /** 获取窗口关闭时最终选择的颜色
    */
    UiColor GetSelectedColor() const;

    /** 监听选择颜色的事件
    * @param[in] callback 选择颜色变化时的回调函数
    *            参数说明:
                        wParam: 当前新选择的颜色值，可以用UiColor((uint32_t)wParam)生成颜色
                        lParam: 原来旧选择的颜色值，可以用UiColor((uint32_t)lParam)生成颜色
    */
    void AttachSelectColor(const EventCallback& callback);

    /** 监听窗口关闭事件
     * @param[in] callback 指定关闭后的回调函数, 在这个回调中，可以调用GetSelectedColor()函数获取选择的颜色值
                           参数的wParam代表窗口关闭的触发情况：
     *                     0 - 表示 "确认" 关闭本窗口
                           1 - 表示点击窗口的 "关闭" 按钮关闭本窗口(默认值)
                           2 - 表示 "取消" 关闭本窗口
     */
    void AttachWindowClose(const EventCallback& callback);

    /** 设置是否在窗口创建时自动调整位置，避免完全遮挡父窗口
    * @param[in] bAdjust 是否启用"不遮挡父窗口"的贴边定位：
    *             true  - 窗口创建后会自动贴到父窗口的右侧或左侧显示，避免遮挡父窗口内容；
    *             false - 默认值，沿用原有逻辑（按创建参数决定是否居中显示），不改变窗口定位。
    *   说明：默认关闭，避免影响 ColorPicker 的其他调用方（如 PropertyGrid 颜色属性）；
    *        仅示例程序 ColorPickerDemo 显式开启本开关。
    */
    void SetAdjustWindowPos(bool bAdjust);

protected:
    /** 当要创建的控件不是标准的控件名称时会调用该函数
    * @param [in] strClass 控件名称
    * @return 返回一个自定义控件指针，一般情况下根据 strClass 参数创建自定义的控件
    */
    virtual Control* CreateControl(const DString& strClass) override;

private:
    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 窗口关闭消息（WM_CLOSE）
    * @param [in] wParam 消息的wParam参数
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 键盘按下消息（WM_KEYDOWN）
    *   用于支持对话框的常用键盘操作：Esc 取消、Enter 确认
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键状态键(Shift, Ctrl, Alt等，不包括左右键)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 内部选择了新颜色
    */
    void OnSelectColor(const UiColor& newColor);

    /** 调整窗口位置，避免完全遮挡父窗口
    *   窗口创建时默认在父窗口上居中显示，会盖住父窗口的内容；
    *   这里改为优先贴在父窗口的右侧或左侧显示，两侧都放不下时贴到工作区边缘，
    *   父窗口不存在时保持原有位置不变。
    */
    void AdjustWindowPos();

    /** 把颜色应用到"新颜色"和"原颜色"两个预览控件，以及各个选色控件
    * @param[in] color 需要显示的颜色
    */
    void ApplyColor(const UiColor& color);

    /** 设置颜色预览控件(Label)的背景色与文字颜色
    * @param[in] pLabel 需要设置的控件
    * @param[in] color 需要显示的颜色
    */
    static void SetColorLabel(Label* pLabel, const UiColor& color);

    /** 根据背景色计算对比度最佳的前景文本颜色（黑或白）
    *   使用 YIQ 公式: Y = 0.299R + 0.587G + 0.114B
    *   Y >= 128 时选择黑色文本，否则选择白色文本
    */
    static UiColor GetContrastTextColor(const UiColor& bkColor);

    /** 屏幕取色
    */
    void OnPickColorFromScreen();

    /** 复制颜色值到剪贴板，并弹出 Toast 提示作为反馈
    * @param[in] colorValue 需要复制的颜色文本（如 "#AARRGGBB"）；为空时不做任何操作
    *   说明：Toast 为非模态提示，不抢占父窗口焦点，约 1.5 秒后自动消失；
    *        文本由"已复制"多语言文案与颜色值拼接而成。
    */
    void CopyColorToClipboard(const DString& colorValue);

private:
    /** 新选择的颜色控件接口
    */
    ControlPtrT<Label> m_pNewColor;

    /** 原来的颜色控件接口
    */
    ControlPtrT<Label> m_pOldColor;

    /** 常用颜色控件接口
    */
    ColorPickerRegular* m_pRegularPicker;

    /** 标准颜色控件接口
    */
    ColorPickerStandard* m_pStandardPicker;

    /** 标准颜色控件接口(灰色)
    */
    ColorPickerStandardGray* m_pStandardGrayPicker;

    /** 自定义颜色控件接口
    */
    ColorPickerCustom* m_pCustomPicker;

    /** 选择的颜色
    */
    UiColor m_selectedColor;

    /** 窗口创建之前，通过SetSelectedColor接口设置的颜色值(暂存，OnInitWindow时应用)
    */
    UiColor m_initColor;

    /** m_initColor是否有效
    */
    bool m_bInitColorValid;

    /** 选择颜色的监听事件
    */
    EventCallback m_colorCallback;

    /** 是否启用"不遮挡父窗口"的贴边定位（默认false，沿用原有逻辑）
    */
    bool m_bAdjustWindowPos;
};

} // namespace ui

#endif //UI_CONTROL_COLORPICKER_H_
