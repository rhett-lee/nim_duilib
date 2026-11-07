#ifndef EXAMPLES_FIND_REPLACE_FORM_H_
#define EXAMPLES_FIND_REPLACE_FORM_H_

// duilib
#include "duilib/duilib.h"

/** 查找/替换使用的选项
*/
struct RichEditFindOptions
{
    //查找内容
    DString findText;

    //替换内容
    DString replaceText;

    //搜索方向：true表示向下搜索，false表示向上搜索
    bool bFindDown = true;

    //是否区分大小写
    bool bMatchCase = true;

    //是否全字匹配
    bool bMatchWholeWord = false;
};

class RichEditFindReplace
{
public:
    RichEditFindReplace();

public:
    //设置关联的RichEdit控件
    void SetRichEdit(ui::RichEdit* pRichEdit);

public:
    //查找
    bool FindRichText(const RichEditFindOptions& options, ui::Window* pWndDialog);

    //是否已有查找内容
    bool HasFindText() const;

    //查找下一个
    bool FindNext();

    //替换
    bool ReplaceRichText(const RichEditFindOptions& options, ui::Window* pWndDialog);
    
    //全部替换
    bool ReplaceAllRichText(const RichEditFindOptions& options);

    //上一次替换操作替换的处数
    int32_t GetLastReplaceCount() const;

    /** 统计当前查找条件的匹配情况（不改变当前选区）
    @param [out] nIndex 当前选区所在的匹配序号（从1开始；当前选区不是匹配项时为0）
    @param [out] nTotal 全文匹配总数
    */
    void GetMatchInfo(int32_t& nIndex, int32_t& nTotal) const;

protected:
    //没找到，回调函数
    virtual void OnTextNotFound(const DString& findText);

    //开始全部替换
    virtual void OnReplaceAllCoreBegin();

    //完成全部替换
    virtual void OnReplaceAllCoreEnd(int replaceCount);

private:
    //保存新的查找选项（条件变化时重新开始查找）
    void ApplyOptions(const RichEditFindOptions& options);

    //按当前保存的查找选项查找并选择，查找范围不包含当前选区
    bool FindTextSimple();

    //查找并选择
    bool FindAndSelect(const ui::FindTextParam& findParam);

    //比较当前选择与文本是否匹配
    bool SameAsSelected(const DString& replaceText, bool bMatchCase);

    //没找到，回调函数
    void TextNotFound(const DString& findText);

    //调整查找替换窗口的位置
    void AdjustDialogPosition(ui::Window* pWndDialog);

private:
    //RichEdit控件接口
    ui::RichEdit* m_pRichEdit;

    //查找文字
    DString m_sFindNext;

    //替换文字
    DString m_sReplaceWith;

    //搜索方向选项
    bool m_bFindDown;

    //是否区分大小写
    bool m_bMatchCase;

    //是否全字匹配
    bool m_bMatchWholeWord;

    //上一次替换操作替换的处数
    int32_t m_nLastReplaceCount;

private:
    //查找状态: 是否为第一次搜索
    bool m_bFirstSearch;

    //初始搜索的位置
    int32_t m_nInitialSearchPos;

    //旧的光标
    ui::CursorID m_nOldCursor;
};

#endif //EXAMPLES_FIND_REPLACE_FORM_H_
