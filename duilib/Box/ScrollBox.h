#ifndef UI_BOX_SCROLLBOX_H_
#define UI_BOX_SCROLLBOX_H_

#include "duilib/Box/HLayout.h"
#include "duilib/Box/VLayout.h"
#include "duilib/Box/HTileLayout.h"
#include "duilib/Box/VTileLayout.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Core/Box.h"
#include "duilib/Animation/AnimationPlayer.h"

namespace ui 
{

 /** 带有垂直或水平滚动条的容器，使容器可以容纳更多内容
 *   通过修改布局，形成 HScrollBox 和 VScrollBox 和 TileScrollBox三个子类
 */
class UILIB_API ScrollBox : public Box
{
    typedef Box BaseClass;
public:
    explicit ScrollBox(Window* pWindow, Layout* pLayout = new Layout);
    ScrollBox(const ScrollBox& r) = delete;
    ScrollBox& operator=(const ScrollBox& r) = delete;
    virtual ~ScrollBox() override;

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& pstrName, const DString& pstrValue) override;
    virtual void SetPos(UiRect rc) override;
    virtual void HandleEvent(const EventArgs& msg) override;
    virtual bool MouseEnter(const EventArgs& msg) override;
    virtual bool MouseLeave(const EventArgs& msg) override;
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;
    virtual void SetParent(Box* pParent) override;
    virtual void SetWindow(Window* pWindow) override;
    virtual Control* FindControl(FINDCONTROLPROC Proc, void* pProcData,
                                 uint32_t uFlags, const UiPoint& ptMouse,
                                 const UiPoint& scrollPos = UiPoint()) override;
    virtual void ClearImageCache() override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 获取滚动条的真实偏移量, 以32位整型值标志偏移 (虚表会使用虚拟滚动条位置)
    *   如果设置了ScrollVirtualOffset，那么这个函数会将滚动条的位置减去这个虚拟偏移；
    *   如果没有设置ScrollVirtualOffset，那么这个函数返回与UiSize64 GetScrollPos()相同的结果，但会检查是否越界；
    *   这个函数存在的意义是支持大数据量的虚表（VirtualScrollBox），避免UiRect越界。
    */
    UiSize GetScrollOffset() const;

    /** ScrollOffset 值变化通知接口
    * @param [in] oldScrollOffset 旧值
    * @param [in] newScrollOffset 新值
    */
    virtual void OnScrollOffsetChanged(const UiSize& /*oldScrollOffset*/, const UiSize& /*newScrollOffset*/) {}

    /** 获取滚动条位置(cx代表横向滚动条的位置，cy代表纵向滚动条的位置)
     */
    virtual UiSize64 GetScrollPos() const;

    /** 获取滚动条的范围(cx代表横向滚动条的范围，cy代表纵向滚动条的范围)
     */
    virtual UiSize64 GetScrollRange() const;

    /** 设置滚动条位置
     * @param[in] szPos 要设置的位置数据(cx代表横向滚动条的位置，cy代表纵向滚动条的位置)
     */
    virtual void SetScrollPos(UiSize64 szPos);

    /** 设置滚动条 Y 轴坐标
     * @param[in] y 要设置的 Y 轴坐标数值
     */
    virtual void SetScrollPosY(int64_t y);
    
    /** 设置滚动条 X 轴坐标
     * @param[in] x 要设置的 X 轴坐标数值
     */
    virtual void SetScrollPosX(int64_t x);
    
    /** 向上滚动滚动条
     * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
     * @param[in] withAnimation 是否附带动画效果，默认为 true
     */
    virtual void LineUp(int32_t deltaValue = DUI_NOSET_VALUE, bool withAnimation = true);
    
    /** 向下滚动滚动条
     * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
     * @param[in] withAnimation 是否附带动画效果，默认为 true
     */
    virtual void LineDown(int32_t deltaValue = DUI_NOSET_VALUE, bool withAnimation = true);
    
    /** 向左滚动滚动条
     * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
     */
    virtual void LineLeft(int32_t deltaValue = DUI_NOSET_VALUE);
    
    /** 向右滚动滚动条
     * @param[in] deltaValue 滚动距离，默认为 DUI_NOSET_VALUE
     */
    virtual void LineRight(int32_t deltaValue = DUI_NOSET_VALUE);
    
    /** 向上滚动一个页面大小的距离
     */
    virtual void PageUp();

    /** 向下滚动一个页面大小的距离
     */
    virtual void PageDown();

    /** 回到滚动条最上方
     */
    virtual void HomeUp();

