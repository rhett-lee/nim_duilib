#ifndef UI_BOX_PANEL_H_
#define UI_BOX_PANEL_H_

#include "duilib/Core/Box.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

/** 手风琴分组接口：使 Panel / PanelHBox / PanelVBox 三种模板实例可被统一识别与操作。
*
*   不需要窗口级注册表——面板展开时在所属窗口的控件树中查找同组面板并折叠即可，
*   分组作用域为单个 Window，与 Option 的 group 作用域一致。
*/
class IPanelGroup
{
public:
    virtual ~IPanelGroup() = default;

    /** 获取手风琴分组名（空字符串表示不属于任何分组）
    */
    virtual DString GetPanelGroupName() const = 0;

    /** 当前是否处于折叠状态
    */
    virtual bool IsPanelCollapsed() const = 0;

    /** 设置折叠状态（供同组其他面板联动调用）
    */
    virtual void SetPanelCollapsed(bool bCollapsed, bool bFireEvent) = 0;
};

/** 面板控件：带标题栏的容器，支持折叠/展开
*
*   与 GroupBox 的区别：GroupBox 用线条勾勒分组；Panel 拥有独立的标题栏区域，
*   可设置标题栏背景色/文字颜色/字体/高度，且支持点击标题栏或箭头折叠、展开内容区。
*
*   XML 属性（除 Box/Control 通用属性外）：
*     title             标题文字
*     title_height      标题栏高度（像素，默认 28，DPI 自适应）
*     title_bk_color    标题栏背景色（语义色名，不设置则不绘制背景）
*     title_text_color  标题文字颜色（语义色名，默认 text_default）
*     title_text_align  标题文字水平对齐："left"（默认）/ "hcenter" / "right"
*     title_font        标题字体 ID（global.xml 中的 Font id，默认 system_bold_14）
*     collapsible       是否可折叠（true/false，默认 false）
*     collapsed         初始是否处于折叠状态（true/false，默认 false；初始设置不播放动画）
*     collapse_trigger  折叠触发热区："title"（整个标题栏，默认）或 "arrow"（仅箭头区域）
*     collapse_anim     折叠/展开动画时长（毫秒，默认 0 表示无动画，建议 150~300）
*     arrow_align       箭头位置："right"（标题栏右侧，默认）或 "left"（标题栏左侧）
*     group             手风琴分组名：同组面板同时只允许一个展开，展开某面板时其余自动折叠
*                       （作用域为当前窗口；允许全部折叠；初始状态请在 XML 中只标记一个展开）
*     title_slot        标题栏槽位：一个子控件的 name，该控件（建议 float="true"）会被自动
*                       定位到标题栏右侧（箭头左侧），用于放置按钮/复选框等任意控件
*     arrow_expanded_normal_image / arrow_expanded_hovered_image /
*     arrow_expanded_pushed_image / arrow_expanded_disabled_image
*                       展开状态（▼）箭头在各控件状态下的图片
*     arrow_collapsed_normal_image / arrow_collapsed_hovered_image /
*     arrow_collapsed_pushed_image / arrow_collapsed_disabled_image
*                       折叠状态（▶）箭头在各控件状态下的图片
*                       （未配置图片时使用默认字形箭头；某状态缺图时自动回退 normal 图）
*
*   布局说明：
*     1. 标题栏空间由控件自动在内容区顶部预留；padding 属性保持标准语义，
*        设置的是"内容区相对标题栏下方区域"的内边距，可正常使用。
*     2. 折叠时不修改自身高度模式（fixed/auto/stretch 均安全）、不修改子控件 visible
*        状态，而是通过测量、绘制、命中测试三处屏蔽内容区，展开后无任何副作用。
*
*   事件：折叠/展开完成时分别触发 kEventCollapse / kEventExpand（无动画时立即触发，
*   有动画时在动画结束时触发），可用 AttachCollapse / AttachExpand 监听。
*   折叠/展开前分别触发 kEventPanelCollapsing / kEventPanelExpanding，监听回调返回
*   false 可取消本次操作（XML 初始 collapsed 属性与手风琴内部联动不触发取消事件）。
*/
template<typename InheritType = Box>
class PanelTemplate : public InheritType, public IPanelGroup
{
    typedef InheritType BaseClass;
public:
    explicit PanelTemplate(Window* pWindow);
    virtual ~PanelTemplate() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual UiEstSize EstimateSize(UiSize szAvailable) override;
    virtual void SetPos(UiRect rc) override;
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;
    virtual void PaintText(IRender* pRender) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual Control* FindControl(FINDCONTROLPROC Proc, void* pProcData, uint32_t uFlags,
                                 const UiPoint& ptMouse, const UiPoint& scrollPos) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

public:
    /** 设置标题文字
    */
    void SetTitle(const DString& strTitle);

    /** 获取标题文字
    */
    const DString& GetTitle() const;

    /** 设置标题栏高度（像素）
    * @param [in] nHeight 标题栏高度
    * @param [in] bNeedDpiScale 是否按 DPI 缩放
    */
    void SetTitleBarHeight(int32_t nHeight, bool bNeedDpiScale);

    /** 获取标题栏高度（像素，已按 DPI 缩放后的值）
    */
    int32_t GetTitleBarHeight() const;

    /** 设置标题栏背景色（语义色名）
    */
    void SetTitleBkColor(const DString& strColor);

    /** 设置标题文字颜色（语义色名）
    */
    void SetTitleTextColor(const DString& strColor);

    /** 设置标题文字水平对齐方式
    * @param [in] uHAlign TEXT_LEFT（默认）/ TEXT_HCENTER / TEXT_RIGHT
    */
    void SetTitleTextHAlign(uint32_t uHAlign);

    /** 设置标题字体 ID（global.xml 中的 Font id）
    */
    void SetTitleFontId(const DString& strFontId);

    /** 设置是否可折叠
    */
    void SetCollapsible(bool bCollapsible);

    /** 是否可折叠
    */
    bool IsCollapsible() const;

    /** 设置折叠状态
    * @param [in] bCollapsed true 折叠（只显示标题栏），false 展开
    * @param [in] bFireEvent 是否触发 kEventCollapse / kEventExpand 事件
    * @param [in] bPlayAnim 是否播放折叠/展开动画，false 时立即切换到最终状态
    */
    void SetCollapsed(bool bCollapsed, bool bFireEvent = true, bool bPlayAnim = true);

    /** 是否处于折叠状态
    */
    bool IsCollapsed() const;

    /** 折叠触发热区类型
    */
    enum class CollapseTrigger
    {
        kTitle, ///< 整个标题栏可点击（默认）
        kArrow  ///< 仅箭头区域可点击
    };

    /** 箭头在标题栏中的位置
    */
    enum class ArrowAlign
    {
        kRight, ///< 箭头在标题栏右侧（默认）
        kLeft   ///< 箭头在标题栏左侧（文字右侧）
    };

