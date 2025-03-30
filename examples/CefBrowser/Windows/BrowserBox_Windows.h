#ifndef EXAMPLES_BROWSER_BOX_WINDOWS_H_
#define EXAMPLES_BROWSER_BOX_WINDOWS_H_

#include "browser/BrowserBox.h"

/**  标签页盒子的Windows平台实现
*/
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
class TaskbarTabItem;

class BrowserBox_Windows: public BrowserBox
{
    typedef BrowserBox BaseClass;
public:
    /** 构造函数
    * @param [in] pWindow 关联的窗口
    * @param [in] id 浏览器盒子的唯一标识，用于区分不同的标签页
    */
    BrowserBox_Windows(ui::Window* pWindow, std::string id);
    virtual ~BrowserBox_Windows() override {};

public:
    /** 获取与本浏览器盒子绑定的Tab指针
    * @return TaskbarItem*    Tab指针
    */
    TaskbarTabItem* GetTaskbarItem() const;

    /** 控件类型
    */
    virtual DString GetType() const override { return _T("BrowserBox_Windows"); }

    /** 初始化浏览器盒子
    * @param [in] url 初始化URL
    */
    virtual void InitBrowserBox(const DString& url);

    /** 反初始化浏览器盒子
    */
    virtual void UninitBrowserBox();

    /** 覆盖基类虚函数，为了在重绘任务栏显示的缩略图
    */
    virtual void Invalidate() override;

    /** 覆盖基类虚函数，为了在重绘任务栏显示的缩略图
    * @param [in] rc 要设置的控件的位置
    */
    virtual void SetPos(ui::UiRect rc) override;

    /** 标题变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const DString& title) override;

     /** 网站图标下载完成事件（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnDownloadFavIconFinished(CefRefPtr<CefBrowser> browser,
                                           const CefString& image_url,
                                           int http_status_code,
                                           CefRefPtr<CefImage> image) override;

private:
    /** 将CEF的图片转换为图标句柄
    */
    HICON ConvertCefImageToHICON(CefImage& cefImage) const;

private:
    // 任务栏缩略图管理
    TaskbarTabItem* m_pTaskBarItem;
};

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#endif //EXAMPLES_BROWSER_BOX_WINDOWS_H_
