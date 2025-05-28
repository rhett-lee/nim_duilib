#ifndef UI_BOX_LISTBOX_HELPER_H_
#define UI_BOX_LISTBOX_HELPER_H_

#include "duilib/Core/Callback.h"
#include "duilib/Core/EventArgs.h"
#include "duilib/Core/UiTypes.h"

namespace ui 
{
class ListBox;
class IRender;

/** ListBox鼠标键盘操作的辅助类
*/
class UILIB_API ListBoxHelper: public SupportWeakCallback
{
public:
    explicit ListBoxHelper(ListBox* pListBox);
    ~ListBoxHelper();
    ListBoxHelper(const ListBoxHelper&) = delete;
    ListBoxHelper& operator = (const ListBoxHelper&) = delete;

public:
    /** 设置是否支持鼠标框选功能
    */
    void SetEnableFrameSelection(bool bEnable);

    /** 获取是否支持鼠标框选功能
    */
    bool IsEnableFrameSelection() const;

    /** 设置鼠标框选填充颜色
    */
    void SetFrameSelectionColor(const DString& frameSelectionColor);

    /** 获取鼠标框选填充颜色
    */
    DString GetFrameSelectionColor() const;

    /** 设置鼠标框选填充颜色的Alpha值
    */
    void SetframeSelectionAlpha(uint8_t frameSelectionAlpha);

    /** 获取鼠标框选填充颜色的Alpha值
    */
    uint8_t GetFrameSelectionAlpha() const;

    /** 设置鼠标框选边框颜色
    */
    void SetFrameSelectionBorderColor(const DString& frameSelectionBorderColor);

    /** 获取鼠标框选边框颜色
    */
    DString GetFrameSelectionBorderColor() const;

    /** 设置鼠标框选边框的大小
    * @param [in] nBorderSize 边框大小（未经DPI缩放）
    */
    void SetFrameSelectionBorderSize(int32_t nBorderSize);

    /** 获取鼠标框选边框的大小（未经DPI缩放）
    */
    int32_t GetFrameSelectionBorderSize() const;

    /** 设置普通列表项（非Header、非置顶）的top坐标(目前ListCtrl在用此功能)
    */
    void SetNormalItemTop(int32_t nNormalItemTop);

    /** 获取普通列表项（非Header、非置顶）的top坐标(目前ListCtrl在用此功能)
    */
    int32_t GetNormalItemTop() const;

public:
    //鼠标消息处理
    void OnButtonDown(const UiPoint& ptMouse, Control* pSender);
    void OnButtonUp(const UiPoint& ptMouse, Control* pSender);
    void OnRButtonDown(const UiPoint& ptMouse, Control* pSender);
    void OnRButtonUp(const UiPoint& ptMouse, Control* pSender);
    void OnMouseMove(const UiPoint& ptMouse, Control* pSender);

    /** 收到窗口失去焦点消息
    */
    void OnWindowKillFocus();

public:
    /** 绘制鼠标框选的边框和填充颜色
    */
    void PaintFrameSelection(IRender* pRender);

private:

    /** 检查是否需要滚动视图
    */
    void OnCheckScrollView();

private:
    /** 关联的ListBox控件
    */
    ListBox* m_pListBox;

private:
    /** 是否鼠标在视图中按下左键或者右键
    */
    bool m_bMouseDownInView = false;

    /** 是否鼠标左键按下
    */
    bool m_bMouseDown = false;

    /** 是否鼠标右键按下
    */
    bool m_bRMouseDown = false;

    /** 是否处于鼠标滑动操作中
    */
    bool m_bInMouseMove = false;

    /** 鼠标按下时的鼠标位置
    */
    UiSize64 m_ptMouseDown;

    /** 鼠标滑动时的鼠标位置
    */
    UiSize64 m_ptMouseMove;

    /** 鼠标按下时的控件接口
    */
    Control* m_pMouseSender;

    /** 定时器滚动视图时的取消机制
    */
    WeakCallbackFlag m_scrollViewFlag;

    /** 是否支持鼠标框选功能
    */
    bool m_bEnableFrameSelection;

    /** 框选填充颜色
    */
    UiString m_frameSelectionColor;

    /** 框选填充颜色的Alpha值
    */
    uint8_t m_frameSelectionAlpha;

    /** 框选边框颜色
    */
    UiString m_frameSelectionBorderColor;

    /** 框选边框大小
    */
    uint8_t m_frameSelectionBorderSize;

    /** 普通列表项（ListCtrl: 非Header、非置顶）的top坐标
    */
    int32_t m_nNormalItemTop;
private:

};

} // namespace ui

#endif // UI_BOX_LISTBOX_HELPER_H_
