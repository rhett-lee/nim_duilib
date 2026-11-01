#include "duilib/Control/Popconfirm.h"
#include "duilib/Control/Flyout.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Callback.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

namespace
{
/** 解析 XML 中的 icon 属性（忽略大小写、下划线与连字符；无法识别时回退为警告图标）
*/
PopconfirmIcon ParseIcon(const DString& strValue)
{
    DString strIcon = StringUtil::MakeLowerString(strValue);
    StringUtil::ReplaceAll(_T("_"), _T(""), strIcon);
    StringUtil::ReplaceAll(_T("-"), _T(""), strIcon);
    if (strIcon == _T("info")) {
        return PopconfirmIcon::kInfo;
    }
    if (strIcon == _T("success")) {
        return PopconfirmIcon::kSuccess;
    }
    if (strIcon == _T("error")) {
        return PopconfirmIcon::kError;
    }
    if (strIcon == _T("none")) {
        return PopconfirmIcon::kNone;
    }
    return PopconfirmIcon::kWarning;
}

/** 解析 XML 中的 ok_type 属性（忽略大小写、下划线与连字符；无法识别时回退为主色按钮）
*/
PopconfirmOkType ParseOkType(const DString& strValue)
{
    DString strType = StringUtil::MakeLowerString(strValue);
    StringUtil::ReplaceAll(_T("_"), _T(""), strType);
    StringUtil::ReplaceAll(_T("-"), _T(""), strType);
    if (strType == _T("default")) {
        return PopconfirmOkType::kDefault;
    }
    if (strType == _T("danger")) {
        return PopconfirmOkType::kDanger;
    }
    return PopconfirmOkType::kPrimary;
}

/** 多语言键：确定 */
const DString& GetOkLangId()
{
    static const DString kOkId(_T("STRID_PUBLIC_STRING_OK"));
    return kOkId;
}

/** 多语言键：取消 */
const DString& GetCancelLangId()
{
    static const DString kCancelId(_T("STRID_PUBLIC_STRING_CANCEL"));
    return kCancelId;
}
} // namespace

Popconfirm::Popconfirm(Window* pWindow) :
    BaseClass(pWindow)
{
    //按钮文案默认取框架的多语言键，随语言切换自动生效
    m_strOkTextId = GetOkLangId();
    m_strCancelTextId = GetCancelLangId();
}

Popconfirm::~Popconfirm() = default;

DString Popconfirm::GetType() const
{
    return DUI_CTR_POPCONFIRM;
}

