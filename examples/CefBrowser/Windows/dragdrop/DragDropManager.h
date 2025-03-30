#ifndef EXAMPLES_DRAGDROP_MANAGER_H_
#define EXAMPLES_DRAGDROP_MANAGER_H_

#include "browser/BrowserBox.h"

/** 多标签浏览器管理类，用于统一管理多标签拖拽功能
  */
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

class SdkDataObject;
class BrowserForm_Windows;
class DragDropManager : public virtual ui::SupportWeakCallback
{
public:
    DragDropManager();
    DragDropManager(const DragDropManager&) = delete;
    DragDropManager& operator=(const DragDropManager&) = delete;

    /** 单例对象
    */
    static DragDropManager* GetInstance();

private:
    ~DragDropManager();

public:
    /**
    * 设置是否使用自定义的拖拽缩略图
    * @param[in] use true:使用自定义拖拽缩略图，会使用低级键盘钩子，一些安全软件会拦截
    *                 false:使用系统的拖拽缩略图，效果上不如自定义的好
    * @return 无返回值
    */
    void SetUseCustomDragImage(bool use);

    /**
    * 是否使用自定义的拖拽缩略图
    * @return bool true 是，false 否
    */
    bool IsUseCustomDragImage() const;

    /**
    * 当前是否正在拖拽浏览器盒子
    * @return bool true 是，false 否
    */
    bool IsDragingBorwserBox() const;

    /**
    * 设置被放入浏览器盒子的浏览器窗体
    * @param[in] browser_form 浏览器窗口
    * @return void    无返回值
    */
    void SetDropForm(BrowserForm_Windows* browser_form);

    /**
    * 执行拖拽浏览器盒子的操作
    * @param[in] browser_box 浏览器盒子
    * @param[in] bitmap 浏览器盒子生成的位图
    * @param[in] pt_offset 拖拽图像相对于鼠标的偏移
    * @return bool true 成功，false 失败
    */
    bool DoDragBorwserBox(BrowserBox *browser_box, HBITMAP bitmap, POINT pt_offset);

private:
    /**
    * 创建一个用于拖拽的IDataObject对象
    * @param[in] bitmap 用于生成拖拽效果的位图
    * @param[in] pt_offset 拖拽图像相对于鼠标的偏移
    * @return SdkDataObject* IDataObject对象
    */
    SdkDataObject* CreateDragDataObject(HBITMAP bitmap, POINT pt_offset);

    /**
    * 执行拖拽操作前被调用
    * @return void    无返回值
    */
    void OnBeforeDragBorwserBox(BrowserBox *browser_box, HBITMAP bitmap, POINT pt_offset);

    /**
    * 执行拖拽操作后被调用
    * @return void    无返回值
    */
    void OnAfterDragBorwserBox();

private:
    bool m_bUseCustomDragImage;
    BrowserBox* m_pDragingBox;    // 当下正在被拖拽的浏览器盒子
    BrowserForm_Windows* m_pDropBrowserForm;    // 拖入浏览器盒子的浏览器窗口，用于附加拖拽来的浏览器盒子
};

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#endif //EXAMPLES_DRAGDROP_MANAGER_H_
