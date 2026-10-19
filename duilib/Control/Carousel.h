#ifndef UI_CONTROL_CAROUSEL_H_
#define UI_CONTROL_CAROUSEL_H_

#include "duilib/Core/Control.h"
#include "duilib/Image/StateImage.h"
#include "duilib/Core/Callback.h"
#include <vector>
#include <memory>

namespace ui {

/** 轮播索引变化数据（用于 kEventValueChanged 事件，pEventData 指向本结构）
*/
struct CarouselValue
{
    int32_t index = 0;      //新的当前索引
    int32_t oldIndex = 0;   //变化前的索引
};

/** Carousel 图片轮播控件（继承 Control，图片驱动）
 *
 *  用于首页横幅、广告位、图片集等场景，逐张展示幻灯片（同一时刻显示一张）。
 *  外观完全由 SVG 图片驱动（不做图形自绘）：幻灯片、左右切换按钮、底部页码指示点
 *  均以图片承载，颜色经 `svg_replace_colors` 把占位色映射到主题语义色，切换主题自动生效。
 *
 *  特性：
 *  - 左右切换按钮 + 底部可点击的页码指示点（当前页高亮）
 *  - `loop` 无限循环（首尾相接）
 *  - `auto_play` + `interval` 自动播放，`hover_pause` 鼠标悬停暂停
 *  - `transition_ms` 水平滑动切换动画（0 = 立即切换）
 *  - 按住鼠标左右拖拽切换
 *
 *  XML 属性：
 *  - item_image：幻灯片图片，`|` 分隔的图片属性串，每项如 "file='.../slide1.png'"
 *  - current_index：当前索引，默认 0，钳位到 [0, count-1]
 *  - loop：是否循环，默认 true
 *  - auto_play：是否自动播放，默认 false
 *  - interval：自动播放间隔（毫秒），默认 3000
 *  - hover_pause：鼠标悬停暂停自动播放，默认 true
 *  - transition_ms：切换动画时长（毫秒），默认 300，0 表示立即切换
 *  - show_indicator：是否显示页码指示点，默认 true
 *  - indicator_normal_image / indicator_active_image：指示点常态 / 当前页图片
 *  - indicator_size：指示点尺寸（像素，DPI 缩放），默认 8
 *  - indicator_gap：指示点间距（像素，DPI 缩放），默认 8
 *  - indicator_bottom：指示点距内容区底边距离（像素，DPI 缩放），默认 10
 *  - show_nav_button：是否显示左右切换按钮，默认 true
 *  - btn_prev_image / btn_next_image：上一张 / 下一张按钮图片
 *  - btn_prev_hovered_image / btn_next_hovered_image：按钮悬停态图片（可选）
 *  - btn_prev_pressed_image / btn_next_pressed_image：按钮按下态图片（可选，回退悬停 / 常态）
 *  - btn_size：切换按钮尺寸（像素，DPI 缩放），默认 32
 *  - btn_margin：切换按钮距内容区边缘的距离（像素，DPI 缩放），默认 12
 */
class DUILIB_API Carousel : public Control
{
    typedef Control BaseClass;

public:
    explicit Carousel(Window* pWindow);
    Carousel(const Carousel& r) = delete;
    Carousel& operator=(const Carousel& r) = delete;
    ~Carousel() override;

    /** 获取控件类型
    */
    DString GetType() const override;

    /** 设置XML属性
    */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 绘制幻灯片 / 切换按钮 / 页码指示点
    */
    void PaintStateImages(IRender* pRender) override;

    /** 鼠标左键按下：命中切换按钮 / 指示点，或进入拖拽
    */
    bool ButtonDown(const EventArgs& msg) override;

    /** 鼠标左键抬起：结束拖拽并吸附到目标页
    */
    bool ButtonUp(const EventArgs& msg) override;

    /** 鼠标移动：拖拽切换 / 更新按钮悬停态
    */
    bool MouseMove(const EventArgs& msg) override;

    /** 鼠标进入控件：悬停暂停自动播放
    */
    bool MouseEnter(const EventArgs& msg) override;

    /** 鼠标离开控件：恢复自动播放，清除交互态
    */
    bool MouseLeave(const EventArgs& msg) override;

    /** 窗口关联变化：清理定时器
    */
    void SetWindow(Window* pWindow) override;

    /** 清理图片缓存
    */
    void ClearImageCache() override;

public:
    /** 获取当前索引（无幻灯片时为 -1）
    */
    int32_t GetCurrentIndex() const { return m_nCurrentIndex; }

    /** 设置当前索引（钳位到有效范围，变化时派发 kEventValueChanged 事件）
    */
    void SetCurrentIndex(int32_t nIndex);

