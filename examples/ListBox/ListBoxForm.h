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

    /** 语言切换后刷新动态文本（列表项标题与提示日志）
    */
    virtual bool OnLanguageChanged() override;

private:
    /** 测试列表的事件
    */
    void TestListBoxEvents(ui::ListBox* pListBox);

    /** 获取消息的基本信息（用于显示日志）
    */
    DString GetEventDisplayInfo(const ui::EventArgs& args, ui::ListBox* pListBox);

    /** 输出测试日志（显示在窗口底部的日志区）
    */
    void OutputDebugLog(const DString& logMsg);

    /** 追加一行日志到日志区
    */
    void PrintLog(const DString& logMsg);

    /** 清空日志区
    */
    void ClearLog();

    /** 刷新状态栏（项数、已选数量）
    */
    void RefreshStatus();

    /** 列表项被删除后，刷新状态栏
    */
    void OnItemRemoved();

    /** 多选开关的勾选状态变化
    */
    bool OnMultiSelectChanged(const ui::EventArgs& args);

    /** "显示日志"开关的勾选状态变化
    */
    bool OnShowLogChanged(const ui::EventArgs& args);

private:
    /** 列表容器
    */
    ui::ListBox* m_pListBox;

    /** 容器类型显示
    */
    ui::Label* m_pTypeLabel;

    /** 项数显示
    */
    ui::Label* m_pItemCountLabel;

    /** 已选数量显示
    */
    ui::Label* m_pSelCountLabel;

    /** 多选开关
    */
    ui::CheckBox* m_pMultiCheck;

    /** "显示日志"开关
    */
    ui::CheckBox* m_pLogCheck;

    /** 日志区容器（用于整体显示/隐藏）
    */
    ui::Box* m_pLogPanel;

    /** 日志区
    */
    ui::RichEdit* m_pLogView;

    /** 日志序号
    */
    int32_t m_nLogId;

    /** 日志区是否只包含初始提示（语言切换时需要重新生成提示）
    */
    bool m_bOnlyHintLogged;
};

#endif //EXAMPLES_LISTBOX_FORM_H_
