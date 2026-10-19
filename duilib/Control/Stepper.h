#ifndef UI_CONTROL_STEPPER_H_
#define UI_CONTROL_STEPPER_H_

#include "duilib/Core/Control.h"
#include "duilib/Image/StateImage.h"
#include <vector>

namespace ui {

/** Stepper 步骤状态
*/
enum class StepStatus
{
    kPending = 0,   //待开始（灰色，显示序号）
    kActive = 1,    //当前进行中（强调色，显示序号）
    kCompleted = 2, //已完成（成功色，显示对勾）
    kDisabled = 3   //禁用（灰淡，不可跳转）
};

/** Stepper 向导步骤条控件（继承 Control，纯自绘）
 *
 *  用于安装 / 注册 / 配置向导、审批流程、订单进度等"流程状态机"场景；
 *  与 Timeline（时间轴）语义不同：Steps 关注的是"走到第几步"，而非时间序列。
 *
 *  状态由"当前索引 + 禁用集合"派生：位于当前索引之前的步骤为已完成，当前步骤为进行中，
 *  之后的为待开始；被显式禁用的步骤优先显示为禁用态。
 *
 *  XML 属性：
 *  - step_title / step_title_id：步骤标题，`|` 分隔多条（`_id` 为多语言 ID，语言切换自动刷新）
 *  - step_desc / step_desc_id：步骤描述，`|` 分隔多条
 *  - current / current_index：当前步骤索引，默认 0
 *  - direction / orientation：排列方向，horizontal（默认）/ vertical
 *  - click_to_jump：是否允许鼠标点击步骤跳转，默认 true
 *  - disabled_steps：禁用步骤索引列表，`|` 或 `,` 分隔，如 "2|3"
 *  - node_size：节点直径（像素，DPI 缩放），默认 24
 *  - node_color_pending / node_color_active / node_color_completed / node_color_disabled：各状态节点颜色
 *  - connector_color：未完成段连接线颜色；connector_active_color：已完成段连接线颜色
 *  - title_color_active / title_color_completed / title_color_pending：各状态标题颜色
 *  - desc_color：描述文本颜色
 *  - title_font / desc_font：标题 / 描述的字体 ID
 *  - node_hovered_image（别名 node_hot_image）：节点悬停态图片，可选
 *  - node_pressed_image（别名 node_pushed_image）：节点按下态图片，可选；未配置时回退悬停态图片
 *
 *  说明：悬停/按下态图片绘制在节点圆形下层，仅对"可交互步骤"（控件启用、click_to_jump 开启且该步非禁用）
 *        在鼠标悬停/按下时显示，用于呈现强调色光晕/描边等反馈，不影响节点本色与序号/对勾。
 */
class DUILIB_API Stepper : public Control
{
    typedef Control BaseClass;

public:
    /** 步骤数据
    */
    struct Step
    {
        DString title;       //标题
        DString description; //描述（可为空）
    };

    /** 当前步骤变化数据（用于 kEventValueChanged 事件，pEventData 指向本结构）
    */
    struct StepValue
    {
        size_t currentIndex = 0; //新的当前步骤索引
        size_t oldIndex = 0;     //变化前的当前步骤索引
    };

public:
    explicit Stepper(Window* pWindow);
    Stepper(const Stepper& r) = delete;
    Stepper& operator=(const Stepper& r) = delete;
    ~Stepper() override;

    /** 获取控件类型
    */
    DString GetType() const override;

    /** 设置XML属性
    */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 绘制步骤条内容
    */
    void PaintStateImages(IRender* pRender) override;

    /** 语言切换时刷新多语言数据绑定
    */
    void OnLanguageChanged(bool bRedraw) override;

    /** 鼠标左键按下：点击步骤跳转
    */
    bool ButtonDown(const EventArgs& msg) override;

    /** 鼠标左键抬起：结束按下态
    */
    bool ButtonUp(const EventArgs& msg) override;

    /** 鼠标移动：更新悬停的步骤（用于 hover 态显示）
    */
    bool MouseMove(const EventArgs& msg) override;

    /** 鼠标离开控件：清除悬停/按下态
    */
    bool MouseLeave(const EventArgs& msg) override;

    /** 清理图片缓存
    */
    void ClearImageCache() override;

public:
    /** 追加一个步骤
    */
    void AddStep(const Step& step);

    /** 追加一个步骤（便捷重载）
    */
    void AddStep(const DString& title, const DString& description = DString());

    /** 在指定位置插入步骤
    */
    bool InsertStep(size_t index, const Step& step);

    /** 删除指定步骤
    */
    bool RemoveStepAt(size_t index);

    /** 清空所有步骤
    */
    void ClearSteps();

    /** 批量设置步骤（替换全部）
    */
    void SetSteps(const std::vector<Step>& steps);