    /** 设置箭头位置
    */
    void SetArrowAlign(ArrowAlign align);

    /** 获取箭头位置
    */
    ArrowAlign GetArrowAlign() const;

    /** 设置折叠触发热区
    */
    void SetCollapseTrigger(CollapseTrigger trigger);

    /** 获取折叠触发热区
    */
    CollapseTrigger GetCollapseTrigger() const;

    /** 设置折叠/展开动画时长
    * @param [in] nMillSeconds 动画时长（毫秒），0 表示无动画，立即切换
    */
    void SetCollapseAnimMillSeconds(int32_t nMillSeconds);

    /** 获取折叠/展开动画时长（毫秒）
    */
    int32_t GetCollapseAnimMillSeconds() const;

    /** 设置手风琴分组名
    * @param [in] strGroupName 分组名，空字符串表示退出分组
    */
    void SetGroup(const DString& strGroupName);

    /** 获取手风琴分组名
    */
    DString GetGroup() const;

    //IPanelGroup 接口实现（供同组面板联动，遍历控件树时统一识别三种 Panel 类型）
    virtual DString GetPanelGroupName() const override;
    virtual bool IsPanelCollapsed() const override;
    virtual void SetPanelCollapsed(bool bCollapsed, bool bFireEvent) override;

    /** 设置内容区内边距（标题栏空间由控件自动预留，padding 保持标准语义）
    * @note 与 PlaceHolder::SetPadding 同名隐藏：通过 Panel 类型指针调用时生效；
    *       XML 的 padding 属性也会进入此实现
    */
    void SetPadding(UiPadding rcPadding, bool bNeedDpiScale);

    /** 监听展开事件
    */
    void AttachExpand(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventExpand, callback, callbackID); }

    /** 监听折叠事件
    */
    void AttachCollapse(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventCollapse, callback, callbackID); }

    /** 监听"即将折叠"事件：回调返回 false 可取消本次折叠
    */
    void AttachCollapsing(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventPanelCollapsing, callback, callbackID); }

    /** 监听"即将展开"事件：回调返回 false 可取消本次展开
    */
    void AttachExpanding(const EventCallback& callback, EventCallbackID callbackID = 0) { this->AttachEvent(kEventPanelExpanding, callback, callbackID); }

    /** 设置箭头状态图片
    * @param [in] bExpanded true 设置展开态（▼）箭头图，false 设置折叠态（▶）箭头图
    * @param [in] stateType 控件状态（普通/悬停/按下/禁用）
    * @param [in] strImage 图片属性字符串（file='...' width='..' height='..' 等）
    */
    void SetArrowStateImage(bool bExpanded, ControlStateType stateType, const DString& strImage);

    /** 获取箭头状态图片属性字符串
    */
    DString GetArrowStateImage(bool bExpanded, ControlStateType stateType) const;

    /** 设置标题栏槽位：值为子控件的 name，该子控件会被定位到标题栏右侧（箭头左侧）
    * @note 槽位子控件建议设置 float="true"，避免其参与正文区布局占位
    */
    void SetTitleSlotName(const DString& strSlotName);

    /** 获取标题栏槽位子控件的 name
    */
    DString GetTitleSlotName() const;

private:
    /** 应用内容区 padding（用户 padding + 标题栏高度），写入基类的 padding 存储
    */
    void ApplyContentPadding();

    /** 切换折叠状态（标题栏点击时调用）
    */
    void ToggleCollapsed();

    /** 获取标题栏区域（相对窗口的坐标）
    */
    UiRect GetTitleBarRect() const;

    /** 获取箭头热区（标题栏右侧正方形区域）
    */
    UiRect GetArrowRect() const;

    /** 停止当前折叠动画（不触发完成事件，用于反向切换或控件状态变更）
    */
    void StopCollapseAnim();

    /** 进入折叠布局：临时把最小高度放宽到标题栏高度
    * 布局器会用 min_height 钳制 EstimateSize，不放宽则折叠后仍占 min_height 高度
    */
    void BeginCollapseLayout();

    /** 退出折叠布局：恢复用户配置的最小高度
    */
    void EndCollapseLayout();

    /** 启动折叠/展开动画
    * @param [in] bCollapse true 折叠（展开高度 → 标题栏高度），false 展开（反向）
    * @param [in] nFromHeight 动画起始高度（像素）
    * @param [in] bFireEvent 动画完成后是否触发 kEventCollapse / kEventExpand
    */
    void StartCollapseAnim(bool bCollapse, int32_t nFromHeight, bool bFireEvent);

    /** 估算展开状态下的完整内容高度（像素）
    * @param [in] nFallbackHeight 无法从布局估算时（如 stretch 模式）的兜底高度
    */
    int32_t EstimateExpandedHeight(int32_t nFallbackHeight);

    /** 动画帧回调：应用当前动画高度并请求重新布局
    */
    void OnAnimHeightChanged(int32_t nHeight);

    /** 动画完成：退出动画态，按逻辑折叠状态做最终布局
    * @param [in] bFireEvent 是否触发 kEventCollapse / kEventExpand
    */
    void OnCollapseAnimFinished(bool bFireEvent);

    /** 手风琴联动：折叠当前窗口内同组的其他展开面板
    * @param [in] bFireEvent 折叠其他面板时是否触发其 kEventCollapse 事件
    */
    void CollapseGroupPeers(bool bFireEvent);

    /** 折叠/展开的内部实现（不触发"即将"取消事件，供手风琴联动等内部路径调用）
    */
    void ApplyCollapsed(bool bCollapsed, bool bFireEvent, bool bPlayAnim = true);

    /** 触发"即将折叠/展开"取消事件
    * @return false 表示有监听者要求取消本次操作
    */
    bool FireBeforeCollapseEvent(bool bCollapsed);

    /** 查找标题栏槽位子控件（带缓存，槽位销毁后缓存自动失效并重查）
    */
    Control* GetTitleSlotControl();

    /** 将标题栏槽位子控件定位到标题栏右侧（箭头左侧）
    */
    void LayoutTitleSlot();

    /** 绘制箭头图片（配置了状态图片时替代默认字形）
    * @return true 已绘制图片；false 未配置图片，应回退绘制字形
    */
    bool PaintArrowImage(IRender* pRender, const UiRect& rcArrow);

