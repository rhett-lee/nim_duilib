#pragma once

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

class RichEditFindReplace
{
public:
    RichEditFindReplace();

public:
	//设置关联的RichEdit控件
	void SetRichEdit(ui::RichEdit* pRichEdit);

public:
	//查找
	bool FindRichText(const std::wstring& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog);

	//查找下一个
	bool FindNext();

	//替换
	bool ReplaceRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog);
	
	//全部替换
	bool ReplaceAllRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog);

protected:
	//没找到，回调函数
	virtual void OnTextNotFound(const std::wstring& findText);

	//开始全部替换
	virtual void OnReplaceAllCoreBegin();

	//完成全部替换
	virtual void OnReplaceAllCoreEnd(int replaceCount);

private:
	//查找函数实现
	bool FindTextSimple(const std::wstring& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord);

	//查找并选择
	long FindAndSelect(DWORD dwFlags, FINDTEXTEX& ft);

	//比较当前选择与文本是否匹配
	bool SameAsSelected(const std::wstring& replaceText, BOOL bMatchCase);

	//没找到，回调函数
	void TextNotFound(const std::wstring& findText);

	//调整查找替换窗口的位置
	void AdjustDialogPosition(HWND hWndDialog);

private:
    //RichEdit控件接口
    ui::RichEdit* m_pRichEdit;

	//查找文字
	std::wstring m_sFindNext;

	//替换文字
	std::wstring m_sReplaceWith;

	//搜索方向选项
	bool m_bFindDown;

	//是否区分大小写
	bool m_bMatchCase;

	//是否全字匹配
	bool m_bMatchWholeWord;

private:
	//查找状态: 是否为第一次搜索
	bool m_bFirstSearch;

	//初始搜索的位置
	long m_nInitialSearchPos;

	//旧的光标
	HCURSOR m_hOldCursor;
};

