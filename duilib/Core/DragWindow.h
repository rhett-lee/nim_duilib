#ifndef UI_CORE_DRAG_WINDOW_H_
#define UI_CORE_DRAG_WINDOW_H_

#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Utils/WinImplBase.h"
#include "duilib/Render/IRender.h"

namespace ui
{
class DpiManager;

/** 拖动过程中显示的窗口
*/
class DUILIB_API DragWindow : public WindowImplBase
{
public:
    DragWindow();

protected:
    //析构函数不允许外部调用，因在OnFinalMessage函数中，有delete this操作
    virtual ~DragWindow() override;

public:
    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录
    */
    virtual DString GetSkinFolder() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual DString GetSkinFile() override;

    /** 在窗口收到 WM_NCDESTROY 消息时会被调用
    * @param [in] hWnd 要销毁的窗口句柄
    */
    virtual void OnFinalMessage() override;

public:
    /** 设置显示的图片
    * @param [in] pBitmap 图片资源的接口
    */
    virtual void SetDragImage(const std::shared_ptr<IBitmap>& pBitmap);

    /** 调整窗口位置，跟随鼠标
    */
    virtual void AdjustPos();

    /** 计算拖拽窗口的位置和大小（屏幕坐标，包含阴影区域）
    *   用于创建窗口时直接指定位置和大小，避免窗口显示瞬间先出现在创建位置再跳转
    * @param [in] pBitmap 拖拽时显示的位图（可以为空，为空时使用XML中配置的初始大小）
    * @param [in] ptCursor 鼠标位置（屏幕坐标）
    * @param [in] dpi 窗口使用的DPI管理器（用于像素密度换算）
    * @return 窗口的位置和大小（屏幕坐标）
    */
    UiRect CalcWindowRect(const IBitmap* pBitmap, const UiPoint& ptCursor, const DpiManager& dpi);

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
