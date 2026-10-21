#include "Dialog.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/Button.h"
#include "duilib/Utils/StringUtil.h"

#include <algorithm>
#include <utility>

namespace ui
{

Dialog::Dialog() :
    m_skinFolder(_T("public")),
    m_nWidth(480),
    m_nHeight(360),
    m_bTextId(false),
    m_bTitleBarVisible(true),
    m_nClickedIndex(-1)
{
}

Dialog::~Dialog()
{
}

void Dialog::SetTitle(const DString& title)
{
    m_title = title;
}

void Dialog::SetTextIdMode(bool bTextId)
{
    m_bTextId = bTextId;
}

bool Dialog::IsTextIdMode() const
{
    return m_bTextId;
}

void Dialog::SetSkinFolder(const DString& skinFolder)
{
    m_skinFolder = skinFolder;
}

void Dialog::SetSize(int32_t nWidth, int32_t nHeight)
{
    //避免窗口过小导致标题栏和按钮区不可见
    m_nWidth = std::max<int32_t>(nWidth, 160);
    m_nHeight = std::max<int32_t>(nHeight, 120);
}

void Dialog::SetTitleBarVisible(bool bVisible)
{
    m_bTitleBarVisible = bVisible;
}

void Dialog::SetContentXml(const DString& contentXml)
{
    m_contentXml = contentXml;
}

bool Dialog::AddButton(const DString& id, const DString& text, bool bDefault)
{
    //按钮标识必须非空：DoModal 返回后需要通过它区分被点击的按钮
    ASSERT(!id.empty());
    if (id.empty()) {
        return false;
    }
    //按钮标识必须唯一：GetClickedButtonId 按标识返回结果，重复时无法区分
    for (const ButtonItem& button : m_buttons) {
        if (button.id == id) {
            ASSERT(0);
            return false;
        }
    }

    if (bDefault) {
        //默认按钮只保留一个：后设置的生效
        for (ButtonItem& button : m_buttons) {
            button.bDefault = false;
        }
    }
    ButtonItem item;
    item.id = id;
    item.text = text;
    item.bDefault = bDefault;
    m_buttons.push_back(std::move(item));
    return true;
}

void Dialog::AttachInit(std::function<void(Dialog*)> callback)
{
    m_initCallback = std::move(callback);
}

void Dialog::AttachButtonClick(std::function<bool(Dialog*, int32_t)> callback)
{
    m_buttonClickCallback = std::move(callback);
}

DString Dialog::GetSkinFolder()
{
    return m_skinFolder;
}

DString Dialog::GetSkinFile()
{
    return BuildWindowXml();
}

DString Dialog::BuildWindowXml()
{
    DString xml;
    xml += _T("<Window size=\"");
    xml += StringUtil::Int32ToString(m_nWidth);
    xml += _T(",");
    xml += StringUtil::Int32ToString(m_nHeight);
    xml += _T("\" caption=\"0,0,0,");
    xml += m_bTitleBarVisible ? _T("36") : _T("0");
    xml += _T("\" use_system_caption=\"false\" sys_menu=\"");
    xml += m_bTitleBarVisible ? _T("true") : _T("false");
    xml += _T("\" sys_menu_rect=\"0,0,36,36\"");
    xml += _T(" shadow_type=\"default\" shadow_attached=\"true\"");
    xml += _T(" layered_window=\"true\" size_box=\"0,0,0,0\"");
    xml += _T(" icon=\"public/caption/logo.ico\">");

    //根容器：标题栏 + 内容区 + 按钮区
    xml += _T("<VBox width=\"stretch\" height=\"stretch\" bkcolor=\"bg_window_main\">");
    if (m_bTitleBarVisible) {
        xml += _T("<HBox name=\"window_title_bar\" width=\"stretch\" height=\"36\" bkcolor=\"bg_titlebar\">");
        xml += _T("<Control width=\"18\" height=\"18\" bkimage=\"public/caption/logo.svg\" valign=\"center\" margin=\"8\" tab_stop=\"false\"/>");
        xml += _T("<Label name=\"dialog_title\" valign=\"center\" margin=\"8,0,0,0\" mouse_enabled=\"false\" tab_stop=\"false\"/>");
        xml += _T("<Control tab_stop=\"false\"/>");
        xml += _T("<Button class=\"btn_wnd_close_11\" height=\"stretch\" width=\"40\" name=\"btn_window_close\" tab_stop=\"false\"/>");
        xml += _T("</HBox>");
    }

    //内容区：调用方提供的 XML 片段原样插入
    xml += _T("<VBox name=\"dialog_content\" width=\"stretch\" height=\"stretch\" padding=\"20,16,20,8\">");
    xml += m_contentXml;
    xml += _T("</VBox>");

    //按钮区：右对齐，按钮数量与文字由调用方决定
    xml += _T("<HBox name=\"dialog_buttons\" width=\"stretch\" height=\"auto\" child_halign=\"right\" padding=\"20,8,20,20\" child_margin_x=\"10\">");
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const ButtonItem& item = m_buttons[i];
        xml += _T("<Button class=\"");
        xml += item.bDefault ? _T("btn_global_blue_80x30") : _T("btn_global_white_80x30");
        xml += _T("\" name=\"dialog_btn_");
        xml += StringUtil::Int32ToString((int32_t)i);
        xml += _T("\" width=\"80\" height=\"32\"");
        if ((m_bTextId && GlobalManager::Instance().Lang().HasStringByID(item.text))) {
            xml += _T(" text_id=\"");
            xml += XmlEscape(item.text);
            xml += _T("\"");
        }
        else {
            xml += _T(" text=\"");
            xml += XmlEscape(item.text);
            xml += _T("\"");
        }
        xml += _T("/>");
    }
    xml += _T("</HBox>");
    xml += _T("</VBox>");
    xml += _T("</Window>");
    return xml;
}