private:
    //标题文字
    UiString m_title;

    //标题栏高度（像素，已按 DPI 缩放）
    int32_t m_nTitleBarHeight;

    //标题栏背景色
    UiString m_titleBkColor;

    //标题文字颜色
    UiString m_titleTextColor;

    //标题文字水平对齐方式（TEXT_LEFT / TEXT_HCENTER / TEXT_RIGHT）
    uint32_t m_uTitleTextHAlign;

    //标题字体 ID
    UiString m_titleFontId;

    //箭头位置（默认右侧）
    ArrowAlign m_arrowAlign;

    //是否可折叠
    bool m_bCollapsible;

    //是否处于折叠状态
    bool m_bCollapsed;

    //折叠触发热区
    CollapseTrigger m_collapseTrigger;

    //用户设置的内容区 padding（已按 DPI 缩放，不含标题栏预留空间）
    UiPadding m_contentPadding;

    //折叠/展开动画时长（毫秒），0 表示无动画
    int32_t m_nCollapseAnimMs;

    //动画播放器（仅在动画进行时非空）
    std::unique_ptr<AnimationPlayer> m_pAnimationPlayer;

    //动画进行中的当前高度（像素）；-1 表示未在播放动画
    int32_t m_nAnimHeight;

    //最近一次完全展开时的实际高度（像素），作为折叠动画起点与展开兜底目标
    int32_t m_nExpandedHeight;

    //折叠期间临时保存的用户最小高度（像素）；-1 表示当前未处于折叠布局
    //折叠时布局器仍会用 min_height 钳制 EstimateSize 结果，需临时放宽到标题栏高度
    int32_t m_nSavedMinHeight;

    //手风琴分组名（空表示不属于任何分组）
    UiString m_groupName;

    //标题栏槽位子控件的 name
    UiString m_titleSlotName;

    //标题栏槽位子控件（弱引用缓存，控件销毁后自动置空）
    ControlPtr m_pTitleSlot;
};

template<typename InheritType>
PanelTemplate<InheritType>::PanelTemplate(Window* pWindow):
    InheritType(pWindow),
    m_nTitleBarHeight(0),
    m_uTitleTextHAlign(TEXT_LEFT),
    m_arrowAlign(ArrowAlign::kRight),
    m_bCollapsible(false),
    m_bCollapsed(false),
    m_collapseTrigger(CollapseTrigger::kTitle),
    m_nCollapseAnimMs(0),
    m_nAnimHeight(-1),
    m_nExpandedHeight(0),
    m_nSavedMinHeight(-1)
{
    this->SetMouseEnabled(true);
    SetTitleBarHeight(28, true);
}

template<typename InheritType>
PanelTemplate<InheritType>::~PanelTemplate()
{
    //析构前停止动画定时器，避免定时器回调访问已析构对象（回调本身有弱引用保护）
    if (m_pAnimationPlayer != nullptr) {
        m_pAnimationPlayer->SetCompleteCallback(nullptr);
        m_pAnimationPlayer->SetPlayCallback(nullptr);
        m_pAnimationPlayer->Stop();
    }
}

template<typename InheritType>
inline DString PanelTemplate<InheritType>::GetType() const { return DUI_CTR_PANEL; }

template<>
inline DString PanelTemplate<HBox>::GetType() const { return DUI_CTR_PANEL_HBOX; }

template<>
inline DString PanelTemplate<VBox>::GetType() const { return DUI_CTR_PANEL_VBOX; }

