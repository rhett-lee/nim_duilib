#ifndef UI_CONTROL_RATING_H_
#define UI_CONTROL_RATING_H_

#include "duilib/Core/Control.h"
#include "duilib/Image/StateImage.h"
#include <array>

namespace ui {

/** 星图类型（星形内容图片）
*/
enum class StarImageType
{
    kFilled = 0,    //实心星
    kHalf = 1,      //半星
    kEmpty = 2      //空心星
};

/** 评分值变化数据（用于 kEventValueChanged 事件，pEventData 指向本结构）
*/
struct RatingValue
{
    double value = 0;       //新的评分值
    double oldValue = 0;    //变化前的评分值
};

/** Rating 星级评分控件（继承 Control）
 *
 *  用于评价 / 打分 / 满意度调查等场景，支持只读展示、半星、鼠标悬停预览与点击 / 拖拽打分。
 *
 *  外观完全由 SVG 图片驱动（不做星形自绘）：
 *  - 星形内容图（实心 / 半星 / 空心）经 `svg_replace_colors` 把占位色映射到主题语义色；
 *  - 悬停 / 按下态图标为可选的状态反馈图，绘制在星形下层（形成外圈光晕 / 描边效果），
 *    仅对"可交互的星星"（控件启用、非只读且鼠标启用）在鼠标悬停 / 按下时显示。
 *
 *  XML 属性：
 *  - value：当前评分值（double），钳位到 [0, max]，按 allow_half 对齐步长
 *  - max（别名 star_count）：星星数量，默认 5
 *  - allow_half：是否允许半星（步长 0.5，否则 1），默认 false
 *  - read_only（别名 readonly）：只读展示模式，禁止鼠标交互，默认 false
 *  - star_size：单颗星绘制直径（像素，DPI 缩放），默认 20
 *  - star_gap：星星间距（像素，DPI 缩放），默认 4
 *  - star_filled_image（别名 star_full_image）：实心星图片
 *  - star_half_image：半星图片（可选，未配置时对实心星做左半裁剪绘制）
 *  - star_empty_image：空心星图片
 *  - star_hovered_image（别名 star_hover_image）：星悬停态图标，可选
 *  - star_pressed_image（别名 star_pushed_image）：星按下态图标，可选；未配置时回退悬停态图标
 */
class DUILIB_API Rating : public Control
{
    typedef Control BaseClass;

public:
    explicit Rating(Window* pWindow);
    Rating(const Rating& r) = delete;
    Rating& operator=(const Rating& r) = delete;
    ~Rating() override;

    /** 获取控件类型
    */
    DString GetType() const override;

    /** 设置XML属性
    */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 绘制评分星星
    */
    void PaintStateImages(IRender* pRender) override;

    /** 鼠标左键按下：命中星并进入拖动打分
    */
    bool ButtonDown(const EventArgs& msg) override;

    /** 鼠标左键抬起：结束拖动
    */
    bool ButtonUp(const EventArgs& msg) override;

    /** 鼠标移动：拖动连续打分 / 更新悬停预览
    */
    bool MouseMove(const EventArgs& msg) override;

    /** 鼠标离开控件：清除悬停预览
    */
    bool MouseLeave(const EventArgs& msg) override;

    /** 清理图片缓存
    */
    void ClearImageCache() override;

public:
    /** 获取当前评分值
    */
    double GetValue() const { return m_fValue; }

    /** 设置当前评分值（钳位 + 步长对齐，变化时派发 kEventValueChanged 事件）
    */
    void SetValue(double fValue);

    /** 设置当前评分值（不触发事件）
    */
    void SetValueNoEvent(double fValue);

    /** 获取星星数量
    */
    int32_t GetMax() const { return m_nMax; }

    /** 获取星星数量（别名）
    */
    int32_t GetStarCount() const { return m_nMax; }

    /** 设置星星数量（调小 max 时同步钳位 value）
    */
    void SetMax(int32_t nMax);

    /** 是否允许半星
    */
    bool IsAllowHalf() const { return m_bAllowHalf; }

    /** 设置是否允许半星
    */
    void SetAllowHalf(bool bAllow);

    /** 是否为只读模式
    */
    bool IsReadOnly() const { return m_bReadOnly; }

    /** 设置只读模式
    */
    void SetReadOnly(bool bReadOnly);

    /** 获取单颗星绘制直径（未缩放原始值）
    */
    int32_t GetStarSize() const { return m_nStarSize; }

    /** 设置单颗星绘制直径（像素，自动 DPI 缩放）
    */
    void SetStarSize(int32_t nSize);

    /** 获取星星间距（未缩放原始值）
    */
    int32_t GetStarGap() const { return m_nStarGap; }

    /** 设置星星间距（像素，自动 DPI 缩放）
    */
    void SetStarGap(int32_t nGap);

    /** 设置某类星形内容图属性字符串
    * @param [in] type 星图类型（kFilled / kHalf / kEmpty）
    * @param [in] strImage 图片属性字符串，例如 "file='public/rating/star_filled.svg'"
    */
    void SetStarImage(StarImageType type, const DString& strImage);

