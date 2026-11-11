#ifndef UI_CONTROL_RICHTEXT_IMPL_H_
#define UI_CONTROL_RICHTEXT_IMPL_H_

#include "duilib/Core/Control.h"
#include "duilib/Render/IRender.h"

#include <set>

namespace ui 
{
/** RichText文本XML解析后的格式化文本片段
*/
class DUILIB_API RichTextSlice
{
public:
    /** 节点名称
    */
    UiString m_nodeName;

    /** 文字内容(UTF16编码)
    */
    DStringW m_text;

    /** 超链接的URL: "href"
    */
    UiString m_linkUrl;

    /** 超链接的自定义提示文本: "title"（为空时使用URL作为提示）
    */
    UiString m_linkTitle;

    /** 文字颜色: "color"
    */
    UiString m_textColor;

    /** 背景颜色: "color"
    */
    UiString m_bgColor;

    /** 字体信息
    */
    UiFont m_fontInfo;

    /** 字体缩放比例（用于 <h1>~<h6>、<sup>、<sub>），1.0 表示不缩放
    */
    float m_fFontScale = 1.0f;

    /** 基线偏移（96 DPI 下的像素值，负数向上、正数向下），用于 <sup>/<sub>
    */
    int32_t m_nBaselineOffset = 0;

    /** 行间距附加量（96 DPI 下的像素值），用于段落的段间距
    */
    float m_fRowSpacingAdd = 0.0f;

    /** 水平对齐方式：-1表示继承，0左对齐，1居中，2右对齐，3两端对齐（用于 <p align>）
    */
    int8_t m_nHAlign = -1;

    /** 是否为水平分割线（用于 <hr>）
    */
    bool m_bHorizontalRule = false;

public:
    /** 子节点
    */
    std::vector<RichTextSlice> m_children;
};

/** 格式化文本（类HTML格式）
*/
class DUILIB_API RichTextImpl
{
public:
    explicit RichTextImpl(Control* pOwner);
    RichTextImpl(const RichTextImpl& r) = delete;
    RichTextImpl& operator=(const RichTextImpl& r) = delete;
    ~RichTextImpl();

    bool SetAttribute(const DString& strName, const DString& strValue);
    void PaintText(IRender* pRender);

    /** 绑定的窗口发生了变化
    */
    void OnWindowChanged();

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale);

    /** 计算文本区域大小（宽和高）
     *  @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
     *  @return 控件的文本估算大小，包含内边距(Box)，不包含外边距
     */
    UiSize EstimateText(UiSize szAvailable);

public:
    /** 获取文字内边距
     */
    UiPadding GetTextPadding() const;

    /** 设置文字内边距信息, 函数内部会进行DPI自适应操作
     * @param [in] padding 矩形的四个边值分别代表对应的四个内边距值
     * @param [in] bNeedDpiScale 兼容 DPI 缩放，默认为 true
     */
    void SetTextPadding(UiPadding padding, bool bNeedDpiScale = true);

    /** 获取当前字体ID
     * @return 返回字体ID，该编号在 global.xml 中标识
     */
    DString GetFontId() const;

    /** 设置当前字体ID
     * @param [in] strFontId 要设置的字体ID，该ID可在 global.xml 中存在
     */
    void SetFontId(const DString& strFontId);

    /** 获取默认文本颜色
     */
    DString GetTextColor() const;

    /** 设置默认文本颜色
     */
    void SetTextColor(const DString& sTextColor);

    /** 获取行间距倍数
    */
    float GetRowSpacingMul() const;

    /** 行间距倍数: 字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小），用于按比例调整行间距
    */
    void SetRowSpacingMul(float fRowSpacingMul);

    /** 获取行间距附加量
    */
    float GetRowSpacingAdd() const;

    /** 行间距附加量: 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移（像素）
    */
    void SetRowSpacingAdd(float fRowSpacingAdd);

    /** 获取超出矩形区域的文本显示方式
     * @return 返回 true 时并且在多行模式下内容被换行显示，false 则表示截断显示
     */
    bool IsWordWrap() const;

