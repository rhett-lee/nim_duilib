#ifndef UI_CONTROL_SLIDER_H_
#define UI_CONTROL_SLIDER_H_

#include "duilib/Control/Progress.h"
#include "duilib/Core/ControlFinder.h"
#include <string>

namespace ui
{

/** 刻度线位置
*/
enum class SliderTickPos : uint8_t
{
    kBottom,    //水平滑块：轨道下方（默认）
    kTop,       //水平滑块：轨道上方
    kRight,     //垂直滑块：轨道右侧（默认）
    kLeft       //垂直滑块：轨道左侧
};

/** 滑块控件
*/
class DUILIB_API Slider : public Progress
{
    typedef Progress BaseClass;
public:
    explicit Slider(Window* pWindow);

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual UiRect GetProgressPos() const override;
    virtual void HandleEvent(const EventArgs& msg) override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void PaintBkColor(IRender* pRender) override;
    virtual void ClearImageCache() override;

    /** 获取控件标志，键盘可达性需要（Tab 可聚焦）
    */
    virtual uint32_t GetControlFlags() const override;

    /** 按下鼠标左键（轨道区域）是否立即跳值到点击位置
    * @param [in] bEnable true 立即跳值（默认），false 保持"松开鼠标时才跳值"的旧行为
    */
    void SetClickToJump(bool bEnable);

    /** 获取按下鼠标左键（轨道区域）是否立即跳值
    */
    bool IsClickToJump() const { return m_bClickToJump; }

    /** 设置 PageUp/PageDown 与 Shift+方向键的大步长
    * @param [in] nPageStep 大步长，<=0 表示自动取 (max-min)/10
    */
    void SetPageStep(int32_t nPageStep);

    /** 获取大步长（<=0 表示自动模式，返回的是实际计算出的步长）
    */
    int32_t GetPageStep() const;

    /** 设置是否显示刻度线
    */
    void SetShowTicks(bool bShow);

    /** 获取是否显示刻度线
    */
    bool IsShowTicks() const { return m_bShowTicks; }

    /** 设置刻度线数量（至少为 2，含首尾两端）
    */
    void SetTickCount(int32_t nCount);

    /** 获取刻度线数量
    */
    int32_t GetTickCount() const { return m_nTickCount; }

    /** 设置刻度线长度（像素，内部按当前 DPI 缩放）
    */
    void SetTickLength(int32_t nLength);

    /** 获取刻度线长度
    */
    int32_t GetTickLength() const;

    /** 设置刻度线颜色（语义色名或颜色值，为空使用 text_muted）
    */
    void SetTickColor(const DString& strColor);

    /** 获取刻度线颜色
    */
    const DString& GetTickColor() const { return m_strTickColor; }

    /** 设置刻度线位置（水平滑块用Bottom/Top，垂直滑块用Right/Left）
    */
    void SetTickPos(SliderTickPos pos);

    /** 获取刻度线位置
    */
    SliderTickPos GetTickPos() const { return m_tickPos; }

    /** 设置是否显示刻度值文本（需要 show_ticks 为 true）
    */
    void SetShowTickLabels(bool bShow);

    /** 获取是否显示刻度值文本
    */
    bool IsShowTickLabels() const { return m_bShowTickLabels; }

    /** 设置每隔多少个刻度显示一次刻度值文本（最小为 1）
    */
    void SetTickLabelInterval(int32_t nInterval);

    /** 获取刻度值文本的显示间隔
    */
    int32_t GetTickLabelInterval() const { return m_nTickLabelInterval; }

    /** 设置刻度值文本的字体ID
    */
    void SetTickFontId(const DString& strFontId);

    /** 获取刻度值文本的字体ID
    */
    const DString& GetTickFontId() const { return m_strTickFontId; }

    /** 设置刻度值文本的颜色（语义色名或颜色值，为空使用 text_muted）
    */
    void SetTickLabelColor(const DString& strColor);

    /** 获取刻度值文本的颜色
    */
    const DString& GetTickLabelColor() const { return m_strTickLabelColor; }

    /** 刻度值文本格式化回调：自定义刻度标签上显示的文本
    *   @param value     该刻度对应的进度值
    *   @param index     刻度序号（从0开始）
    *   @return 要显示的文本（如 "5"、"50%"、"03:20"）
    */
    typedef std::function<DString(double value, int32_t index)> TickLabelCallback;

