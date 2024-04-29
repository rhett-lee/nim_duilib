#pragma once
#include "app_db.h"

enum ThreadId
{
	kThreadUI
};

class MoveControlForm : public ui::WindowImplBase
{
public:
	MoveControlForm(const std::wstring& class_name, const std::wstring& theme_directory, const std::wstring& layout_xml);
	~MoveControlForm();

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
	virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

private:
	std::wstring m_class_name;
	std::wstring m_theme_directory;
	std::wstring m_layout_xml;

	ui::Box* m_frequent_app=nullptr;
	ui::Box* m_my_app = nullptr;
};

