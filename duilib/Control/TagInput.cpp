#include "TagInput.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/Label.h"
#include "duilib/Box/HBox.h"
#include "duilib/Layout/HFlowLayout.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>
#include <list>

namespace ui
{
/** TagInput 专用的水平流式布局：最后一个子控件（编辑框）始终占满其所在行的剩余宽度，
*   当剩余宽度小于编辑框最小宽度时，编辑框另起一行并占满整行。
*   其余子控件（标签）按 HFlowLayout 原规则依次排列、自动换行。
*/
class TagInputLayout : public HFlowLayout
{
    typedef HFlowLayout BaseClass;
public:
    TagInputLayout() = default;

    /** 设置编辑框控件（最后一个可见的非浮动子控件）
    */
    void SetEditControl(Control* pEdit) { m_pEdit = pEdit; }

    virtual UiSize64 ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly = false) override
    {
        if (m_pEdit != nullptr) {
            AdjustEditWidth(items, rc);
        }
        return BaseClass::ArrangeChildren(items, rc, bEstimateOnly);
    }

    virtual UiSize64 EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable) override
    {
        if (m_pEdit != nullptr) {
            UiRect rc(0, 0, szAvailable.cx, szAvailable.cy);
            AdjustEditWidth(items, rc);
        }
        return BaseClass::EstimateLayoutSize(items, szAvailable);
    }

private:
    /** 按标签的排列结果计算编辑框宽度，并设置为固定宽度
    */
    void AdjustEditWidth(const std::vector<Control*>& items, UiRect rc)
    {
        if (m_pEdit == nullptr) {
            return;
        }
        //与 HFlowLayout 内部保持一致，先去掉容器内边距
        DeflatePadding(rc);

        const UiSize szAvailable(rc.Width(), rc.Height());
        int32_t nPosX = rc.left;
        bool bRowHasItem = false;
        for (Control* pControl : items) {
            if ((pControl == nullptr) || (pControl == m_pEdit) || !pControl->IsVisible() || pControl->IsFloat()) {
                continue;
            }
            const UiMargin rcMargin = pControl->GetMargin();
            UiEstSize estSize = pControl->EstimateSize(szAvailable);
            int32_t nWidth = estSize.cx.GetInt32();
            if (estSize.cx.IsStretch()) {
                //标签不应为拉伸宽度，这里做兜底
                nWidth = 0;
            }
            nWidth = std::clamp(nWidth, pControl->GetMinWidth(), pControl->GetMaxWidth());
            nWidth = std::max(nWidth, 0);

            if (bRowHasItem && (nPosX + rcMargin.left + nWidth + rcMargin.right > rc.right)) {
                //换行
                nPosX = rc.left;
            }
            nPosX += rcMargin.left + nWidth + rcMargin.right + GetChildMarginX();
            bRowHasItem = true;
        }

        //编辑框放在标签之后，至少保证最小宽度，放不下则另起一行
        const UiMargin rcEditMargin = m_pEdit->GetMargin();
        const int32_t nMinWidth = std::max(m_pEdit->GetMinWidth(), 0);
        if (bRowHasItem && (nPosX + rcEditMargin.left + nMinWidth + rcEditMargin.right > rc.right)) {
            nPosX = rc.left;
        }
        int32_t nEditWidth = rc.right - nPosX - rcEditMargin.left - rcEditMargin.right;
        nEditWidth = std::max(nEditWidth, nMinWidth);
        //布局过程中设置固定宽度：不触发重新布局、不再做 DPI 缩放（当前已是物理像素）
        m_pEdit->SetFixedWidth(UiFixedInt(nEditWidth), false, false);
    }

private:
    /** 内部编辑框
    */
    Control* m_pEdit = nullptr;
};

/** TagInput 专用编辑框：空文本时按 Backspace，删除最后一个标签
*/
class TagInputEdit : public RichEdit
{
    typedef RichEdit BaseClass;
public:
    explicit TagInputEdit(Window* pWindow) : RichEdit(pWindow), m_pOwner(nullptr) {}

    /** 设置所属的 TagInput 控件
    */
    void SetOwner(TagInput* pOwner) { m_pOwner = pOwner; }

