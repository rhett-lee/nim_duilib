#ifndef UI_BOX_TABBOX_H_
#define UI_BOX_TABBOX_H_

#include "duilib/Core/Box.h"

namespace ui
{
/** 堆叠布局/卡片布局，内部的多个子控件，只有一个可见，其他是隐藏的，可动态切换，支持切换动画
*/
class UILIB_API TabBox : public Box
{
    typedef Box BaseClass;
public:
    explicit TabBox(Window* pWindow, Layout* pLayout = new Layout());

    // 用于初始化xml属性
    virtual void OnInit() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual bool AddItem(Control* pControl) override;
    virtual bool AddItemAt(Control* pControl, size_t iIndex) override;
    virtual bool RemoveItem(Control* pControl) override;
    virtual bool RemoveItemAt(size_t iIndex) override;
    virtual void RemoveAllItems() override;    

public:
    /** 获取当前选择项
     * @return 返回当前选择项索引(如果无有效索引，则返回Box::InvalidIndex)
     */
    size_t GetCurSel() const;

    /** 根据控件索引选择一个子项
     * @param [in] iIndex 要选择的子项索引
     * @return 成功返回 true，否则返回 false
     */
    bool SelectItem(size_t iIndex);

    /** 根据控件指针选择一个子项
     * @param [in] pControl 要选择的子项指针
     * @return 成功返回 true，否则返回 false
     */
    bool SelectItem(Control* pControl);

    /** 根据控件名称选择一个子项
     * @param [in] pControlName 要选择的控件名称
     * @return 成功返回 true，否则返回 false
     */
    bool SelectItem(const DString& pControlName);
   
    /** 监听Tab页面选择事件
     * @param [in] callback 事件处理的回调函数，请参考 EventCallback 声明
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     */
    void AttachTabSelect(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventTabSelect, callback, callbackID); }

public:
    /** 页面切换动画的类型
    */
    enum class FadeSwitchType : int8_t
    {
        kNone,          //不显示动画效果
        kFadeInOutX,    //内容区横向滑动（左右切换）
        kFadeInOut,     //内容区淡入淡出
    };

    /** 设置是否显示动画效果
     * @param [in] bFadeSwitch 设置为 true 则显示，false 为不显示动画
     */
    void SetFadeSwitch(bool bFadeSwitch);

    /** 判断是否需要显示动画效果
     * @return 返回 true 显示动画，false 为不显示动画效果
     */
    bool IsFadeSwitch() const;

    /** 设置页面切换动画的类型
    */
    void SetFadeSwitchType(FadeSwitchType fadeSwitchType);

    /** 获取页面切换动画的类型
    */
    FadeSwitchType GetFadeSwitchType() const;

    /** 设置播放切换动画的定时器时间间隔（毫秒）
    * @param [in] frameIntervalMillSeconds 播放动画的定时器时间间隔（毫秒）
    */
    void SetFadeSwitchFrameIntervalMillSeconds(int32_t frameIntervalMillSeconds);

    /** 获取切换动画播放的定时器时间间隔（毫秒）
    */
    int32_t GetFadeSwitchFrameIntervalMillSeconds() const;

    /** 设置切换动画总的播放时间（毫秒）
    * @param [in] totalMillSeconds 动画总的播放时间（毫秒）
    */
    void SetFadeSwitchTotalMillSeconds(int32_t totalMillSeconds);

    /** 获取切换动画总的播放时间（毫秒）
    */
    int32_t GetFadeSwitchTotalMillSeconds() const;

    /** 设置切换动画缓动函数类型
    */
    void SetFadeSwitchEasingFunctionType(EasingFunctionType easingFunctionType);

    /** 获取切换动画缓动函数类型
    */
    EasingFunctionType GetFadeSwitchEasingFunctionType() const;

protected:
    /** 显示一个 TAB 项时，处理一些属性
     * @param [in] it TAB 项索引
     */
    void OnShowTabItem(size_t index);

    /** 隐藏一个 TAB 项时，处理一些属性
     * @param [in] it TAB 项索引
     */
    void OnHideTabItem(size_t index);

protected:
    /** 开始播放动画
     * @param [in] pNewItemControl 当前新选择的标签页的接口
     * @param [in] pOldItemControl 原来选择的标签页的接口
     */
    virtual bool StartSwitchItemAnimation(Control* pNewItemControl, Control* pOldItemControl);

protected:
    /** 设置可见状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetVisible(bool bChanged) override;

private:
    /** 根据控件索引选择一个子项
     * @param [in] nItemIndex 要选择的子项索引
     * @param [in] bFadeSwith 是否需要切换动画
     * @param [in] bCheckChanged 是否需要检查是否变化
     * @return 成功返回 true，否则返回 false
     */
    bool DoSelectItem(size_t nItemIndex, bool bFadeSwith, bool bCheckChanged);

    /** 设置页面切换动画的类型
    */
    void SetFadeSwitchTypeByString(const DString& fadeSwitchType);

    /** 开始播放动画(动态切换X坐标，内容区横向滑动)
     * @param [in] pNewItemControl 当前新选择的标签页的接口
     * @param [in] pOldItemControl 原来选择的标签页的接口
     */
    bool StartSwitchItemAnimationFadeInOutX(Control* pNewItemControl, Control* pOldItemControl);

    /** 开始播放动画(内容区淡入淡出)
     * @param [in] pNewItemControl 当前新选择的标签页的接口
     * @param [in] pOldItemControl 原来选择的标签页的接口
     */
    bool StartSwitchItemAnimationFadeInOut(Control* pNewItemControl, Control* pOldItemControl);

private:
    /** 动画的播放接口
    */
    std::unique_ptr<AnimationPlayer> m_pAnimationPlayer;

    /** 播放动画的定时器时间间隔（毫秒）
    */
    int32_t m_frameIntervalMillSeconds;

    /** 播放总的时间（毫秒）
    */
    int32_t m_totalMillSeconds;

    /** 当前选择项索引号
    */
    size_t m_nCurSel;

    /** 初始化时需要选择项的索引号
    */
    size_t m_nInitSel;

    /** 缓动函数类型
    */
    EasingFunctionType m_easingFunctionType;

    /** 是否需要切换动画
    */
    FadeSwitchType m_fadeSwithType;
};

} //namespace ui

#endif // UI_BOX_TABBOX_H_