    /** 设置当前索引（不触发事件）
    */
    void SetCurrentIndexNoEvent(int32_t nIndex);

    /** 获取幻灯片数量
    */
    int32_t GetItemCount() const { return static_cast<int32_t>(m_itemImages.size()); }

    /** 追加一张幻灯片（图片属性串）
    */
    void AddItem(const DString& strImage);

    /** 在指定位置插入一张幻灯片
    */
    void InsertItem(int32_t nIndex, const DString& strImage);

    /** 移除指定位置的幻灯片
    */
    bool RemoveItem(int32_t nIndex);

    /** 清空所有幻灯片
    */
    void ClearItems();

    /** 批量设置幻灯片（替换全部）
    */
    void SetItems(const std::vector<DString>& items);

    /** 获取指定幻灯片的图片属性串
    */
    DString GetItem(int32_t nIndex) const;

    /** 前进一张（loop 时越界回到 0；否则越界返回 false），变化时派发事件
    */
    bool Next();

    /** 后退一张（loop 时越界回到最后一张；否则越界返回 false），变化时派发事件
    */
    bool Prev();

    /** 跳转到指定索引（越界返回 false），变化时派发事件
    */
    bool GoTo(int32_t nIndex);

    /** 前进一张（不触发事件）
    */
    bool NextNoEvent();

    /** 后退一张（不触发事件）
    */
    bool PrevNoEvent();

    /** 跳转到指定索引（不触发事件）
    */
    bool GoToNoEvent(int32_t nIndex);

    /** 是否循环
    */
    bool IsLoop() const { return m_bLoop; }

    /** 设置是否循环
    */
    void SetLoop(bool bLoop);

    /** 是否自动播放
    */
    bool IsAutoPlay() const { return m_bAutoPlay; }

    /** 设置是否自动播放
    */
    void SetAutoPlay(bool bAutoPlay);

    /** 获取自动播放间隔（毫秒）
    */
    int32_t GetInterval() const { return m_nInterval; }

    /** 设置自动播放间隔（毫秒）
    */
    void SetInterval(int32_t nIntervalMs);

    /** 鼠标悬停是否暂停自动播放
    */
    bool IsHoverPause() const { return m_bHoverPause; }

    /** 设置鼠标悬停是否暂停自动播放
    */
    void SetHoverPause(bool bHoverPause);

    /** 获取切换动画时长（毫秒）
    */
    int32_t GetTransitionMs() const { return m_nTransitionMs; }

    /** 设置切换动画时长（毫秒，0 = 立即切换）
    */
    void SetTransitionMs(int32_t nMs);

    /** 监听当前索引变化事件
    * @param [in] callback 索引变化后的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    * 参数说明：
    *   wParam: 0
    *   lParam: 0
    *   pEventData: 指向 CarouselValue（含 index / oldIndex），
    *               仅在本次回调派发期间有效，请勿保存该指针
    */
    void AttachCarouselChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID); }

private:
    /** 派发索引变化事件（m_bMuteEvent 为 true 时不派发）
    */
    void EmitValueChanged(int32_t nOldIndex);

    /** 把当前索引钳位到 [0, count-1]（无幻灯片时为 -1）
    */
    void ClampCurrentIndex();

    /** 重建幻灯片图片缓存（数量与 m_itemImages 同步）
    */
    void RebuildSlideImages();

    /** 设置指定幻灯片的图片属性串
    */
    void SetSlideImageAt(int32_t nIndex, const DString& strImage);

    /** 获取指定幻灯片的图片接口
    */
    Image* GetSlideImagePtr(int32_t nIndex) const;

    /** 计算内容区（去内边距）
    */
    UiRect GetContentRect() const;

    /** 计算上一张 / 下一张按钮矩形（控件坐标）
    */
    UiRect GetNavButtonRect(bool bNext) const;

    /** 计算第 N 个指示点矩形（控件坐标）
    */
    UiRect GetIndicatorRect(int32_t nIndex) const;

    /** 命中切换按钮：返回 -1 = 上一张，1 = 下一张，0 = 未命中
    */
    int32_t HitTestNavButton(const UiPoint& ptMouse) const;

    /** 命中指示点：返回指示点索引，未命中返回 -1
    */
    int32_t HitTestIndicator(const UiPoint& ptMouse) const;

    /** 绘制单张图片（带可选透明度）
    */
    void PaintImageAt(IRender* pRender, Image* pImage, const UiRect& rcDest, int32_t nFade) const;

    /** 绘制一张幻灯片（目标区域为控件坐标）
    */
    void PaintSlide(IRender* pRender, int32_t nIndex, int32_t nOffsetX, const UiRect& rcContent, int32_t nFade);

    /** 绘制切换按钮
    */
    void PaintNavButton(IRender* pRender, bool bNext, int32_t nFade);

    /** 绘制页码指示点
    */
    void PaintIndicators(IRender* pRender, int32_t nFade);

    /** 索引变化后的统一处理（提交新索引、启动动画、更新自动播放）
    */
    void ApplyIndexChange(int32_t nNewIndex);

    // ---- 切换动画 ----
    /** 启动从当前显示位置到目标索引的切换动画
    */
    void StartAnimation(int32_t nFromIndex, int32_t nToIndex);

    /** 停止切换动画并复位动画状态
    */
    void StopAnimation();

    /** 动画定时器回调
    */
    void OnAnimationTick();

    /** 切换动画是否进行中
    */
    bool IsAnimating() const { return m_bAnimating; }

    /** 当前动画进度 [0,1]
    */
    float GetAnimProgress() const;

    // ---- 自动播放 ----
    /** 更新自动播放定时器（按当前配置与状态启动 / 停止）
    */
    void UpdateAutoPlayTimer();

    /** 自动播放定时器回调
    */
    void OnAutoPlayTick();

    /** 当前是否应暂停自动播放
    */
    bool IsAutoPlayPaused() const;

    /** 停止全部定时器（析构 / 窗口变化时）
    */
    void StopAllTimers();

