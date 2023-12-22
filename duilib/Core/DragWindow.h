#ifndef UI_CORE_DRAG_WINDOW_H_
#define UI_CORE_DRAG_WINDOW_H_

#pragma once

#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Utils/WinImplBase.h"
#include "duilib/Render/IRender.h"

namespace ui
{
/** 拖动过程中显示的窗口
*/
class DragWindow : public WindowImplBase
{
public:
    DragWindow();

protected:
    //析构函数不允许外部调用，因在OnFinalMessage函数中，有delete this操作
    virtual ~DragWindow();

public:
    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录
    */
    virtual std::wstring GetSkinFolder() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual std::wstring GetSkinFile() override;

    /** 创建窗口时被调用，由子类实现用以获取窗口唯一的类名称
    * @return 子类需实现并返回窗口唯一的类名称
    */
    virtual std::wstring GetWindowClassName() const override;

    /** 在窗口收到 WM_NCDESTROY 消息时会被调用
    * @param [in] hWnd 要销毁的窗口句柄
    */
    virtual void OnFinalMessage(HWND hWnd) override;

public:
    /** 设置显示的图片
    * @param [in] pBitmap 图片资源的接口
    */
    virtual void SetDragImage(const std::shared_ptr<IBitmap>& pBitmap);

    /** 调整窗口位置，跟随鼠标
    */
    virtual void AdjustPos();

public:
    /** 增加引用计数
    */
    void AddRef();

    /** 减少引用计数
    */
    void Release();

private:
    /** 引用计数
    */
    int32_t m_nRefCount;
};
}

#endif // UI_CORE_DRAG_WINDOW_H_