#ifndef UI_CONTROL_PAGINATION_H_
#define UI_CONTROL_PAGINATION_H_

#include "duilib/Box/HBox.h"
#include "duilib/Core/Callback.h"

namespace ui {

/** 页码变化数据（用于 kEventValueChanged 事件，pEventData 指向本结构）
*/
struct PaginationValue
{
    int32_t page = 1;           //新的当前页码（1 起）
    int32_t oldPage = 1;        //变化前的页码
    int32_t pageSize = 0;       //新的每页条数（0 表示未启用 / 未知）
    int32_t oldPageSize = 0;    //变化前的每页条数
};

/** Pagination 分页器控件（继承 HBox，数据驱动，动态生成页码按钮）
 *
 *  用于报表 / 表格 / 列表等场景的页码导航：上一页 / 下一页 + 页码按钮，
 *  当前页按钮呈选中态；页码超出显示上限时以 `…` 收纳，始终保留首页 / 末页与当前页附近。
 *
 *  皮肤驱动：页码按钮通过 `button_class` 引用 global.xml 中的样式（含常态 / 悬停 / 选中态），
 *  颜色走语义色，C++ 侧不写死色值。
 *
 *  XML 属性：
 *  - page_count（别名 total_page）：总页数，默认 0
 *  - current_page（别名 page）：当前页码（1 起），默认 1，钳位到 [1, page_count]
 *  - total_count：总记录数（与 page_size 配合推导 page_count，可选）
 *  - page_size：每页记录数（可选；与 total_count 同时 > 0 时优先推导 page_count）
 *  - show_page_size：是否显示每页条数选择器（可选功能，默认 false）
 *  - page_size_options：每页条数候选项，默认 10,20,50,100（逗号或 | 分隔）
 *  - page_size_format / page_size_format_id：候选项文本格式，含 %d 占位符（如 "%d 条/页"），ID 优先
 *  - page_size_class：每页条数选择器引用的皮肤 Class，默认 pagination_page_size
 *  - page_size_width：每页条数选择器宽度（像素，DPI 缩放），默认 96
 *  - max_visible_count：序号区最多显示的页码按钮数（含首尾），默认 7，超出用 `…` 收纳
 *  - show_prev_next：是否显示上一页 / 下一页，默认 true
 *  - show_first_last：是否显示首页 / 末页按钮，默认 false
 *  - prev_text / next_text：上一页 / 下一页文本，默认 ‹ / ›
 *  - first_text / last_text：首页 / 末页文本，默认 « / »
 *  - ellipsis_text：省略号文本，默认 …
 *  - button_class：页码按钮引用的皮肤 Class，默认 pagination_button
 *  - button_width / button_height：页码按钮尺寸（像素，DPI 缩放），默认 32 / 32，0 = 自适应
 */
class DUILIB_API Pagination : public HBox
{
    typedef HBox BaseClass;

public:
    explicit Pagination(Window* pWindow);
    Pagination(const Pagination& r) = delete;
    Pagination& operator=(const Pagination& r) = delete;
    ~Pagination() override;

    /** 获取控件类型
    */
    DString GetType() const override;

    /** 设置XML属性
    */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 语言发生变化，刷新每页条数选择器的文本
    */
    void OnLanguageChanged(bool bRedraw) override;

public:
    /** 获取总页数
    */
    int32_t GetPageCount() const { return m_nPageCount; }

    /** 设置总页数（变化时重建页码按钮）
    */
    void SetPageCount(int32_t nPageCount);

    /** 获取当前页码（1 起）
    */
    int32_t GetCurrentPage() const { return m_nCurrentPage; }

    /** 设置当前页码（钳位到 [1, page_count]，变化时派发 kEventValueChanged 事件）
    */
    void SetCurrentPage(int32_t nPage);

    /** 设置当前页码（不触发事件）
    */
    void SetCurrentPageNoEvent(int32_t nPage);

    /** 由总记录数与每页记录数推导总页数（并重建页码按钮）
    */
    void SetTotalCount(int32_t nTotalCount, int32_t nPageSize);

    /** 前进一页（越界返回 false），变化时派发事件
    */
    bool Next();

    /** 后退一页（越界返回 false），变化时派发事件
    */
    bool Prev();

    /** 跳转到指定页（越界返回 false），变化时派发事件
    */
    bool GoTo(int32_t nPage);

    /** 获取序号区最多显示的页码按钮数
    */
    int32_t GetMaxVisibleCount() const { return m_nMaxVisibleCount; }

    /** 设置序号区最多显示的页码按钮数
    */
    void SetMaxVisibleCount(int32_t nCount);

    /** 设置页码按钮引用的皮肤 Class
    */
    void SetButtonClass(const DString& strClass);

    /** 获取页码按钮引用的皮肤 Class
    */
    DString GetButtonClass() const { return m_strButtonClass; }

public:
    /** 是否显示每页条数选择器（默认 false）
    */
    bool IsShowPageSize() const { return m_bShowPageSize; }

    /** 设置是否显示每页条数选择器
    */
    void SetShowPageSize(bool bShow);

    /** 获取当前每页条数（0 表示未设置）
    */
    int32_t GetPageSize() const { return m_nPageSize; }

    /** 设置每页条数（变化时重置到第 1 页并派发 kEventValueChanged 事件）
    * @param [in] nPageSize 每页条数，必须 > 0
    */
    void SetPageSize(int32_t nPageSize);

