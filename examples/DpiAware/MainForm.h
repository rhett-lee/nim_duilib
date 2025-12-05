//MainForm.h
#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

/** 应用程序的主窗口实现
*/
class MainForm : public ui::WindowImplBase
{
public:
    MainForm();
    virtual ~MainForm() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录
    */
    virtual DString GetSkinFolder() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 窗口的DPI缩放比发生变化，更新控件大小和布局(供子类使用)
    * @param [in] nOldScaleFactor 旧的DPI缩放百分比
    * @param [in] nNewScaleFactor 新的DPI缩放百分比，与Dpi().GetDisplayScaleFactor()的值一致，该值可能与nOldScaleFactor相同
    */
    virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor) override;

protected:
    /** 窗口大小发生改变(WM_SIZE)
    * @param [in] sizeType 触发窗口大小改变的类型
    * @param [in] newWindowSize 新的窗口大小（宽度和高度）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnSizeMsg(ui::WindowSizeType sizeType, const ui::UiSize& newWindowSize, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

private:
    /** 刷新界面
    */
    void UpdateUI();
};

#endif //EXAMPLES_MAIN_FORM_H_
