//MainForm.h
#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

/** 应用程序的主窗口实现
*/
class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
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

private:
    /** 启动自动刷新的定时器
    */
    void StartRefreshTimer(int32_t nIntervalSeconds);

    /** 停止自动刷新的定时器
    */
    void StopRefreshTimer();

    /** 检查并刷新XML文件的预览结果
    */
    void CheckXmlPreview();

private:
    /** 当前预览的XML文件路径
    */
    ui::FilePath m_xmlFilePath;

    /** XML文件内容，用于比较是否有变化
    */
    std::vector<uint8_t> m_xmlFileData;

    /** 当前自动刷新的定时器事件间隔
    */
    int32_t m_nIntervalSeconds;

    /** 当前定时器的ID
    */
    size_t m_timerId;
};

#endif //EXAMPLES_MAIN_FORM_H_
