#ifndef EXAMPLES_BROWSER_FORM_WINDOWS_H_
#define EXAMPLES_BROWSER_FORM_WINDOWS_H_

// duilib
#include "duilib/duilib.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "Browser/BrowserForm.h"
#include "Windows/taskbar/TaskbarManager.h"
#include "ShObjidl.h"

interface IDropTargetHelper;
class BrowserBox;

/** 离屏模式Cef多标签浏览器窗口(Windows实现部分)
*/
class BrowserForm_Windows: public BrowserForm, public IDropTarget, public TaskbarManager::ITaskbarDelegate
{
    typedef BrowserForm BaseClass;
public:
    BrowserForm_Windows();
    virtual ~BrowserForm_Windows() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口已经被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

    /** 拦截并处理底层窗体消息
    * @param[in] uMsg 消息类型
    * @param[in] wParam 附加参数
    * @param[in] lParam 附加参数
    * @param[out] bHandled 是否处理了消息，如果处理了不继续传递消息
    * @return LRESULT 处理结果
    */
    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

public:
    /** 在执行拖拽操作前，如果被拖拽的浏览器盒子属于本窗口，则通知本窗口
    * @param[in] browser_id 浏览器id
    * @return void    无返回值
    */
    void OnBeforeDragBoxCallback(const DString &browser_id);

    /** 在执行拖拽操作后，如果被拖拽的浏览器盒子属于本窗口，则通知本窗口操作结果
    * @param[in] drop_succeed 浏览器盒子是否被拖拽到了外部
    * @return void    无返回值
    */
    void OnAfterDragBoxCallback(bool drop_succeed);

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

public:
    /**
    * 初始化窗口拖放功能
    * @return void    无返回值
    */
    bool InitDragDrop();

    /**
    * 反初始化窗口拖放功能
    * @return void    无返回值
    */
    void UnInitDragDrop();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT STDMETHODCALLTYPE DragLeave(void);
    HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD __RPC_FAR *pdwEffect);

private:
    /**
    * 判断是否要拖拽浏览器盒子
    * @param[in] param 处理浏览器窗口左侧会话合并列表项发送的事件
    * @return bool 返回值true: 继续传递控件消息， false: 停止传递控件消息
    */
    bool OnProcessTabItemDrag(const ui::EventArgs& param);

    /**
    * 生成当前窗体中某个区域对应的位图，用于离屏渲染模式
    * @param[in] src_rect 目标位图的位置
    * @return HBITMAP 生成的位图
    */
    ui::IBitmap* GenerateBoxOffsetRenderBitmap(const ui::UiRect &src_rect);

    /**
    * 生成当前激活的浏览器盒子的位图，用于有窗模式
    * @param[in] src_rect 目标位图的位置
    * @return HBITMAP 生成的位图
    */
    ui::IBitmap* GenerateBoxWindowBitmap();

    /** 新建了一个标签
    * @param [in] tab_item 标签页的接口
    * @param [in] browser_box 网页盒子的接口
    */
    virtual void OnCreateNewTabPage(ui::TabCtrlItem* tab_item, BrowserBox* browser_box) override;

    /** 关闭了一个标签
    * @param [in] browser_box 网页盒子的接口
    */
    virtual void OnCloseTabPage(BrowserBox* browser_box) override;

private:
    // 处理浏览器盒子拖放事件
    IDropTargetHelper* m_pDropHelper;

    // 处理浏览器盒子拖拽事件
    bool m_bDragState;
    POINT m_oldDragPoint;
    DString m_dragingBrowserId;

    // 任务栏缩略图管理器
    TaskbarManager m_taskbarManager;
};

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#endif //EXAMPLES_BROWSER_FORM_WINDOWS_H_
