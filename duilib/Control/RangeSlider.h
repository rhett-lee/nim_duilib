#ifndef UI_CONTROL_RANGE_SLIDER_H_
#define UI_CONTROL_RANGE_SLIDER_H_

#include "duilib/Control/Progress.h"
#include "duilib/Image/StateImage.h"
#include <string>

namespace ui
{

/** 双滑块范围选择控件
*  在一条轨道上有下限、上限两个滑块，拖拽滑块调整区间，被选中的区间会以高亮显示。
*  常用于价格 / 时间等区间筛选场景。
*/
class DUILIB_API RangeSlider : public Progress
{
    typedef Progress BaseClass;
public:
    /** 范围值（用于 kEventValueChanged 事件，同时携带下限与上限）
    */
    struct RangeValue
    {
        double lowValue = 0;    //下限值
        double highValue = 0;   //上限值
    };

public:
    explicit RangeSlider(Window* pWindow);

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void HandleEvent(const EventArgs& msg) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void ClearImageCache() override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 获取下限值
    */
    double GetLowValue() const { return m_fLowValue; }

    /** 设置下限值（自动钳位到 [min, high - min_range]，会触发值变化事件）
    * @param [in] fValue 要设置的下限值
    */
    void SetLowValue(double fValue);

    /** 设置下限值（不触发 kEventValueChanged 事件，供程序内部同步数值使用）
    */
    void SetLowValueNoEvent(double fValue);

    /** 获取上限值（与 GetValue() 等价）
    */
    double GetHighValue() const { return GetValue(); }

    /** 设置上限值（自动钳位到 [low + min_range, max]，会触发值变化事件）
    * @param [in] fValue 要设置的上限值
    */
    void SetHighValue(double fValue);

    /** 设置上限值（不触发 kEventValueChanged 事件）
    */
    void SetHighValueNoEvent(double fValue);

    /** 同时设置下限与上限（自动钳位；会触发一次值变化事件）
    * @param [in] fLowValue  下限值
    * @param [in] fHighValue 上限值
    */
    void SetRange(double fLowValue, double fHighValue);

    /** 同时设置下限与上限（不触发 kEventValueChanged 事件）
    */
    void SetRangeNoEvent(double fLowValue, double fHighValue);

    /** 获取两个滑块之间的最小间距（下限与上限的最小差值），默认 0
    */
    double GetMinRange() const { return m_fMinRange; }

    /** 设置两个滑块之间的最小间距
    * @param [in] fMinRange 最小间距，小于 0 时按 0 处理
    */
    void SetMinRange(double fMinRange);

    /** 设置滑块的大小（两个滑块共用同一尺寸）
    * @param [in] szXY 要设置的大小
    * @param [in] bNeedDpiScale 是否 DPI 缩放
    */
    void SetThumbSize(UiSize szXY, bool bNeedDpiScale);

    /** 获取滑块的大小
    */
    UiSize GetThumbSize() const;

    /** 获取下限滑块（左侧/下方）的矩形信息
    */
    UiRect GetLowThumbRect() const { return ThumbRectFromValue(GetLowValue()); }

    /** 获取上限滑块（右侧/上方）的矩形信息
    */
    UiRect GetHighThumbRect() const { return ThumbRectFromValue(GetHighValue()); }

    /** 获取指定状态下下限滑块的图片
    * @param[in] stateType 要获取的状态标识，参考 ControlStateType 枚举
    */
    DString GetThumbStateImage(ControlStateType stateType) const;

    /** 设置指定状态下限滑块的图片
    * @param[in] stateType 要设置的状态标识，参考 ControlStateType 枚举
    * @param[in] strImage 要设置的图片位置
    */
    void SetThumbStateImage(ControlStateType stateType, const DString& strImage);

    /** 获取指定状态下上限滑块的图片
    * @param[in] stateType 要获取的状态标识，参考 ControlStateType 枚举
    */
    DString GetHighThumbStateImage(ControlStateType stateType) const;

    /** 设置指定状态下上限滑块的图片（为空时回退使用下限滑块的图片）
    * @param[in] stateType 要设置的状态标识，参考 ControlStateType 枚举
    * @param[in] strImage 要设置的图片位置
    */
    void SetHighThumbStateImage(ControlStateType stateType, const DString& strImage);

