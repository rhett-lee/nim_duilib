#ifndef UI_CONTROL_MASK_H_
#define UI_CONTROL_MASK_H_

#include "duilib/Core/Box.h"

namespace ui {

/** 遮罩/蒙版层控件（继承Box）
 *  用于以下场景，跨平台通用（Windows/Linux/macOS/FreeBSD）：
 *  - 模态对话框/弹层的背景遮罩：半透明蒙层覆盖整个区域并拦截鼠标输入；
 *  - 新手引导蒙层：在蒙层上"镂空"高亮某个控件，其余区域保持半透明，且镂空区域内点击可穿透到目标控件；
 *  - 加载遮罩：蒙层覆盖内容区并拦截交互，可在蒙层内放置 Loading 动画、提示文案等子控件。
 *
 *  Mask 本质是一个容器(Box)：在蒙层之上可以继续放置任意子控件（进度动画、引导文案、按钮等）。
 *  作为浮动的覆盖层使用时，建议配置 float="true" width="stretch" height="stretch"，
 *  并作为父容器的最后一个子节点（或配合 paint_order）以保证绘制在最上层、命中测试优先命中。
 *
 *  XML 用法示例（默认蒙层色为语义色 bg_overlay，默认拦截鼠标输入）：
 *  <Mask class="mask"/>                                        // 铺满父容器、拦截鼠标
 *  <Mask class="mask" mask_color="bg_overlay" mouse_enabled="false"/>  // 仅遮挡视觉，不拦截鼠标
 *  <Mask class="mask" highlight_control="btn_guide" highlight_padding="6" highlight_corner="6"
 *        click_through="true"/>                                // 镂空高亮 btn_guide，且点击可穿透
 */
class DUILIB_API Mask : public Box
{
    typedef Box BaseClass;
public:
    explicit Mask(Window* pWindow);
    Mask(const Mask& r) = delete;
    Mask& operator=(const Mask& r) = delete;
    ~Mask() override;

    /// 重写父类接口，提供个性化功能。方法具体说明请查看 Control 控件
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintBkColor(IRender* pRender) override;
    virtual Control* FindControl(FINDCONTROLPROC Proc, void* pProcData, uint32_t uFlags,
                                 const UiPoint& ptMouse = UiPoint(),
                                 const UiPoint& scrollPos = UiPoint()) override;
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

public:
    /** 设置/获取蒙层颜色（语义色名，默认 bg_overlay；为空表示不绘制蒙层底色）
    */
    void SetMaskColor(const DString& strColor);
    const DString& GetMaskColor() const { return m_strMaskColor; }

    /** 设置/获取需要镂空(高亮)的控件名称；为空表示不镂空，整块区域均为蒙层
    */
    void SetHighlightControl(const DString& strControlName);
    const DString& GetHighlightControl() const { return m_strHighlightControl; }

    /** 设置/获取镂空区域相对高亮控件的内边距（DPI自适应），默认 4
    */
    void SetHighlightPadding(int32_t nPadding, bool bNeedDpiScale = true);
    int32_t GetHighlightPadding() const { return m_nHighlightPadding; }

    /** 设置/获取镂空区域的圆角半径（DPI自适应），默认 0（直角）
    */
    void SetHighlightCorner(int32_t nCorner, bool bNeedDpiScale = true);
    int32_t GetHighlightCorner() const { return m_nHighlightCorner; }

    /** 设置/获取镂空区域内的鼠标点击是否穿透到下层控件，默认 false
    *   为 true 时（通常配合 highlight_control 使用）：点击镂空区域会命中下方的目标控件，
    *   可用于新手引导"让用户点击高亮按钮"的流程。
    */
    void SetClickThrough(bool bThrough);
    bool IsClickThrough() const { return m_bClickThrough; }

public:
    /** 便捷接口：在当前窗口上覆盖一个遮罩层（使用默认蒙层色 bg_overlay、铺满整个窗口、拦截鼠标）
    * @param [in] pWindow 目标窗口
    * @return 返回创建的遮罩控件指针（失败返回 nullptr）；调用方可继续设置镂空控件、
    *         追加子控件等；不再需要时调用 Remove() 移除。
    */
    static Mask* ShowOverWindow(Window* pWindow);

    /** 从父容器中移除本遮罩
    *   注意：若父容器开启了自动销毁子控件（IsAutoDestroyChild 为 true），调用后本对象会被销毁，
    *        返回后不可再访问本指针。
    */
    void Remove();

private:
    /** 获取镂空(高亮)区域（窗口客户区坐标），返回 false 表示无有效镂空区域
    */
    bool GetHighlightRect(UiRect& rcHighlight);

    /** 向路径中添加一个圆角矩形（顺时针方向）
    */
    static void AddRoundRectPath(IPath* pPath, const UiRectF& rc, float rx, float ry);

private:
    /** 蒙层颜色（语义色名）
    */
    DString m_strMaskColor;

    /** 镂空(高亮)的控件名称，为空表示不镂空
    */
    DString m_strHighlightControl;

    /** 镂空区域内边距（DPI缩放后的物理像素）
    */
    int32_t m_nHighlightPadding;

    /** 镂空区域圆角半径（DPI缩放后的物理像素）
    */
    int32_t m_nHighlightCorner;

    /** 镂空区域内的点击是否穿透到下层控件
    */
    bool m_bClickThrough;
};

} // namespace ui

#endif // UI_CONTROL_MASK_H_
