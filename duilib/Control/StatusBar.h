#ifndef UI_CONTROL_STATUS_BAR_H_
#define UI_CONTROL_STATUS_BAR_H_

#include "duilib/Box/HBox.h"

namespace ui
{

/** 状态栏控件
*   用于显示光标位置、字数、连接状态、后台进度等状态提示，是编辑器 / IDE / 通信类软件的常用界面元素。
*   实现为一个水平容器（继承 HBox），其中的子控件即为一个个"状态面板"（通常为 Label / IconControl / Progress 等）：
*   - 子控件可设置固定宽度、拉伸（width="stretch"）、内边距等，沿用 HBox 的全部布局能力；
*   - 自动在相邻子控件之间绘制竖直分隔线，可通过 separator_width / separator_color 调整或关闭；
*   - 可选在右下角绘制尺寸手柄（size_grip）。当状态栏停靠在窗口客户区右下角、
*     且窗口本身具备可缩放能力时（Windows 默认具备；SDL 等平台需在 XML 中声明非零 size_box），
*     手柄区域会自动并入窗口的可拖拽缩放区（通过放大窗口的 size_box 实现），
*     可以用鼠标真正拖拽调整窗口大小；不需要此行为时可设置 size_grip_resize="false"。
*
*   面板（分格）索引 API：业务代码无需再 FindControl + dynamic_cast 找子 Label，
*   可以直接用 GetPanel / SetPanelText 等按索引操作各个状态面板。
*
*   XML 节点名称为 "StatusBar"，继承 HBox 的全部属性。
*/
class DUILIB_API StatusBar : public HBox
{
    typedef HBox BaseClass;
public:
    explicit StatusBar(Window* pWindow);

public:
    /// 重写父类接口，提供个性化功能。方法具体说明请查看 Control / Box 控件
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void SetPos(UiRect rc) override;
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

public:
    /** @name 状态面板（分格）索引接口
    *
    *   面板索引按布局顺序编号，从 0 开始，且只统计可见面板（不可见的面板不占索引），
    *   与视觉上看到的分格顺序一致。面板内容发生变化（增删子控件、改可见性）后索引会重新编号。
    * @{
    */

    /** 获取状态面板的数量（按布局顺序，不含不可见的面板）
    */
    size_t GetPanelCount() const;

    /** 按索引获取状态面板控件
    * @param [in] nIndex 面板索引，从 0 开始，按布局顺序
    * @return 面板控件指针；索引越界时返回 nullptr
    */
    Control* GetPanel(size_t nIndex) const;

    /** 获取指定控件在本状态栏中的面板索引
    * @param [in] pPanel 待查询的控件指针
    * @return 面板索引（从 0 开始）；该控件不是本状态栏的可见面板时返回 -1
    */
    int32_t GetPanelIndex(Control* pPanel) const;

    /** 按名称查找状态面板（等价于在 StatusBar 的子控件中按 name 属性查找）
    * @param [in] strName 面板的 name
    * @return 面板控件指针；未找到时返回 nullptr
    */
    Control* FindPanel(const DString& strName) const;

    /** 设置指定索引面板的文本内容
    * @param [in] nIndex 面板索引，从 0 开始
    * @param [in] strText 要设置的文本内容
    * @return true 表示设置成功；
    *         false 表示索引越界，或该面板不是文本类控件（Label / RichEdit 等支持文本的面板）
    */
    bool SetPanelText(size_t nIndex, const DString& strText);

    /** 获取指定索引面板的文本内容
    * @param [in] nIndex 面板索引，从 0 开始
    * @return 面板的文本内容；索引越界或面板不支持文本时返回空字符串
    */
    DString GetPanelText(size_t nIndex) const;

    /** @} */

public:
    /** 设置相邻状态面板之间分隔线的宽度（像素，0 表示不绘制分隔线）
    */
    void SetSeparatorWidth(int32_t nWidth);
    int32_t GetSeparatorWidth() const;

