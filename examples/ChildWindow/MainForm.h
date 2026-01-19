#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"
#include "ChildWindowPaintScheduler.h"

class MyChildWindowEvents;

class MainForm : public ui::WindowImplBase, public ChildWindowPaintScheduler
{
    typedef ui::WindowImplBase BaseClass;
public:
    MainForm();
    virtual ~MainForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

public:
    /** 立即绘制该子窗口
    * @param [in] pChildWindow 子窗口的指针
    */
    virtual bool PaintChildWindow(ui::ChildWindow* pChildWindow) override;

    /** 立即绘制该子窗口的下一个子窗口（按顺序依次绘制每个子窗口）
    * @param [in] pChildWindow 子窗口的指针
    */
    virtual bool PaintNextChildWindow(ui::ChildWindow* pChildWindow) override;

protected:
    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnPreCloseWindow() override;

    /** 窗口的层窗口属性发生变化
    */
    virtual void OnLayeredWindowChanged() override;

private:
    /** 创建子窗口，并保存关联关系
    */
    void CreateChildWindows();

    /** 关闭子窗口，并取消关联关系
    */
    void CloseChildWindows();

private:
    /** 子窗口关联的事件
    */
    std::vector<MyChildWindowEvents*> m_childWindowEvents;
};

#endif //EXAMPLES_MAIN_FORM_H_
