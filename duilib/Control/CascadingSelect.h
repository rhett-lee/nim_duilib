#ifndef UI_CONTROL_CASCADINGSELECT_H_
#define UI_CONTROL_CASCADINGSELECT_H_

#include "duilib/Core/Control.h"
#include <functional>
#include <vector>

namespace ui {

class Flyout;

/** CascadingSelect 选择变化数据（用于 kEventValueChanged 事件，pEventData 指向本结构）
 */
struct CascadingSelectValue
{
    /** 每级选中索引（长度等于数据深度），-1 表示该级未选
     */
    std::vector<int32_t> indexes;

    /** 以 value_separator 连接的显示文本，未选择时为空
     */
    DString text;

    /** 以 path_separator 连接的完整路径文本，未选择时为空
     */
    DString path;
};

/** CascadingSelect 级联选择器（继承 Control，纯自绘，内嵌多列形态）
 *
 *  解决"父项变更后子项需要自动过滤、深层旧选择需要自动重置"的语义：
 *  控件在自身矩形内按数据深度绘制多列，第 i 列的内容由前 i 级选择决定；
 *  点击某一列后，深层选择立即被截断（自动重置），子列内容立即切换（自动过滤）。
 *  与 Combo（只能模拟两级、无重置语义）和 TreeView（树形展开，非列式）语义不同，故独立实现。
 *
 *  两种形态：
 *  - 内嵌形态（默认）：在自身矩形内直接绘制多列；
 *  - 下拉列表框形态（dropdown="true"）：自身绘制一个选择框，点击后弹出 Flyout 浮层，
 *    浮层内是同一套多列级联逻辑；支持清空按钮、自定义显示文本、移入展开与禁用路径。
 *
 *  数据模型（XML 为路径列表，相同前缀自动合并为同一节点）：
 *  - items：字面路径，`|` 分隔多条路径，路径内用 path_separator 分隔层级
 *           示例："华东/上海/浦东|华东/上海/徐汇|华南/深圳/南山"
 *  - items_id：同上，但每段是语言 ID（语言切换自动刷新文本并保持选择）
 *
 *  XML 属性：
 *  - items / items_id：路径数据（见上）
 *  - path_separator：路径层级分隔符，默认 "/"
 *  - value_separator：GetSelectionText 使用的连接符，默认 " / "
 *  - column_width：固定列宽（像素，DPI 缩放）；0 = 按控件宽度等分，默认 0
 *  - row_height：行高（像素，DPI 缩放），默认 28
 *  - text_padding：行内文本左右内边距（像素，DPI 缩放），默认 10
 *  - disabled_paths：禁用路径列表，`|` 分隔；路径等于节点文本路径或 ID 路径、
 *                    或作为其前缀时，该节点及整棵子树禁用
 *  - item_color / selected_color / disabled_color：文本颜色
 *  - hover_bg_color / selected_bg_color：悬停（未选中）/ 选中行背景色
 *  - divider_color：列分隔线颜色；chevron_color：父项右侧箭头颜色
 *  - scrollbar_color：列溢出时右缘滚动条颜色
 *  - font：字体 ID，默认 system_regular_14
 *
 *  下拉列表框形态额外属性：
 *  - dropdown：true = 下拉列表框形态，默认 false
 *  - placeholder / placeholder_id：未选择时的提示文本
 *  - allow_clear：是否显示清空按钮（悬停且已有选择时显示），默认 true
 *  - expand_on_hover：鼠标悬停在父项上时自动展开子列，默认 false
 *  - popup_width：浮层宽度（像素）；0 = 按列数 × 列宽自动计算，显式值不足时自动放宽到可容纳全部列，默认 0
 *  - popup_max_rows：浮层最多可见行数，默认 8
 *  - popup_class：浮层内级联列表的样式 Class，默认 "cascading_select_popup"
 *  - text_color / placeholder_color / disabled_text_color：选择框文本颜色
 *  - bg_color / border_color / hover_border_color：选择框背景与边框颜色
 *  - clear_color / clear_hover_color：清空按钮颜色
 *  - border_round / border_size：选择框圆角与边框宽度（像素）
 *
 *  说明：列内容超出可见行数时，在对应列上滚轮可滚动（每格 3 行）；
 *        禁用优先于选中/悬停；未选满前缀的列留空（不绘制内容）。
 *        本控件暂不支持键盘导航。
 */
class DUILIB_API CascadingSelect : public Control
{
    typedef Control BaseClass;

public:
    explicit CascadingSelect(Window* pWindow);
    CascadingSelect(const CascadingSelect& r) = delete;
    CascadingSelect& operator=(const CascadingSelect& r) = delete;
    ~CascadingSelect() override;

