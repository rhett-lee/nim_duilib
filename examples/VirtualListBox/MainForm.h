#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

#include "DataProvider.h"

class MainForm : public ui::WindowImplBase
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

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

protected:
    /** 语言切换事件（当前语言可通过GlobalManager获取）
    * @return 返回false表示不再触发窗口的kWindowLanguageChangedMsg事件, 否则触发该事件
    */
    virtual bool OnLanguageChanged() override;

private:
    bool OnClicked(const ui::EventArgs& args);

    /** 测试虚表的事件
    */
    void TestVirtualListBoxEvents(ui::VirtualListBox* pListBox);

    /** 获取消息的基本信息（用于显示日志）
    */
    DString GetEventDisplayInfo(const ui::EventArgs& args, ui::VirtualListBox* pListBox);

    /** 输出测试日志
    */
    void OutputDebugLog(const DString& logMsg);

    /** 把事件信息追加到界面的事件日志中
    */
    void AppendEventLog(const DString& logMsg);

    /** 清空界面的事件日志
    */
    void ClearEventLog();

    /** 刷新当前已选择项的数量显示
    */
    void UpdateSelectionStats();

    /** 从编辑框读取数据元素索引号，无效时返回 false
    */
    bool GetElementIndexFromEdit(ui::RichEdit* pEdit, size_t nElementCount, size_t& nIndex) const;

    /** 记录一条非法的索引输入日志
    */
    void LogInvalidIndex(ui::RichEdit* pEdit, size_t nElementCount);

    /** 事件日志显示开关状态变化
    */
    bool OnShowLogChanged(const ui::EventArgs& args);

    /** 获取语言字符串
    */
    DString GetLangString(const DString& stringId) const;

private:
    ui::VirtualListBox* m_pTileList;
    DataProvider* m_pDataProvider;

private:
    ui::RichEdit* m_pEditColumn;
    ui::RichEdit* m_pEditTotal;
    ui::Option* m_pOptionColumnFix;
    ui::RichEdit* m_pEditUpdate;
    ui::RichEdit* m_pEditTaskName;
    ui::RichEdit* m_pEditDelete;
    ui::RichEdit* m_pEditChildMarginX;
    ui::RichEdit* m_pEditChildMarginY;

private:
    ui::Label* m_pLabelSelection;
    ui::CheckBox* m_pLogCheck;
    ui::Box* m_pLogPanel;
    ui::RichEdit* m_pEditEventLog;
    int32_t m_nEventLogLines;
};

#endif //EXAMPLES_MAIN_FORM_H_