    /** 设置超出矩形区域的文本显示方式
     * @param[in] bWordWrap 为 true 时并且在多行模式下内容被换行显示，false 则表示截断显示
     */
    void SetWordWrap(bool bWordWrap);

    /** 设置文本水平对齐方式
    */
    void SetTextHAlignType(HorAlignType alignType);

    /** 获取文本水平对齐方式
    */
    HorAlignType GetHAlignType() const;

    /** 设置文本垂直对齐方式
    */
    void SetTextVAlignType(VerAlignType alignType);

    /** 获取文本垂直对齐方式
    */
    VerAlignType GetVAlignType() const;

public:
    /** 设置格式的文本
    * @param [in] richText 带有格式的文本内容
    * @param [in] bRedraw true表示触发重绘，false表示不触发重绘
    */
    bool SetText(const DString& richText, bool bRedraw = true);

    /** 设置格式的文本ID
    * @param [in] richTextId 带有格式的文本内容ID
    * @param [in] bRedraw true表示触发重绘，false表示不触发重绘
    */
    bool SetTextId(const DString& richTextId, bool bRedraw = true);

    /** 清空原来的格式文本
    */
    void Clear();

    /** 追加一个文本片段
    * @param [in] textSlice 文本片段
    * @param [in] bRedraw true表示立即触发重绘，false表示不触发重绘
    *                    （批量追加或解析过程中设为false，完成后调用Redraw统一重绘）
    */
    void AppendTextSlice(RichTextSlice&& textSlice, bool bRedraw = true);

    /** 追加一个文本片段
    * @param [in] textSlice 文本片段
    * @param [in] bRedraw true表示立即触发重绘，false表示不触发重绘
    *                    （批量追加或解析过程中设为false，完成后调用Redraw统一重绘）
    */
    void AppendTextSlice(const RichTextSlice& textSlice, bool bRedraw = true);

    /** 追加格式化文本（在已有内容之后追加，不清空原有内容）
    * @param [in] richText 带有格式的文本内容（可以是纯文本，也可以是类HTML标签）
    * @param [in] bRedraw true表示追加后触发重绘，false表示不触发重绘
    */
    bool AppendText(const DString& richText, bool bRedraw = true);

    /** 设置超级链接的文本颜色
    * @param [in] normalColor 常规状态的颜色，为空表示不修改
    * @param [in] hoveredColor 鼠标悬停状态的颜色，为空表示不修改
    * @param [in] pressedColor 鼠标按下状态的颜色，为空表示不修改
    * @param [in] visitedColor 访问过（点击过）状态的颜色，为空表示不修改
    */
    void SetLinkColor(const DString& normalColor, const DString& hoveredColor,
                      const DString& pressedColor, const DString& visitedColor = _T(""));

    /** 获取超级链接的数量（相同URL且连续出现的文本片段算作一个链接）
    */
    size_t GetLinkCount() const;

    /** 获取指定链接的URL
    * @param [in] nLinkIndex 链接的索引号，取值范围：[0, GetLinkCount())
    * @return 返回链接的URL，索引无效时返回空字符串
    */
    DString GetLinkUrl(size_t nLinkIndex) const;

    /** 获取指定链接的绘制区域
    * @param [in] nLinkIndex 链接的索引号，取值范围：[0, GetLinkCount())
    * @param [out] rcLink 返回链接的绘制区域（多个文本片段的区域合并后的范围）
    * @return 返回true表示成功，false表示索引无效或链接尚未完成布局
    */
    bool GetLinkRect(size_t nLinkIndex, UiRect& rcLink) const;

    /** 获取段落的段间距（96 DPI 下的像素值）
    */
    int32_t GetParagraphSpacing() const { return m_nParagraphSpacing; }

    /** 根据Trim方案，对文本进行Trim处理，去掉多余的空格
    * @param [in,out] text 传入需要处理的文本，传出处理完成后的文本
    * @return 返回text的引用
    */
    const DString& TrimText(DString& text);

