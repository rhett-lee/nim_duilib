#pragma once

enum ThreadId
{
	kThreadUI,
	kThreadGlobalMisc
};

class ControlForm : public ui::WindowImplBase
{
public:
	ControlForm();
	~ControlForm();

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

	/**
	 * 标识窗口 class name
	 */
	static const std::wstring kClassName;

private:

	/** 显示菜单
	* @param [in] point 显示位置坐标，屏幕坐标
	*/
	void ShowPopupMenu(const ui::UiPoint& point);

	/**
	 * 被投递到杂事线程读取 xml 数据的任务函数
	 */
	void LoadRichEditData();

	/**
	 * 用于在杂事线程读取 xml 完成后更新 UI 内容的接口
	 */
	void OnResourceFileLoaded(const std::wstring& xml);

	/**
	 * 动态更新进度条接口
	 */
	void OnProgressValueChagned(float value);

	/** 显示拾色器窗口
	*/
	void ShowColorPicker();

private:
	/** 接收键盘按键按下消息(WM_HOTKEY)时被调用
	* @param [in] uMsg 消息内容
	* @param [in] wParam 消息附加参数
	* @param [in] lParam 消息附加参数
	* @param [out] bHandled 返回 false 则继续派发该消息，否则不再派发该消息
	* @return 返回消息处理结果
	*/
	virtual LRESULT OnHotKey(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
};

