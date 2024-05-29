#ifndef EXAMPLES_MOVE_CONTROL_H_
#define EXAMPLES_MOVE_CONTROL_H_

#include "app_db.h"

class MoveControlForm : public ui::WindowImplBase
{
public:
    MoveControlForm(const DString& class_name, const DString& theme_directory, const DString& layout_xml);
    ~MoveControlForm();

    /**
     * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
     * GetSkinFolder        接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile            接口设置你要绘制的窗口的 xml 描述文件
     * GetWindowClassName    接口设置窗口唯一的类名称
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual DString GetWindowClassName() const override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

private:
    DString m_class_name;
    DString m_theme_directory;
    DString m_layout_xml;

    ui::Box* m_frequent_app=nullptr;
    ui::Box* m_my_app = nullptr;
};
#endif //EXAMPLES_MOVE_CONTROL_H_