    /** 获取步骤数量
    */
    size_t GetStepCount() const { return m_steps.size(); }

    /** 获取指定步骤
    */
    const Step* GetStepAt(size_t index) const;

    /** 获取全部步骤
    */
    const std::vector<Step>& GetSteps() const { return m_steps; }

    /** 获取当前步骤索引（已钳位到有效范围）
    */
    size_t GetCurrentIndex() const;

    /** 设置当前步骤索引（自动钳位，索引变化时触发 kEventValueChanged 事件）
    */
    void SetCurrentIndex(size_t nIndex);

    /** 设置当前步骤索引（不触发事件，供程序内部同步使用）
    */
    void SetCurrentIndexNoEvent(size_t nIndex);

    /** 前进到下一步（越界返回 false，触发事件）
    */
    bool Next();

    /** 前进到下一步（不触发事件）
    */
    bool NextNoEvent();

    /** 后退到上一步（越界返回 false，触发事件）
    */
    bool Back();

    /** 后退到上一步（不触发事件）
    */
    bool BackNoEvent();

    /** 跳转到指定步骤（目标为禁用步骤或索引越界时返回 false，触发事件）
    */
    bool GoTo(size_t nIndex);

    /** 跳转到指定步骤（不触发事件）
    */
    bool GoToNoEvent(size_t nIndex);

    /** 设置指定步骤是否禁用
    */
    void SetStepDisabled(size_t index, bool bDisabled);

    /** 获取指定步骤是否禁用
    */
    bool IsStepDisabled(size_t index) const;

    /** 批量设置禁用步骤（替换全部）
    */
    void SetDisabledSteps(const std::vector<size_t>& disabledSteps);

    /** 获取禁用步骤列表
    */
    const std::vector<size_t>& GetDisabledSteps() const { return m_disabledSteps; }

    /** 获取指定步骤的派生状态
    */
    StepStatus GetStepStatus(size_t index) const;

    /** 设置排列方向：true 水平（默认），false 垂直
    */
    void SetHorizontal(bool bHorizontal);

    /** 获取是否水平排列
    */
    bool IsHorizontal() const { return m_bHorizontal; }

    /** 设置节点直径（像素，自动 DPI 缩放）
    */
    void SetNodeSize(int32_t nSize);

    /** 获取节点直径（未缩放原始值）
    */
    int32_t GetNodeSize() const { return m_nNodeSize; }

    /** 设置是否允许鼠标点击步骤跳转
    */
    void SetClickToJump(bool bEnable);

    /** 获取是否允许鼠标点击步骤跳转
    */
    bool IsClickToJump() const { return m_bClickToJump; }

    /** 设置指定状态的节点颜色（语义色名或颜色值）
    */
    void SetStateColor(StepStatus status, const DString& strColor);

    /** 获取指定状态的节点颜色
    */
    const DString& GetStateColor(StepStatus status) const;

    /** 设置指定状态的标题颜色（语义色名或颜色值）
    */
    void SetTitleColor(StepStatus status, const DString& strColor);

    /** 获取指定状态的标题颜色
    */
    const DString& GetTitleColor(StepStatus status) const;

    /** 设置未完成段连接线颜色
    */
    void SetConnectorColor(const DString& strColor);

    /** 获取未完成段连接线颜色
    */
    const DString& GetConnectorColor() const { return m_strConnectorColor; }

    /** 设置已完成段连接线颜色
    */
    void SetConnectorActiveColor(const DString& strColor);

    /** 获取已完成段连接线颜色
    */
    const DString& GetConnectorActiveColor() const { return m_strConnectorActiveColor; }

    /** 设置描述文本颜色
    */
    void SetDescColor(const DString& strColor);

    /** 获取描述文本颜色
    */
    const DString& GetDescColor() const { return m_strDescColor; }

    /** 设置标题字体 ID
    */
    void SetTitleFont(const DString& strFontId);

    /** 获取标题字体 ID
    */
    const DString& GetTitleFont() const { return m_strTitleFont; }

    /** 设置描述字体 ID
    */
    void SetDescFont(const DString& strFontId);

    /** 获取描述字体 ID
    */
    const DString& GetDescFont() const { return m_strDescFont; }

    /** 监听当前步骤变化事件
    * @param [in] callback 当前步骤变化后的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    * 参数说明：
    *   wParam: 2（步骤模式标记）
    *   lParam: 0
    *   pEventData: 指向 StepValue（含 currentIndex / oldIndex），
    *               仅在本次回调派发期间有效，请勿保存该指针
    */
    void AttachStepChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID); }

    /** 设置节点状态图片（可选，用于悬停/按下态的视觉反馈）
    * @param [in] stateType 状态类型（有效值：kControlStateHovered / kControlStatePressed）
    * @param [in] strImage 图片属性字符串，例如 "file='public/stepper/stepper_node_hovered.svg'"
    */
    void SetNodeStateImage(ControlStateType stateType, const DString& strImage);

    /** 获取节点状态图片属性字符串
    * @param [in] stateType 状态类型（kControlStateHovered / kControlStatePressed）
    */
    DString GetNodeStateImage(ControlStateType stateType) const;

