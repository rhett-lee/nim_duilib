#ifndef EXAMPLES_MULTI_BROWSER_FORM_H_
#define EXAMPLES_MULTI_BROWSER_FORM_H_

// duilib
#include "duilib/duilib.h"

/** @file multi_browser_form.h
* @brief 离屏模式Cef多标签浏览器窗口
* @copyright (c) 2016, NetEase Inc. All rights reserved
* @author Redrain
* @date 2019/3/20
*/
#include "taskbar/taskbar_manager.h"
#include "ShObjidl.h"

interface IDropTargetHelper;

class BrowserBox;
class BrowserTabItem;
class MultiBrowserForm : public ui::ShadowWnd, public IDropTarget, public TaskbarManager::ITaskbarDelegate
{
public:
    MultiBrowserForm();
    virtual ~MultiBrowserForm() override;
    
    //覆盖虚函数
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;   

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /**
    * 根据控件类名创建自定义控件
    * @param[in] pstrClass 控件类名
    * @return Control* 创建的控件的指针
    */
    virtual ui::Control* CreateControl(const DString& pstrClass) override;

    /**
    * 拦截并处理底层窗体消息
    * @param[in] uMsg 消息类型
    * @param[in] wParam 附加参数
    * @param[in] lParam 附加参数
    * @param[out] bHandled 是否处理了消息，如果处理了不继续传递消息
    * @return LRESULT 处理结果
    */
    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    /** 窗口关闭消息（WM_CLOSE）
    * @param [in] wParam 消息的wParam参数
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息
    */
    virtual void OnFinalMessage() override;

private:
    void OnWndSizeMax(bool max);
    bool OnClicked(const ui::EventArgs& arg);
    bool OnReturn(const ui::EventArgs& arg);

public:
    /**
    * 在本窗口内创建一个新浏览器盒子
    * @param[in] browser_id 浏览器id
    * @param[in] url 初始化URL
    * @return BrowserBox* 浏览器盒子
    */
    BrowserBox* CreateBox(const std::string &browser_id, DString url);

    /**
    * 关闭本窗口内的一个浏览器盒子
    * @param[in] browser_id 浏览器id
    * @return void    无返回值
    */
    void CloseBox(const std::string &browser_id);

    /**
    * 把一个其他窗口内的浏览器盒子附加到本窗口内
    * @param[in] browser_box 浏览器盒子
    * @return bool true 成功，false 失败
    */
    bool AttachBox(BrowserBox *browser_box);

    /**
    * 把本窗口内的一个浏览器盒子脱离窗口
    * @param[in] browser_box 浏览器盒子
    * @return bool true 成功，false 失败
    */
    bool DetachBox(BrowserBox *browser_box);

    /**
    * 获取当前显示的浏览器盒子
    * @return BrowserBox* 浏览器盒子
    */
    BrowserBox* GetSelectedBox();

    /**
    * 激活并切换到某个浏览器盒子(同时让浏览器窗口激活)
    * @param[in] browser_id 浏览器id
    * @return void 无返回值
    */
    void SetActiveBox(const std::string &browser_id);

    /**
    * 判断浏览器盒子是否处于激活状态(同时判断浏览器窗口是否被激活)
    * @param[in] browser_box 浏览器盒子
    * @return bool true 是，false 否
    */
    bool IsActiveBox(const BrowserBox *browser_box);

    /**
    * 判断浏览器盒子是否处于激活状态(同时判断浏览器窗口是否被激活)
    * @param[in] browser_id 浏览器id
    * @return bool true 是，false 否
    */
    bool IsActiveBox(const DString &browser_id);

    /**
    * 获取本窗口内浏览器盒子的总量
    * @return int    总量
    */
    int GetBoxCount() const;

    /**
    * 在执行拖拽操作前，如果被拖拽的浏览器盒子属于本窗口，则通知本窗口
    * @param[in] browser_id 浏览器id
    * @return void    无返回值
    */
    void OnBeforeDragBoxCallback(const DString &browser_id);

    /**
    * 在执行拖拽操作后，如果被拖拽的浏览器盒子属于本窗口，则通知本窗口操作结果
    * @param[in] drop_succeed 浏览器盒子是否被拖拽到了外部
    * @return void    无返回值
    */
    void OnAfterDragBoxCallback(bool drop_succeed);

private:

    /**
    * 处理标签控件的选中消息
    * @param[in] param 消息的相关信息
    * @return bool true 继续传递控件消息，false 停止传递控件消息
    */
    bool OnTabItemSelected(const ui::EventArgs& param);

    /**
    * 处理标签控件的关闭按钮的单击消息
    * @param[in] param 消息的相关信息
    * @param[in] browser_id 列表项对应的浏览器id
    * @return bool true 继续传递控件消息，false 停止传递控件消息
    */
    bool OnTabItemClose(const ui::EventArgs& param, const std::string& browser_id);

    /**
    * 在本窗口内查找浏览器盒子
    * @param[in] browser_id 浏览器id
    * @return BrowserBox* 浏览器盒子
    */
    BrowserBox* FindBox(const DString &browser_id);

    /**
    * 在本窗口内查找标签控件
    * @param[in] browser_id 浏览器id
    * @return BrowserBox* 浏览器盒子
    */
    BrowserTabItem* FindTabItem(const DString &browser_id);

    /**
    * 切换某个浏览器盒子为显示状态
    * @param[in] browser_id 浏览器id
    * @return bool true 成功，false 失败
    */
    bool ChangeToBox(const DString &browser_id);

public:
    /**
    * 获取窗体句柄
    * @return HWND    窗体句柄
    */
    virtual HWND GetHandle() const override { return this->NativeWnd()->GetHWND(); };

    /**
    * 获取渲染接口
    * @return IRender*    渲染接口
    */
    virtual ui::IRender* GetTaskbarRender() const override { return this->GetRender(); };

    /**
    * 关闭一个任务栏项
    * @param[in] id 任务栏项id
    * @return void    无返回值
    */
    virtual void CloseTaskbarItem(const std::string &id) override { CloseBox(id); }

    /**
    * 激活并切换到一个任务栏项
    * @param[in] id 任务栏项id
    * @return void 无返回值
    */
    virtual void SetActiveTaskbarItem(const std::string &id) override { SetActiveBox(id); }

    /**
    * 设置某个浏览器对应的标签控件的标题
    * @param[in] browser_id 浏览器id
    * @param[in] name 标题
    * @return void    无返回值
    */
    void SetTabItemName(const DString &browser_id, const DString &name);

    /**
    * 设置某个浏览器对应的标签控件的URL
    * @param[in] browser_id 浏览器id
    * @param[in] url URL
    * @return void    无返回值
    */
    void SetURL(const std::string &browser_id, const DString &url);

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

private:
    ui::Label        *lbl_title_;
    ui::Button        *btn_max_restore_;
    ui::RichEdit    *edit_url_;

    ui::ListBox            *tab_list_;
    ui::TabBox            *borwser_box_tab_;
    BrowserBox            *active_browser_box_;

    // 处理浏览器盒子拖放事件
    IDropTargetHelper    *drop_helper_;

    // 处理浏览器盒子拖拽事件
    bool                is_drag_state_;
    POINT                old_drag_point_;
    DString        draging_browser_id_;

    // 任务栏缩略图管理器
    TaskbarManager        taskbar_manager_;
};
#endif //EXAMPLES_MULTI_BROWSER_FORM_H_