    /** 根据Trim方案，对文本进行Trim处理，去掉多余的空格
    * @param [in] text 传入需要处理的文本
    * @return 返回处理好的字符串
    */
    DString TrimText(const DString::value_type* text);

    /** 设置是否允许重绘
    * @param [in] bEnable true表示允许重绘，false表示禁止重绘
    */
    void SetEnableRedraw(bool bEnable);

    /** 获取是否允许重绘
    */
    bool IsEnableRedraw() const;

public:
    /** 输出带格式化文本
    */
    DString ToString() const;

public:
    /** 重绘
    */
    void Redraw();

    /** 重绘
    */
    void Invalidate();

    //鼠标消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
    void ButtonDown(const EventArgs& msg);
    void ButtonUp(const EventArgs& msg);
    void MouseMove(const EventArgs& msg);
    void MouseHover(const EventArgs& msg);
    void MouseLeave(const EventArgs& msg);
    bool OnSetCursor(const EventArgs& msg);

    /** 键盘消息处理（用于链接的Tab聚焦与Enter触发）
    * @return 返回true表示消息已处理
    */
    bool OnKeyDown(const EventArgs& msg);

    /** 鼠标右键抬起（用于复制链接地址）
    */
    void RButtonUp(const EventArgs& msg);

private:
    /** 格式化文本，解析后的结构(内部使用)
    */
    class RichTextDataEx :
        public RichTextData
    {
    public:
        /** 对象绘制区域(输出参数)
        */
        std::vector<UiRect> m_textRects;

        /** 超链接的URL
        */
        UiString m_linkUrl;

        /** 超链接的提示文本（为空时使用URL）
        */
        UiString m_linkTitle;

        /** 超链接使用的字体（解析时生成，绘制时共享，避免每帧创建字体对象）
        */
        SharePtr<UiFontEx> m_pLinkFontInfo;

        /** 鼠标是否按下
        */
        bool m_bMouseDown = false;

        /** 是否处于鼠标悬停状态
        */
        bool m_bMouseHover = false;
    };

private:
    /** 设置格式的文本, 但不重绘
    * @param [in] richText 带有格式的文本内容
    */
    bool DoSetText(const DString& richText);

    /** 解析格式化文本, 生成解析后的数据结构
    */
    bool ParseText(std::vector<RichTextDataEx>& outTextData) const;

    /** 检查按需解析文本
    */
    void CheckParseText();

    /** 文本片段解析为绘制结构
    * @param [in] textSlice 文本片段
    * @param [in] parentTextData 父对象信息
    * @param [out] textData 解析后的文本结构
    */
    bool ParseTextSlice(const RichTextSlice& textSlice, 
                        const RichTextDataEx& parentTextData,
                        std::vector<RichTextDataEx>& textData) const;

    /** 输出带格式化文本
    */
    DString ToString(const RichTextSlice& textSlice, const DString& indent) const;

    /** 获取当前绘制文字的属性
    */
    uint16_t GetTextStyle() const;

    /** 计算绘制后的目标区域大小
    */
    void CalcDestRect(IRender* pRender, const UiRect& rc, UiRect& rect);

    /** 获取全部链接的分组信息（相同URL且连续的文本片段归为一组）
    * @param [out] groupStartIndexes 每个链接在m_textData中的起始索引
    * @param [out] groupUrls 每个链接的URL
    */
    void GetLinkGroups(std::vector<size_t>& groupStartIndexes, std::vector<DString>& groupUrls) const;

    /** 查找坐标所在的链接分组索引
    * @return 返回链接的分组索引，未找到时返回-1
    */
    int32_t HitTestLinkGroup(const UiPoint& ptMouse) const;

    /** 更新鼠标悬停的链接，并派发Hover/Leave事件
    */
    void UpdateHoveredLink(int32_t nLinkGroupIndex);

private:
    /** 关联控件
    */
    Control* m_pOwner;

    /** 绘制缓存
    */
    std::shared_ptr<DrawRichTextCache> m_spDrawRichTextCache;

