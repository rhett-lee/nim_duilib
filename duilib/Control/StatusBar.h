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
*   - 可选在右下角绘制尺寸手柄（size_grip），用于提示窗口可调整大小。
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
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

public:
    /** 设置相邻状态面板之间分隔线的宽度（像素，0 表示不绘制分隔线）
    */
    void SetSeparatorWidth(int32_t nWidth);
    int32_t GetSeparatorWidth() const;

    /** 设置分隔线颜色（语义色名或颜色值字符串，如 "border_window" 或 "#FFD5D5D5"）
    */
    void SetSeparatorColor(const DString& strColor);
    DString GetSeparatorColor() const;

    /** 设置是否绘制右下角的尺寸手柄（仅作视觉提示，不改变窗口本身的可调整大小行为）
    */
    void SetShowSizeGrip(bool bShow);
    bool IsShowSizeGrip() const;

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

private:
    //分隔线宽度（像素，0 表示不绘制分隔线）
    int32_t m_nSeparatorWidth;

    //分隔线颜色
    UiString m_separatorColor;

    //是否绘制右下角的尺寸手柄
    bool m_bShowSizeGrip;

    //尺寸手柄颜色
    UiString m_sizeGripColor;
};

} // namespace ui

#endif // UI_CONTROL_STATUS_BAR_H_