private:
    /** 幻灯片图片属性串列表
    */
    std::vector<DString> m_itemImages;

    /** 幻灯片图片缓存（与 m_itemImages 一一对应）
    */
    std::vector<std::unique_ptr<StateImage>> m_slideImages;

    /** 上一张 / 下一张按钮图片（常态存 kControlStateNormal，悬停 / 按下态分别存对应状态）
    */
    StateImage m_prevImage;
    StateImage m_nextImage;

    /** 页码指示点常态 / 当前页图片
    */
    StateImage m_indicatorNormalImage;
    StateImage m_indicatorActiveImage;

    /** 当前索引（无幻灯片时为 -1）
    */
    int32_t m_nCurrentIndex;

    /** 是否循环
    */
    bool m_bLoop;

    /** 是否自动播放
    */
    bool m_bAutoPlay;

    /** 自动播放间隔（毫秒）
    */
    int32_t m_nInterval;

    /** 鼠标悬停是否暂停自动播放
    */
    bool m_bHoverPause;

    /** 切换动画时长（毫秒，0 = 立即切换）
    */
    int32_t m_nTransitionMs;

    /** 是否显示页码指示点
    */
    bool m_bShowIndicator;

    /** 是否显示左右切换按钮
    */
    bool m_bShowNavButton;

    /** 指示点尺寸 / 间距 / 距底边距离（DPI 缩放前原始值）
    */
    int32_t m_nIndicatorSize;
    int32_t m_nIndicatorGap;
    int32_t m_nIndicatorBottom;

    /** 切换按钮尺寸 / 距边缘距离（DPI 缩放前原始值）
    */
    int32_t m_nBtnSize;
    int32_t m_nBtnMargin;

    /** 切换动画状态
    */
    bool m_bAnimating;
    int32_t m_nAnimFromIndex;   //动画起始索引
    int32_t m_nAnimToIndex;     //动画目标索引
    int32_t m_nAnimDir;         //动画方向：+1 前进，-1 后退
    int32_t m_nAnimElapsed;     //已播放毫秒数
    bool m_bAnimTimerStarted;   //动画定时器是否已启动
    size_t m_nAnimTimerId;      //动画定时器 ID

    /** 自动播放定时器
    */
    bool m_bAutoTimerStarted;   //自动播放定时器是否已启动
    size_t m_nAutoTimerId;      //自动播放定时器 ID

    /** 动画定时器取消标志
    */
    WeakCallbackFlag m_animFlag;

    /** 自动播放定时器取消标志
    */
    WeakCallbackFlag m_autoFlag;

    /** 鼠标是否悬停
    */
    bool m_bHovering;

    /** 鼠标悬停的切换按钮：-1 = 上一张，1 = 下一张，0 = 无
    */
    int32_t m_nHoverBtn;

    /** 鼠标按下的切换按钮：-1 = 上一张，1 = 下一张，0 = 无
    */
    int32_t m_nPressedBtn;

    /** 是否正在拖拽
    */
    bool m_bDragging;

    /** 拖拽是否已产生有效位移
    */
    bool m_bDragMoved;

    /** 拖拽起点（鼠标位置）
    */
    UiPoint m_ptDragStart;

    /** 拖拽水平位移（像素，正数向右）
    */
    int32_t m_nDragOffset;

    /** 是否抑制事件派发（NoEvent 期间为 true）
    */
    bool m_bMuteEvent;
};

} // namespace ui

#endif // UI_CONTROL_CAROUSEL_H_