    /** 滚动到最下方位置
     * @param[in] arrange 是否重置滚动条位置，默认为 true
     * @param[in] withAnimation 是否包含动画特效，默认为 true
     */
    virtual void EndDown(bool arrange = true, bool withAnimation = true);

    /** 向左滚动一个页面大小的距离
     */
    virtual void PageLeft();

    /** 向右滚动一个页面大小的距离
     */
    virtual void PageRight();

    /** 滚动到最左侧
     */
    virtual void HomeLeft();

    /** 滚动到最右侧
     */
    virtual void EndRight();

    /** 触摸向上滚动（响应 WM_TOUCH 消息）
     * @param[in] deltaValue 滚动距离
     */
    virtual void TouchUp(int32_t deltaValue);

    /** 触摸向下滚动（响应 WM_TOUCH 消息）
     * @param[in] deltaValue 滚动距离
     */
    virtual void TouchDown(int32_t deltaValue);

    /** 启用滚动条
     * @param[in] bEnableVertical 是否启用垂直滚动条，默认为 true
     * @param[in] bEnableHorizontal 是否启用水平滚动条，默认为 true
     */
    virtual void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);

    /** 获取垂直滚动条对象指针
     */
    ScrollBar* GetVScrollBar() const;

    /** 获取水平滚动条对象指针
     */
    ScrollBar* GetHScrollBar() const;

    /** 判断垂直滚动条是否有效
     */
    bool IsVScrollBarValid() const;

    /** 判断水平滚动条是否有效
     */
    bool IsHScrollBarValid() const;

    /** 播放动画
     */
    void PlayRenderOffsetYAnimation(int64_t nRenderY);

    /** 是否已经在底部
     */
    bool IsAtEnd() const;

    /** 是否锁定到底部
     */
    bool IsHoldEnd() const;

    /** 设置滚动条是否始终锁定到底部位置
     * @param[in] bHoldEnd 设置 true 表示锁定，false 为不锁定
     */
    void SetHoldEnd(bool bHoldEnd);
    
    /** 获取垂直滚动条滚动步长
     */
    int32_t GetVerScrollUnitPixels() const;

    /** 设置垂直滚动条滚动步长
     * @param [in] nUnitPixels 要设置的步长
     * @param [in] bNeedDpiScale 是否需要DPI缩放
     */
    void SetVerScrollUnitPixels(int32_t nUnitPixels, bool bNeedDpiScale);

    /** 获取横向滚动条滚动步长
    */
    int32_t GetHorScrollUnitPixels() const;

    /** 设置横向滚动条滚动步长
    * @param [in] nUnitPixels 要设置的步长
    * @param [in] bNeedDpiScale 是否需要DPI缩放
    */
    void SetHorScrollUnitPixels(int32_t nUnitPixels, bool bNeedDpiScale);

    /** 获取容器的滚动条是否悬浮在子控件上面
     * @return 返回 true 表示悬浮在滚动条上，否则为 false
     */
    bool GetScrollBarFloat() const;

    /** 设置容器的滚动条是否悬浮在子控件上面
     * @param[in] bScrollBarFloat true 表示悬浮在滚动条上，false 表示不悬浮在控件上
     */
    void SetScrollBarFloat(bool bScrollBarFloat);

    /** 获取容器的滚动条是否在左侧显示
     * @return 返回 true 表示在左侧，false 为右侧
     */
    bool IsVScrollBarAtLeft() const;

    /** 设置容器的滚动条是否在左侧显示
    * @param[in] bAtLeft true 表示在左侧，false 为右侧
    */
    void SetVScrollBarAtLeft(bool bAtLeft);

    /** 获取滚动条的外边距
     */
    const UiPadding& GetScrollBarPadding() const;

    /** 设置滚动条的外边距，可以让滚动条不占满容器
     * @param [in] rcScrollBarPadding 要设置的边距
     * @param [in] bNeedDpiScale 是否支持DPI缩放
     */
    void SetScrollBarPadding(UiPadding rcScrollBarPadding, bool bNeedDpiScale);

    /** 停止滚动条动画
    */
    void StopScrollAnimation();

    /** 监听滚动条位置变化事件
     * @param[in] callback 有变化后通知的回调函数
     */
    void AttachScrollChange(const EventCallback& callback) { AttachEvent(kEventScrollChange, callback); }