    /** 获取某类星形内容图属性字符串
    */
    DString GetStarImage(StarImageType type) const;

    /** 设置实心星图片
    */
    void SetFilledImage(const DString& strImage) { SetStarImage(StarImageType::kFilled, strImage); }

    /** 设置半星图片
    */
    void SetHalfImage(const DString& strImage) { SetStarImage(StarImageType::kHalf, strImage); }

    /** 设置空心星图片
    */
    void SetEmptyImage(const DString& strImage) { SetStarImage(StarImageType::kEmpty, strImage); }

    /** 设置星悬停态图标
    */
    void SetHoveredImage(const DString& strImage) { SetStarStateImage(kControlStateHovered, strImage); }

    /** 设置星按下态图标
    */
    void SetPressedImage(const DString& strImage) { SetStarStateImage(kControlStatePressed, strImage); }

    /** 获取星悬停态图标属性字符串
    */
    DString GetHoveredImage() const { return GetStarStateImage(kControlStateHovered); }

    /** 获取星按下态图标属性字符串
    */
    DString GetPressedImage() const { return GetStarStateImage(kControlStatePressed); }

    /** 设置星交互状态图标
    * @param [in] stateType 状态类型（有效值：kControlStateHovered / kControlStatePressed）
    * @param [in] strImage 图片属性字符串，例如 "file='public/rating/star_hovered.svg'"
    */
    void SetStarStateImage(ControlStateType stateType, const DString& strImage);

    /** 获取星交互状态图标属性字符串
    * @param [in] stateType 状态类型（kControlStateHovered / kControlStatePressed）
    */
    DString GetStarStateImage(ControlStateType stateType) const;

    /** 监听评分值变化事件
    * @param [in] callback 评分值变化后的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    * 参数说明：
    *   wParam: 0
    *   lParam: 0
    *   pEventData: 指向 RatingValue（含 value / oldValue），
    *               仅在本次回调派发期间有效，请勿保存该指针
    */
    void AttachRatingChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID); }

private:
    /** 派发评分值变化事件
    * @param [in] fOldValue 变化前的值
    */
    void EmitValueChanged(double fOldValue);

    /** 钳位到 [0, max] 并对齐步长（0.5 或 1）
    */
    double NormalizeValue(double fValue) const;

    /** 获取指定类型的星形内容图接口
    */
    Image* GetStarImagePtr(StarImageType type) const;

    /** 获取星交互状态图标接口
    */
    Image* GetStarStateImagePtr(ControlStateType stateType) const;

    /** 绘制单张图片
    */
    void PaintStarImage(IRender* pRender, Image* pImage, const DString& strModify, int32_t nFade) const;

    /** 绘制某颗星的悬停 / 按下态图标（绘制在星形下层；未配置或非可交互星时不绘制）
    * @param [in] nIndex 星索引
    * @param [in] nLeft 该星左边界（窗口坐标）
    * @param [in] nTop 该星上边界（窗口坐标）
    * @param [in] nSize 星绘制尺寸（已 DPI 缩放）
    */
    void PaintStarStateIcon(IRender* pRender, int32_t nIndex, int32_t nLeft, int32_t nTop, int32_t nSize);

    /** 命中检测：由鼠标位置求星索引与左右半边
    * @param [in] rc 控件可用区域（已去掉内边距）
    * @param [in] ptMouse 鼠标位置
    * @param [out] pHalf 是否落在星星右半边（可为 nullptr）
    * @return 命中的星索引，未命中返回 -1
    */
    int32_t HitTestStar(const UiRect& rc, const UiPoint& ptMouse, bool* pHalf) const;

    /** 由命中结果计算评分值
    * @param [in] nIndex 星索引（从 0 开始）
    * @param [in] bHalf 是否落在右半边（右半边按整颗计；半星模式下左半边计 0.5）
    */
    double ValueFromHit(int32_t nIndex, bool bHalf) const;

private:
    double m_fValue;        //当前评分值
    int32_t m_nMax;         //星星数量
    bool m_bAllowHalf;      //是否允许半星
    bool m_bReadOnly;       //是否只读
    int32_t m_nStarSize;    //单颗星绘制直径（像素）
    int32_t m_nStarGap;     //星星间距（像素）

    //星形内容图（索引对应 StarImageType，图片存入 kControlStateNormal）
    std::array<StateImage, 3> m_starImages;

    //星交互状态图标（悬停 / 按下态，存入 kControlStateHovered / kControlStatePressed）
    StateImage m_starStateImage;

    bool m_bDrag;           //是否正在拖动打分
    double m_fHoverValue;   //悬停预览值，-1 表示无悬停
    int32_t m_nHoverStar;   //鼠标悬停的星索引，-1 表示无
    int32_t m_nPressedStar; //鼠标按下的星索引，-1 表示无
    bool m_bMuteEvent;      //为 true 时不派发事件
};

} // namespace ui

#endif // UI_CONTROL_RATING_H_
