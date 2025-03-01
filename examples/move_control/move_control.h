#ifndef EXAMPLES_MOVE_CONTROL_H_
#define EXAMPLES_MOVE_CONTROL_H_

#include "app_db.h"

class MoveControlForm : public ui::WindowImplBase
{
public:
    MoveControlForm(const DString& theme_directory, const DString& layout_xml);
    virtual ~MoveControlForm() override;

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
    DString m_theme_directory;
    DString m_layout_xml;

    ui::Box* m_frequent_app=nullptr;
    ui::Box* m_my_app = nullptr;
};
#endif //EXAMPLES_MOVE_CONTROL_H_