    /** 获取每页条数候选项
    */
    const std::vector<int32_t>& GetPageSizeOptions() const { return m_pageSizeOptions; }

    /** 设置每页条数候选项（自动去重并过滤非正数）
    */
    void SetPageSizeOptions(const std::vector<int32_t>& options);

    /** 设置每页条数候选项（逗号或 | 分隔的字符串）
    */
    void SetPageSizeOptions(const DString& strOptions);

    /** 设置每页条数候选项文本格式（含 %d 占位符，如 "%d 条/页"）
    */
    void SetPageSizeFormat(const DString& strFormat);

    /** 设置每页条数候选项文本格式的语言 ID（优先于 SetPageSizeFormat）
    */
    void SetPageSizeFormatId(const DString& strFormatId);

    /** 设置每页条数选择器引用的皮肤 Class
    */
    void SetPageSizeClass(const DString& strClass);

    /** 设置每页条数选择器宽度（像素，DPI 缩放；0 = 自适应）
    */
    void SetPageSizeWidth(int32_t nWidth);

    /** 监听页码变化事件
    * @param [in] callback 页码变化后的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    * 参数说明：
    *   wParam: 0
    *   lParam: 0
    *   pEventData: 指向 PaginationValue（含 page / oldPage / pageSize / oldPageSize），
    *               仅在本次回调派发期间有效，请勿保存该指针
    */
    void AttachPageChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID); }

private:
    /** 请求重建页码按钮（通过一次性定时器推迟到当前事件派发返回之后执行，
    *  避免在子控件自身的点击事件中删除该子控件）
    */
    void RequestRebuild();

    /** 重建定时器回调
    */
    void OnRebuildTimer();

    /** 重建页码按钮（先移除全部按钮，再按数据重新创建）
    */
    void RebuildButtons();

    /** 计算序号区要显示的页码序列（0 表示省略号）
    */
    std::vector<int32_t> CalcVisiblePages() const;

    /** 创建一个文本按钮（首页 / 上一页 / 下一页 / 末页 / 省略号）
    */
    Control* CreateTextButton(const DString& strText);

    /** 创建一个页码按钮（CheckBox，支持选中态高亮）
    * @param [in] nPage 该按钮对应的页码
    * @param [in] bSelected 是否为当前页（选中态）
    */
    Control* CreatePageButton(int32_t nPage, bool bSelected);

    /** 创建一个每页条数选择器（Combo，仅在 show_page_size 为 true 时创建）
    */
    Control* CreatePageSizeCombo();

    /** 格式化一个每页条数候选项的显示文本（应用 page_size_format / page_size_format_id）
    */
    DString FormatPageSizeText(int32_t nPageSize) const;

    /** 派发页码变化事件（m_bMuteEvent 为 true 时不派发）
    * @param [in] nOldPage 变化前的页码
    * @param [in] nOldPageSize 变化前的每页条数
    */
    void EmitPageChanged(int32_t nOldPage, int32_t nOldPageSize);

    /** 把当前页码钳位到 [1, page_count]
    */
    void ClampCurrentPage();

private:
    /** 总页数
    */
    int32_t m_nPageCount;

    /** 当前页码（1 起）
    */
    int32_t m_nCurrentPage;

    /** 总记录数（可选，用于推导总页数）
    */
    int32_t m_nTotalCount;

    /** 每页记录数（可选）
    */
    int32_t m_nPageSize;

    /** 序号区最多显示的页码按钮数
    */
    int32_t m_nMaxVisibleCount;

    /** 是否显示上一页 / 下一页
    */
    bool m_bShowPrevNext;

    /** 是否显示首页 / 末页
    */
    bool m_bShowFirstLast;

    /** 上一页 / 下一页 / 首页 / 末页 / 省略号文本
    */
    DString m_strPrevText;
    DString m_strNextText;
    DString m_strFirstText;
    DString m_strLastText;
    DString m_strEllipsisText;

    /** 页码按钮引用的皮肤 Class
    */
    DString m_strButtonClass;

    /** 页码按钮宽度 / 高度（DPI 缩放前原始值，0 = 自适应）
    */
    int32_t m_nButtonWidth;
    int32_t m_nButtonHeight;

    /** 是否有待执行的重建
    */
    bool m_bRebuildPending;

    /** 是否抑制事件派发（NoEvent 期间为 true）
    */
    bool m_bMuteEvent;

    /** 是否显示每页条数选择器
    */
    bool m_bShowPageSize;

    /** 每页条数候选项
    */
    std::vector<int32_t> m_pageSizeOptions;

    /** 每页条数选择器引用的皮肤 Class
    */
    DString m_strPageSizeClass;

    /** 每页条数候选项文本格式（含 %d 占位符）
    */
    DString m_strPageSizeFormat;

    /** 每页条数候选项文本格式的语言 ID（优先于 m_strPageSizeFormat）
    */
    DString m_strPageSizeFormatId;

    /** 每页条数选择器宽度（DPI 缩放前原始值，0 = 自适应）
    */
    int32_t m_nPageSizeWidth;

    /** 重建定时器取消标志
    */
    WeakCallbackFlag m_rebuildFlag;
};

} // namespace ui

#endif // UI_CONTROL_PAGINATION_H_
