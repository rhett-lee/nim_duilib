#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

class MainForm : public ui::WindowImplBase
{
public:
    MainForm();
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

    /** 完成界面布局的初始化，各个控件的位置大小等布局信息完成初始化，供子类在界面启动后调整界面布局等操作
    */
    virtual void OnInitLayout() override;

private:
    /** 填充数据
    */
    void InsertItemData(int32_t nRows, int32_t nColumns, int32_t nImageId);

    /** 初始化本程序的测试功能相关UI事件
    */
    void InitListCtrlEvents(ui::ListCtrl* pListCtrl);

    /** 执行一些功能测试
    */
    void RunListCtrlTest();

    /** 控制该列
    */
    void OnColumnChanged(size_t nColumnId);

    /** 测试ListCtrl的loading功能
    */
    void TestListCtrlLoading(ui::ListCtrl* pListCtrl);

    /** 测试Loading功能
    */
    void OnTestLoadingProgress();

    /** 测试ListCtrl的事件回调接口
    */
    void TestListCtrlEvents(ui::ListCtrl* pListCtrl);

    /** 获取消息的基本信息（用于显示日志）
    */
    DString GetEventDisplayInfo(const ui::EventArgs& args);

    /** 获取消息的基本信息（用于显示日志）
    */
    DString GetItemFilledEventDisplayInfo(const ui::EventArgs& args);

    /** 输出测试日志
    */
    void OutputDebugLog(const DString& logMsg);

private:
    //测试功能: 鼠标划入时显示子控件，鼠标移出时隐藏子控件
    void OnReportViewSubItemFilled(const ui::EventArgs& args);
    void OnReportViewSubItemMouseEnter(const ui::EventArgs& args);
    void OnReportViewSubItemMouseLeave(const ui::EventArgs& args);

private:
    /** loading状态的加载进度（模拟值，实际应用可用按情况计算进度）
    */
    float m_fLoadingPercent;
};

#endif //EXAMPLES_MAIN_FORM_H_
