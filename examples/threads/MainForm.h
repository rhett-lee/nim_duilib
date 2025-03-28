#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"
#include <chrono>

class MainThread;
class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    explicit MainForm(MainThread* pMainThread);
    virtual ~MainForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

public:
    /** 更新UI状态(可以在子线程中调用)
    */
    void UpdateUI();

    /** 输出日志(可以在子线程中调用)
    */
    void PrintLog(const DString& log);

private:
    /** 在子线程中执行任务
    * @param [in] nThreadIdentifier 线程标识符
    */
    bool RunTaskInThread(int32_t nThreadIdentifier);

    /** 在子线程中执行具体的任务
    */
    void ExecuteTaskInThread();

    /** 更新运行时间
    */
    void UpdateRunningTime();

private:
    /** 日志显示控件
    */
    ui::RichEdit* m_pLogEdit;

    /** 运行时间显示
    */
    ui::Label* m_pRunningTimeLabel;

    /** 开始时间
    */
    std::chrono::steady_clock::time_point m_startTime;

    /** 日志序号
    */
    int32_t m_nLogLineNumber;

    /** 线程管理接口
    */
    MainThread* m_pMainThread;
};

#endif //EXAMPLES_MAIN_FORM_H_
