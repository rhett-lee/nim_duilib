#ifndef EXAMPLES_BROWSER_BOX_H_
#define EXAMPLES_BROWSER_BOX_H_

// duilib
#include "duilib/duilib.h"

// ui components
#include "ui_components/ui_components.h"
#include "ui_components/ui_cef_control.h"

/** @file browser_box.h
* @brief 标签页盒子
* @copyright (c) 2016, NetEase Inc. All rights reserved
* @author Redrain
* @date 2019/3/20
*/
class TaskbarTabItem;
class MultiBrowserForm;
class BrowserBox : public ui::VBox
{
    typedef ui::VBox BaseClass;
public:
    friend class MultiBrowserForm;

    /**
    * 构造函数
    * @param[in] id    浏览器盒子的唯一标识，用于区分不同的标签页
    */
    BrowserBox(ui::Window* pWindow, std::string id);
    ~BrowserBox() { };

    /**
    * 获取id
    * @return string 标识id
    */
    std::string GetId() const { return browser_id_; }

    /**
    * 获取本浏览器盒子所属的浏览器窗口指针
    * @return MultiBrowserForm* 浏览器窗口
    */
    MultiBrowserForm* GetBrowserForm() const;

    /**
    * 获取Cef控件指针
    * @return ui::CefControlBase* Cef控件
    */
    nim_comp::CefControlBase* GetCefControl();

    /**
    * 获取网页标题
    * @return DString& 网页标题
    */
    DString& GetTitle();

    /**
    * 初始化浏览器盒子
    * @param[in] url 初始化URL
    * @return void    无返回值
    */
    virtual void InitBrowserBox(const DString &url);

    /**
    * 反初始化浏览器盒子
    * @return void    无返回值
    */
    virtual void UninitBrowserBox();

    /**
    * 根据控件类名创建自定义控件
    * @param[in] pstrClass 控件类名
    * @return Control* 创建的控件的指针
    */
    ui::Control* CreateControl(const DString& pstrClass);

    //////////////////////////////////////////////////////////////////////////
    //窗口合并功能相关的操作
public:
    /**
    * 获取与本浏览器盒子绑定的Tab指针
    * @return TaskbarItem*    Tab指针
    */
    TaskbarTabItem* GetTaskbarItem();

    /**
    * 覆盖父类虚函数，用于指定本控件所属窗体
    * @param[in] pWindow 所属窗口指针
    */
    virtual void SetWindow(ui::Window* pWindow) override;

    /**
    * 覆盖基类虚函数，为了在重绘任务栏显示的缩略图
    * @return void    无返回值
    */
    virtual void Invalidate() override;

    /**
    * 覆盖基类虚函数，为了在重绘任务栏显示的缩略图
    * @param[in] rc 要设置的控件的位置
    * @return void    无返回值
    */
    virtual void SetPos(ui::UiRect rc) override;

private:
    void OnBeforeMenu(CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model);
    bool OnMenuCommand(CefRefPtr<CefContextMenuParams> params, int command_id, CefContextMenuHandler::EventFlags event_flags);
    void OnTitleChange(const DString& title);
    void OnUrlChange(const DString& url);
    bool OnLinkClick(const DString& url);
    CefRequestHandler::ReturnValue OnBeforeNavigate(CefRefPtr<CefRequest> request, bool is_redirect);
    void OnLoadingStateChange(bool isLoading, bool canGoBack, bool canGoForward);
    void OnLoadStart();
    void OnLoadEnd(int httpStatusCode);
    void OnLoadError(CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl);

private:
    nim_comp::CefControlBase    *cef_control_;

    // 任务栏缩略图管理
    TaskbarTabItem        *taskbar_item_;
    MultiBrowserForm    *browser_form_;
    std::string            browser_id_;
    DString        url_;
    DString        title_;
};

#endif //EXAMPLES_BROWSER_BOX_H_