    /** 文本绘制的内边距(分别对应四个边的内边距大小)
    */
    UiPadding16 m_rcTextPadding;

    /** 默认字体
    */
    UiString m_sFontId;

    /** 默认文本颜色
    */
    UiString m_sTextColor;

    /** 文本水平对齐方式
    */
    HorAlignType m_hAlignType;

    /** 是否使用两端对齐（text_align="justify"）
    */
    bool m_bHAlignJustify = false;

    /** 文本垂直对齐方式
    */
    VerAlignType m_vAlignType;

    /** 行间距倍数
    */
    float m_fRowSpacingMul;

    /** 行间距附加量
    */
    float m_fRowSpacingAdd;

    /** 绘制的文本内容（解析前）
    */
    std::vector<RichTextSlice> m_textSlice;

    /** 绘制的文本内容（解析后）
    */
    std::vector<RichTextDataEx> m_textData;

    /** 绘制用的文本数据缓冲区（复用，避免每帧堆分配）
    */
    std::vector<RichTextData> m_richTextDataBuffer;

    /** 绘制用的文本颜色缓冲区（复用，避免每帧堆分配）
    */
    std::vector<UiColor> m_richTextTextColorBuffer;

    /** 文本测量结果的缓存（避免重复测量）
    */
    UiRect m_cachedTextRect;

    /** 文本测量时使用的可用大小
    */
    UiSize m_cachedAvailableSize;

    /** 文本测量缓存是否有效
    */
    bool m_bCachedTextRectValid = false;

    /** 解析文本对应的界面缩放比
    */
    uint32_t m_nTextDataDisplayScaleFactor;

    /** 超级链接的文本：常规文本颜色
    */
    UiString m_linkNormalTextColor;

    /** 超级链接的文本：Hover文本颜色
    */
    UiString m_linkHoveredTextColor;

    /** 超级链接的文本：鼠标按下文本颜色
    */
    UiString m_linkPressedTextColor;

    /** 超级链接的文本：访问过（点击过）的文本颜色
    */
    UiString m_linkVisitedTextColor;

    /** 已经访问过的链接URL集合
    */
    std::set<DString> m_visitedLinks;

    /** 鼠标当前悬停的链接分组索引（-1表示无）
    */
    int32_t m_nHoveredLinkGroup = -1;

    /** 键盘当前聚焦的链接分组索引（-1表示无）
    */
    int32_t m_nFocusedLinkGroup = -1;

    /** 字间距（像素）
    */
    float m_fLetterSpacing = 0.0f;

    /** 首行缩进（96 DPI 下的像素值）
    */
    int32_t m_nTextIndent = 0;

    /** 段间距（96 DPI 下的像素值），用于 <p>/<h1>~<h6> 等块级标签
    */
    int32_t m_nParagraphSpacing = 0;

    /** 最多显示的行数（0表示不限制）
    */
    int32_t m_nMaxLines = 0;

    /** 超出最大行数时是否显示省略号（false表示直接裁剪）
    */
    bool m_bTextOverflowEllipsis = true;

    /** 文本ID
    */
    UiString m_richTextId;

    /** 语言文件
    */
    UiString m_langFileName;

    /** 是否显示下划线字体风格
    */
    bool m_bLinkUnderlineFont;

    /** 文本的Trim策略
    */
    enum class TrimPolicy: int8_t {
        kNone    = 0, //不处理
        kAll     = 1, //去掉所有空格
        kKeepOne = 2, //去掉多余的空格，只保留一个空格
    };

    /** 文本的Trim策略
    */
    TrimPolicy m_trimPolicy = TrimPolicy::kAll;

    /** 是否自动换行（默认为true）
    */
    bool m_bWordWrap;

    /** 设置text属性时，是否替换花括号
    */
    bool m_bReplaceBrace;

    /** 是否允许重绘
    */
    bool m_bEnableRedraw;
};

} // namespace ui

#endif // UI_CONTROL_RICHTEXT_IMPL_H_