    /** 设置分隔线颜色（语义色名或颜色值字符串，如 "border_window" 或 "#FFD5D5D5"）
    */
    void SetSeparatorColor(const DString& strColor);
    DString GetSeparatorColor() const;

    /** 设置是否绘制右下角的尺寸手柄
    *   手柄是否可拖拽，由 IsSizeGripResize 控制（见 SetSizeGripResize）
    */
    void SetShowSizeGrip(bool bShow);
    bool IsShowSizeGrip() const;

    /** 设置尺寸手柄是否参与窗口的拖拽缩放
    *   为 true（默认）时，若本状态栏停靠在窗口客户区的右下角，
    *   则会把窗口的右下角可拉伸区域（size_box 的 right/bottom）扩大到覆盖手柄的大小，
    *   使手柄可以真正拖拽调整窗口大小（Windows 走 WM_NCHITTEST，SDL 走命中测试回调）。
    *   前提是窗口本身具备可缩放能力：Windows 默认具备（WS_THICKFRAME）；
    *   SDL 等平台需在 XML 中给 Window 声明非零 size_box，否则窗口创建时未开启可缩放。
    *   注意：该效果是窗口级的，扩大后窗口底边一带也会成为可拉伸区域。
    *   为 false 时，手柄仅作视觉提示（历史行为）。
    *   只有在 IsShowSizeGrip() 为 true 时本设置才生效。
    */
    void SetSizeGripResize(bool bResize);
    bool IsSizeGripResize() const;

    /** 设置尺寸手柄的颜色
    */
    void SetSizeGripColor(const DString& strColor);
    DString GetSizeGripColor() const;

private:
    /** 绘制相邻面板之间的分隔线
    * @param [in] rcContent 状态栏内容区的矩形（已扣除内边距）
    */
    void PaintSeparator(IRender* pRender, const UiRect& rcContent);

    /** 绘制右下角的尺寸手柄
    * @param [in] rcContent 状态栏内容区的矩形（已扣除内边距）
    */
    void PaintSizeGrip(IRender* pRender, const UiRect& rcContent);

    /** 获取尺寸手柄的绘制区域（客户区坐标，未做DPI缩放前的逻辑值由调用方处理）
    * @param [in] rcContent 状态栏内容区的矩形（已扣除内边距）
    */
    UiRect GetSizeGripRect(const UiRect& rcContent) const;

    /** 获取尺寸手柄需要的最小命中尺寸（像素，已做 DPI 缩放）
    *   该尺寸用于把窗口的右下角可拉伸区域扩大到覆盖手柄
    */
    int32_t GetSizeGripHitSize() const;

    /** 刷新尺寸手柄的拖拽命中区域（在布局位置变化、手柄开关变化时调用）
    */
    void UpdateSizeGripHitArea();

    /** 还原被本控件临时放大的窗口 size_box
    */
    void RestoreSizeBox();

private:
    //分隔线宽度（像素，0 表示不绘制分隔线）
    int32_t m_nSeparatorWidth;

    //分隔线颜色
    UiString m_separatorColor;

    //是否绘制右下角的尺寸手柄
    bool m_bShowSizeGrip;

    //尺寸手柄是否参与窗口拖拽缩放
    bool m_bSizeGripResize;

    //尺寸手柄颜色
    UiString m_sizeGripColor;

    //本控件临时放大的窗口 size_box 的原始值（m_bSizeBoxBackupValid 为 true 时有效）
    UiRect m_rcSizeBoxBackup;

    //本控件写入窗口的 size_box 值（用于还原前确认没有被他人修改）
    UiRect m_rcSizeBoxApplied;

    //m_rcSizeBoxBackup / m_rcSizeBoxApplied 是否有效
    bool m_bSizeBoxBackupValid;
};

} // namespace ui

#endif // UI_CONTROL_STATUS_BAR_H_