    /** 设置高亮区间（选中部分）的颜色（语义色名或颜色值，为空则回退使用 progress_color）
    */
    void SetRangeColor(const DString& strColor);

    /** 获取高亮区间的颜色
    */
    const DString& GetRangeColor() const { return m_strRangeColor; }

    /** 设置高亮区间的圆角半径（像素，绘制时按当前 DPI 缩放，默认 0 为直角）
    */
    void SetRangeRadius(int32_t nRadius);

    /** 获取高亮区间的圆角半径
    */
    int32_t GetRangeRadius() const { return m_nRangeRadius; }

    /** 设置按下鼠标左键（轨道区域）是否立即跳值到点击位置
    * @param [in] bEnable true 立即跳值（默认），false 保持"松开鼠标时才跳值"的旧行为
    */
    void SetClickToJump(bool bEnable);

    /** 获取按下鼠标左键（轨道区域）是否立即跳值
    */
    bool IsClickToJump() const { return m_bClickToJump; }

    /** 监听范围值变化事件
    * @param [in] callback 范围值变化后的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    * 参数说明：
    *   wParam: 1（范围模式标记）
    *   lParam: 0
    *   pEventData: 指向 RangeValue（含 lowValue / highValue），
    *               仅在本次回调派发期间有效，请勿保存该指针
    * 推荐用法：
    *   if (msg.pEventData != nullptr) {
    *       auto* pRange = static_cast<const ui::RangeSlider::RangeValue*>(msg.pEventData);
    *       // 读取 pRange->lowValue / pRange->highValue
    *   }
    */
    void AttachRangeChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID); }

protected:
    /** 计算指定值对应的滑块矩形（含钳位）
    * @param [in] fValue 进度值
    */
    UiRect ThumbRectFromValue(double fValue) const;

    /** 计算指定值对应的区间矩形（以控件左上角为原点的相对坐标，含钳位）
    *  水平：rc.right 为该值对应的横向填充量；垂直：rc.top 为该值对应的纵向起点
    * @param [in] fValue 进度值
    */
    UiRect CalcRangeRect(double fValue) const;

    /** 根据鼠标位置计算对应的进度值（带钳位）
    * @param [in] ptMouse 控件内坐标
    */
    double ValueFromMousePt(const UiPoint& ptMouse) const;

    /** 把下限值钳位到 [min, high - min_range] 区间内
    */
    void ClampLowValue();

private:
    /** 滑块标识
    */
    enum ThumbPart : uint8_t
    {
        kThumbNone = 0, //无
        kThumbLow,      //下限滑块
        kThumbHigh      //上限滑块
    };

    /** 派发范围值变化事件（m_bMuteRangeEvent 为 true 时不派发）
    */
    void EmitRangeChanged();

    /** 绘制单个滑块
    * @param [in] pRender 绘制接口
    * @param [in] part 要绘制的滑块（下限/上限）
    */
    void PaintThumb(IRender* pRender, ThumbPart part);

    /** 下限值
    */
    double m_fLowValue;

    /** 两个滑块之间的最小间距
    */
    double m_fMinRange;

    /** 滑块大小（未缩放原始值）
    */
    UiSize m_szThumb;

    /** 下限滑块的图片
    */
    StateImage m_thumbStateImage;

    /** 上限滑块的图片（为空时回退使用下限滑块图片）
    */
    StateImage m_highThumbStateImage;

    /** 高亮区间颜色
    */
    DString m_strRangeColor;

    /** 高亮区间圆角半径（未缩放原始值）
    */
    int32_t m_nRangeRadius;

    /** 按下轨道是否立即跳值（默认 true）
    */
    bool m_bClickToJump;

    /** 是否抑制范围值变化事件（SetXXXNoEvent 期间为 true）
    */
    bool m_bMuteRangeEvent;

    /** 上限值是否已被设置过（未设置时，下限不受"最小间距"约束，便于初始化时自由赋值）
    */
    bool m_bHighValueSet;

    /** 当前正在拖动的滑块
    */
    ThumbPart m_dragThumb;

    /** 鼠标当前悬停的滑块
    */
    ThumbPart m_hoverThumb;

    /** 滑块图片绘制参数缓存
    */
    UiString m_sImageModify;
};

} // namespace ui

#endif // UI_CONTROL_RANGE_SLIDER_H_
