#pragma once

#include "resource.h"

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

#include "RichEditFindReplace.h"

class FindForm;
class ReplaceForm;
class MainForm : public ui::WindowImplBase
{
public:
	MainForm();
	~MainForm();

	/**
	 * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
	 * GetSkinFolder		接口设置你要绘制的窗口皮肤资源路径
	 * GetSkinFile			接口设置你要绘制的窗口的 xml 描述文件
	 * GetWindowClassName	接口设置窗口唯一的类名称
	 */
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;

	/**
	 * 收到 WM_CREATE 消息时该函数会被调用，通常做一些控件初始化的操作
	 */
	virtual void OnInitWindow() override;

	/**
	 * 收到 WM_CLOSE 消息时该函数会被调用
	 */
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
	virtual LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
	virtual LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

	static const std::wstring kClassName;

	//查找/替换接口
	void FindRichText(const std::wstring& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog);
	void ReplaceRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog);
	void ReplaceAllRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog);

	//获取RichEdit接口
	ui::RichEdit* GetRichEdit() const;

private:
	//加载默认的文本内容
	void LoadRichEditData();

	//打开文件、保存文件、另存为文件
	void OnOpenFile();
	void OnSaveFile();
	void OnSaveAsFile();

	bool LoadFile(const std::wstring& filePath);
	bool SaveFile(const std::wstring& filePath);

	//判断一个文件扩展名是否为RTF文件
	bool IsRtfFile(const std::wstring& filePath) const;

	static DWORD CALLBACK StreamReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb);
	static DWORD CALLBACK StreamWriteCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb);

	//更新保存状态
	void UpdateSaveStatus();

private:
	//查找/替换
	void OnFindText();
	void OnFindNext();
	void OnReplaceText();

private:

	//RichEdit控件接口
	ui::RichEdit* m_pRichEdit;

	//当前打开的文件
	std::wstring m_filePath;

	//保存按钮的文字
	std::wstring m_saveBtnText;

	//查找
	FindForm* m_pFindForm;

	//替换
	ReplaceForm* m_pReplaceForm;

	//查找替换实现
	RichEditFindReplace m_findReplace;
};