DString Dialog::XmlEscape(const DString& text)
{
    DString result;
    result.reserve(text.size());
    for (DString::value_type ch : text) {
        switch (ch) {
        case _T('&'):   result += _T("&amp;");  break;
        case _T('<'):   result += _T("&lt;");   break;
        case _T('>'):   result += _T("&gt;");   break;
        case _T('\"'):  result += _T("&quot;"); break;
        case _T('\''):  result += _T("&apos;"); break;
        default:        result += ch;           break;
        }
    }
    return result;
}

void Dialog::OnInitWindow()
{
    BaseClass::OnInitWindow();

    ui::Label* pTitleLabel = dynamic_cast<ui::Label*>(FindControl(_T("dialog_title")));
    if (pTitleLabel != nullptr) {
        if (m_bTextId) {
            pTitleLabel->SetTextId(m_title);
        }
        else {
            pTitleLabel->SetText(m_title);
        }
    }

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        DString btnName = _T("dialog_btn_") + StringUtil::Int32ToString((int32_t)i);
        ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(btnName));
        if (pButton == nullptr) {
            continue;
        }
        const int32_t nIndex = (int32_t)i;
        pButton->AttachClick([this, nIndex](const ui::EventArgs& /*args*/) {
            m_nClickedIndex = nIndex;
            //回调在窗口销毁前执行，便于读取内容区控件状态；返回 false 时保持对话框打开
            if (m_buttonClickCallback && !m_buttonClickCallback(this, nIndex)) {
                m_nClickedIndex = -1;
                return true;
            }
            CloseWnd(kResultButtonBase + nIndex);
            return true;
        });
    }

    //初始焦点与 Enter 的默认按钮映射保持一致：优先第一个默认按钮，否则第一个按钮
    const int32_t nDefaultIndex = GetDefaultButtonIndex();
    if (nDefaultIndex >= 0) {
        DString btnName = _T("dialog_btn_") + StringUtil::Int32ToString(nDefaultIndex);
        ui::Button* pDefaultButton = dynamic_cast<ui::Button*>(FindControl(btnName));
        if (pDefaultButton != nullptr) {
            pDefaultButton->SetFocus();
        }
    }

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    //注册到 NativeWindow_Windows 的 Enter 键放行列表，让 IsDialogMessageDuiLib 对本窗口放行 VK_RETURN
    ui::NativeWindow_Windows::SetEnterKeyPassthrough(NativeWnd()->GetHWND(), true);
