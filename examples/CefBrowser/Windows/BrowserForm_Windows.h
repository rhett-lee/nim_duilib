#ifndef EXAMPLES_BROWSER_FORM_WINDOWS_H_
#define EXAMPLES_BROWSER_FORM_WINDOWS_H_

// duilib
#include "duilib/duilib.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "Browser/BrowserForm.h"
#include "Windows/taskbar/TaskbarManager.h"

class BrowserBox;

/** 离屏模式Cef多标签浏览器窗口(Windows实现部分，增加了任务栏缩略图功能)
*/
class BrowserForm_Windows: public BrowserForm, public TaskbarManager::ITaskbarDelegate
{
    typedef BrowserForm BaseClass;
public:
    BrowserForm_Windows();
    virtual ~BrowserForm_Windows() override;

    /** 创建BrowserBox对象
    * @param [in] pWindow 关联的窗口
    * @param [in] id 浏览器盒子的唯一标识，用于区分不同的标签页
    */
    virtual BrowserBox* CreateBrowserBox(ui::Window* pWindow, std::string id) override;

    /** 拦截并处理底层窗体消息
    * @param[in] uMsg 消息类型
    * @param[in] wParam 附加参数
    * @param[in] lParam 附加参数
    * @param[out] bHandled 是否处理了消息，如果处理了不继续传递消息
    * @return LRESULT 处理结果
    */
    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;   

public:
    /** 获取窗体句柄
    * @return HWND    窗体句柄
    */
    virtual HWND GetHandle() const override { return this->NativeWnd()->GetHWND(); };

    /** 获取渲染接口
    * @return IRender*    渲染接口
    */
    virtual ui::IRender* GetTaskbarRender() const override { return this->GetRender(); };

    /** 关闭一个任务栏项
    * @param[in] id 任务栏项id
    * @return void    无返回值
    */
    virtual void CloseTaskbarItem(const std::string &id) override { CloseBox(id); }

    /** 激活并切换到一个任务栏项
    * @param[in] id 任务栏项id
    * @return void 无返回值
    */
    virtual void SetActiveTaskbarItem(const std::string &id) override { SetActiveBox(id); }

private:
    /** 新建了一个标签
    * @param [in] pTabItem 标签页的接口
    * @param [in] pBrowserBox 网页盒子的接口
    */
    virtual void OnCreateNewTabPage(ui::TabCtrlItem* pTabItem, BrowserBox* pBrowserBox) override;

    /** 关闭了一个标签
    * @param [in] pBrowserBox 网页盒子的接口
    */
    virtual void OnCloseTabPage(BrowserBox* pBrowserBox) override;

private:
    // 任务栏缩略图管理器
    TaskbarManager m_taskbarManager;
};

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#endif //EXAMPLES_BROWSER_FORM_WINDOWS_H_