    /** 获取控件类型（DUI_CTR_CASCADING_SELECT）
     */
    DString GetType() const override;

    /** 设置XML属性
     */
    void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 绘制列、行、分隔线与滚动条
     */
    void PaintStateImages(IRender* pRender) override;

    /** 语言切换：重建 items_id 对应的显示文本（保持选择与滚动状态）
     */
    void OnLanguageChanged(bool bRedraw) override;

    /** 鼠标左键按下：选择某列中的一项，并截断深层选择
     */
    bool ButtonDown(const EventArgs& msg) override;

    /** 鼠标移动：更新悬停行与光标
     */
    bool MouseMove(const EventArgs& msg) override;

    /** 鼠标离开：清除悬停行
     */
    bool MouseLeave(const EventArgs& msg) override;

    /** 鼠标滚轮：滚动鼠标所在列
     */
    bool MouseWheel(const EventArgs& msg) override;

public:
    /** 设置路径数据（字面路径，替换全部；会清空当前选择）
     */
    void SetItems(const std::vector<DString>& paths);

    /** 设置路径数据（每段为语言 ID，替换全部；会清空当前选择）
     */
    void SetItemIds(const std::vector<DString>& pathIds);

    /** 追加一条路径（相同前缀自动合并；尽量保留仍有效的选择）
     * @param [in] path 完整路径串；items_id 模式下每段为语言 ID，items 模式下为字面文本
     */
    void AddPath(const DString& path);

    /** 清空全部路径数据（同时清空选择与滚动状态）
     */
    void ClearItems();

    /** 获取数据深度（列数）
     */
    size_t GetDepth() const;

    /** 获取第 level 列当前可见的条目数量（该列未激活时返回 0）
     */
    size_t GetItemCount(size_t level) const;

    /** 获取第 level 列、第 index 项的显示文本
     * @return 索引有效返回 true
     */
    bool GetItemText(size_t level, size_t index, DString& text) const;

    /** 获取当前选择（长度等于深度，每级 -1 表示未选）
     */
    const std::vector<int32_t>& GetSelection() const { return m_selection; }

    /** 设置选择（深层自动截断；禁用/越界的部分被忽略）
     * @param [in] indexes 每级索引，可只给前几级
     * @param [in] bTriggerEvent 选择实际变化时是否派发事件
     * @return 选择是否发生变化
     */
    bool SetSelection(const std::vector<size_t>& indexes, bool bTriggerEvent = true);

    /** 按 GetSelectionText 格式的文本设置选择（逐级精确匹配显示文本）
     * @param [in] strText 如 "华东 / 上海 / 浦东"
     * @param [in] bTriggerEvent 选择实际变化时是否派发事件
     * @return 是否成功匹配并改变选择
     */
    bool SetSelectionByText(const DString& strText, bool bTriggerEvent = true);

    /** 清空选择
     * @param [in] bTriggerEvent 选择实际变化时是否派发事件
     */
    void ClearSelection(bool bTriggerEvent = true);

    /** 获取以 value_separator 连接的选中文本，未选择时为空
     */
    DString GetSelectionText() const;

    /** 获取以 path_separator 连接的选中路径文本，未选择时为空
     */
    DString GetSelectionPath() const;

    /** 设置禁用路径列表（替换全部；路径匹配规则见类说明）
     */
    void SetDisabledPaths(const std::vector<DString>& paths);

    /** 获取禁用路径列表
     */
    const std::vector<DString>& GetDisabledPaths() const { return m_disabledPaths; }

    /** 判断第 level 列第 index 项是否被禁用（含祖先禁用导致的禁用）
     */
    bool IsItemDisabled(size_t level, size_t index) const;

    /** 设置路径层级分隔符（默认 "/"）
     */
    void SetPathSeparator(const DString& strSeparator);

    /** 获取路径层级分隔符
     */
    const DString& GetPathSeparator() const { return m_strPathSeparator; }

    /** 设置选择文本连接符（默认 " / "）
     */
    void SetValueSeparator(const DString& strSeparator);