#endif

    if (m_initCallback) {
        m_initCallback(this);
    }
}

void Dialog::OnPreCloseWindow()
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    //注销 Enter 键放行，避免窗口句柄复用后影响其他模态对话框
    ui::NativeWindow_Windows::SetEnterKeyPassthrough(NativeWnd()->GetHWND(), false);
#endif
    BaseClass::OnPreCloseWindow();
}

LRESULT Dialog::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    if (vkCode == kVK_TAB) {
        //首次 TAB 导航时，为所有按钮开启焦点矩形显示（仅焦点按钮实际可见）
        for (size_t i = 0; i < m_buttons.size(); ++i) {
            DString btnName = _T("dialog_btn_") + StringUtil::Int32ToString((int32_t)i);
            ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(btnName));
            if (pButton != nullptr) {
                pButton->SetShowFocusedRect(true);
            }
        }
    }
    return BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

int32_t Dialog::DoModal(ui::WindowBase* pParentWindow)
{
    m_nClickedIndex = -1;

    //未指定按钮时，提供一个"确定"按钮，避免对话框无法关闭
    if (m_buttons.empty()) {
        DString text;
        if (m_bTextId) {
            text = _T("STRID_MSGBOX_OK");
        }
        else {
            text = ui::GlobalManager::GetTextById(_T("STRID_MSGBOX_OK"));
            if (text.empty()) {
                text = _T("Ok");
            }
        }
        AddButton(_T("ok"), text, true);
    }

    //bTextId 模式下，任务栏/Alt+Tab 的原生窗口标题需要使用翻译后的文本
    DString strWindowTitle = m_title;
    if (m_bTextId) {
        strWindowTitle = ui::GlobalManager::GetTextById(m_title);
    }
    ui::WindowCreateParam createParam(strWindowTitle, true, m_bTextId ? m_title : _T(""));

    //bCloseByEsc=true：ESC 关闭，返回 kWindowCloseCancel(2)
#if defined (DUILIB_BUILD_FOR_SDL)
    //SDL 平台的模态循环会把 Enter 直接当作关闭窗口处理，导致内容区输入控件无法收到回车；
    //这里关闭该行为，Enter 交由焦点控件处理（默认按钮初始持有焦点）
    int32_t nResult = BaseClass::DoModal(pParentWindow, createParam, true, false);
#else
    //Windows 下 bCloseByEnter=true：Enter 关闭返回 kWindowCloseOK(1)，再映射为默认按钮；
    //存在内联钩子时 Enter 会放行给窗口过程，由焦点按钮响应，不会走到该分支
    int32_t nResult = BaseClass::DoModal(pParentWindow, createParam, true, true);
#endif
    if (nResult < 0) {
        //窗口创建失败（NativeWindow 返回 -1），与取消语义区分开
        return kResultFailed;
    }

    if (nResult == kWindowCloseOK) {
        //Enter 键：等同点击默认按钮
        m_nClickedIndex = GetDefaultButtonIndex();
        return m_nClickedIndex;
    }

    if (nResult >= kResultButtonBase) {
        //点击了底部按钮：去掉内部编码，返回按钮索引
        const int32_t nIndex = nResult - kResultButtonBase;
        if (nIndex < (int32_t)m_buttons.size()) {
            m_nClickedIndex = nIndex;
            return nIndex;
        }
    }

    //ESC 键、标题栏关闭按钮(kWindowCloseNormal=0)
    m_nClickedIndex = -1;
    return kResultCancel;
}

int32_t Dialog::GetDefaultButtonIndex() const
{
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].bDefault) {
            return (int32_t)i;
        }
    }
    return m_buttons.empty() ? -1 : 0;
}

int32_t Dialog::GetClickedButtonIndex() const
{
    return m_nClickedIndex;
}

DString Dialog::GetClickedButtonId() const
{
    if ((m_nClickedIndex < 0) || (m_nClickedIndex >= (int32_t)m_buttons.size())) {
        return DString();
    }
    return m_buttons[(size_t)m_nClickedIndex].id;
}

}//namespace ui
