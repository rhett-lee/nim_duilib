#ifndef EXAMPLES_LISTBOX_FORM_H_
#define EXAMPLES_LISTBOX_FORM_H_

// duilib
#include "duilib/duilib.h"

class ListBoxForm : public ui::WindowImplBase
{
public:
    ListBoxForm();
    virtual ~ListBoxForm() override;

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
    /** 测试列表的事件
    */
    void TestListBoxEvents(ui::ListBox* pListBox);

    /** 获取消息的基本信息（用于显示日志）
    */
    DString GetEventDisplayInfo(const ui::EventArgs& args, ui::ListBox* pListBox);

    /** 输出测试日志
    */
    void OutputDebugLog(const DString& logMsg);

private:
    ui::ListBox* m_pListBox;
};

#endif //EXAMPLES_LISTBOX_FORM_H_
