#ifndef UI_CONTROL_COLORPICKER_H_
#define UI_CONTROL_COLORPICKER_H_

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Control/Label.h"

namespace ui
{

/** 拾色器，独立窗口
*/
class Control;
class ColorPickerRegular;
class ColorPickerStatard;
class ColorPickerStatardGray;
class ColorPickerCustom;
class ColorPicker : public WindowImplBase
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

    /** 内部选择了新颜色
    */
    void OnSelectColor(const UiColor& newColor);

    /** 屏幕取色
    */
    void OnPickColorFromScreen();

private:
    /** 新选择的颜色控件接口
    */
    Label* m_pNewColor;

    /** 原来的颜色控件接口
    */
    Label* m_pOldColor;

    /** 常用颜色控件接口
    */
    ColorPickerRegular* m_pRegularPicker;

    /** 标准颜色控件接口
    */
    ColorPickerStatard* m_pStatardPicker;

    /** 标准颜色控件接口(灰色)
    */
    ColorPickerStatardGray* m_pStatardGrayPicker;

    /** 自定义颜色控件接口
    */
    ColorPickerCustom* m_pCustomPicker;

    /** 选择的颜色
    */
    UiColor m_selectedColor;

    /** 选择颜色的监听事件
    */
    EventCallback m_colorCallback;
};

} // namespace ui

#endif //UI_CONTROL_COLORPICKER_H_