template<typename InheritType>
void PanelTemplate<InheritType>::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = this->GetExpandVarStrings(strValue2);
    if (strName == _T("title")) {
        SetTitle(strValue);
    }
    else if (strName == _T("title_height")) {
        //标题栏高度
        SetTitleBarHeight(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("title_bk_color")) {
        //标题栏背景色
        SetTitleBkColor(strValue);
    }
    else if (strName == _T("title_text_color")) {
        //标题文字颜色
        SetTitleTextColor(strValue);
    }
    else if (strName == _T("title_text_align")) {
        //标题文字水平对齐：left（默认）/ hcenter / right
        if (strValue == _T("hcenter")) {
            SetTitleTextHAlign(TEXT_HCENTER);
        }
        else if (strValue == _T("right")) {
            SetTitleTextHAlign(TEXT_RIGHT);
        }
        else {
            SetTitleTextHAlign(TEXT_LEFT);
        }
    }
    else if (strName == _T("arrow_align")) {
        //箭头位置：right（默认）/ left
        SetArrowAlign(strValue == _T("left") ? ArrowAlign::kLeft : ArrowAlign::kRight);
    }
    else if (strName == _T("title_font")) {
        //标题字体 ID
        SetTitleFontId(strValue);
    }
    else if (strName == _T("collapsible")) {
        //是否可折叠
        SetCollapsible(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("collapsed")) {
        //初始折叠状态（不触发事件、不播放动画；此时子控件可能尚未解析完成，
        //折叠状态由测量/绘制/命中测试统一处理）
        SetCollapsed(StringUtil::IsValueTrue(strValue), false, false);
    }
    else if (strName == _T("collapse_trigger")) {
        //折叠触发热区
        if (strValue == _T("arrow")) {
            SetCollapseTrigger(CollapseTrigger::kArrow);
        }
        else {
            SetCollapseTrigger(CollapseTrigger::kTitle);
        }
    }
    else if (strName == _T("collapse_anim")) {
        //折叠/展开动画时长（毫秒，0 关闭）
        SetCollapseAnimMillSeconds(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("group")) {
        //手风琴分组名
        SetGroup(strValue);
    }
    else if (strName == _T("title_slot")) {
        //标题栏槽位子控件 name
        SetTitleSlotName(strValue);
    }
    else if (strName == _T("arrow_expanded_normal_image")) {
        SetArrowStateImage(true, kControlStateNormal, strValue);
    }
    else if ((strName == _T("arrow_expanded_hovered_image")) || (strName == _T("arrow_expanded_hot_image"))) {
        SetArrowStateImage(true, kControlStateHovered, strValue);
    }
    else if ((strName == _T("arrow_expanded_pushed_image")) || (strName == _T("arrow_expanded_pressed_image"))) {
        SetArrowStateImage(true, kControlStatePressed, strValue);
    }
    else if (strName == _T("arrow_expanded_disabled_image")) {
        SetArrowStateImage(true, kControlStateDisabled, strValue);
    }
    else if (strName == _T("arrow_collapsed_normal_image")) {
        SetArrowStateImage(false, kControlStateNormal, strValue);
    }
    else if ((strName == _T("arrow_collapsed_hovered_image")) || (strName == _T("arrow_collapsed_hot_image"))) {
        SetArrowStateImage(false, kControlStateHovered, strValue);
    }
    else if ((strName == _T("arrow_collapsed_pushed_image")) || (strName == _T("arrow_collapsed_pressed_image"))) {
        SetArrowStateImage(false, kControlStatePressed, strValue);
    }
    else if (strName == _T("arrow_collapsed_disabled_image")) {
        SetArrowStateImage(false, kControlStateDisabled, strValue);
    }
    else if (strName == _T("padding")) {
        //padding 保持标准语义：内容区内边距（标题栏空间自动额外预留）
        UiPadding rcPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcPadding);
        SetPadding(rcPadding, true);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetTitle(const DString& strTitle)
{
    if (m_title != strTitle) {
        m_title = strTitle;
        this->Invalidate();
    }
}

template<typename InheritType>
const DString& PanelTemplate<InheritType>::GetTitle() const
{
    return m_title.c_str();
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetTitleBarHeight(int32_t nHeight, bool bNeedDpiScale)
{
    if (nHeight < 0) {
        nHeight = 0;
    }
    if (bNeedDpiScale) {
        nHeight = this->Dpi().GetScaleInt(nHeight);
    }
    if (m_nTitleBarHeight != nHeight) {
        m_nTitleBarHeight = nHeight;
        ApplyContentPadding();
        this->ArrangeAncestor();
        this->Invalidate();
    }
}

template<typename InheritType>
int32_t PanelTemplate<InheritType>::GetTitleBarHeight() const
{
    return m_nTitleBarHeight;
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetTitleBkColor(const DString& strColor)
{
    if (m_titleBkColor != strColor) {
        m_titleBkColor = strColor;
        this->Invalidate();
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetTitleTextColor(const DString& strColor)
{
    if (m_titleTextColor != strColor) {
        m_titleTextColor = strColor;
        this->Invalidate();
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetTitleTextHAlign(uint32_t uHAlign)
{
    if ((uHAlign != TEXT_LEFT) && (uHAlign != TEXT_HCENTER) && (uHAlign != TEXT_RIGHT)) {
        uHAlign = TEXT_LEFT;
    }
    if (m_uTitleTextHAlign != uHAlign) {
        m_uTitleTextHAlign = uHAlign;
        this->Invalidate();
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetArrowAlign(ArrowAlign align)
{
    if (m_arrowAlign != align) {
        m_arrowAlign = align;
        //箭头热区位置变化会影响槽位与标题文字布局，需要重新布局
        this->ArrangeAncestor();
        this->Invalidate();
    }
}

template<typename InheritType>
typename PanelTemplate<InheritType>::ArrowAlign PanelTemplate<InheritType>::GetArrowAlign() const
{
    return m_arrowAlign;
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetTitleFontId(const DString& strFontId)
{
    if (m_titleFontId != strFontId) {
        m_titleFontId = strFontId;
        this->Invalidate();
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetCollapsible(bool bCollapsible)
{
    if (m_bCollapsible != bCollapsible) {
        m_bCollapsible = bCollapsible;
        //切换可折叠能力时终止进行中的动画，避免动画与最终状态不一致
        StopCollapseAnim();
        //切换可折叠能力后重新布局：若此前已记录 collapsed=true，启用后直接呈现折叠态；
        //禁用后恢复展开态，不再屏蔽内容区
        if (bCollapsible && m_bCollapsed) {
            BeginCollapseLayout();
        }
        else if (!bCollapsible) {
            EndCollapseLayout();
        }
        this->ArrangeAncestor();
        this->Invalidate();
    }
}

template<typename InheritType>
bool PanelTemplate<InheritType>::IsCollapsible() const
{
    return m_bCollapsible;
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetCollapsed(bool bCollapsed, bool bFireEvent, bool bPlayAnim)
{
    if (m_bCollapsed == bCollapsed) {
        //目标状态相同（无论是否正在播放动画）都忽略；只有方向相反时才在动画中途反向
        return;
    }
    //允许在 collapsible 设置之前记录初始折叠状态（与 XML 属性书写顺序无关）；
    //实际折叠行为统一由 (m_bCollapsible && m_bCollapsed) 判定
    if (!m_bCollapsible) {
        m_bCollapsed = bCollapsed;
        return;
    }

    //折叠/展开前的可取消事件（XML 初始 collapsed 属性 bFireEvent=false 时不触发，
    //此时控件树可能尚未解析完成）；任一监听者返回 false 即放弃本次操作
    if (bFireEvent && !FireBeforeCollapseEvent(bCollapsed)) {
        return;
    }
    ApplyCollapsed(bCollapsed, bFireEvent, bPlayAnim);
}

template<typename InheritType>
void PanelTemplate<InheritType>::ApplyCollapsed(bool bCollapsed, bool bFireEvent, bool bPlayAnim)
{
    m_bCollapsed = bCollapsed;

    //手风琴联动：本面板展开时，折叠同窗口内同组的其他面板（折叠动作不递归触发联动/取消事件）
    if (!bCollapsed) {
        CollapseGroupPeers(bFireEvent);
    }

    if (!bPlayAnim || (m_nCollapseAnimMs <= 0) || (m_nTitleBarHeight <= 0)) {
        //无动画：立即切换
        StopCollapseAnim();
        if (bCollapsed) {
            BeginCollapseLayout();
        }
        else {
            EndCollapseLayout();
        }
        this->ArrangeAncestor();
        this->Invalidate();
        if (bFireEvent) {
            this->SendEvent(bCollapsed ? kEventCollapse : kEventExpand);
        }
        return;
    }

    //折叠动画开始前放宽最小高度，避免动画收缩到 min_height 即被钳制（展开方向在完成时恢复）
    if (bCollapsed) {
        BeginCollapseLayout();
    }

    //动画：以当前呈现高度为起点（支持动画中途反向），目标高度由方向决定
    int32_t nFromHeight = m_nTitleBarHeight;
    if (m_pAnimationPlayer != nullptr && m_pAnimationPlayer->IsPlaying()) {
        nFromHeight = (m_nAnimHeight >= 0) ? m_nAnimHeight : m_nTitleBarHeight;
        StopCollapseAnim();
    }
    else {
        UiRect rc = this->GetRect();
        if (!rc.IsEmpty()) {
            nFromHeight = rc.Height();
        }
        else if (m_nExpandedHeight > m_nTitleBarHeight) {
            nFromHeight = m_nExpandedHeight;
        }
    }
    StartCollapseAnim(bCollapsed, nFromHeight, bFireEvent);
}

template<typename InheritType>
bool PanelTemplate<InheritType>::IsCollapsed() const
{
    return m_bCollapsed;
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetCollapseTrigger(CollapseTrigger trigger)
{
    m_collapseTrigger = trigger;
}

template<typename InheritType>
typename PanelTemplate<InheritType>::CollapseTrigger PanelTemplate<InheritType>::GetCollapseTrigger() const
{
    return m_collapseTrigger;
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetPadding(UiPadding rcPadding, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        this->Dpi().ScalePadding(rcPadding);
    }
    m_contentPadding = rcPadding;
    ApplyContentPadding();
}

template<typename InheritType>
void PanelTemplate<InheritType>::ApplyContentPadding()
{
    //标题栏通过顶部 padding 预留空间，Layout 排布子控件时自动避开标题栏区域
    UiPadding rcPadding = m_contentPadding;
    rcPadding.top += m_nTitleBarHeight;
    //显式调用基类实现写入 padding 存储（SetPadding 在本类中被同名隐藏）
    InheritType::SetPadding(rcPadding, false);
}

template<typename InheritType>
void PanelTemplate<InheritType>::ToggleCollapsed()
{
    SetCollapsed(!m_bCollapsed);
}

template<typename InheritType>
UiRect PanelTemplate<InheritType>::GetTitleBarRect() const
{
    UiRect rc = this->GetRect();
    rc.bottom = rc.top + m_nTitleBarHeight;
    return rc;
}

template<typename InheritType>
UiRect PanelTemplate<InheritType>::GetArrowRect() const
{
    UiRect rcTitle = GetTitleBarRect();
    //箭头区域为正方形，尺寸等于标题栏高度
    if (m_arrowAlign == ArrowAlign::kLeft) {
        rcTitle.right = rcTitle.left + m_nTitleBarHeight;
    }
    else {
        rcTitle.left = rcTitle.right - m_nTitleBarHeight;
    }
    return rcTitle;
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetCollapseAnimMillSeconds(int32_t nMillSeconds)
{
    if (nMillSeconds < 0) {
        nMillSeconds = 0;
    }
    m_nCollapseAnimMs = nMillSeconds;
}

template<typename InheritType>
int32_t PanelTemplate<InheritType>::GetCollapseAnimMillSeconds() const
{
    return m_nCollapseAnimMs;
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetGroup(const DString& strGroupName)
{
    m_groupName = strGroupName;
}

template<typename InheritType>
DString PanelTemplate<InheritType>::GetGroup() const
{
    return m_groupName.c_str();
}

template<typename InheritType>
DString PanelTemplate<InheritType>::GetPanelGroupName() const
{
    return m_groupName.c_str();
}

template<typename InheritType>
bool PanelTemplate<InheritType>::IsPanelCollapsed() const
{
    return m_bCollapsible && m_bCollapsed;
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetPanelCollapsed(bool bCollapsed, bool bFireEvent)
{
    //手风琴内部联动路径：绕过取消事件，直接执行
    if (m_bCollapsed == bCollapsed) {
        return;
    }
    if (!m_bCollapsible) {
        return;
    }
    ApplyCollapsed(bCollapsed, bFireEvent);
}

namespace PanelGroupDetail
{
    /** 递归遍历控件树，折叠与指定分组同名的其他展开面板
    */
    inline void CollapsePeersRecursive(Control* pControl, const IPanelGroup* pSelf,
                                       const DString& strGroupName, bool bFireEvent)
    {
        if (pControl == nullptr) {
            return;
        }
        IPanelGroup* pPanel = dynamic_cast<IPanelGroup*>(pControl);
        if ((pPanel != nullptr) && (pPanel != pSelf)
            && (pPanel->GetPanelGroupName() == strGroupName)
            && !pPanel->IsPanelCollapsed()) {
            //折叠其他面板：SetCollapsed(true) 不会再次触发同组联动，无递归
            pPanel->SetPanelCollapsed(true, bFireEvent);
        }
        Box* pBox = dynamic_cast<Box*>(pControl);
        if (pBox != nullptr) {
            const size_t nCount = pBox->GetItemCount();
            for (size_t i = 0; i < nCount; ++i) {
                CollapsePeersRecursive(pBox->GetItemAt(i), pSelf, strGroupName, bFireEvent);
            }
        }
    }
} //namespace PanelGroupDetail

template<typename InheritType>
void PanelTemplate<InheritType>::CollapseGroupPeers(bool bFireEvent)
{
    if (m_groupName.empty()) {
        return;
    }
    Window* pWindow = this->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    Box* pRoot = pWindow->GetRoot();
    if (pRoot != nullptr) {
        PanelGroupDetail::CollapsePeersRecursive(pRoot, this, m_groupName.c_str(), bFireEvent);
    }
}

template<typename InheritType>
bool PanelTemplate<InheritType>::FireBeforeCollapseEvent(bool bCollapsed)
{
    //派发"即将折叠/展开"事件：所有监听者都返回 true 才放行
    //（不走 HandleEvent，这两个事件没有框架内部处理逻辑）
    EventArgs msg;
    msg.eventType = bCollapsed ? kEventPanelCollapsing : kEventPanelExpanding;
    msg.SetSender(this);
    Window* pWindow = this->GetWindow();
    if (pWindow != nullptr) {
        msg.ptMouse = pWindow->GetLastMousePos();
    }
    return this->FireAllEvents(msg);
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetArrowStateImage(bool bExpanded, ControlStateType stateType, const DString& strImage)
{
    this->SetStateImage(bExpanded ? kStateImagePanelArrowExpanded : kStateImagePanelArrowCollapsed,
                        stateType, strImage);
    this->Invalidate();
}

template<typename InheritType>
DString PanelTemplate<InheritType>::GetArrowStateImage(bool bExpanded, ControlStateType stateType) const
{
    return this->GetStateImage(bExpanded ? kStateImagePanelArrowExpanded : kStateImagePanelArrowCollapsed,
                               stateType);
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetTitleSlotName(const DString& strSlotName)
{
    if (m_titleSlotName != strSlotName) {
        m_titleSlotName = strSlotName;
        //槽位 name 变化后清空缓存，下次布局时重新查找
        m_pTitleSlot = nullptr;
        this->ArrangeAncestor();
    }
}

template<typename InheritType>
DString PanelTemplate<InheritType>::GetTitleSlotName() const
{
    return m_titleSlotName.c_str();
}

template<typename InheritType>
Control* PanelTemplate<InheritType>::GetTitleSlotControl()
{
    if (m_titleSlotName.empty()) {
        return nullptr;
    }
    //弱引用缓存：控件被销毁后自动为空，重新按 name 查找
    if (m_pTitleSlot == nullptr) {
        Control* pSlot = this->FindSubControl(m_titleSlotName.c_str());
        if (pSlot != nullptr) {
            m_pTitleSlot = ControlPtr(pSlot);
        }
    }
    return m_pTitleSlot.get();
}

template<typename InheritType>
void PanelTemplate<InheritType>::LayoutTitleSlot()
{
    Control* pSlot = GetTitleSlotControl();
    if (pSlot == nullptr) {
        return;
    }
    UiRect rcTitle = GetTitleBarRect();
    if (rcTitle.IsEmpty()) {
        return;
    }
    //槽位垂直居中贴右侧；箭头在右侧时才需要避让到箭头左侧
    const int32_t nMargin = this->Dpi().GetScaleInt(6);
    int32_t nRight = rcTitle.right - nMargin;
    if (m_bCollapsible && (m_arrowAlign == ArrowAlign::kRight)) {
        nRight = GetArrowRect().left;
    }

    UiSize szAvailable(rcTitle.Width(), rcTitle.Height());
    UiEstSize estSize = pSlot->EstimateSize(szAvailable);
    int32_t nWidth = estSize.cx.IsInt32() ? estSize.cx.GetInt32() : (rcTitle.Height() * 2);
    int32_t nHeight = estSize.cy.IsInt32() ? estSize.cy.GetInt32() : rcTitle.Height();
    nWidth = std::min(nWidth, nRight - rcTitle.left - nMargin);
    nHeight = std::min(nHeight, rcTitle.Height() - 2);
    if ((nWidth <= 0) || (nHeight <= 0)) {
        return;
    }

    UiRect rcSlot;
    rcSlot.right = nRight;
    rcSlot.left = nRight - nWidth;
    rcSlot.top = rcTitle.CenterY() - nHeight / 2;
    rcSlot.bottom = rcSlot.top + nHeight;
    pSlot->SetPos(rcSlot);
}

template<typename InheritType>
bool PanelTemplate<InheritType>::PaintArrowImage(IRender* pRender, const UiRect& rcArrow)
{
    //动画结束才切换箭头方向（与字形箭头行为一致）
    const bool bExpanded = !(m_bCollapsed && (m_nAnimHeight < 0));
    const StateImageType imageType = bExpanded ? kStateImagePanelArrowExpanded
                                               : kStateImagePanelArrowCollapsed;
    if (!this->HasStateImage(imageType)) {
        return false;
    }
    //当前控件状态（悬停标题栏时为 Hovered）；某状态未配图时回退普通态
    ControlStateType stateType = this->GetState();
    if (!this->IsEnabled()) {
        stateType = kControlStateDisabled;
    }
    if (this->GetStateImage(imageType, stateType).empty() &&
        !this->GetStateImage(imageType, kControlStateNormal).empty()) {
        stateType = kControlStateNormal;
    }

    //以图片自然尺寸为准，在箭头热区内居中，且不超出热区
    UiSize imageSize = this->GetStateImageSize(imageType, stateType);
    int32_t nW = imageSize.cx;
    int32_t nH = imageSize.cy;
    if ((nW <= 0) || (nH <= 0)) {
        nW = nH = rcArrow.Height() - this->Dpi().GetScaleInt(8);
    }
    const int32_t nMaxSize = rcArrow.Height() - this->Dpi().GetScaleInt(4);
    if ((nW > nMaxSize) || (nH > nMaxSize)) {
        const double dScale = static_cast<double>(nMaxSize) / std::max(nW, nH);
        nW = static_cast<int32_t>(nW * dScale);
        nH = static_cast<int32_t>(nH * dScale);
    }
    UiRect rcDest;
    rcDest.left = rcArrow.CenterX() - nW / 2;
    rcDest.right = rcDest.left + nW;
    rcDest.top = rcArrow.CenterY() - nH / 2;
    rcDest.bottom = rcDest.top + nH;
    //必须显式传入目标矩形：PaintStateImage 会忽略外部矩形、按图片自身 align 相对整个控件布局，
    //导致箭头被画到面板左侧而非标题栏右侧
    Image* pImage = this->GetStateImageData(imageType, stateType);
    if (pImage == nullptr) {
        return false;
    }
    return this->PaintImage(pRender, pImage, _T(""), -1, nullptr, &rcDest, nullptr);
}

template<typename InheritType>
int32_t PanelTemplate<InheritType>::EstimateExpandedHeight(int32_t nFallbackHeight)
{
    //以当前宽度为约束、充足的可用高度做一次展开态测量
    UiRect rc = this->GetRect();
    int32_t nWidth = rc.Width();
    if (nWidth <= 0) {
        nWidth = INT32_MAX / 2;
    }
    UiSize szAvailable(nWidth, INT32_MAX / 2);
    UiEstSize estSize = BaseClass::EstimateSize(szAvailable);
    if (estSize.cy.IsInt32()) {
        int32_t nHeight = estSize.cy.GetInt32();
        if (nHeight >= m_nTitleBarHeight) {
            return nHeight;
        }
    }
    //stretch 等模式测量结果不是确定像素值时，使用兜底高度
    return nFallbackHeight > m_nTitleBarHeight ? nFallbackHeight : m_nTitleBarHeight;
}

template<typename InheritType>
void PanelTemplate<InheritType>::OnAnimHeightChanged(int32_t nHeight)
{
    m_nAnimHeight = nHeight;
    //请求父级重新布局：EstimateSize 会返回当前动画高度，面板随之平滑收缩/展开
    this->ArrangeAncestor();
    this->Invalidate();
}

template<typename InheritType>
void PanelTemplate<InheritType>::OnCollapseAnimFinished(bool bFireEvent)
{
    m_nAnimHeight = -1;
    m_pAnimationPlayer.reset();
    //展开结束后恢复用户最小高度（折叠态保持放宽，否则静止高度仍会被 min_height 钳制）
    if (!m_bCollapsed) {
        EndCollapseLayout();
    }
    //动画结束后做一次最终布局，恢复自然测量结果（fixed/auto/stretch 均无副作用）
    this->ArrangeAncestor();
    this->Invalidate();
    if (bFireEvent) {
        this->SendEvent(m_bCollapsed ? kEventCollapse : kEventExpand);
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::StopCollapseAnim()
{
    if (m_pAnimationPlayer == nullptr) {
        return;
    }
    m_pAnimationPlayer->SetCompleteCallback(nullptr);
    m_pAnimationPlayer->SetPlayCallback(nullptr);
    m_pAnimationPlayer->Stop();
    m_pAnimationPlayer.reset();
    m_nAnimHeight = -1;
}

template<typename InheritType>
void PanelTemplate<InheritType>::BeginCollapseLayout()
{
    if (m_nSavedMinHeight < 0) {
        m_nSavedMinHeight = this->GetMinHeight();
        if (m_nSavedMinHeight > m_nTitleBarHeight) {
            this->SetMinHeight(m_nTitleBarHeight, false);
        }
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::EndCollapseLayout()
{
    if (m_nSavedMinHeight >= 0) {
        this->SetMinHeight(m_nSavedMinHeight, false);
        m_nSavedMinHeight = -1;
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::StartCollapseAnim(bool bCollapse, int32_t nFromHeight, bool bFireEvent)
{
    int32_t nToHeight = bCollapse ? m_nTitleBarHeight : EstimateExpandedHeight(m_nExpandedHeight);
    if (nToHeight == nFromHeight) {
        //起止高度相同，无需动画，直接落到最终状态
        m_nAnimHeight = -1;
        if (!bCollapse) {
            EndCollapseLayout();
        }
        this->ArrangeAncestor();
        this->Invalidate();
        if (bFireEvent) {
            this->SendEvent(bCollapse ? kEventCollapse : kEventExpand);
        }
        return;
    }

    //先以起始高度占位，避免首帧定时器触发前出现高度跳变
    m_nAnimHeight = nFromHeight;
    this->ArrangeAncestor();

    AnimationPlayer* pAnimationPlayer = new AnimationPlayer;
    m_pAnimationPlayer.reset(pAnimationPlayer);
    pAnimationPlayer->SetAnimationType(AnimationType::kAnimationNone);
    pAnimationPlayer->SetTotalMillSeconds(m_nCollapseAnimMs);
    pAnimationPlayer->SetFrameIntervalMillSeconds(16); //约 60 帧/秒
    //折叠用缓入（越收越快），展开用缓出（越展越慢），符合常见手感动效
    pAnimationPlayer->SetEasingFunctionType(bCollapse ? EaseInCubic : EaseOutCubic);
    pAnimationPlayer->SetStartValue(nFromHeight);
    pAnimationPlayer->SetEndValue(nToHeight);

    ControlPtrT<PanelTemplate> pPanel(this);
    AnimationPlayCallback playCallback = [pPanel](int32_t nNewValue) {
        if (pPanel == nullptr) {
            return;
        }
        pPanel->OnAnimHeightChanged(nNewValue);
    };
    pAnimationPlayer->SetPlayCallback(playCallback);

    AnimationCompleteCallback completeCallback = [pPanel, bFireEvent]() {
        if (pPanel == nullptr) {
            return;
        }
        pPanel->OnCollapseAnimFinished(bFireEvent);
    };
    pAnimationPlayer->SetCompleteCallback(completeCallback);
    pAnimationPlayer->Start();
}

template<typename InheritType>
UiEstSize PanelTemplate<InheritType>::EstimateSize(UiSize szAvailable)
{
    if (m_nAnimHeight >= 0) {
        //动画进行中：测量高度就是当前动画高度，驱动父级布局实时变化
        UiEstSize estSize = BaseClass::EstimateSize(szAvailable);
        estSize.cy.SetInt32(m_nAnimHeight);
        return estSize;
    }
    UiEstSize estSize = BaseClass::EstimateSize(szAvailable);
    if (m_bCollapsible && m_bCollapsed) {
        //折叠时高度固定为标题栏高度，由布局系统自然收缩；
        //不修改自身 fixed/auto/stretch 高度模式，展开时测量自动恢复
        estSize.cy.SetInt32(m_nTitleBarHeight);
    }
    return estSize;
}

template<typename InheritType>
void PanelTemplate<InheritType>::SetPos(UiRect rc)
{
    if (m_nAnimHeight >= 0) {
        //动画进行中：呈现为动画当前高度
        rc.bottom = rc.top + m_nAnimHeight;
    }
    else if (m_bCollapsible && m_bCollapsed) {
        //折叠状态下，无论父级分配多高，只占标题栏高度
        rc.bottom = rc.top + m_nTitleBarHeight;
    }
    BaseClass::SetPos(rc);

    //标题栏槽位跟随定位（float 子控件不参与正文布局，由 Panel 手动摆放到标题栏右侧）
    LayoutTitleSlot();

    //记录最近一次完全展开的实际高度，作为后续折叠动画起点与展开兜底目标
    if ((m_nAnimHeight < 0) && !(m_bCollapsible && m_bCollapsed)) {
        int32_t nHeight = rc.Height();
        if (nHeight > m_nTitleBarHeight) {
            m_nExpandedHeight = nHeight;
        }
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    if ((m_nAnimHeight < 0) && m_bCollapsible && m_bCollapsed) {
        //折叠静止态：正文子控件全部不绘制，但标题栏槽位（及其子控件）仍然显示
        Control* pSlot = GetTitleSlotControl();
        if ((pSlot != nullptr) && pSlot->IsVisible()) {
            pSlot->AlphaPaint(pRender, rcPaint);
        }
        return;
    }
    if (m_nAnimHeight >= 0) {
        //动画进行中：子控件仍按完整位置布局，裁剪到当前面板矩形内，形成高度收放效果
        UiRect rcClip = this->GetRect();
        float fRoundWidth = 0;
        float fRoundHeight = 0;
        bool bHasRound = this->GetBorderRound(fRoundWidth, fRoundHeight);
        int32_t nClipState = bHasRound
            ? pRender->SetRoundClip(rcClip, fRoundWidth, fRoundHeight, true)
            : pRender->SetClip(rcClip, true);
        BaseClass::PaintChild(pRender, rcPaint);
        pRender->ClearClip(nClipState);
        return;
    }
    BaseClass::PaintChild(pRender, rcPaint);
}

template<typename InheritType>
Control* PanelTemplate<InheritType>::FindControl(FINDCONTROLPROC Proc, void* pProcData,
                                                  uint32_t uFlags, const UiPoint& ptMouse,
                                                  const UiPoint& scrollPos)
{
    if (m_bCollapsible && m_bCollapsed && ((uFlags & UIFIND_HITTEST) != 0)) {
        //折叠时命中测试：正文子控件不接收鼠标事件，但标题栏槽位（及其内部按钮等）保持可用
        Control* pSlot = GetTitleSlotControl();
        if ((pSlot != nullptr) && pSlot->IsVisible() && pSlot->GetRect().ContainsPt(ptMouse)) {
            Control* pHit = pSlot->FindControl(Proc, pProcData, uFlags, ptMouse, scrollPos);
            if (pHit != nullptr) {
                return pHit;
            }
        }
        //其余区域只匹配面板自身；按名称/条件查找（非 HITTEST）不受影响
        return Control::FindControl(Proc, pProcData, uFlags, ptMouse, scrollPos);
    }
    return BaseClass::FindControl(Proc, pProcData, uFlags, ptMouse, scrollPos);
}

template<typename InheritType>
bool PanelTemplate<InheritType>::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (m_bCollapsible && this->IsEnabled()) {
        //点击事件来自子控件（标题栏上的交互控件等）时，不切换折叠状态
        Control* pSender = msg.GetSender();
        if (pSender == static_cast<Control*>(this)) {
            //msg.ptMouse 是窗口客户区坐标，而本控件位于滚动容器内时 GetRect() 返回的是虚拟内容坐标，
            //需要叠加滚动偏移后再做命中判断（与 Slider 的处理方式一致），否则滚动后点击标题栏无法折叠
            UiPoint ptMouse(msg.ptMouse);
            ptMouse.Offset(this->GetScrollOffsetInScrollBox());
            bool bHit = false;
            UiRect rcTitle = GetTitleBarRect();
            if (rcTitle.ContainsPt(ptMouse)) {
                if (m_collapseTrigger == CollapseTrigger::kArrow) {
                    bHit = GetArrowRect().ContainsPt(ptMouse);
                }
                else {
                    //标题栏槽位区域不触发折叠（槽位内放置的是交互控件）
                    Control* pSlot = GetTitleSlotControl();
                    bHit = !((pSlot != nullptr) && pSlot->IsVisible()
                             && pSlot->GetRect().ContainsPt(ptMouse));
                }
            }
            if (bHit) {
                ToggleCollapsed();
            }
        }
    }
    return bRet;
}

template<typename InheritType>
void PanelTemplate<InheritType>::PaintText(IRender* pRender)
{
    BaseClass::PaintText(pRender);
    if (pRender == nullptr) {
        return;
    }

    UiRect rcTitle = GetTitleBarRect();
    if (rcTitle.IsEmpty()) {
        return;
    }

    //绘制标题栏背景：若面板设置了圆角，与面板圆角区域相交裁剪，使标题栏顶部两角随圆角、底部保持直角
    float fRoundWidth = 0;
    float fRoundHeight = 0;
    bool bHasRound = this->GetBorderRound(fRoundWidth, fRoundHeight);
    int32_t nClipState = -1;
    if (!m_titleBkColor.empty()) {
        if (bHasRound) {
            //注意：SetRoundClip 第 4 个参数为 bIntersect，必须传 true（在圆角区域内绘制）；
            //传 false 会使用 kDifference 反向裁剪，标题背景/文字/箭头会被整体挖空
            nClipState = pRender->SetRoundClip(this->GetRect(), fRoundWidth, fRoundHeight, true);
        }
        UiColor bkColor = this->GetUiColor(m_titleBkColor.c_str());
        if (!bkColor.IsEmpty()) {
            pRender->FillRect(UiRectF::MakeFromRect(rcTitle), bkColor);
        }
    }

    //标题文字颜色
    UiColor textColor;
    if (!m_titleTextColor.empty()) {
        textColor = this->GetUiColor(m_titleTextColor.c_str());
    }
    if (textColor.IsEmpty()) {
        textColor = this->GetUiColor(_T("text_default"));
    }

    //标题字体
    DString fontId = m_titleFontId.c_str();
    if (fontId.empty()) {
        fontId = _T("system_bold_14");
    }
    IFont* pFont = this->GetIFontById(fontId);

    //绘制折叠/展开箭头（可折叠时显示在标题栏右侧或左侧，由 arrow_align 决定）：优先状态图片，未配置则用字形
    int32_t nArrowWidth = 0;
    if (m_bCollapsible) {
        nArrowWidth = m_nTitleBarHeight; //箭头区域宽度等于标题栏高度，保证为正方形热区
        UiRect rcArrow = GetArrowRect();
        if (!PaintArrowImage(pRender, rcArrow)) {
            //未配置箭头图片时用矢量三角形绘制，避免 U+25B6 字形在 Windows 下被按 emoji 样式渲染成蓝底白三角：
            //折叠静止态指向右，其余状态（展开/动画中）指向下
            IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
            ASSERT(pRenderFactory != nullptr);
            if (pRenderFactory != nullptr) {
                //悬停时用强调色，与配置了 hover 图片的箭头行为对齐（文字颜色保持不变）
                UiColor arrowColor = textColor;
                if (this->GetState() == kControlStateHovered) {
                    UiColor accentColor = this->GetUiColor(_T("color_accent"));
                    if (!accentColor.IsEmpty()) {
                        arrowColor = accentColor;
                    }
                }
                const float fPad = rcArrow.Width() * 0.32f; //三角形与热区边缘的留白
                const float fL = (float)rcArrow.left + fPad;
                const float fT = (float)rcArrow.top + fPad;
                const float fR = (float)rcArrow.right - fPad;
                const float fB = (float)rcArrow.bottom - fPad;
                const float fCx = (fL + fR) / 2.0f;
                const float fCy = (fT + fB) / 2.0f;
                UiPointF pts[3];
                if (m_bCollapsed && (m_nAnimHeight < 0)) {
                    pts[0] = UiPointF(fL, fT); //右指三角
                    pts[1] = UiPointF(fL, fB);
                    pts[2] = UiPointF(fR, fCy);
                }
                else {
                    pts[0] = UiPointF(fL, fT); //下指三角
                    pts[1] = UiPointF(fR, fT);
                    pts[2] = UiPointF(fCx, fB);
                }
                std::unique_ptr<IPath> pPath(pRenderFactory->CreatePath());
                pPath->AddPolygon(pts, 3);
                pPath->Close();
                std::unique_ptr<IBrush> pBrush(pRenderFactory->CreateBrush(arrowColor));
                pRender->FillPath(pPath.get(), pBrush.get());
            }
        }
    }

    //绘制标题文字（垂直居中，水平对齐由 title_text_align 决定，边界同时避开槽位与箭头区域）
    if (!m_title.empty()) {
        const int32_t nTextPadding = this->Dpi().GetScaleInt(8);
        UiRect rcText = rcTitle;
        rcText.left += nTextPadding;
        rcText.right -= nTextPadding;
        if (nArrowWidth > 0) {
            //箭头所在侧整块让出，文字不与箭头重叠
            if (m_arrowAlign == ArrowAlign::kLeft) {
                rcText.left += nArrowWidth;
            }
            else {
                rcText.right -= nArrowWidth;
            }
        }
        Control* pSlot = GetTitleSlotControl();
        if ((pSlot != nullptr) && pSlot->IsVisible()) {
            UiRect rcSlot = pSlot->GetRect();
            if (!rcSlot.IsEmpty() && (rcSlot.left > rcTitle.left) && (rcSlot.left < rcText.right)) {
                rcText.right = rcSlot.left - this->Dpi().GetScaleInt(6);
            }
        }
        rcText.Validate();

        DrawStringParam drawParam;
        drawParam.textRect = rcText;
        drawParam.dwTextColor = textColor;
        drawParam.pFont = pFont;
        drawParam.uFormat = m_uTitleTextHAlign | TEXT_VCENTER | TEXT_SINGLELINE | TEXT_END_ELLIPSIS;
        pRender->DrawString(m_title.c_str(), drawParam);
    }

    if (nClipState >= 0) {
        pRender->ClearClip(nClipState);
    }
}

template<typename InheritType>
void PanelTemplate<InheritType>::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!this->Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    //DPI 变化期间终止折叠动画，直接落到逻辑状态，避免动画高度按旧缩放比插值
    StopCollapseAnim();

    //先缩放标题栏高度与用户内容 padding
    m_nTitleBarHeight = this->Dpi().GetScaleInt(m_nTitleBarHeight, nOldDpiScale);
    m_contentPadding = this->Dpi().GetScalePadding(m_contentPadding, nOldDpiScale);
    m_nExpandedHeight = this->Dpi().GetScaleInt(m_nExpandedHeight, nOldDpiScale);
    if (m_nSavedMinHeight >= 0) {
        //折叠布局生效中：同步缩放已保存的最小高度，并按新标题栏高度重新放宽
        m_nSavedMinHeight = this->Dpi().GetScaleInt(m_nSavedMinHeight, nOldDpiScale);
        if (m_nSavedMinHeight > m_nTitleBarHeight) {
            this->SetMinHeight(m_nTitleBarHeight, false);
        }
    }

    //基类会按旧的合并 padding 写入基类存储（SetPadding 非虚，无法拦截），
    //返回后用缩放后的标题栏高度与内容 padding 重新合并覆盖
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    ApplyContentPadding();
}

/** 面板控件/水平面板/垂直面板
*/
typedef PanelTemplate<Box>  Panel;
typedef PanelTemplate<HBox> PanelHBox;
typedef PanelTemplate<VBox> PanelVBox;

}

#endif // UI_BOX_PANEL_H_