void Popconfirm::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("description")) {
        SetDescription(strValue);
    }
    else if ((strName == _T("description_id")) || (strName == _T("descriptionid"))) {
        SetDescriptionId(strValue);
    }
    else if ((strName == _T("content")) ) {
        //兼容从 Popover 迁移过来的写法：Popconfirm 的描述文本即原 Popover 的正文
        SetDescription(strValue);
    }
    else if ((strName == _T("content_id")) || (strName == _T("contentid"))) {
        SetDescriptionId(strValue);
    }
    else if (strName == _T("icon")) {
        SetIcon(ParseIcon(strValue));
    }
    else if ((strName == _T("ok_text")) || (strName == _T("oktext"))) {
        SetOkText(strValue);
    }
    else if ((strName == _T("ok_text_id")) || (strName == _T("oktextid"))) {
        SetOkTextId(strValue);
    }
    else if ((strName == _T("cancel_text")) || (strName == _T("canceltext"))) {
        SetCancelText(strValue);
    }
    else if ((strName == _T("cancel_text_id")) || (strName == _T("canceltextid"))) {
        SetCancelTextId(strValue);
    }
    else if ((strName == _T("show_cancel")) || (strName == _T("showcancel"))) {
        SetShowCancel(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("ok_type")) || (strName == _T("oktype"))) {
        SetOkType(ParseOkType(strValue));
    }
    else if ((strName == _T("close_on_ok")) || (strName == _T("closeonok"))) {
        SetCloseOnOk(StringUtil::IsValueTrue(strValue));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Popconfirm::SetDescription(const DString& strDescription)
{
    //描述变化只影响下次打开：已打开的卡片保持原内容，避免闪烁
    m_strDescription = strDescription;
    m_strDescriptionId.clear();
}

DString Popconfirm::GetDescription() const
{
    if (!m_strDescriptionId.empty()) {
        return GlobalManager::Instance().Lang().GetStringByID(m_strDescriptionId.c_str());
    }
    return m_strDescription;
}

void Popconfirm::SetDescriptionId(const DString& strDescriptionId)
{
    m_strDescriptionId = strDescriptionId;
    m_strDescription.clear();
}

void Popconfirm::SetIcon(PopconfirmIcon icon)
{
    //图标影响卡片 XML 的结构，已显示的卡片保持原样，下次打开时生效
    m_icon = icon;
}

void Popconfirm::SetOkText(const DString& strText)
{
    m_strOkText = strText;
    if (!strText.empty()) {
        //显式设置了字面文本：多语言 ID 让位（否则 ID 优先生效，设置看起来"没反应"）
        m_strOkTextId.clear();
    }
    else {
        m_strOkTextId = GetOkLangId();
    }
}

DString Popconfirm::GetOkText() const
{
    if (!m_strOkTextId.empty()) {
        return GlobalManager::Instance().Lang().GetStringByID(m_strOkTextId.c_str());
    }
    return m_strOkText;
}

void Popconfirm::SetOkTextId(const DString& strTextId)
{
    m_strOkTextId = strTextId;
    if (!strTextId.empty()) {
        m_strOkText.clear();
    }
    else {
        m_strOkTextId = GetOkLangId();
    }
}

void Popconfirm::SetCancelText(const DString& strText)
{
    m_strCancelText = strText;
    if (!strText.empty()) {
        m_strCancelTextId.clear();
    }
    else {
        m_strCancelTextId = GetCancelLangId();
    }
}

DString Popconfirm::GetCancelText() const
{
    if (!m_strCancelTextId.empty()) {
        return GlobalManager::Instance().Lang().GetStringByID(m_strCancelTextId.c_str());
    }
    return m_strCancelText;
}

void Popconfirm::SetCancelTextId(const DString& strTextId)
{
    m_strCancelTextId = strTextId;
    if (!strTextId.empty()) {
        m_strCancelText.clear();
    }
    else {
        m_strCancelTextId = GetCancelLangId();
    }
}

void Popconfirm::SetShowCancel(bool bShowCancel)
{
    //按钮行影响卡片 XML 的结构，已显示的卡片保持原样，下次打开时生效
    m_bShowCancel = bShowCancel;
}

void Popconfirm::SetOkType(PopconfirmOkType okType)
{
    m_okType = okType;
}

void Popconfirm::SetCloseOnOk(bool bCloseOnOk)
{
    m_bCloseOnOk = bCloseOnOk;
}

void Popconfirm::Confirm()
{
    SendEvent(kEventPopconfirmConfirm);
    if (m_bCloseOnOk) {
        HideWithReason(PopoverCloseReason::kConfirm);
    }
}

void Popconfirm::Cancel()
{
    SendEvent(kEventPopconfirmCancel);
    HideWithReason(PopoverCloseReason::kCancel);
}

void Popconfirm::AppendTextAttribute(DString& xml, const DString& strTextId, const DString& strText) const
{
    if (!strTextId.empty()) {
        xml += _T(" text_id=\"");
        xml += EscapeXmlText(strTextId);
        xml += _T("\"");
    }
    else {
        xml += _T(" text=\"");
        xml += EscapeXmlText(strText);
        xml += _T("\"");
    }
}

DString Popconfirm::GetOkClassName() const
{
    switch (m_okType) {
    case PopconfirmOkType::kDefault:
        return _T("popconfirm_ok_default");
    case PopconfirmOkType::kDanger:
        return _T("popconfirm_ok_danger");
    default:
        return _T("popconfirm_ok_primary");
    }
}

DString Popconfirm::BuildCardBodyXml() const
{
    //标题行：图标 + 标题（标题为空时只显示图标）
    DString xml;
    xml += _T("<HBox width=\"stretch\" height=\"auto\">");
    if (m_icon != PopconfirmIcon::kNone) {
        DString strIconClass = _T("popconfirm_icon_warning");
        switch (m_icon) {
        case PopconfirmIcon::kInfo:
            strIconClass = _T("popconfirm_icon_info");
            break;
        case PopconfirmIcon::kSuccess:
            strIconClass = _T("popconfirm_icon_success");
            break;
        case PopconfirmIcon::kError:
            strIconClass = _T("popconfirm_icon_error");
            break;
        default:
            break;
        }
        xml += _T("<Control class=\"");
        xml += strIconClass;
        //图标与标题首行文字垂直居中：图标 16、标题行高 20 左右，向下偏移 1 像素即可对齐
        xml += _T("\" width=\"16\" height=\"16\" margin=\"0,1,8,0\" mouse_enabled=\"false\" tab_stop=\"false\"/>");
    }
    xml += _T("<Label class=\"popconfirm_title\"");
    AppendTextAttribute(xml, GetTitleId(), GetTitle());
    xml += _T("/>");
    xml += _T("</HBox>");

    //描述（可选）
    if (!m_strDescription.empty() || !m_strDescriptionId.empty()) {
        xml += _T("<Label class=\"popconfirm_description\"");
        AppendTextAttribute(xml, GetDescriptionId(), GetDescription());
        xml += _T(" margin=\"0,6,0,0\"/>");
    }

    //按钮行：右对齐；取消按钮在前，确定按钮在后（与 Ant Design 一致）
    xml += _T("<HBox width=\"auto\" height=\"auto\" halign=\"right\" margin=\"0,12,0,0\">");
    if (m_bShowCancel) {
        xml += _T("<Button class=\"popconfirm_cancel\" name=\"popconfirm_cancel\"");
        AppendTextAttribute(xml, GetCancelTextId(), GetCancelText());
        xml += _T("/>");
    }
    xml += _T("<Button class=\"");
    xml += GetOkClassName();
    xml += _T("\" name=\"popconfirm_ok\"");
    AppendTextAttribute(xml, GetOkTextId(), GetOkText());
    if (m_bShowCancel) {
        xml += _T(" margin=\"8,0,0,0\"");
    }
    xml += _T("/>");
    xml += _T("</HBox>");
    return xml;
}

void Popconfirm::InitializeCard(Flyout* pFlyout)
{
    if (pFlyout != nullptr) {
        Control* pOk = pFlyout->FindControl(_T("popconfirm_ok"));
        if (pOk != nullptr) {
            pOk->AttachEvent(kEventClick, UiBind(this, [this](const EventArgs& /*msg*/) -> bool {
                Confirm();
                return true;
                }, std::placeholders::_1), 0);
        }
        Control* pCancel = pFlyout->FindControl(_T("popconfirm_cancel"));
        if (pCancel != nullptr) {
            pCancel->AttachEvent(kEventClick, UiBind(this, [this](const EventArgs& /*msg*/) -> bool {
                Cancel();
                return true;
                }, std::placeholders::_1), 0);
        }
    }
    //基类负责绑定卡片自身的鼠标进入/离开事件（hover 触发时的关闭时序）
    BaseClass::InitializeCard(pFlyout);
}

} // namespace ui