    /** 获取选择文本连接符
     */
    const DString& GetValueSeparator() const { return m_strValueSeparator; }

    /** 设置固定列宽（像素，DPI 缩放；0 = 等分控件宽度）
     */
    void SetColumnWidth(int32_t nWidth);

    /** 获取固定列宽（未缩放原始值；0 = 等分）
     */
    int32_t GetColumnWidth() const { return m_nColumnWidth; }

    /** 设置行高（像素，DPI 缩放）
     */
    void SetRowHeight(int32_t nHeight);

    /** 获取行高（未缩放原始值）
     */
    int32_t GetRowHeight() const { return m_nRowHeight; }

    /** 设置行内文本左右内边距（像素，DPI 缩放）
     */
    void SetTextPadding(int32_t nPadding);

    /** 获取行内文本左右内边距（未缩放原始值）
     */
    int32_t GetTextPadding() const { return m_nTextPadding; }

    /** 设置普通文本颜色
     */
    void SetItemColor(const DString& strColor);
    /** 获取普通文本颜色 */
    const DString& GetItemColor() const { return m_strItemColor; }

    /** 设置选中文本颜色
     */
    void SetSelectedColor(const DString& strColor);
    /** 获取选中文本颜色 */
    const DString& GetSelectedColor() const { return m_strSelectedColor; }

    /** 设置禁用文本颜色
     */
    void SetDisabledColor(const DString& strColor);
    /** 获取禁用文本颜色 */
    const DString& GetDisabledColor() const { return m_strDisabledColor; }

    /** 设置悬停行背景色（留空则不绘制）
     */
    void SetHoverBgColor(const DString& strColor);
    /** 获取悬停行背景色 */
    const DString& GetHoverBgColor() const { return m_strHoverBgColor; }

    /** 设置选中行背景色（留空则不绘制）
     */
    void SetSelectedBgColor(const DString& strColor);
    /** 获取选中行背景色 */
    const DString& GetSelectedBgColor() const { return m_strSelectedBgColor; }

    /** 设置列分隔线颜色
     */
    void SetDividerColor(const DString& strColor);
    /** 获取列分隔线颜色 */
    const DString& GetDividerColor() const { return m_strDividerColor; }

    /** 设置父项箭头颜色
     */
    void SetChevronColor(const DString& strColor);
    /** 获取父项箭头颜色 */
    const DString& GetChevronColor() const { return m_strChevronColor; }

    /** 设置滚动条颜色
     */
    void SetScrollbarColor(const DString& strColor);
    /** 获取滚动条颜色 */
    const DString& GetScrollbarColor() const { return m_strScrollbarColor; }

    /** 设置字体 ID
     */
    void SetFont(const DString& strFontId);
    /** 获取字体 ID */
    const DString& GetFont() const { return m_strFont; }

    /** 监听选择变化事件
     * @param [in] callback 选择变化后的回调函数
     * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
     * 参数说明：
     *   wParam: 4（级联选择模式标记）
     *   lParam: 0
     *   pEventData: 指向 CascadingSelectValue（含 indexes / text / path），
     *               仅在本次回调派发期间有效，请勿保存该指针
     */
    void AttachSelectionChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID); }

