#ifndef UI_CONTROL_RICHTEXT_H_
#define UI_CONTROL_RICHTEXT_H_

#include "duilib/Core/Control.h"
#include "duilib/Render/IRender.h"

namespace ui 
{
/** XML解析后的格式化文本片段
*/
class RichTextSlice
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

    /** 文字颜色: "color"
    */
    UiString m_textColor;

    /** 背景颜色: "color"
    */
    UiString m_bgColor;

    /** 字体信息
    */
    UiFont m_fontInfo;

public:
    /** 子节点
    */
    std::vector<RichTextSlice> m_childs;
};

/** 格式化文本，解析后的结构
*/
class RichTextDataEx:
    public RichTextData
{
public:
    /** 对象绘制区域(输出参数)
    */
    std::vector<UiRect> m_textRects;

    /** 超链接的URL
    */
    UiString m_linkUrl;

    /** 鼠标是否按下
    */
    bool m_bMouseDown = false;

    /** 是否处于鼠标悬停状态
    */
    bool m_bMouseHover = false;
};

/** 格式化文本（类HTML格式）
*/
class UILIB_API RichText : public Control
{
    typedef Control BaseClass;
public:
    explicit RichText(Window* pWindow);
    RichText(const RichText& r) = delete;
    RichText& operator=(const RichText& r) = delete;
    virtual ~RichText() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintText(IRender* pRender) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 计算文本区域大小（宽和高）
     *  @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
     *  @return 控件的文本估算大小，包含内边距(Box)，不包含外边距
     */
    virtual UiSize EstimateText(UiSize szAvailable) override;

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

    /** 设置行间距倍数
    */
    void SetRowSpacingMul(float fRowSpacingMul);

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
    void SetHAlignType(HorAlignType alignType);

    /** 获取文本水平对齐方式
    */
    HorAlignType GetHAlignType() const;

    /** 设置文本垂直对齐方式
    */
    void SetVAlignType(VerAlignType alignType);

    /** 获取文本垂直对齐方式
    */
    VerAlignType GetVAlignType() const;

public:
    /** 设置格式的文本
    * @param [in] richText 带有格式的文本内容
    */
    bool SetText(const DString& richText);

    /** 设置格式的文本ID
    * @param [in] richTextId 带有格式的文本内容ID
    */
    bool SetTextId(const DString& richTextId);

    /** 清空原来的格式文本
    */
    void Clear();

    /** 追加一个文本片段
    */
    void AppendTextSlice(const RichTextSlice&& textSlice);

    /** 追加一个文本片段
    */
    void AppendTextSlice(const RichTextSlice& textSlice);

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

public:
    /** 输出带格式化文本
    */
    DString ToString() const;

    /** 监听超级链接被点击事件
     * @param[in] callback 超级链接被点击后的回调函数
     */
    void AttachLinkClick(const EventCallback& callback) { AttachEvent(kEventLinkClick, callback); }

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

    /** 重绘
    */
    void Redraw();

    /** 获取当前绘制文字的属性
    */
    uint16_t GetTextStyle() const;

    /** 计算绘制后的目标区域大小
    */
    void CalcDestRect(IRender* pRender, const UiRect& rc, UiRect& rect);

private:
    //鼠标消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool MouseHover(const EventArgs& msg) override;
    virtual bool MouseLeave(const EventArgs& msg) override;
    virtual bool OnSetCursor(const EventArgs& msg) override;

private:
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

    /** 文本垂直对齐方式
    */
    VerAlignType m_vAlignType;

    /** 行间距倍数
    */
    float m_fRowSpacingMul;

    /** 绘制的文本内容（解析前）
    */
    std::vector<RichTextSlice> m_textSlice;

    /** 绘制的文本内容（解析后）
    */
    std::vector<RichTextDataEx> m_textData;

    /** 解析文本对应的DPI值
    */
    uint32_t m_nTextDataDPI;

    /** 超级链接的文本：常规文本颜色
    */
    UiString m_linkNormalTextColor;

    /** 超级链接的文本：Hover文本颜色
    */
    UiString m_linkHoverTextColor;

    /** 超级链接的文本：鼠标按下文本颜色
    */
    UiString m_linkMouseDownTextColor;

    /** 是否显示下划线字体风格
    */
    bool m_bLinkUnderlineFont;

    /** 文本ID
    */
    UiString m_richTextId;

    /** 语言文件
    */
    UiString m_langFileName;

    /** 文本的Trim策略
    */
    enum class TrimPolicy {
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

    /** 绘制缓存
    */
    std::shared_ptr<DrawRichTextCache> m_spDrawRichTextCache;
};

} // namespace ui

#endif // UI_CONTROL_RICHTEXT_H_
