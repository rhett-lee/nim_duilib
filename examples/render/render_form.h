#pragma once

// base header
#include "base/base.h"

// duilib
#include "duilib/duilib.h"

class RenderForm : public ui::WindowImplBase
{
public:
	RenderForm();
	~RenderForm();

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

	/**@brief 当要创建的控件不是标准的控件名称时会调用该函数
	 * @param[in] strClass 控件名称
	 * @return 返回一个自定义控件指针，一般情况下根据 strClass 参数创建自定义的控件
	 */
	virtual ui::Control* CreateControl(const std::wstring& strClass) override;

	/** 窗口类名
	*/
	static const std::wstring kClassName;

private:
	//测试PropertyGrid控件
	void TestPropertyGrid();

	//获取PropertyGrid控件的结果
	void CheckPropertyGridResult();
};

