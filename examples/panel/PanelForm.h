#ifndef EXAMPLES_PANEL_FORM_H_
#define EXAMPLES_PANEL_FORM_H_

// duilib
#include "duilib/duilib.h"
#include "duilib/Box/Panel.h"

/** Panel 控件功能测试窗口
*/
class PanelForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    PanelForm();
    virtual ~PanelForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 语言切换后由框架调用：带 text_id/title_id 的控件会自动刷新；
    *   这里只需重设 C++ 控制面板被“修改标题”按钮改写过的动态标题（含 %d 计数）
    */
    virtual bool OnLanguageChanged() override;

private:
    /** 更新状态栏文字
    */
    void SetStatusText(const DString& strText);

private:
    //C++ 控制的可折叠面板
    ui::PanelVBox* m_pCppPanel;

    //状态栏文本
    ui::Label* m_pStatusLabel;

    //可见性保持测试：用户控制显隐的子标签
    ui::Label* m_pHiddenChildLabel;

    //C++ 控制面板标题被“修改标题”按钮修改的次数（0 表示默认标题）
    int32_t m_nCppTitleModified;
};

#endif //EXAMPLES_PANEL_FORM_H_
