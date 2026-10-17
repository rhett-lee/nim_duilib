#ifndef EXAMPLES_STATUSBAR_FORM_H_
#define EXAMPLES_STATUSBAR_FORM_H_

// duilib
#include "duilib/duilib.h"

/** StatusBar 状态栏演示窗口
 *  演示状态栏的分隔线、右下角尺寸手柄，以及"一个事件源更新多个状态面板"的用法：
 *  状态面板（就绪/忙碌）、连接状态、光标位置、后台进度。
*/
class StatusBarForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    StatusBarForm();
    virtual ~StatusBarForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

private:
    /** 切换"状态"面板：就绪 <-> 忙碌
    */
    void ToggleState();

    /** 切换"连接状态"面板：已连接 <-> 未连接
    */
    void ToggleConnected();

    /** 推进"后台进度"面板的进度值（循环 0 -> 100）
    */
    void AdvanceProgress();

private:
    //当前是否为忙碌状态
    bool m_bBusy;

    //当前是否处于连接状态
    bool m_bConnected;

    //后台进度值（0 - 100）
    int32_t m_nProgress;

    //后台进度条控件接口
    ui::Progress* m_pProgress;
};

#endif //EXAMPLES_STATUSBAR_FORM_H_