private:
    /** 解析 XML 数据绑定字段（`|` 分隔）
    * @param [in] strValue 字段值字符串
    * @param [in] nField 字段编号：0=title, 1=description
    * @param [in] bTranslate 是否为多语言 ID 模式（每个分段按 STRID 从语言文件取实际文本）
    */
    void ParseItemsField(const DString& strValue, int32_t nField, bool bTranslate = false);

    /** 解析禁用步骤索引列表
    */
    void ParseDisabledSteps(const DString& strValue);

    /** 派发当前步骤变化事件（m_bMuteStepEvent 为 true 时不派发）
    * @param [in] nOldIndex 变化前的索引
    */
    void EmitStepChanged(size_t nOldIndex);

    /** 把当前索引钳位到 [0, count-1]（空列表时为 0）
    */
    void ClampCurrentIndex();

    /** 计算指定步骤的槽位矩形（控件相对坐标）
    */
    UiRect CalcStepSlotRect(const UiRect& rc, size_t nCount, size_t index) const;

    /** 命中检测：返回鼠标所在的步骤索引，未命中返回 -1
    */
    int32_t HitTestStep(const UiRect& rc, size_t nCount, const UiPoint& ptMouse) const;

    /** 绘制水平方向步骤条
    */
    void PaintHorizontal(IRender* pRender, const UiRect& rc);

    /** 绘制垂直方向步骤条
    */
    void PaintVertical(IRender* pRender, const UiRect& rc);

    /** 绘制单个节点（圆 + 序号/对勾）
    */
    void PaintNode(IRender* pRender, const UiPoint& ptCenter, int32_t nRadius, StepStatus status,
                   size_t index, bool bDimmed);

    /** 绘制文本（封装 DrawStringParam）
    */
    void DrawStepperText(IRender* pRender, const DString& strText, const UiRect& rc, uint32_t uFormat,
                         UiColor textColor, const DString& strFontId);

    /** 测量文本高度
    */
    int32_t MeasureTextHeight(IRender* pRender, const DString& strText, int32_t nWidth, const DString& strFontId);

    /** 绘制节点悬停/按下态图标（绘制在节点圆形下层；未配置或非可交互步骤时不绘制）
    * @param [in] ptCenter 节点圆心（控件坐标）
    * @param [in] nRadius 节点半径（已 DPI 缩放）
    * @param [in] status 该步骤的派生状态
    * @param [in] index 步骤索引
    */
    void PaintNodeStateIcon(IRender* pRender, const UiPoint& ptCenter, int32_t nRadius,
                            StepStatus status, size_t index);

private:
    /** 步骤数据
    */
    std::vector<Step> m_steps;

    /** 禁用步骤索引列表
    */
    std::vector<size_t> m_disabledSteps;

    /** 当前步骤索引
    */
    size_t m_nCurrentIndex;

    /** 是否水平排列（false 为垂直）
    */
    bool m_bHorizontal;

    /** 节点直径（DPI 缩放前原始值）
    */
    int32_t m_nNodeSize;

    /** 是否允许点击步骤跳转
    */
    bool m_bClickToJump;

    /** 是否抑制当前步骤变化事件（NoEvent 期间为 true）
    */
    bool m_bMuteStepEvent;

    /** 各状态节点颜色（语义色名，索引对应 StepStatus）
    */
    DString m_stateColors[4];

    /** 各状态标题颜色（语义色名，索引对应 StepStatus）
    */
    DString m_titleColors[4];

    /** 未完成段连接线颜色
    */
    DString m_strConnectorColor;

    /** 已完成段连接线颜色
    */
    DString m_strConnectorActiveColor;

    /** 描述文本颜色
    */
    DString m_strDescColor;

    /** 标题字体 ID
    */
    DString m_strTitleFont;

    /** 描述字体 ID
    */
    DString m_strDescFont;

    /** 数据绑定的多语言 ID 原始串（step_title_id / step_desc_id，语言切换时重新解析）
    */
    DString m_strItemTitleIds;
    DString m_strItemDescIds;

    /** 节点状态图片（悬停态 kControlStateHovered / 按下态 kControlStatePressed，可选）
    */
    StateImage m_nodeStateImage;

    /** 鼠标悬停的步骤索引（-1 表示无）
    */
    int32_t m_nHoverStep;

    /** 鼠标按下的步骤索引（-1 表示无）
    */
    int32_t m_nPressedStep;
};

} // namespace ui

#endif // UI_CONTROL_STEPPER_H_