public:
    /** 自定义显示文本回调：返回空串时回退为默认的 value_separator 文本
     */
    typedef std::function<DString(const CascadingSelectValue& value)> DisplayTextEvent;

    /** 是否下拉列表框形态
     */
    bool IsDropdown() const { return m_bDropdown; }

    /** 设置下拉列表框形态（true = 选择框 + 浮层；false = 内嵌多列）
     */
    void SetDropdown(bool bDropdown);

    /** 是否移入展开（悬停父项时自动展开子列）
     */
    bool IsExpandOnHover() const { return m_bExpandOnHover; }

    /** 设置移入展开
     */
    void SetExpandOnHover(bool bExpand);

    /** 是否允许清空按钮（悬停且已有选择时显示）
     */
    bool IsAllowClear() const { return m_bAllowClear; }

    /** 设置是否允许清空按钮
     */
    void SetAllowClear(bool bAllow);

    /** 设置未选择时的提示文本（字面文本）
     */
    void SetPlaceholder(const DString& strText);
    /** 获取未选择时的提示文本（多语言 ID 优先） */
    DString GetPlaceholder() const;
    /** 设置未选择时的提示文本（多语言 ID） */
    void SetPlaceholderId(const DString& strTextId);

    /** 设置浮层宽度（像素；0 = 按列数 × 列宽自动计算）
     */
    void SetPopupWidth(int32_t nWidth);
    /** 获取浮层宽度（0 = 自动） */
    int32_t GetPopupWidth() const { return m_nPopupWidth; }

    /** 设置浮层最多可见行数
     */
    void SetPopupMaxRows(int32_t nRows);
    /** 获取浮层最多可见行数 */
    int32_t GetPopupMaxRows() const { return m_nPopupMaxRows; }

    /** 设置浮层内级联列表的样式 Class
     */
    void SetPopupClass(const DString& strClass);
    /** 获取浮层内级联列表的样式 Class */
    const DString& GetPopupClass() const { return m_strPopupClass; }

    /** 获取选择框当前显示文本（自定义显示回调优先，否则为默认选择文本）
     */
    DString GetDisplayText() const;

    /** 监听自定义显示文本（返回空串时回退默认文本）
     */
    void AttachDisplayText(const DisplayTextEvent& callback);

    /** 设置选择框文本颜色 */
    void SetTextColor(const DString& strColor);
    /** 获取选择框文本颜色 */
    const DString& GetTextColor() const { return m_strTextColor; }

    /** 设置选择框提示文本颜色 */
    void SetPlaceholderColor(const DString& strColor);
    /** 获取选择框提示文本颜色 */
    const DString& GetPlaceholderColor() const { return m_strPlaceholderColor; }

    /** 设置选择框禁用文本颜色 */
    void SetDisabledTextColor(const DString& strColor);
    /** 获取选择框禁用文本颜色 */
    const DString& GetDisabledTextColor() const { return m_strDisabledTextColor; }

    /** 设置选择框背景色（留空则不绘制） */
    void SetBgColor(const DString& strColor);
    /** 获取选择框背景色 */
    const DString& GetBgColor() const { return m_strBgColor; }

    /** 设置选择框边框颜色 */
    void SetBorderColor(const DString& strColor);
    /** 获取选择框边框颜色 */
    const DString& GetBorderColor() const { return m_strBorderColor; }
    /** 引入基类的 GetBorderColor(ControlStateType) 重载，避免被本类的无参版本隐藏
        （消除 GCC -Woverloaded-virtual 警告） */
    using BaseClass::GetBorderColor;

    /** 设置选择框悬停/展开状态的边框颜色 */
    void SetHoverBorderColor(const DString& strColor);
    /** 获取选择框悬停/展开状态的边框颜色 */
    const DString& GetHoverBorderColor() const { return m_strHoverBorderColor; }

    /** 设置清空按钮颜色 */
    void SetClearColor(const DString& strColor);
    /** 获取清空按钮颜色 */
    const DString& GetClearColor() const { return m_strClearColor; }

    /** 设置清空按钮悬停颜色 */
    void SetClearHoverColor(const DString& strColor);
    /** 获取清空按钮悬停颜色 */
    const DString& GetClearHoverColor() const { return m_strClearHoverColor; }

    /** 设置选择框圆角（像素） */
    void SetBorderRound(int32_t nRound);
    /** 获取选择框圆角 */
    int32_t GetBorderRound() const { return m_nBorderRound; }

    /** 设置选择框边框宽度（像素） */
    void SetBorderSize(int32_t nSize);
    /** 获取选择框边框宽度 */
    int32_t GetBorderSize() const { return m_nBorderSize; }

private:
    /** 树节点
     */
    struct Node
    {
        DString text;                  //显示文本
        DString textId;                //语言 ID（items_id 模式；字面路径为空）
        std::vector<Node> children;    //子节点
    };

    /** 列布局（绘制与命中检测共用）
     */
    struct ColumnLayout
    {
        UiRect rcColumn;                    //列矩形（已去掉控件内边距）
        bool bActive = false;               //该列是否已激活（前缀选满且节点非空）
        std::vector<int32_t> nodeIndexes;   //每行对应的节点索引（与 itemRects 一一对应）
        std::vector<UiRect> itemRects;      //每行的命中/绘制矩形
    };

