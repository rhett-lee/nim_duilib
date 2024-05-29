#ifndef EXAMPLES_RICHLIST_FORM_H_
#define EXAMPLES_RICHLIST_FORM_H_

// duilib
#include "duilib/duilib.h"

class RichlistForm : public ui::WindowImplBase
{
public:
    RichlistForm();
    ~RichlistForm();

    /**
     * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
     * GetSkinFolder        接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile            接口设置你要绘制的窗口的 xml 描述文件
     * GetWindowClassName    接口设置窗口唯一的类名称
     */
    virtual std::wstring GetSkinFolder() override;
    virtual std::wstring GetSkinFile() override;
    virtual std::wstring GetWindowClassName() const override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

    /** 窗口类名
    */
    static const std::wstring kClassName;

private:
    bool OnSelected(const ui::EventArgs& args);

private:
    ui::ListBox* m_pListBox;
};

#endif //EXAMPLES_RICHLIST_FORM_H_
