#ifndef EXAMPLES_RICHEDIT_FIND_FORM_H_
#define EXAMPLES_RICHEDIT_FIND_FORM_H_

// duilib
#include "duilib/duilib.h"

#include "RichEditFindReplace.h"

class MainForm;

/** 查找/替换窗口的公共基类
*/
class FindReplaceFormBase : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    explicit FindReplaceFormBase(MainForm* pMainForm);
    virtual ~FindReplaceFormBase() override;

    virtual DString GetSkinFolder() override;

    /** 键盘按下：处理 Esc 关闭、F3 查找下一个
    */
    virtual LRESULT OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

protected:
    //初始化查找相关控件与公共事件
    void InitFindBase();

    //从界面控件收集查找/替换选项（子类可覆盖以补充替换内容）
    virtual RichEditFindOptions CollectSearchOptions() const;

    //保存查找选项到主窗口
    void SaveFindOptions(const RichEditFindOptions& options);

    //查找下一个
    void OnFindNext();

    //更新查找结果的计数显示（如"第 2 处，共 8 处"）
    void UpdateFindResultLabel(bool bFound);

    //绑定取消按钮
    void BindCancelButton();

protected:
    MainForm* m_pMainForm;
    ui::Option* m_pDirectionDown;
    ui::Option* m_pDirectionUp;
    ui::CheckBox* m_pCaseSensitive;
    ui::CheckBox* m_pMatchWholeWord;
    ui::RichEdit* m_pFindText;
    ui::Label* m_pFindResult;
};

class FindForm : public FindReplaceFormBase
{
    typedef FindReplaceFormBase BaseClass;
public:
    explicit FindForm(MainForm* pMainForm);
    virtual ~FindForm() override;

    virtual DString GetSkinFile() override;

    virtual void OnInitWindow() override;
};

#endif //EXAMPLES_RICHEDIT_FIND_FORM_H_