public:
    /** 获取滚动条虚拟偏移量(cx代表横向滚动条的虚拟偏移量，cy代表纵向滚动条的虚拟偏移量)
     */
    const UiSize64& GetScrollVirtualOffset() const;

    /** 设置滚动条虚拟偏移量
     * @param[in] szPos 要设置的位置数据(cx代表横向滚动条的位置，cy代表纵向滚动条的位置)
     */
    void SetScrollVirtualOffset(const UiSize64& szOffset);

    /** 设置滚动条 Y 轴虚拟偏移量
     */
    void SetScrollVirtualOffsetY(int64_t yOffset);

    /** 设置滚动条 X 轴虚拟偏移量
     */
    void SetScrollVirtualOffsetX(int64_t xOffset);

protected:
    /** 计算所需的尺寸
     * @param[in] rc 当前位置信息, 外部调用时，不需要剪去内边距
     * @return 返回所需尺寸大小, 包含ScrollBox自身的内边距，不包含外边距
     */
    virtual UiSize64 CalcRequiredSize(const UiRect& rc);

    /** 设置鼠标可用状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetMouseEnabled(bool bChanged) override;

private:
    /** 设置位置大小
    * @param [in] rc外部传入的矩形范围
    * @param [in] bScrollProcess true表示内部递归调用，false表示外部调用
    */
    void DoSetPos(UiRect rc, bool bScrollProcess);

    /** 设置位置大小(内部逻辑实现)
    * @param [in] rc外部传入的矩形范围
    * @param [in] bScrollProcess true表示内部递归调用，false表示外部调用
    */
    void SetPosInternally(const UiRect& rc, bool bScrollProcess);

    /** 设置纵向滚动条的位置
    */
    void ProcessVScrollBar(UiRect rcScrollBarPos, int64_t nScrollRange, bool bShowHScrollBar, bool& bNeedResetPos);

    /** 设置横向滚动条的位置
     */
    void ProcessHScrollBar(UiRect rcScrollBarPos, int64_t nScrollRange, bool bShowVScrollBar, bool& bNeedResetPos);

    /** 判断是否显示水平滚动条
    */
    bool NeedShowHScrollBar(UiRect rcBox, int64_t cxRequired,
                            UiRect& rcScrollBarPos, int64_t& nScrollRange) const;

    /** 判断是否显示垂直滚动条
    */
    bool NeedShowVScrollBar(UiRect rcBox, int64_t cyRequired,
                            UiRect& rcScrollBarPos, int64_t& nScrollRange) const;

private:
    //垂直滚动条接口
    std::unique_ptr<ScrollBar> m_pVScrollBar;

    //水平滚动条接口
    std::unique_ptr<ScrollBar> m_pHScrollBar;

    //滚动条的虚拟偏移量
    UiSize64 m_scrollVirtualOffset;

private:
    //滚动条动画效果支持
    AnimationPlayer* m_pScrollAnimation;

    //滚动条动画效果支持
    AnimationPlayer* m_pRenderOffsetYAnimation;

    //滚动条的外边距
    UiPadding m_rcScrollBarPadding;

    //垂直滚动条滚动步长
    int32_t m_nVScrollUnitPixels;

    //水平滚动条滚动步长
    int32_t m_nHScrollUnitPixels;

    //是否锁定到底部
    bool m_bHoldEnd;

    //容器的滚动条是否悬浮在子控件上面
    bool m_bScrollBarFloat;

    //容器的滚动条是否在左侧显示
    bool m_bVScrollBarAtLeft;
};

/** 横向布局的ScrollBox
*/
class UILIB_API HScrollBox : public ScrollBox
{
public:
    explicit HScrollBox(Window* pWindow) :
        ScrollBox(pWindow, new HLayout)
    {
    }

    virtual DString GetType() const override { return DUI_CTR_HSCROLLBOX; }
};

/** 纵向布局的ScrollBox
*/
class UILIB_API VScrollBox : public ScrollBox
{
public:
    explicit VScrollBox(Window* pWindow) :
        ScrollBox(pWindow, new VLayout)
    {
    }

    virtual DString GetType() const override { return DUI_CTR_VSCROLLBOX; }
};

/** 瓦片布局的ScrollBox(横向布局)
*/
class UILIB_API HTileScrollBox : public ScrollBox
{
public:
    explicit HTileScrollBox(Window* pWindow) :
        ScrollBox(pWindow, new HTileLayout)
    {
    }

    virtual DString GetType() const override { return DUI_CTR_HTILE_SCROLLBOX; }
};

/** 瓦片布局的ScrollBox(纵向布局)
*/
class UILIB_API VTileScrollBox : public ScrollBox
{
public:
    explicit VTileScrollBox(Window* pWindow) :
        ScrollBox(pWindow, new VTileLayout)
    {
    }

    virtual DString GetType() const override { return DUI_CTR_VTILE_SCROLLBOX; }
};

} // namespace ui

#endif // UI_BOX_SCROLLBOX_H_
