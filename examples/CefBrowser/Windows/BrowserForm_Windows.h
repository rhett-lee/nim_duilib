#ifndef EXAMPLES_BROWSER_FORM_WINDOWS_H_
#define EXAMPLES_BROWSER_FORM_WINDOWS_H_

// duilib
#include "duilib/duilib.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "Browser/BrowserForm.h"
#include "Windows/taskbar/TaskbarManager.h"

class BrowserBox;

/** 离屏模式Cef多标签浏览器窗口(Windows实现部分)
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
    /** 在执行拖拽操作前，如果被拖拽的浏览器盒子属于本窗口，则通知本窗口
    * @param [in] browserId 浏览器id
    */
    void OnBeforeDragBoxCallback(const DString& browserId);

    /** 在执行拖拽操作后，如果被拖拽的浏览器盒子属于本窗口，则通知本窗口操作结果
    * @param [in] bDropSucceed 浏览器盒子是否被拖拽到了外部
    */
    void OnAfterDragBoxCallback(bool bDropSucceed);

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

    /** 鼠标移动消息（WM_MOUSEMOVE）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] bFromNC true表示这是NC消息（WM_NCMOUSEMOVE）, false 表示是WM_MOUSEMOVE消息
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMoveMsg(const ui::UiPoint& pt, uint32_t modifierKey, bool bFromNC, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标左键弹起消息（WM_LBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonUpMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口丢失鼠标捕获（WM_CAPTURECHANGED）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCaptureChangedMsg(const ui::NativeMsg& nativeMsg, bool& bHandled) override;

private:
    // 处理浏览器盒子拖拽事件
    bool m_bButtonDown;
    bool m_bDragState;
    POINT m_oldDragPoint;
    DString m_dragingBrowserId;

    // 任务栏缩略图管理器
    TaskbarManager m_taskbarManager;
};

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#endif //EXAMPLES_BROWSER_FORM_WINDOWS_H_
