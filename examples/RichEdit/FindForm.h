#pragma once

#include "resource.h"

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

class MainForm;
class FindForm : public ui::WindowImplBase
{
public:
	explicit FindForm(MainForm* pMainForm);
	~FindForm();

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
	 * 标识窗口 class name
	 */
	static const LPCTSTR kClassName;

private:
	//查找下一个
	void OnFindNext();

private:
	//查找接口
	MainForm* m_pMainForm;

	//搜索方向选项
	ui::Option* m_pDirectionOption;

	//是否区分大小写
	ui::CheckBox* m_pCaseSensitive;

	//是否全字匹配
	ui::CheckBox* m_pMatchWholeWord;

	//查找内容
	ui::RichEdit* m_pFindText;
};