    /** 设置刻度值文本格式化回调
    * @param [in] callback 格式化回调，置空则恢复默认的值格式化
    */
    void SetTickLabelCallback(const TickLabelCallback& callback);

    /** 监听进度条进度改变事件
    * @param [in] callback 进度条进度改变后调用的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachValueChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID);    }

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 获取步进步长
     * @return 返回步进步长
     */
    int GetChangeStep();

    /** 设置步进步长
    * @param[in] step 要设置的步长
     */
    void SetChangeStep(int step);

    /** 设置滑块的大小
    * @param [in] szXY 要设置的大小
    * @param [in] bNeedDpiScale 是否 DPI 缩放
     */
    void SetThumbSize(UiSize szXY, bool bNeedDpiScale);

    /** 获取滑块的大小
    */
    UiSize GetThumbSize() const;

    /** 获取滑块的矩形信息
    * @return 返回滑块的矩形信息
    */
    UiRect GetThumbRect() const;

    /** 获取指定状态下滑块的图片
    * @param[in] stateType 要获取的状态标识，参考 ControlStateType 枚举
    * @return 返回图片路径
    */
    DString GetThumbStateImage(ControlStateType stateType) const;

    /** 设置指定状态下滑块的图片
    * @param[in] stateType 要设置的状态标识，参考 ControlStateType 枚举
    * @param[in] pStrImage 要设置的图片位置
    */
    void SetThumbStateImage(ControlStateType stateType, const DString& pStrImage);

    /** 获取进度条内边距
    */
    const UiPadding& GetProgressBarPadding() const;

    /** 设置进度条内边距
    * @param [in] padding 要设置的内边距信息
    * @param [in] bNeedDpiScale 是否 DPI 缩放
    */
    void SetProgressBarPadding(UiPadding padding, bool bNeedDpiScale);

protected:
    /** 根据鼠标位置计算对应的进度值（带钳位）
    * @param [in] ptMouse 控件内坐标
    * @return 返回该位置对应的进度值
    */
    double ValueFromMousePt(const UiPoint& ptMouse) const;

    /** 按键盘按键调整当前值
    * @param [in] vkCode 虚拟键码
    * @param [in] bShiftDown 是否按下 Shift 键（Shift+方向键 = 大步进）
    * @return 返回 true 表示已处理该按键
    */
    bool HandleKeyDown(uint32_t vkCode, bool bShiftDown);

    /** 按步长调整当前值（自动钳位到 [min, max]）
    * @param [in] fDelta 增量（可为负）
    * @param [in] nStepCount 步长倍数（负数为反向）
    */
    void StepValue(double fDelta, int32_t nStepCount = 1);

    /** 绘制刻度线与刻度值文本
    */
    void PaintTicks(IRender* pRender);

    /** 绘制刻度值文本
    * @param [in] uAlign 对齐方式（TEXT_LEFT/TEXT_CENTER/TEXT_RIGHT 与 TEXT_TOP/VCENTER/BOTTOM 的组合）
    */
    void DrawTickText(IRender* pRender, double fValue, int32_t nIndex, const UiRect& rcLabel, uint32_t uAlign);

    /** 计算第 nIndex 个刻度对应的进度值
    */
    double GetTickValue(int32_t nIndex) const;

    int m_nStep;
    UiSize m_szThumb;
    StateImage m_thumbStateImage;
    UiPadding m_rcProgressBarPadding;
    UiString m_sImageModify;

private:
    //是否按下轨道立即跳值（默认 true）
    bool m_bClickToJump;

    //PageUp/PageDown 大步长，<=0 表示自动
    int32_t m_nPageStep;

    //刻度相关
    bool m_bShowTicks;
    int32_t m_nTickCount;
    int32_t m_nTickLength;      //已DPI 缩放
    DString m_strTickColor;
    SliderTickPos m_tickPos;
    bool m_bShowTickLabels;
    int32_t m_nTickLabelInterval;
    DString m_strTickFontId;
    DString m_strTickLabelColor;
    TickLabelCallback m_tickLabelCallback;
};

}

#endif // UI_CONTROL_SLIDER_H_
