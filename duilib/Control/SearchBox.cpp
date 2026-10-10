#include "SearchBox.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Core/Control.h"

namespace ui
{

SearchBox::SearchBox(Window* pWindow) :
    BaseClass(pWindow),
    m_pEdit(nullptr)
{
}

DString SearchBox::GetType() const
{
    return DUI_CTR_SEARCH_BOX;
}

void SearchBox::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    //这些属性需要转发给内部编辑框；OnInit 之前子控件尚未创建，先缓存
    if ((strName == _T("prompt_text")) || (strName == _T("prompttext"))) {
        m_promptText = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if ((strName == _T("prompt_text_id")) || (strName == _T("prompt_textid")) || (strName == _T("prompttextid"))) {
        m_promptTextId = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if ((strName == _T("prompt_color")) || (strName == _T("promptcolor"))) {
        m_promptColor = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if (strName == _T("text")) {
        m_initText = strValue;
        if (m_pEdit != nullptr) {
            //与其它属性保持一致，走 SetAttribute 转发，复用 RichEdit 的完整 text 处理
            //（含 IsReplaceNewline 换行符替换等）
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))) {
        m_initTextId = strValue;
        if (m_pEdit != nullptr) {
            m_pEdit->SetAttribute(strName, strValue);
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void SearchBox::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //左侧搜索图标（不响应鼠标、不获取焦点，点击穿透到容器）
    Control* pIcon = new Control(GetWindow());
    pIcon->SetClass(_T("search_box_icon"));
    pIcon->SetNoFocus();
    pIcon->SetMouseEnabled(false);
    AddItem(pIcon);

    //中间编辑框
    m_pEdit = new RichEdit(GetWindow());
    m_pEdit->SetClass(_T("search_box_edit"));
    m_pEdit->SetAttribute(_T("want_tab"), _T("false"));
    //默认开启占位提示
    m_pEdit->SetAttribute(_T("prompt_mode"), _T("true"));
    if (!m_promptText.empty()) {
        m_pEdit->SetAttribute(_T("prompt_text"), DString(m_promptText.c_str()));
    }
    if (!m_promptTextId.empty()) {
        m_pEdit->SetAttribute(_T("prompt_text_id"), DString(m_promptTextId.c_str()));
    }
    if (!m_promptColor.empty()) {
        m_pEdit->SetAttribute(_T("prompt_color"), DString(m_promptColor.c_str()));
    }
    //右侧清除按钮（文本非空时自动显示，点击清空，复用 RichEdit 内置机制）
    m_pEdit->SetAttribute(_T("clear_btn_class"), _T("search_box_clear_btn"));
    if (!m_initText.empty()) {
        m_pEdit->SetText(m_initText.c_str());
    }
    if (!m_initTextId.empty()) {
        m_pEdit->SetAttribute(_T("text_id"), DString(m_initTextId.c_str()));
    }
    AddItem(m_pEdit);

    //转发内部编辑框事件：文本变化、回车搜索
    m_pEdit->AttachTextChanged([this](const EventArgs& /*args*/) {
        SendEvent(kEventTextChanged);
        return true;
        });
    m_pEdit->AttachReturn([this](const EventArgs& /*args*/) {
        SendEvent(kEventReturn);
        return true;
        });
}

void SearchBox::SetFocus()
{
    if (m_pEdit != nullptr) {
        m_pEdit->SetFocus();
    }
    else {
        BaseClass::SetFocus();
    }
}

RichEdit* SearchBox::GetEditControl() const
{
    return m_pEdit;
}

void SearchBox::SetSearchText(const DString& text)
{
    if (m_pEdit != nullptr) {
        m_pEdit->SetText(text);
    }
    else {
        m_initText = text;
    }
}

DString SearchBox::GetSearchText() const
{
    if (m_pEdit != nullptr) {
        return m_pEdit->GetText();
    }
    return DString(m_initText.c_str());
}

} // namespace ui