    virtual bool OnKeyDown(const EventArgs& msg) override
    {
        if ((msg.vkCode == kVK_BACK) && (m_pOwner != nullptr)) {
            DString text = GetText();
            if (StringUtil::Trim(text).empty()) {
                //编辑框内容为空时，Backspace 交给 TagInput 删除最后一个标签
                if (m_pOwner->HandleEditBackspace()) {
                    return true;
                }
            }
        }
        return BaseClass::OnKeyDown(msg);
    }

private:
    TagInput* m_pOwner;
};

TagInput::TagInput(Window* pWindow) :
    BaseClass(pWindow, new TagInputLayout()),
    m_pEdit(nullptr),
    m_bAllowDuplicate(false),
    m_nMaxCount(0)
{
}

DString TagInput::GetType() const
{
    return DUI_CTR_TAG_INPUT;
}

void TagInput::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
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
    else if ((strName == _T("allow_duplicate")) || (strName == _T("allowduplicate"))) {
        SetAllowDuplicate(strValue == _T("true") || strValue == _T("1"));
    }
    else if ((strName == _T("max_count")) || (strName == _T("maxcount"))) {
        SetMaxCount(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if (strName == _T("delimiter")) {
        m_delimiter = strValue;
    }
    else if (strName == _T("tags")) {
        if (m_pEdit != nullptr) {
            RemoveAllTags();
            ParseInitTags(strValue);
        }
        else {
            m_initTags = strValue;
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void TagInput::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //先创建内部编辑框，保证编辑框始终是最后一个子控件；新增标签时插到它前面
    m_pEdit = new TagInputEdit(GetWindow());
    static_cast<TagInputEdit*>(m_pEdit)->SetOwner(this);
    m_pEdit->SetClass(_T("tag_input_edit"));
    m_pEdit->SetAttribute(_T("single_line"), _T("true"));
    m_pEdit->SetAttribute(_T("want_tab"), _T("false"));
    //不响应回车换行，由 TagInput 接收回车键生成标签
    m_pEdit->SetAttribute(_T("want_return"), _T("false"));
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
    AddItem(m_pEdit);
    static_cast<TagInputLayout*>(GetLayout())->SetEditControl(m_pEdit);

    //回车：提交标签
    m_pEdit->AttachReturn([this](const EventArgs& /*args*/) {
        CommitInput();
        return true;
        });
    //文本变化：分隔符成标签，并转发文本变化事件
    m_pEdit->AttachTextChanged([this](const EventArgs& /*args*/) {
        HandleEditTextChanged();
        return true;
        });

    //点击容器空白区域时，焦点落到编辑框
    AttachButtonDown([this](const EventArgs& /*args*/) {
        if (m_pEdit != nullptr) {
            m_pEdit->SetFocus();
        }
        return true;
        });

    //创建初始标签（此时编辑框已就位，标签会插到编辑框之前）
    if (!m_initTags.empty()) {
        ParseInitTags(m_initTags.c_str());
    }
}

void TagInput::SetFocus()
{
    if (m_pEdit != nullptr) {
        m_pEdit->SetFocus();
    }
    else {
        BaseClass::SetFocus();
    }
}

bool TagInput::AddTag(const DString& text)
{
    if (m_pEdit == nullptr) {
        //控件尚未初始化，缓存到初始标签中
        DString strText = StringUtil::Trim(text.c_str());
        if (strText.empty()) {
            return false;
        }
        if (m_initTags.empty()) {
            m_initTags = strText;
        }
        else {
            DString init = m_initTags.c_str();
            init += _T(";");
            init += strText;
            m_initTags = init;
        }
        return true;
    }

    DString strText = StringUtil::Trim(text.c_str());
    if (strText.empty()) {
        return false;
    }
    if (!m_bAllowDuplicate && IsTagExists(strText)) {
        return false;
    }
    if ((m_nMaxCount > 0) && (static_cast<int32_t>(m_tags.size()) >= m_nMaxCount)) {
        return false;
    }

    Control* pTag = CreateTagItem(strText);
    const size_t nIndex = m_tags.size();
    AddItemAt(pTag, nIndex);
    m_tags.push_back(strText);
    SendEvent(kEventTagAdded, static_cast<WPARAM>(nIndex));
    return true;
}

bool TagInput::RemoveTagAt(size_t index)
{
    if (index >= m_tags.size()) {
        return false;
    }
    //先触发事件，回调中仍可读取被删除标签的文本
    SendEvent(kEventTagRemoved, static_cast<WPARAM>(index));
    RemoveItemAt(index);
    m_tags.erase(m_tags.begin() + index);
    return true;
}

void TagInput::RemoveAllTags()
{
    while (!m_tags.empty()) {
        RemoveTagAt(m_tags.size() - 1);
    }
}

size_t TagInput::GetTagCount() const
{
    return m_tags.size();
}

DString TagInput::GetTagText(size_t index) const
{
    if (index >= m_tags.size()) {
        return DString();
    }
    return DString(m_tags[index].c_str());
}

void TagInput::SetAllowDuplicate(bool bAllowDuplicate)
{
    m_bAllowDuplicate = bAllowDuplicate;
}

bool TagInput::IsAllowDuplicate() const
{
    return m_bAllowDuplicate;
}

void TagInput::SetMaxCount(int32_t nMaxCount)
{
    m_nMaxCount = (nMaxCount > 0) ? nMaxCount : 0;
}

int32_t TagInput::GetMaxCount() const
{
    return m_nMaxCount;
}

RichEdit* TagInput::GetEditControl() const
{
    return m_pEdit;
}

Control* TagInput::CreateTagItem(const DString& text)
{
    HBox* pTag = new HBox(GetWindow());
    pTag->SetClass(_T("tag_input_tag"));

    Label* pText = new Label(GetWindow());
    pText->SetClass(_T("tag_input_tag_text"));
    //文本不拦截鼠标，点击穿透到标签容器
    pText->SetMouseEnabled(false);
    pText->SetText(text);
    pTag->AddItem(pText);

    Button* pClose = new Button(GetWindow());
    pClose->SetClass(_T("tag_input_tag_close"));
    //删除按钮不参与 Tab 键导航
    pClose->SetTabStop(false);
    pTag->AddItem(pClose);

    //点击标签正文区域（非删除按钮）
    pTag->AttachButtonUp([this, pTag](const EventArgs& /*args*/) {
        size_t nIndex = GetItemIndex(pTag);
        if (nIndex != Box::InvalidIndex) {
            SendEvent(kEventTagClick, static_cast<WPARAM>(nIndex));
        }
        return true;
        });

    //点击删除按钮：按当前实际序号删除（删除期间序号可能已变化）
    pClose->AttachClick([this, pTag](const EventArgs& /*args*/) {
        size_t nIndex = GetItemIndex(pTag);
        if (nIndex != Box::InvalidIndex) {
            RemoveTagAt(nIndex);
        }
        return true;
        });
    return pTag;
}

void TagInput::CommitInput()
{
    if (m_pEdit == nullptr) {
        return;
    }
    DString text = m_pEdit->GetText();
    text = StringUtil::Trim(text.c_str());
    if (text.empty()) {
        return;
    }
    if (AddTag(text)) {
        m_pEdit->SetText(_T(""));
        m_pEdit->SetFocus();
    }
}

bool TagInput::HandleEditBackspace()
{
    if (m_tags.empty()) {
        return false;
    }
    return RemoveTagAt(m_tags.size() - 1);
}

void TagInput::HandleEditTextChanged()
{
    //转发文本变化事件
    SendEvent(kEventTextChanged);

    if ((m_pEdit == nullptr) || m_delimiter.empty()) {
        return;
    }
    DString text = m_pEdit->GetText();
    const DString delimiter = m_delimiter.c_str();
    size_t nPos = text.find(delimiter);
    if (nPos == DString::npos) {
        return;
    }
    DString prefix = StringUtil::Trim(text.substr(0, nPos).c_str());
    DString suffix = text.substr(nPos + delimiter.size());
    if (!prefix.empty()) {
        AddTag(prefix);
    }
    m_pEdit->SetText(suffix);
    m_pEdit->SetFocus();
}

bool TagInput::IsTagExists(const DString& text) const
{
    for (const UiString& tag : m_tags) {
        if (text == tag.c_str()) {
            return true;
        }
    }
    return false;
}

void TagInput::ParseInitTags(const DString& tags)
{
    if (tags.empty()) {
        return;
    }
    std::list<DString> tagList = StringUtil::Split(DString(tags), DString(_T(";")));
    for (const DString& tag : tagList) {
        DString strTag = StringUtil::Trim(tag.c_str());
        if (!strTag.empty()) {
            AddTag(strTag);
        }
    }
}

} // namespace ui