private:
    /** 按路径列表重建树（相同前缀合并；保持节点首次出现顺序）
     * @param [in] paths 路径列表（每项已按 path_separator 分段前的完整串）
     * @param [in] bUseIds true = 每段作为语言 ID 解析显示文本
     */
    void BuildTree(const std::vector<DString>& paths, bool bUseIds);

    /** 向现有树插入一条路径（相同前缀合并）
     * @param [in] path 完整路径串
     * @param [in] bUseIds true = 每段作为语言 ID 解析显示文本
     */
    void InsertPath(const DString& path, bool bUseIds);

    /** 按已保存的语言 ID 串重建树（语言切换时调用，保留选择）
     */
    void RebuildFromIds();

    /** 获取指定选择前缀下第 level 列的兄弟节点列表；前缀未选满返回 nullptr
     */
    const std::vector<Node>* GetSiblings(const std::vector<int32_t>& selection, size_t level) const;

    /** 获取第 level 列当前可见的兄弟节点列表（基于当前选择）
     */
    const std::vector<Node>* GetSiblings(size_t level) const;

    /** 获取指定节点（基于当前选择）
     */
    const Node* GetNode(size_t level, size_t index) const;

    /** 构造节点路径文本（使用给定选择前缀，ancestors 取 selection[0..level-1]）
     * @param [in] selection 选择前缀
     * @param [out] textPath 显示文本路径
     * @param [out] idPath ID 路径（任一祖先无 ID 时为空）
     */
    void MakeNodePath(const std::vector<int32_t>& selection, size_t level, size_t index,
                      DString& textPath, DString& idPath) const;

    /** 判断禁用路径是否命中节点或其祖先（前缀匹配，使用给定选择前缀）
     */
    bool IsNodeDisabled(const std::vector<int32_t>& selection, size_t level, size_t index) const;

    /** 把选择钳位到有效范围（无效/禁用项及其深层选择被清空）
     */
    void ClampSelection();

    /** 应用新选择并派发事件（选择变化后统一入口）
     * @param [in] newSelection 新选择（内部会补齐到数据深度）
     * @param [in] bTriggerEvent 是否派发事件
     * @return 选择是否变化
     */
    bool CommitSelection(std::vector<int32_t> newSelection, bool bTriggerEvent);

    /** 派发选择变化事件（栈上临时数据）
     * @param [in] oldSelection 变化前的选择
     */
    void EmitSelectionChanged(const std::vector<int32_t>& oldSelection);

    /** 把选择转换为文本（text 用 value_separator，path 用 path_separator）
     */
    void MakeSelectionText(const std::vector<int32_t>& selection, DString& text, DString& path) const;

    /** 构建列布局（用于绘制与命中检测）
     */
    void BuildLayout(const UiRect& rc);

    /** 命中检测：返回鼠标所在的列与节点索引
     * @return 命中返回 true
     */
    bool HitTest(const UiPoint& ptMouse, size_t& level, size_t& nodeIndex) const;

    /** 获取内容区（控件矩形去掉内边距）
     */
    UiRect GetContentRect() const;

    /** 计算第 level 列的可见行数
     */
    int32_t GetVisibleRowCount(const UiRect& rcContent) const;

    /** 把指定列的滚动偏移钳位到有效范围
     */
    void ClampColumnScroll(size_t level);

    /** 重置所有列的滚动偏移
     */
    void ResetAllScroll();

    /** 绘制单行文本（自动省略号）
     */
    void DrawItemText(IRender* pRender, const DString& strText, const UiRect& rc, UiColor textColor);

    /** 绘制父项右侧箭头
     */
    void DrawChevron(IRender* pRender, const UiRect& rc, UiColor color);

    /** 绘制列滚动条
     */
    void DrawScrollbar(IRender* pRender, const UiRect& rcColumn, int32_t nItemCount, int32_t nVisibleRows, int32_t nOffset);

    /** 根据悬停行更新鼠标光标
     */
    void UpdateCursor();

    /** 下拉形态：绘制选择框（背景、边框、文本、箭头、清空按钮）
     */
    void PaintDropdown(IRender* pRender);

    /** 打开浮层（无数据或控件禁用时不打开）
     */
    void OpenPopup();

    /** 当前是否已打开本控件的浮层
     */
    bool IsPopupOpen() const;

    /** 关闭本控件的浮层
     */
    void ClosePopup();

    /** 构造浮层窗口 XML
     * @param [in] nWidth 浮层宽度（未缩放像素）
     * @param [in] nHeight 浮层高度（未缩放像素）
     */
    DString BuildPopupXml(int32_t nWidth, int32_t nHeight) const;

    /** 浮层内列表的选择变化：只有选满整条路径才提交并关闭浮层
     */
    void OnPopupSelection(const CascadingSelectValue& value);

    /** 估算浮层最大行数（各列兄弟节点数量的最大值）
     */
    int32_t EstimateMaxRows() const;

    /** 获取浮层使用的列宽（固定列宽或默认 140）
     */
    int32_t GetPopupColumnWidth() const;

    /** 构建当前选择值（不发事件）
     */
    void BuildValue(CascadingSelectValue& value) const;

    /** 按 `|` 拆分路径列表
     */
    static std::vector<DString> SplitPathList(const DString& strList);

    /** 选择框右侧箭头矩形
     */
    UiRect GetChevronRect() const;

    /** 选择框清空按钮矩形（不允许清空或未选择时返回空矩形）
     */
    UiRect GetClearRect() const;

    /** 绘制选择框向下箭头
     */
    void DrawDownChevron(IRender* pRender, const UiRect& rc, UiColor color);

    /** 绘制选择框清空按钮
     */
    void DrawClearIcon(IRender* pRender, const UiRect& rc, UiColor color);

