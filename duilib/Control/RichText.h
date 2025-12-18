#ifndef UI_CONTROL_RICHTEXT_H_
#define UI_CONTROL_RICHTEXT_H_

#include "duilib/Control/RichTextImpl.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"

namespace ui 
{
/** 格式化文本（类HTML格式）
*/
template<typename T = Control>
class UILIB_API RichTextT: public T
{
    typedef T BaseClass;
public:
    explicit RichTextT(Window* pWindow);
    RichTextT(const RichTextT& r) = delete;
    RichTextT& operator=(const RichTextT& r) = delete;
    virtual ~RichTextT() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintText(IRender* pRender) override;

    /** 设置容器所属窗口
     * @param [in] pWindow 窗口指针
     */
    virtual void SetWindow(Window* pWindow) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 语言发生变化，刷新界面文字显示相关的内容
    */
    virtual void OnLanguageChanged() override;

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
    void SetTextPadding(const UiPadding& padding, bool bNeedDpiScale = true);

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
    void AttachLinkClick(const EventCallback& callback) { this->AttachEvent(kEventLinkClick, callback); }

    /** 获取实现接口
    */
    RichTextImpl* GetRichTextImpl() const { return m_impl.get(); }

private:
    //鼠标消息（返回true：表示消息已处理；返回false：则表示消息未处理，需转发给父控件）
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool MouseHover(const EventArgs& msg) override;
    virtual bool MouseLeave(const EventArgs& msg) override;
    virtual bool OnSetCursor(const EventArgs& msg) override;

private:

    //功能内部实现
    std::unique_ptr<RichTextImpl> m_impl;
};

template<typename T>
RichTextT<T>::RichTextT(Window* pWindow) :
    T(pWindow)
{
    m_impl = std::make_unique<RichTextImpl>(this);
}

template<typename T>
RichTextT<T>::~RichTextT()
{
    m_impl.reset();
}

template<typename T>
inline DString RichTextT<T>::GetType() const { return DUI_CTR_RICHTEXT; }

template<>
inline DString RichTextT<Box>::GetType() const { return DUI_CTR_RICHTEXT_BOX; }

template<>
inline DString RichTextT<HBox>::GetType() const { return DUI_CTR_RICHTEXT_HBOX; }

template<>
inline DString RichTextT<VBox>::GetType() const { return DUI_CTR_RICHTEXT_VBOX; }

template<typename T>
void RichTextT<T>::SetAttribute(const DString& strName, const DString& strValue)
{
    if (!m_impl->SetAttribute(strName, strValue)) {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename T>
void RichTextT<T>::SetWindow(Window* pWindow)
{
    BaseClass::SetWindow(pWindow);
    m_impl->OnWindowChanged();
}

template<typename T>
void RichTextT<T>::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    m_impl->ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

template<typename T>
void RichTextT<T>::OnLanguageChanged()
{
    BaseClass::OnLanguageChanged();
    m_impl->Redraw();
}

template<typename T>
UiSize RichTextT<T>::EstimateText(UiSize szAvailable)
{
    return m_impl->EstimateText(szAvailable);
}

template<typename T>
void RichTextT<T>::PaintText(IRender* pRender)
{
    m_impl->PaintText(pRender);
}

template<typename T>
UiPadding RichTextT<T>::GetTextPadding() const
{
    return m_impl->GetTextPadding();
}

template<typename T>
void RichTextT<T>::SetTextPadding(const UiPadding& padding, bool bNeedDpiScale)
{
    m_impl->SetTextPadding(padding, bNeedDpiScale);
}

template<typename T>
const DString& RichTextT<T>::TrimText(DString& text)
{
    return m_impl->TrimText(text);
}

template<typename T>
DString RichTextT<T>::TrimText(const DString::value_type* text)
{
    return m_impl->TrimText(text);
}

template<typename T>
bool RichTextT<T>::SetText(const DString& richText)
{
    return m_impl->SetText(richText);
}

template<typename T>
bool RichTextT<T>::SetTextId(const DString& richTextId)
{
    return m_impl->SetTextId(richTextId);
}

template<typename T>
void RichTextT<T>::Clear()
{
    m_impl->Clear();
}

template<typename T>
DString RichTextT<T>::GetFontId() const
{
    return m_impl->GetFontId();
}

template<typename T>
void RichTextT<T>::SetFontId(const DString& strFontId)
{
    m_impl->SetFontId(strFontId);
}

template<typename T>
DString RichTextT<T>::GetTextColor() const
{
    return m_impl->GetTextColor();
}

template<typename T>
void RichTextT<T>::SetTextColor(const DString& sTextColor)
{
    m_impl->SetTextColor(sTextColor);
}

template<typename T>
float RichTextT<T>::GetRowSpacingMul() const
{
    return m_impl->GetRowSpacingMul();
}

template<typename T>
void RichTextT<T>::SetRowSpacingMul(float fRowSpacingMul)
{
    m_impl->SetRowSpacingMul(fRowSpacingMul);
}

template<typename T>
float RichTextT<T>::GetRowSpacingAdd() const
{
    return m_impl->GetRowSpacingAdd();
}

template<typename T>
void RichTextT<T>::SetRowSpacingAdd(float fRowSpacingAdd)
{
    m_impl->SetRowSpacingAdd(fRowSpacingAdd);
}

template<typename T>
bool RichTextT<T>::IsWordWrap() const
{
    return m_impl->IsWordWrap();
}

template<typename T>
void RichTextT<T>::SetWordWrap(bool bWordWrap)
{
    m_impl->SetWordWrap(bWordWrap);
}

template<typename T>
void RichTextT<T>::SetTextHAlignType(HorAlignType alignType)
{
    m_impl->SetTextHAlignType(alignType);
}

template<typename T>
HorAlignType RichTextT<T>::GetHAlignType() const
{
    return m_impl->GetHAlignType();
}

template<typename T>
void RichTextT<T>::SetTextVAlignType(VerAlignType alignType)
{
    m_impl->SetTextVAlignType(alignType);
}

template<typename T>
VerAlignType RichTextT<T>::GetVAlignType() const
{
    return m_impl->GetVAlignType();
}

template<typename T>
void RichTextT<T>::AppendTextSlice(const RichTextSlice&& textSlice)
{
    m_impl->AppendTextSlice(textSlice);
}

template<typename T>
void RichTextT<T>::AppendTextSlice(const RichTextSlice& textSlice)
{
    m_impl->AppendTextSlice(textSlice);
}

template<typename T>
DString RichTextT<T>::ToString() const
{
    return m_impl->ToString();
}

template<typename T>
bool RichTextT<T>::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    m_impl->ButtonDown(msg);
    return bRet;
}

template<typename T>
bool RichTextT<T>::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    m_impl->ButtonUp(msg);
    return bRet;
}

template<typename T>
bool RichTextT<T>::MouseMove(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseMove(msg);
    m_impl->MouseMove(msg);
    return bRet;
}

template<typename T>
bool RichTextT<T>::MouseHover(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseHover(msg);
    m_impl->MouseHover(msg);
    return bRet;
}

template<typename T>
bool RichTextT<T>::MouseLeave(const EventArgs& msg)
{
    m_impl->MouseLeave(msg);
    return BaseClass::MouseLeave(msg);
}

template<typename T>
bool RichTextT<T>::OnSetCursor(const EventArgs& msg)
{
    if (!m_impl->OnSetCursor(msg)) {
        return BaseClass::OnSetCursor(msg);
    }
    return true;
}

typedef RichTextT<Control> RichText;
typedef RichTextT<Box>     RichTextBox;
typedef RichTextT<HBox>    RichTextHBox;
typedef RichTextT<VBox>    RichTextVBox;

} // namespace ui

#endif // UI_CONTROL_RICHTEXT_H_