private:
    /** 根节点列表
     */
    std::vector<Node> m_roots;

    /** items_id 模式的原始 ID 串（语言切换时重建显示文本）
     */
    DString m_strIdPaths;

    /** items 模式的原始字面路径串（`|` 分隔；下拉形态复制给浮层）
     */
    DString m_strItemPaths;

    /** 数据是否为 items_id 模式
     */
    bool m_bItemIdMode = false;

    /** 每级选中索引（长度等于深度，-1 = 未选）
     */
    std::vector<int32_t> m_selection;

    /** 每列的首行偏移（滚轮滚动）
     */
    std::vector<int32_t> m_columnScroll;

    /** 禁用路径列表（原始串，匹配显示文本路径或 ID 路径）
     */
    std::vector<DString> m_disabledPaths;

    /** 列布局缓存
     */
    std::vector<ColumnLayout> m_columns;

    /** 悬停列 / 行（-1 = 无）
     */
    int32_t m_nHoverLevel = -1;
    int32_t m_nHoverIndex = -1;

    /** 事件静默标记（内部批量更新时使用）
     */
    bool m_bMuteEvent = false;

    /** 路径层级分隔符
     */
    DString m_strPathSeparator;

    /** 选择文本连接符
     */
    DString m_strValueSeparator;

    /** 固定列宽（0 = 等分）
     */
    int32_t m_nColumnWidth = 0;

    /** 行高
     */
    int32_t m_nRowHeight = 28;

    /** 行内文本左右内边距
     */
    int32_t m_nTextPadding = 10;

    /** 字体 ID
     */
    DString m_strFont;

    /** 各状态/部件颜色（语义色名）
     */
    DString m_strItemColor;
    DString m_strSelectedColor;
    DString m_strDisabledColor;
    DString m_strHoverBgColor;
    DString m_strSelectedBgColor;
    DString m_strDividerColor;
    DString m_strChevronColor;
    DString m_strScrollbarColor;

    /** 下拉列表框形态 */
    bool m_bDropdown = false;

    /** 移入展开 */
    bool m_bExpandOnHover = false;

    /** 是否允许清空按钮 */
    bool m_bAllowClear = true;

    /** 选择框是否处于悬停状态 */
    bool m_bHovered = false;

    /** 鼠标是否悬停在清空按钮上 */
    bool m_bClearHovered = false;

    /** 提示文本（字面 / 多语言 ID） */
    DString m_strPlaceholder;
    DString m_strPlaceholderId;

    /** 浮层宽度（0 = 自动）与最多可见行数 */
    int32_t m_nPopupWidth = 0;
    int32_t m_nPopupMaxRows = 8;

    /** 浮层内级联列表的样式 Class */
    DString m_strPopupClass;

    /** 选择框颜色与边框 */
    DString m_strTextColor;
    DString m_strPlaceholderColor;
    DString m_strDisabledTextColor;
    DString m_strBgColor;
    DString m_strBorderColor;
    DString m_strHoverBorderColor;
    DString m_strClearColor;
    DString m_strClearHoverColor;
    int32_t m_nBorderRound = 4;
    int32_t m_nBorderSize = 1;

    /** 自定义显示文本回调 */
    DisplayTextEvent m_displayTextCallback;
};

} // namespace ui

#endif // UI_CONTROL_CASCADINGSELECT_H_
