#include "AddressBar.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/Label.h"
#include "duilib/Control/Menu.h"
#include "duilib/Control/AutoComplete.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Keycode.h"
#include "duilib/Core/ControlDropTarget.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/Clipboard.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#ifdef DUILIB_BUILD_FOR_WIN
    #include "duilib/Utils/DiskUtils_Windows.h"
    #include <shellapi.h>
#endif

namespace ui
{

/** AddressBar 内部使用的单行编辑框：联想浮层打开时拦截上下方向键、回车、Esc
*/
class AddressBarEdit : public RichEdit
{
    typedef RichEdit BaseClass;
public:
    explicit AddressBarEdit(Window* pWindow) :
        RichEdit(pWindow),
        m_pOwner(nullptr)
    {
    }

    /** 设置宿主控件
    */
    void SetOwner(AddressBar* pOwner)
    {
        m_pOwner = pOwner;
    }

    virtual bool OnKeyDown(const EventArgs& msg) override
    {
        //浮层打开时，导航类按键交给宿主处理，不传递给编辑框
        if ((m_pOwner != nullptr) && m_pOwner->HandleEditKeyDown(msg)) {
            return true;
        }
        return BaseClass::OnKeyDown(msg);
    }

private:
    /** 宿主控件
    */
    AddressBar* m_pOwner;
};

AddressBar::AddressBar(Window* pWindow):
    HBox(pWindow),
    m_pRichEdit(nullptr),
    m_pBarBox(nullptr),
    m_pResetBtn(nullptr),
    m_pCollapseBtn(nullptr),
    m_pHistoryBtn(nullptr),
    m_bEnableTooltip(true),
    m_bReturnUpdateUI(true),
    m_bEscUpdateUI(true),
    m_bKillFocusUpdateUI(true),
    m_bUpdatingUI(false),
    m_nRightPadding(16),
    m_nMaxHistory(20),
    m_bHistoryEnabled(true),
    m_bEnableDropNavigation(true),
    m_pFlyout(nullptr),
    m_bUpdatingPopup(false),
    m_bUserNavigated(false),
    m_bDefaultContextMenu(false),
    m_bPathError(false),
    m_bDropHighlight(false),
    m_nLayoutWidth(-1),
    m_nLayoutHeight(-1),
    m_nPathVersion(0),
    m_nLayoutPathVersion(0),
    m_editClass(_T("address_bar_edit")),
    m_resetBtnClass(_T("address_bar_reset_button")),
    m_subPathHBoxClass(_T("address_bar_sub_path_hbox")),
    m_subPathBtnClass(_T("address_bar_sub_path_button")),
    m_subPathRootClass(_T("address_bar_sub_path_root")),
    m_subPathDriveClass(_T("address_bar_sub_path_drive")),
    m_subPathCurrentClass(_T("address_bar_sub_path_button_current")),
    m_collapseBtnClass(_T("address_bar_sub_path_collapse")),
    m_pathSeparatorClass(_T("address_bar_path_separator"))
{
}

AddressBar::~AddressBar()
{
    CloseAutoCompletePopup();
}

DString AddressBar::GetType() const { return DUI_CTR_ADDRESS_BAR; }

void AddressBar::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("address_path")) {
        SetAddressPath(strValue);
    }
    else if (strName == _T("path_tooltip")) {
        SetEnablePathTooltip(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("return_update_ui")) {
        SetReturnUpdateUI(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("esc_update_ui")) {
        SetEscUpdateUI(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("kill_focus_update_ui")) {
        SetKillFocusUpdateUI(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("rich_edit_class")) {
        SetRichEditClass(strValue);
    }
    else if (strName == _T("reset_button_class")) {
        SetResetButtonClass(strValue);
    }
    else if (strName == _T("sub_path_hbox_class")) {
        SetSubPathHBoxClass(strValue);
    }
    else if (strName == _T("sub_path_button_class")) {
        SetSubPathBtnClass(strValue);
    }
    else if (strName == _T("sub_path_root_class")) {
        SetSubPathRootClass(strValue);
    }
    else if (strName == _T("sub_path_drive_class")) {
        SetSubPathDriveClass(strValue);
    }
    else if (strName == _T("sub_path_current_class")) {
        SetSubPathCurrentClass(strValue);
    }
    else if (strName == _T("collapse_button_class")) {
        SetCollapseBtnClass(strValue);
    }
    else if (strName == _T("path_separator_class")) {
        SetPathSeparatorClass(strValue);
    }
    else if (strName == _T("right_padding")) {
        SetRightPadding(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if (strName == _T("empty_display_text")) {
        SetEmptyDisplayText(strValue);
    }
    else if (strName == _T("empty_display_text_id")) {
        SetEmptyDisplayTextId(strValue);
    }
    else if (strName == _T("history_enabled")) {
        SetHistoryEnabled(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("max_history")) {
        SetMaxHistory(StringUtil::StringToInt32(strValue.c_str()));
    }
    else if (strName == _T("drop_navigation")) {
        SetEnableDropNavigation(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("default_context_menu")) {
        SetDefaultContextMenuEnabled(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("history_file")) {
        SetHistoryFile(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void AddressBar::SetEnablePathTooltip(bool bEnable)
{
    m_bEnableTooltip = bEnable;
}

bool AddressBar::IsEnablePathTooltip() const
{
    return m_bEnableTooltip;
}

void AddressBar::SetReturnUpdateUI(bool bUpdateUI)
{
    m_bReturnUpdateUI = bUpdateUI;
}

bool AddressBar::IsReturnUpdateUI() const
{
    return m_bReturnUpdateUI;
}

void AddressBar::SetEscUpdateUI(bool bUpdateUI)
{
    m_bEscUpdateUI = bUpdateUI;
}

bool AddressBar::IsEscUpdateUI() const
{
    return m_bEscUpdateUI;
}

void AddressBar::SetKillFocusUpdateUI(bool bUpdateUI)
{
    m_bKillFocusUpdateUI = bUpdateUI;
}

bool AddressBar::IsKillFocusUpdateUI() const
{
    return m_bKillFocusUpdateUI;
}

void AddressBar::SetRichEditClass(const DString& editClass)
{
    m_editClass = editClass;
}

DString AddressBar::GetRichEditClass() const
{
    return m_editClass.c_str();
}

void AddressBar::SetResetButtonClass(const DString& resetButtonClass)
{
    m_resetBtnClass = resetButtonClass;
    if (m_pResetBtn != nullptr) {
        m_pResetBtn->SetClass(m_resetBtnClass.c_str());
    }
}

DString AddressBar::GetResetButtonClass() const
{
    return m_resetBtnClass.c_str();
}

void AddressBar::SetSubPathHBoxClass(const DString& hboxClass)
{
    m_subPathHBoxClass = hboxClass;
}

DString AddressBar::GetSubPathHBoxClass() const
{
    return m_subPathHBoxClass.c_str();
}

void AddressBar::SetSubPathBtnClass(const DString& subPathBtnClass)
{
    m_subPathBtnClass = subPathBtnClass;
}

DString AddressBar::GetSubPathBtnClass() const
{
    return m_subPathBtnClass.c_str();
}

void AddressBar::SetSubPathRootClass(const DString& subPathRootClass)
{
    m_subPathRootClass = subPathRootClass;
}

DString AddressBar::GetSubPathRootClass() const
{
    return m_subPathRootClass.c_str();
}

void AddressBar::SetSubPathDriveClass(const DString& subPathDriveClass)
{
    m_subPathDriveClass = subPathDriveClass;
}

DString AddressBar::GetSubPathDriveClass() const
{
    return m_subPathDriveClass.c_str();
}

void AddressBar::SetSubPathCurrentClass(const DString& subPathCurrentClass)
{
    m_subPathCurrentClass = subPathCurrentClass;
}

DString AddressBar::GetSubPathCurrentClass() const
{
    return m_subPathCurrentClass.c_str();
}

void AddressBar::SetCollapseBtnClass(const DString& collapseBtnClass)
{
    m_collapseBtnClass = collapseBtnClass;
    if (m_pCollapseBtn != nullptr) {
        m_pCollapseBtn->SetClass(m_collapseBtnClass.c_str());
    }
}

DString AddressBar::GetCollapseBtnClass() const
{
    return m_collapseBtnClass.c_str();
}

void AddressBar::SetPathSeparatorClass(const DString& pathSeparatorClass)
{
    m_pathSeparatorClass = pathSeparatorClass;
}

DString AddressBar::GetPathSeparatorClass() const
{
    return m_pathSeparatorClass.c_str();
}

void AddressBar::SetRightPadding(int32_t nPadding)
{
    m_nRightPadding = std::max(nPadding, 0);
    if (m_pBarBox != nullptr) {
        m_pBarBox->SetPadding(UiPadding(0, 0, m_nRightPadding, 0), true);
    }
    UpdateAddressBarControlsStatus();
}

int32_t AddressBar::GetRightPadding() const
{
    return m_nRightPadding;
}

void AddressBar::SetEmptyDisplayText(const DString& text)
{
    m_emptyDisplayText = text;
}

DString AddressBar::GetEmptyDisplayText() const
{
    if (!m_emptyDisplayText.empty()) {
        return m_emptyDisplayText.c_str();
    }
    if (!m_emptyDisplayTextId.empty()) {
        return GlobalManager::Instance().Lang().GetStringByID(m_emptyDisplayTextId.c_str());
    }
    return DString();
}

void AddressBar::SetEmptyDisplayTextId(const DString& textId)
{
    m_emptyDisplayTextId = textId;
    if (m_showAddressPath.empty()) {
        UpdateAddressBarControls(m_showAddressPath.c_str(), true);
    }
}

DString AddressBar::GetEmptyDisplayTextId() const
{
    return m_emptyDisplayTextId.c_str();
}

void AddressBar::SetHistoryEnabled(bool bEnable)
{
    m_bHistoryEnabled = bEnable;
}

bool AddressBar::IsHistoryEnabled() const
{
    return m_bHistoryEnabled;
}

void AddressBar::SetMaxHistory(int32_t nMax)
{
    m_nMaxHistory = std::max(nMax, 1);
    while ((int32_t)m_pathHistory.size() > m_nMaxHistory) {
        m_pathHistory.pop_back();
    }
}

int32_t AddressBar::GetMaxHistory() const
{
    return m_nMaxHistory;
}

void AddressBar::AddPathHistory(const DString& path)
{
    if (!m_bHistoryEnabled || path.empty()) {
        return;
    }
    //路径归一化：去掉末尾多余的路径分隔符（如 "E:\dir\" 统一为 "E:\dir"），
    //避免自动补全的历史记录中出现重复项（"E:\dir\" 与 "E:\dir"）
    DString normalizedPath = path;
    while ((normalizedPath.size() > 3) &&
           ((normalizedPath.back() == _T('\\')) || (normalizedPath.back() == _T('/')))) {
        normalizedPath.pop_back();
    }
    UiString historyPath = normalizedPath.c_str();
    for (auto it = m_pathHistory.begin(); it != m_pathHistory.end(); ++it) {
        if (*it == historyPath) {
            m_pathHistory.erase(it);
            break;
        }
    }
    m_pathHistory.insert(m_pathHistory.begin(), historyPath);
    while ((int32_t)m_pathHistory.size() > m_nMaxHistory) {
        m_pathHistory.pop_back();
    }
    SavePathHistory();
    UpdateHistoryButtonVisible();
}

std::vector<DString> AddressBar::GetPathHistory() const
{
    std::vector<DString> history;
    history.reserve(m_pathHistory.size());
    for (const UiString& path : m_pathHistory) {
        history.push_back(path.c_str());
    }
    return history;
}

void AddressBar::ClearPathHistory()
{
    m_pathHistory.clear();
    SavePathHistory();
    UpdateHistoryButtonVisible();
}

void AddressBar::SetDefaultContextMenuEnabled(bool bEnable)
{
    m_bDefaultContextMenu = bEnable;
}

bool AddressBar::IsDefaultContextMenuEnabled() const
{
    return m_bDefaultContextMenu;
}

void AddressBar::SetHistoryFile(const DString& historyFile)
{
    m_historyFile = historyFile.c_str();
}

DString AddressBar::GetHistoryFile() const
{
    return m_historyFile.c_str();
}

void AddressBar::LoadPathHistory()
{
    if (!m_bHistoryEnabled || m_historyFile.empty()) {
        return;
    }
    const DString historyFilePath = GetHistoryFilePath();
    std::vector<uint8_t> fileData;
    if (!FileUtil::ReadFileData(FilePath(historyFilePath.c_str()), fileData) || fileData.empty()) {
        return;
    }
    DStringA utf8Data(fileData.begin(), fileData.end());
    std::vector<UiString> paths;
    size_t nStart = 0;
    while (nStart < utf8Data.size()) {
        size_t nEnd = utf8Data.find('\n', nStart);
        if (nEnd == DStringA::npos) {
            nEnd = utf8Data.size();
        }
        std::string line = utf8Data.substr(nStart, nEnd - nStart);
        if (!line.empty() && (line.back() == '\r')) {
            line.pop_back();
        }
        if (!line.empty()) {
            paths.push_back(StringConvert::UTF8ToT(line).c_str());
        }
        nStart = nEnd + 1;
    }
    if (paths.empty()) {
        return;
    }
    m_pathHistory.clear();
    for (const UiString& path : paths) {
        bool bExists = false;
        for (const UiString& item : m_pathHistory) {
            if (item == path) {
                bExists = true;
                break;
            }
        }
        if (!bExists) {
            m_pathHistory.push_back(path);
        }
    }
    while ((int32_t)m_pathHistory.size() > m_nMaxHistory) {
        m_pathHistory.pop_back();
    }
    UpdateHistoryButtonVisible();
}

void AddressBar::SavePathHistory()
{
    if (m_historyFile.empty()) {
        return;
    }
    DStringA utf8Data;
    for (const UiString& path : m_pathHistory) {
        utf8Data += StringConvert::TToUTF8(path.c_str());
        utf8Data += '\n';
    }
    FilePath historyFilePath(GetHistoryFilePath().c_str());
    if (historyFilePath.IsEmpty()) {
        return;
    }
    //确保历史文件所在目录存在（如 "user" 目录首次使用时可能尚不存在）
    FilePathUtil::CreateDirectories(historyFilePath.GetParentPath().ToString());
    FileUtil::WriteFileData(historyFilePath, utf8Data);
}

DString AddressBar::GetHistoryFilePath() const
{
    if (m_historyFile.empty()) {
        return DString();
    }
    FilePath historyFilePath(m_historyFile.c_str());
    if (historyFilePath.IsRelativePath()) {
        //相对路径：拼接可执行程序所在目录（与 TreeView_settings.ini 的存放逻辑保持一致）
        historyFilePath = FilePathUtil::JoinFilePath(FilePathUtil::GetCurrentModuleDirectory(), historyFilePath);
    }
    return historyFilePath.ToString();
}

void AddressBar::UpdateHistoryButtonVisible()
{
    if (m_pHistoryBtn == nullptr) {
        return;
    }
    const bool bShow = m_bHistoryEnabled && !m_pathHistory.empty() && !IsEditing();
    m_pHistoryBtn->SetVisible(bShow);
}

void AddressBar::SetPathError(bool bError)
{
    if (m_bPathError == bError) {
        return;
    }
    m_bPathError = bError;
    UpdateBorderHighlight();
}

bool AddressBar::IsPathError() const
{
    return m_bPathError;
}

void AddressBar::SetDropHighlight(bool bHighlight)
{
    if (m_bDropHighlight == bHighlight) {
        return;
    }
    m_bDropHighlight = bHighlight;
    UpdateBorderHighlight();
}

void AddressBar::UpdateBorderHighlight()
{
    if (m_bPathError || m_bDropHighlight) {
        if (m_savedBorderColors.empty()) {
            m_savedBorderColors.resize(4);
            for (int32_t i = 0; i < 4; ++i) {
                m_savedBorderColors[(size_t)i] = GetBorderColor((ControlStateType)i);
            }
        }
        //错误状态优先于拖放高亮
        SetBorderColor(m_bPathError ? DString(_T("color_error")) : DString(_T("color_accent")));
    }
    else if (!m_savedBorderColors.empty()) {
        for (int32_t i = 0; i < 4; ++i) {
            SetBorderColor((ControlStateType)i, m_savedBorderColors[(size_t)i]);
        }
        m_savedBorderColors.clear();
    }
    Invalidate();
}

void AddressBar::SetEnableDropNavigation(bool bEnable)
{
    m_bEnableDropNavigation = bEnable;
}

bool AddressBar::IsEnableDropNavigation() const
{
    return m_bEnableDropNavigation;
}

void AddressBar::AttachPathChanging(const AddressBarPathChangingEvent& callback)
{
    m_pathChangingCallback = callback;
}

void AddressBar::DetachPathChanging()
{
    m_pathChangingCallback = nullptr;
}

void AddressBar::OnSetVisible(bool bChanged)
{
    BaseClass::OnSetVisible(bChanged);
    if (IsVisible()) {
        ShowAddressEdit(false);
        UpdateAddressBarControlsStatus();
    }
}

void AddressBar::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    if (m_showAddressPath.empty()) {
        UpdateAddressBarControls(m_showAddressPath.c_str(), true);
    }
}

void AddressBar::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
    //更新子控件的状态
    UpdateAddressBarControlsStatus();
}

bool AddressBar::OnKeyDown(const EventArgs& msg)
{
    //焦点在面包屑按钮上时，由按钮自己的回调处理；这里处理焦点在AddressBar自身的情况
    if (HandleNavigationKeyDown(msg)) {
        return true;
    }
    if ((msg.vkCode == kVK_HOME) || (msg.vkCode == kVK_END)) {
        FocusSubPathEdge(msg.vkCode == kVK_HOME);
        return true;
    }
    if ((msg.vkCode == kVK_RETURN) && IsEditing()) {
        OnAddressBarReturn();
        return true;
    }
    return BaseClass::OnKeyDown(msg);
}

void AddressBar::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //地址栏的基本结构
    //<AddressBar>
    //   <RichEdit/>  编辑框
    //   <Button/>    “恢复当前路径”按钮（仅编辑态显示）
    //   <HBox>
    //        <HBox>
    //            <Button/> 目录
    //            <Button/> 分隔符
    //        </HBox>
    //        ...
    //        ...
    //    </HBox>
    //</AddressBar>

    if (m_pRichEdit == nullptr) {
        AddressBarEdit* pEdit = new AddressBarEdit(GetWindow());
        pEdit->SetOwner(this);
        m_pRichEdit = pEdit;
        AddItem(m_pRichEdit);
        m_pRichEdit->SetClass(GetRichEditClass());
        //不再使用RichEdit内置的“清除文本”按钮
        m_pRichEdit->SetAttribute(_T("clear_btn_class"), _T(""));
        m_pRichEdit->SetFixedWidth(UiFixedInt::MakeStretch(), true, false);
        m_pRichEdit->SetFixedHeight(UiFixedInt::MakeStretch(), true, false);

        m_pRichEdit->AttachSetFocus([this](const EventArgs&) {
            OnAddressBarSetFocus(m_pRichEdit);
            return true;
            });
        m_pRichEdit->AttachKillFocus([this](const EventArgs& args) {
            OnAddressBarKillFocus((Control*)args.wParam);
            return true;
            });
        m_pRichEdit->AttachReturn([this](const EventArgs&) {
            OnAddressBarReturn();
            return true;
            });
        m_pRichEdit->AttachEsc([this](const EventArgs&) {
            OnAddressBarEsc();
            return true;
            });
        m_pRichEdit->AttachEvent(kEventTextChanged, [this](const EventArgs&) {
            //用户重新输入后，清除上一次的路径错误提示
            SetPathError(false);
            if (IsEditing()) {
                UpdateAutoCompletePopup();
            }
            return true;
            }, 0);
    }

    if (m_pResetBtn == nullptr) {
        m_pResetBtn = new Button(GetWindow());
        AddItem(m_pResetBtn);
        m_pResetBtn->SetClass(GetResetButtonClass());
        m_pResetBtn->SetFixedWidth(UiFixedInt(24), true, false);
        m_pResetBtn->SetToolTipText(GlobalManager::Instance().Lang().GetStringByID(_T("STRID_ADDRESS_BAR_RESET_PATH")));
        m_pResetBtn->SetVisible(false);
        m_pResetBtn->AttachClick([this](const EventArgs&) {
            ResetEditText();
            return true;
            });
    }

    if (m_pBarBox == nullptr) {
        m_pBarBox = new HBox(GetWindow());
        AddItem(m_pBarBox);
        m_pBarBox->SetFixedWidth(UiFixedInt::MakeStretch(), true, false);
        m_pBarBox->SetPadding(UiPadding(0, 0, m_nRightPadding, 0), true);

        m_pBarBox->AttachSetFocus([this](const EventArgs&) {
            ShowAddressEdit(true);
            OnAddressBarSetFocus(m_pBarBox);
            return true;
            });
        m_pBarBox->AttachKillFocus([this](const EventArgs& args) {
            OnAddressBarKillFocus((Control*)args.wParam);
            return true;
            });
        m_pBarBox->AttachButtonDown([this](const EventArgs&) {
            ShowAddressEdit(true);
            return true;
            });

        //中间折叠段（"…"）按钮
        m_pCollapseBtn = new Button(GetWindow());
        m_pBarBox->AddItem(m_pCollapseBtn);
        m_pCollapseBtn->SetClass(GetCollapseBtnClass());
        m_pCollapseBtn->SetNoFocus();
        m_pCollapseBtn->SetVisible(false);
        m_pCollapseBtn->AttachClick([this](const EventArgs&) {
            ShowCollapsedPathMenu();
            return true;
            });
    }

    m_pRichEdit->SetVisible(false);

    //“最近路径”下拉按钮：点击后弹出历史路径菜单
    if (m_pHistoryBtn == nullptr) {
        m_pHistoryBtn = new Button(GetWindow());
        AddItem(m_pHistoryBtn);
        m_pHistoryBtn->SetClass(_T("address_bar_history_button"));
        m_pHistoryBtn->SetFixedWidth(UiFixedInt(24), true, false);
        m_pHistoryBtn->SetNoFocus();
        m_pHistoryBtn->SetVisible(false);
        m_pHistoryBtn->SetToolTipText(GlobalManager::Instance().Lang().GetStringByID(_T("STRID_ADDRESS_BAR_RECENT_PATHS")));
        m_pHistoryBtn->AttachClick([this](const EventArgs&) {
            ShowPathHistoryMenu();
            return true;
            });
    }

    //获取焦点的时候，切换到编辑框
    AttachSetFocus([this](const EventArgs&) {
        ShowAddressEdit(true);
        return true;
        });
    AttachButtonDown([this](const EventArgs&) {
        ShowAddressEdit(true);
        return true;
        });

    //右键菜单：根据鼠标位置确定点击的面包屑路径（外部通过GetClickedAddressPath获取）
    AttachContextMenu([this](const EventArgs& msg) {
        //每次右键都按当前位置重新计算点击的路径段
        m_clickedAddressPath = m_addressPath;
        for (size_t index = 0; (index < m_subPathBoxes.size()) && (index < m_pathSegments.size()); ++index) {
            Control* pBox = m_subPathBoxes[index];
            if ((pBox != nullptr) && pBox->IsVisible() && pBox->GetRect().ContainsPt(msg.ptMouse)) {
                m_clickedAddressPath = m_pathSegments[index].m_fullPath;
                break;
            }
        }
        if (m_bDefaultContextMenu) {
            ShowDefaultContextMenu(msg.ptMouse);
            //内置菜单已处理，终止后续派发，避免外部再弹出一个菜单
            return false;
        }
        //返回true表示继续派发该事件，使外部绑定的右键菜单处理函数也能收到事件
        return true;
        });

    //支持拖放目录到地址栏进行导航，并在拖动悬停时高亮边框
    SetAttribute(_T("enable_drag_drop"), _T("true"));
    AttachEvent(kEventDropData, [this](const EventArgs& args) {
        SetDropHighlight(false);
        return OnDropData(args);
        }, 0);
    AttachDropOver([this](const EventArgs&) {
        SetDropHighlight(true);
        return true;
        });
    AttachDropLeave([this](const EventArgs&) {
        SetDropHighlight(false);
        return true;
        });

    //初始化地址栏显示内容（空路径时显示“此电脑”等占位文本）
    m_pRichEdit->SetTextNoEvent(m_addressPath.c_str());
    UpdateAddressBarControls(m_addressPath.c_str(), true);
    LoadPathHistory();
    UpdateHistoryButtonVisible();
}

void AddressBar::ShowAddressEdit(bool bShow)
{
    if (IsEditing() == bShow) {
        if (bShow && (m_pRichEdit != nullptr)) {
            m_pRichEdit->SetFocus();
        }
        return;
    }
    if (!bShow) {
        CloseAutoCompletePopup();
    }
    if (m_pBarBox != nullptr) {
        m_pBarBox->SetVisible(!bShow);
    }
    if (m_pResetBtn != nullptr) {
        m_pResetBtn->SetVisible(bShow);
    }
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetVisible(bShow);
        if (bShow) {
            //进入编辑状态时，用当前显示路径初始化编辑框
            m_pRichEdit->SetTextNoEvent(GetAddressPath());
            m_pRichEdit->SetSel(0, -1);
            m_pRichEdit->SetFocus();
        }
    }
    //编辑态隐藏“最近路径”按钮，面包屑态按历史记录情况显示
    UpdateHistoryButtonVisible();
}

bool AddressBar::IsEditing() const
{
    return (m_pRichEdit != nullptr) && m_pRichEdit->IsVisible();
}

void AddressBar::ResetEditText()
{
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetTextNoEvent(GetAddressPath());
        m_pRichEdit->SetSel(0, -1);
        m_pRichEdit->SetFocus();
    }
}

void AddressBar::EnsureSubPathControls(size_t nIndex)
{
    ASSERT(m_pBarBox != nullptr);
    if (m_pBarBox == nullptr) {
        return;
    }
    //复用已有控件
    if (nIndex < m_subPathBoxes.size()) {
        return;
    }
    while (m_subPathBoxes.size() <= nIndex) {
        HBox* pBox = new HBox(GetWindow());
        m_pBarBox->AddItem(pBox);
        pBox->SetClass(GetSubPathHBoxClass());
        pBox->SetNoFocus();

        Button* pDisplayNameBtn = new Button(GetWindow());
        pBox->AddItem(pDisplayNameBtn);
        pDisplayNameBtn->SetClass(GetSubPathBtnClass());
        pDisplayNameBtn->SetTabStop(true);

        const size_t nSegmentIndex = m_subPathBoxes.size();
        pDisplayNameBtn->AttachClick([this, nSegmentIndex, pDisplayNameBtn](const EventArgs&) {
            if (nSegmentIndex < m_pathSegments.size()) {
                OnClickedSubPath(m_pathSegments[nSegmentIndex].m_fullPath);
            }
            //点击路径段后让面包屑获得焦点（导航过程中焦点可能被重置，因此放在导航之后），
            //便于后续使用键盘（Home/End、Alt+Up、F6 等）
            if (pDisplayNameBtn->IsVisible()) {
                pDisplayNameBtn->SetFocus();
            }
            return true;
            });
        //右键菜单：转发给AddressBar的kEventContextMenu事件
        pDisplayNameBtn->AttachContextMenu([this, nSegmentIndex](const EventArgs&) {
            if (nSegmentIndex < m_pathSegments.size()) {
                m_clickedAddressPath = m_pathSegments[nSegmentIndex].m_fullPath;
                SendEvent(kEventContextMenu);
            }
            return true;
            });
        //键盘：左右方向键在面包屑之间移动焦点，Home/End跳到首末段，回车激活
        pDisplayNameBtn->AttachEvent(kEventKeyDown, [this, nSegmentIndex](const EventArgs& msg) {
            if ((msg.vkCode == kVK_LEFT) || (msg.vkCode == kVK_RIGHT)) {
                MoveSubPathFocus(nSegmentIndex, msg.vkCode == kVK_LEFT);
                return true;
            }
            if ((msg.vkCode == kVK_HOME) || (msg.vkCode == kVK_END)) {
                FocusSubPathEdge(msg.vkCode == kVK_HOME);
                return true;
            }
            if (HandleNavigationKeyDown(msg)) {
                return true;
            }
            if ((msg.vkCode == kVK_RETURN) || (msg.vkCode == kVK_SPACE)) {
                if (nSegmentIndex < m_pathSegments.size()) {
                    OnClickedSubPath(m_pathSegments[nSegmentIndex].m_fullPath);
                }
                return true;
            }
            return false;
            }, 0);

        Button* pArrowBtn = new Button(GetWindow());
        pBox->AddItem(pArrowBtn);
        pArrowBtn->SetClass(GetPathSeparatorClass());
        pArrowBtn->SetNoFocus();
        pArrowBtn->SetMouseEnabled(false);

        m_subPathBoxes.push_back(pBox);
    }
}

void AddressBar::MoveSubPathFocus(size_t nIndex, bool bLeft)
{
    if (nIndex >= m_subPathBoxes.size()) {
        return;
    }
    size_t nTarget = Box::InvalidIndex;
    if (bLeft) {
        for (size_t i = nIndex; i > 0; --i) {
            if (m_subPathBoxes[i - 1]->IsVisible()) {
                nTarget = i - 1;
                break;
            }
        }
    }
    else {
        for (size_t i = nIndex + 1; i < m_subPathBoxes.size(); ++i) {
            if (m_subPathBoxes[i]->IsVisible()) {
                nTarget = i;
                break;
            }
        }
    }
    if (nTarget == Box::InvalidIndex) {
        return;
    }
    HBox* pTargetBox = m_subPathBoxes[nTarget];
    Control* pBtn = pTargetBox->GetItemAt(0);
    if (pBtn != nullptr) {
        pBtn->SetFocus();
    }
}

void AddressBar::FocusSubPathEdge(bool bFirst)
{
    size_t nTarget = Box::InvalidIndex;
    if (bFirst) {
        for (size_t i = 0; i < m_subPathBoxes.size(); ++i) {
            if (m_subPathBoxes[i]->IsVisible()) {
                nTarget = i;
                break;
            }
        }
    }
    else {
        for (size_t i = m_subPathBoxes.size(); i > 0; --i) {
            if (m_subPathBoxes[i - 1]->IsVisible()) {
                nTarget = i - 1;
                break;
            }
        }
    }
    if (nTarget == Box::InvalidIndex) {
        return;
    }
    Control* pBtn = m_subPathBoxes[nTarget]->GetItemAt(0);
    if (pBtn != nullptr) {
        pBtn->SetFocus();
    }
}

bool AddressBar::NavigateToParentPath()
{
    if (m_pathSegments.size() < 2) {
        //已经是根路径
        return false;
    }
    const DString parentPath = m_pathSegments[m_pathSegments.size() - 2].m_fullPath;
    if (parentPath.empty()) {
        return false;
    }
    if (IsEditing()) {
        ShowAddressEdit(false);
    }
    OnClickedSubPath(parentPath);
    return true;
}

bool AddressBar::HandleNavigationKeyDown(const EventArgs& msg)
{
    const bool bCtrl = (msg.modifierKey & kControl) != 0;
    const bool bAlt = (msg.modifierKey & kAlt) != 0;

    //Alt+Up：导航到上一级目录
    if ((msg.vkCode == kVK_UP) && bAlt && !bCtrl) {
        return NavigateToParentPath();
    }
    //F6 / Ctrl+L：进入编辑状态并全选当前路径（与资源管理器的行为一致）
    if ((msg.vkCode == kVK_F6) || ((msg.vkCode == kVK_L) && bCtrl && !bAlt)) {
        ShowAddressEdit(true);
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetSel(0, -1);
            m_pRichEdit->SetFocus();
        }
        return true;
    }
    return false;
}

void AddressBar::UpdateSubPathControls(size_t nIndex, const PathSegment& segment, bool bShowSeparator, bool bCurrent)
{
    ASSERT(nIndex < m_subPathBoxes.size());
    if (nIndex >= m_subPathBoxes.size()) {
        return;
    }
    HBox* pBox = m_subPathBoxes[nIndex];
    Button* pDisplayNameBtn = dynamic_cast<Button*>(pBox->GetItemAt(0));
    Button* pArrowBtn = dynamic_cast<Button*>(pBox->GetItemAt(1));
    if (pDisplayNameBtn == nullptr) {
        return;
    }

    pDisplayNameBtn->SetClass(GetSubPathBtnClass());
    if (segment.m_bComputer) {
        pDisplayNameBtn->SetClass(_T("address_bar_sub_path_computer"));
    }
    else if (segment.m_bRoot) {
        if (segment.m_bDrive) {
            pDisplayNameBtn->SetClass(GetSubPathDriveClass());
        }
        else {
            pDisplayNameBtn->SetClass(GetSubPathRootClass());
        }
    }
    //当前段使用强调样式；“此电脑”/根节点/盘符保留其图标样式，避免图标丢失
    if (bCurrent && !segment.m_bComputer && !segment.m_bRoot) {
        pDisplayNameBtn->SetClass(GetSubPathCurrentClass());
    }
    pDisplayNameBtn->SetText(segment.m_displayName);
    if (m_bEnableTooltip) {
        pDisplayNameBtn->SetToolTipText(segment.m_fullPath);
    }
    else {
        pDisplayNameBtn->SetToolTipText(DString());
    }
    if (pArrowBtn != nullptr) {
        pArrowBtn->SetVisible(bShowSeparator);
    }
}

namespace
{
/** 是否为目录分隔符（Windows下同时接受'/'与'\\'）
*/
bool IsDirSeparator(DString::value_type ch)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return (ch == _T('\\')) || (ch == _T('/'));
#else
    return ch == _T('/');
#endif
}

/** 判断字符串是否以指定前缀开头（不区分大小写）
*/
bool StartsWithNoCase(const DString& text, const DString& prefix)
{
    if (text.size() < prefix.size()) {
        return false;
    }
    const DString head(text.c_str(), prefix.size());
    return StringUtil::MakeUpperString(head) == StringUtil::MakeUpperString(prefix);
}

/** 按目录分隔符拆分路径组件（跳过空组件）
*/
std::vector<DString> SplitPathComponents(const DString& path, size_t nStartIndex)
{
    std::vector<DString> parts;
    size_t i = nStartIndex;
    while (i < path.size()) {
        while ((i < path.size()) && IsDirSeparator(path[i])) {
            ++i;
        }
        if (i >= path.size()) {
            break;
        }
        size_t j = i;
        while ((j < path.size()) && !IsDirSeparator(path[j])) {
            ++j;
        }
        parts.push_back(path.substr(i, j - i));
        i = j;
    }
    return parts;
}
}

DString AddressBar::ExpandUserPath(const DString& path) const
{
    if (path.empty() || (path[0] != _T('~'))) {
        return path;
    }
    //仅处理"~"或者"~/..."、"~\\..."形式，避免把"~abc"这类普通文件名当作主目录
    if ((path.size() > 1) && !IsDirSeparator(path[1])) {
        return path;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    DStringW home;
    const DWORD nLength = ::GetEnvironmentVariableW(L"USERPROFILE", nullptr, 0);
    if (nLength > 0) {
        home.resize(nLength + 1);
        const DWORD nCopied = ::GetEnvironmentVariableW(L"USERPROFILE", &home[0], nLength + 1);
        if (nCopied > 0) {
            home.resize(nCopied);
            return StringConvert::WStringToT(home) + path.substr(1);
        }
    }
    return path;
#else
    const char* home = std::getenv("HOME");
    if (home != nullptr) {
        return DString(home) + path.substr(1);
    }
    return path;
#endif
}

void AddressBar::BuildPathSegments(const DString& addressPath, std::vector<PathSegment>& segments) const
{
    segments.clear();
    if (addressPath.empty()) {
        DString displayText = GetEmptyDisplayText();
        if (!displayText.empty()) {
            PathSegment segment;
            segment.m_displayName = displayText;
            segment.m_bRoot = false;
            segment.m_bDrive = false;
            segment.m_bComputer = true;
            segments.push_back(segment);
        }
        return;
    }

    //展开以"~"开头的用户主目录路径
    DString pathText = ExpandUserPath(addressPath);
    FilePath filePath(pathText);
    filePath.NormalizeFilePath();
    filePath.TrimRightPathSeparator();
    pathText = filePath.ToString();

#ifdef DUILIB_BUILD_FOR_WIN
    //盘符路径（C:、C:\\...）与UNC路径（\\\\server\\...）均视为绝对路径
    const bool bAbsolutePath = ((pathText.size() >= 2) && (pathText[1] == _T(':'))) ||
                               ((pathText.size() >= 2) && IsDirSeparator(pathText[0]) && IsDirSeparator(pathText[1]));
#else
    const bool bAbsolutePath = !pathText.empty() && (pathText[0] == _T('/'));
#endif
    if (!bAbsolutePath) {
        //相对路径：仅按分隔符拆分为普通路径段，不生成根段
        DString fullPath;
        for (const DString& part : SplitPathComponents(pathText, 0)) {
            if (fullPath.empty()) {
                fullPath = part;
            }
            else {
                fullPath += FilePath::GetPathSeparator();
                fullPath += part;
            }
            PathSegment segment;
            segment.m_fullPath = fullPath;
            segment.m_displayName = part;
            segment.m_bRoot = false;
            segment.m_bDrive = false;
            segment.m_bComputer = false;
            segments.push_back(segment);
        }
        if (segments.empty()) {
            PathSegment segment;
            segment.m_fullPath = pathText;
            segment.m_displayName = pathText;
            segment.m_bRoot = false;
            segment.m_bDrive = false;
            segment.m_bComputer = false;
            segments.push_back(segment);
        }
        return;
    }

#ifdef DUILIB_BUILD_FOR_WIN
    //解析根路径：支持盘符、UNC(\\\\server\\share)、扩展长度路径(\\\\?\\、\\\\?\\UNC\\)
    DString rootPath;
    DString rootDisplay;
    size_t nRootEnd = 0;
    bool bDriveRoot = false;

    if (StartsWithNoCase(pathText, _T("\\\\?\\UNC\\"))) {
        //\\\\?\\UNC\\server\\share\\...
        const size_t nPrefixLen = 8;
        size_t nServerEnd = pathText.find_first_of(_T("\\/"), nPrefixLen);
        if (nServerEnd == DString::npos) {
            nRootEnd = pathText.size();
            rootPath = pathText;
            rootDisplay = pathText.substr(nPrefixLen);
        }
        else {
            size_t nShareEnd = pathText.find_first_of(_T("\\/"), nServerEnd + 1);
            if (nShareEnd == DString::npos) {
                nShareEnd = pathText.size();
            }
            nRootEnd = nShareEnd;
            rootPath = pathText.substr(0, nShareEnd);
            //显示为普通的UNC根：\\\\server\\share
            rootDisplay = DString(_T("\\\\")) + pathText.substr(nPrefixLen, nShareEnd - nPrefixLen);
        }
    }
    else if (StartsWithNoCase(pathText, _T("\\\\?\\"))) {
        //\\\\?\\C:\\... 或 \\\\?\\Volume{GUID}\\...
        const size_t nPrefixLen = 4;
        size_t nNameEnd = pathText.find_first_of(_T("\\/"), nPrefixLen);
        if (nNameEnd == DString::npos) {
            nNameEnd = pathText.size();
        }
        nRootEnd = nNameEnd;
        rootPath = pathText.substr(0, nNameEnd);
        rootDisplay = pathText.substr(nPrefixLen, nNameEnd - nPrefixLen);
        if ((rootDisplay.size() == 2) && (rootDisplay[1] == _T(':'))) {
            bDriveRoot = true;
        }
    }
    else if (IsDirSeparator(pathText[0])) {
        //UNC: \\\\server\\share\\...
        size_t nServerEnd = pathText.find_first_of(_T("\\/"), 2);
        if (nServerEnd == DString::npos) {
            nRootEnd = pathText.size();
            rootPath = pathText;
            rootDisplay = pathText;
        }
        else {
            size_t nShareEnd = pathText.find_first_of(_T("\\/"), nServerEnd + 1);
            if (nShareEnd == DString::npos) {
                nShareEnd = pathText.size();
            }
            nRootEnd = nShareEnd;
            rootPath = pathText.substr(0, nShareEnd);
            rootDisplay = rootPath;
        }
    }
    else {
        //盘符根：C:\\...
        nRootEnd = 2;
        rootPath = pathText.substr(0, 2);
        rootDisplay = rootPath;
        bDriveRoot = true;
    }

    //根段
    PathSegment rootSegment;
    rootSegment.m_fullPath = rootPath;
    if (!rootPath.empty() && !IsDirSeparator(rootPath.back())) {
        rootSegment.m_fullPath += FilePath::GetPathSeparator();
    }
    if (bDriveRoot) {
        //查询卷名（\\\\?\\ 前缀的盘符使用普通路径查询）
        DString diskPath = rootPath;
        if (StartsWithNoCase(diskPath, _T("\\\\?\\"))) {
            diskPath = diskPath.substr(4);
        }
        if ((diskPath.size() == 2) && (diskPath[1] == _T(':'))) {
            diskPath += FilePath::GetPathSeparator();
        }
        DiskUtils::DiskInfo diskInfo;
        DiskUtils::GetLogicalDriveInfo(diskPath, diskInfo);
        if (!diskInfo.m_volumeName.empty()) {
            //卷名通常已包含盘符（如“本地磁盘 (C:)”），优先显示卷名
            rootDisplay = diskInfo.m_volumeName;
        }
    }
    rootSegment.m_displayName = rootDisplay;
    rootSegment.m_bRoot = true;
    rootSegment.m_bDrive = bDriveRoot;
    rootSegment.m_bComputer = false;
    segments.push_back(rootSegment);

    //根路径下的各级子路径
    DString currentPath = rootSegment.m_fullPath;
    for (const DString& part : SplitPathComponents(pathText, nRootEnd)) {
        currentPath += part;
        PathSegment segment;
        segment.m_fullPath = currentPath;
        segment.m_displayName = part;
        segment.m_bRoot = false;
        segment.m_bDrive = false;
        segment.m_bComputer = false;
        segments.push_back(segment);
        currentPath += FilePath::GetPathSeparator();
    }
#else
    //Unix：根为"/"
    PathSegment rootSegment;
    rootSegment.m_fullPath = _T("/");
    rootSegment.m_displayName = _T("/");
    rootSegment.m_bRoot = true;
    rootSegment.m_bDrive = false;
    rootSegment.m_bComputer = false;
    segments.push_back(rootSegment);

    DString currentPath = _T("/");
    for (const DString& part : SplitPathComponents(pathText, 0)) {
        currentPath += part;
        PathSegment segment;
        segment.m_fullPath = currentPath;
        segment.m_displayName = part;
        segment.m_bRoot = false;
        segment.m_bDrive = false;
        segment.m_bComputer = false;
        segments.push_back(segment);
        currentPath += FilePath::GetPathSeparator();
    }
#endif
}

bool AddressBar::UpdateAddressBarControls(const DString& addressPath, bool bForce)
{
    bool bChanged = (m_showAddressPath != addressPath);
    if (!bChanged && !bForce) {
        return false;
    }
    m_prevShowAddressPath = m_showAddressPath;
    m_showAddressPath = addressPath;
    m_addressPath = addressPath;
    m_clickedAddressPath.clear();

    std::vector<PathSegment> segments;
    BuildPathSegments(addressPath, segments);
    m_pathSegments.swap(segments);
    //路径段内容变化，使布局缓存失效
    ++m_nPathVersion;

    if (m_pBarBox != nullptr) {
        for (size_t i = 0; i < m_pathSegments.size(); ++i) {
            EnsureSubPathControls(i);
        }
    }
    UpdateAddressBarControlsStatus();
    return bChanged;
}

void AddressBar::UpdateAddressBarControlsStatus()
{
    if ((m_pBarBox == nullptr) || (m_pRichEdit == nullptr)) {
        return;
    }
    UiRect rc = GetPos();
    rc.Deflate(GetPadding());
    int32_t nBarWidth = rc.Width();
    UiPadding rcPadding = m_pBarBox->GetPadding();
    nBarWidth -= (rcPadding.left + rcPadding.right);

    //路径与可用宽高均未变化时，无需重新测量所有路径段（拖动窗口时的常见场景）
    if ((m_nLayoutWidth == nBarWidth) && (m_nLayoutHeight == rc.Height()) &&
        (m_nLayoutPathVersion == m_nPathVersion)) {
        return;
    }

    //调整可见性或最大宽度后，需要重新排列容器内的子控件，否则宽度变化时旧位置会残留
    auto reArrangeSubPaths = [this]() {
        if (m_pBarBox != nullptr) {
            m_pBarBox->SetPos(m_pBarBox->GetPos());
        }
    };

    const size_t nCount = m_subPathBoxes.size();
    if ((nCount == 0) || (m_pathSegments.empty())) {
        for (HBox* pBox : m_subPathBoxes) {
            pBox->SetVisible(false);
        }
        if (m_pCollapseBtn != nullptr) {
            m_pCollapseBtn->SetVisible(false);
        }
        reArrangeSubPaths();
        m_nLayoutWidth = nBarWidth;
        m_nLayoutHeight = rc.Height();
        m_nLayoutPathVersion = m_nPathVersion;
        return;
    }

    //先恢复所有控件的原始宽度，再测量
    for (size_t i = 0; i < nCount; ++i) {
        HBox* pBox = m_subPathBoxes[i];
        Button* pNameBtn = dynamic_cast<Button*>(pBox->GetItemAt(0));
        if (pNameBtn != nullptr) {
            pNameBtn->SetMaxWidth(INT32_MAX, false);
        }
        pBox->SetVisible(i < m_pathSegments.size());
    }
    if (m_pCollapseBtn != nullptr) {
        m_pCollapseBtn->SetVisible(false);
    }

    std::vector<int32_t> widths;
    widths.resize(m_pathSegments.size(), 0);
    const int32_t nLastIndex = (int32_t)m_pathSegments.size() - 1;
    int32_t nTotalWidth = 0;
    const UiSize szAvailable(rc.Width(), rc.Height());
    for (size_t i = 0; i < m_pathSegments.size(); ++i) {
        HBox* pBox = m_subPathBoxes[i];
        Button* pNameBtn = dynamic_cast<Button*>(pBox->GetItemAt(0));
        Button* pSepBtn = dynamic_cast<Button*>(pBox->GetItemAt(1));
        UiPadding boxPadding = pBox->GetPadding();
        UiMargin boxMargin = pBox->GetMargin();
        int32_t nWidth = 0;
        if (pNameBtn != nullptr) {
            nWidth += pNameBtn->EstimateSize(szAvailable).cx.GetInt32();
            UiMargin nameMargin = pNameBtn->GetMargin();
            nWidth += (nameMargin.left + nameMargin.right);
        }
        nWidth += (boxPadding.left + boxPadding.right);
        nWidth += (boxMargin.left + boxMargin.right);
        if ((pSepBtn != nullptr) && (i < (size_t)nLastIndex)) {
            nWidth += pSepBtn->EstimateSize(szAvailable).cx.GetInt32();
            UiMargin sepMargin = pSepBtn->GetMargin();
            nWidth += (sepMargin.left + sepMargin.right);
        }
        widths[i] = nWidth;
        nTotalWidth += nWidth;
    }

    if ((nBarWidth <= 0) || (nTotalWidth <= nBarWidth)) {
        //宽度足够：全部显示
        for (size_t i = 0; i < nCount; ++i) {
            m_subPathBoxes[i]->SetVisible(i < m_pathSegments.size());
        }
        for (size_t i = 0; i < m_pathSegments.size(); ++i) {
            UpdateSubPathControls(i, m_pathSegments[i], i < (size_t)nLastIndex, i == (size_t)nLastIndex);
            Button* pNameBtn = dynamic_cast<Button*>(m_subPathBoxes[i]->GetItemAt(0));
            if (pNameBtn != nullptr) {
                pNameBtn->SetMaxWidth(INT32_MAX, false);
            }
        }
        if (m_pCollapseBtn != nullptr) {
            m_pCollapseBtn->SetVisible(false);
        }
        m_collapsedSegments.clear();
        reArrangeSubPaths();
        m_nLayoutWidth = nBarWidth;
        m_nLayoutHeight = rc.Height();
        m_nLayoutPathVersion = m_nPathVersion;
        return;
    }

    //宽度不足：按优先级决定各段的可见性
    //  1) 当前段（最后一段）优先完整显示；
    //  2) 其次保证"…"折叠入口可见，否则被折叠的父目录无从访问；
    //  3) 剩余宽度从末尾向前依次显示非当前段（越靠近当前段越优先）；
    //  4) 根段优先级最低，放不下时与中间段一起折叠进"…"菜单。
    std::vector<bool> showFlags(m_pathSegments.size(), false);
    showFlags[nLastIndex] = true;

    Button* pCollapseBtn = m_pCollapseBtn;
    int32_t nCollapseWidth = 0;
    if (pCollapseBtn != nullptr) {
        nCollapseWidth = pCollapseBtn->EstimateSize(szAvailable).cx.GetInt32();
        UiMargin collapseMargin = pCollapseBtn->GetMargin();
        nCollapseWidth += (collapseMargin.left + collapseMargin.right);
    }
    const int32_t nMinLastWidth = Dpi().GetScaleInt(28);//当前段的最小可用宽度（DIP）

    bool bShowCollapse = false;
    int32_t nLastWidth = widths[nLastIndex];
    int32_t nRemain = nBarWidth - nLastWidth;
    if (nRemain >= 0) {
        //当前段可以完整显示：统计其余各段的总宽度
        int32_t nOthersWidth = 0;
        for (int32_t i = 0; i < nLastIndex; ++i) {
            nOthersWidth += widths[i];
        }
        if (nOthersWidth <= nRemain) {
            //其余各段也能全部显示（正常情况下已被上面的分支覆盖，这里做兜底）
            for (int32_t i = 0; i < nLastIndex; ++i) {
                showFlags[i] = true;
            }
        }
        else if (nRemain >= nCollapseWidth) {
            //需要"…"：预留其宽度后，从末尾向前尽量多显示非当前段
            bShowCollapse = true;
            int32_t nAvailable = nRemain - nCollapseWidth;
            for (int32_t i = nLastIndex - 1; i >= 0; --i) {
                if (nAvailable >= widths[i]) {
                    showFlags[i] = true;
                    nAvailable -= widths[i];
                }
                else {
                    break;
                }
            }
        }
        else if (nBarWidth >= nCollapseWidth + nMinLastWidth) {
            //当前段完整显示时放不下"…"：收缩当前段，为折叠入口让出空间（仍能显示部分目录名）
            bShowCollapse = true;
            nLastWidth = nBarWidth - nCollapseWidth;
        }
        //否则：连"…"都放不下，只显示当前段
    }
    else if ((nLastIndex > 0) && (nBarWidth >= nCollapseWidth + nMinLastWidth)) {
        //当前段本身就超出地址栏宽度：收缩当前段，保留"…"折叠入口
        bShowCollapse = true;
        nLastWidth = nBarWidth - nCollapseWidth;
    }
    else {
        //只显示当前段，占满可用宽度
        nLastWidth = nBarWidth;
    }

    m_collapsedSegments.clear();
    for (size_t i = 0; i < m_pathSegments.size(); ++i) {
        HBox* pBox = m_subPathBoxes[i];
        bool bCurrent = (i == (size_t)nLastIndex);
        pBox->SetVisible(showFlags[i]);
        UpdateSubPathControls(i, m_pathSegments[i], !bCurrent, bCurrent);
        Button* pNameBtn = dynamic_cast<Button*>(pBox->GetItemAt(0));
        if (pNameBtn != nullptr) {
            if (bCurrent) {
                //当前段占用的最大宽度需扣除容器内边距与外边距，超出部分由文本省略号截断
                UiPadding boxPadding = pBox->GetPadding();
                UiMargin boxMargin = pBox->GetMargin();
                UiMargin nameMargin = pNameBtn->GetMargin();
                int32_t nMaxWidth = nLastWidth - (boxPadding.left + boxPadding.right)
                                  - (boxMargin.left + boxMargin.right)
                                  - (nameMargin.left + nameMargin.right);
                pNameBtn->SetMaxWidth(std::max(nMaxWidth, 0), false);
            }
            else {
                pNameBtn->SetMaxWidth(INT32_MAX, false);
            }
        }
        if (!showFlags[i] && !bCurrent) {
            m_collapsedSegments.push_back(m_pathSegments[i]);
        }
    }

    if (pCollapseBtn != nullptr) {
        if (bShowCollapse) {
            //把折叠按钮放到第一个显示的非当前段（或当前段）之前
            size_t nInsertIndex = (size_t)nLastIndex;
            for (size_t i = 1; i < (size_t)nLastIndex; ++i) {
                if (showFlags[i]) {
                    nInsertIndex = i;
                    break;
                }
            }
            pCollapseBtn->SetVisible(true);
            m_pBarBox->SetItemIndex(pCollapseBtn, nInsertIndex);
        }
        else {
            pCollapseBtn->SetVisible(false);
        }
    }
    reArrangeSubPaths();
    m_nLayoutWidth = nBarWidth;
    m_nLayoutHeight = rc.Height();
    m_nLayoutPathVersion = m_nPathVersion;
}

void AddressBar::OnClickedSubPath(const DString& filePath)
{
    m_clickedAddressPath = filePath;
    if (!m_clickedAddressPath.empty()) {
        SendEvent(kEventPathClick);
    }
}

DString AddressBar::GetClickedAddressPath() const
{
    return m_clickedAddressPath.c_str();
}

void AddressBar::OnAddressBarSetFocus(Control* /*pNewFocus*/)
{
    SendEvent(kEventSetFocus);
}

bool AddressBar::IsChildControl(const Control* pControl) const
{
    const Control* p = pControl;
    while (p != nullptr) {
        if (p == this) {
            return true;
        }
        p = p->GetParent();
    }
    return false;
}

void AddressBar::OnAddressBarKillFocus(Control* pNewFocus)
{
    if (IsChildControl(pNewFocus)) {
        //焦点还在AddressBar内部（如编辑框、复位按钮、面包屑按钮之间切换），不提交
        return;
    }
    bool bAccepted = false;
    bool bChanged = false;
    if (IsEditing()) {
        DString addressPath = m_pRichEdit->GetText();
        if (m_bKillFocusUpdateUI) {
            bAccepted = CommitAddressPath(addressPath, false);
        }
        else {
            ResetEditText();
            ShowAddressEdit(false);
        }
    }
    else {
        ShowAddressEdit(false);
    }
    bChanged = bAccepted && (GetPreviousAddressPath() != GetAddressPath());
    SendEvent(kEventKillFocus, (WPARAM)pNewFocus);
    if (bChanged) {
        SendEvent(kEventPathChanged);
    }
}

void AddressBar::OnAddressBarReturn()
{
    DString addressPath;
    if (m_pRichEdit != nullptr) {
        addressPath = m_pRichEdit->GetText();
    }
    bool bAccepted = false;
    if (m_bReturnUpdateUI) {
        bAccepted = CommitAddressPath(addressPath, true);
    }
    SendEvent(kEventReturn);
    if (bAccepted && (GetPreviousAddressPath() != GetAddressPath())) {
        SendEvent(kEventPathChanged);
    }
}

void AddressBar::OnAddressBarEsc()
{
    if (m_bEscUpdateUI) {
        m_bUpdatingUI = true;
        ShowAddressEdit(false);
        m_bUpdatingUI = false;
    }
    SendEvent(kEventEsc);
}

bool AddressBar::CommitAddressPath(const DString& rawPath, bool bHideEdit)
{
    if (m_bUpdatingUI) {
        return false;
    }
    DString addressPath = rawPath;
    StringUtil::Trim(addressPath);
    //展开"~"为用户主目录（业务层校验与后续显示都使用展开后的路径）
    addressPath = ExpandUserPath(addressPath);

    //预提交：允许业务层校验/规范化路径（例如把文件路径转换为父目录）
    if ((m_pathChangingCallback != nullptr) && !addressPath.empty()) {
        DString newPath = addressPath;
        bool bAccept = m_pathChangingCallback(newPath);
        if (!bAccept) {
            //拒绝提交：显示控件级错误状态（红色边框），恢复编辑框并退出编辑状态，不派发kEventPathChanged
            SetPathError(true);
            ResetEditText();
            if (bHideEdit) {
                ShowAddressEdit(false);
            }
            return false;
        }
        StringUtil::Trim(newPath);
        addressPath = newPath;
    }

    m_bUpdatingUI = true;
    if (m_pBarBox != nullptr) {
        UpdateAddressBarControls(addressPath, true);
    }
    m_addressPath = addressPath;
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetTextNoEvent(GetAddressPath());
    }
    if (bHideEdit) {
        ShowAddressEdit(false);
    }
    if (!addressPath.empty()) {
        AddPathHistory(addressPath);
    }
    //提交成功：清除错误状态
    SetPathError(false);
    m_bUpdatingUI = false;
    return true;
}

void AddressBar::SetAddressPath(const DString& addressPath)
{
    //展开"~"为用户主目录，保持显示路径与实际路径一致
    const DString expandedPath = ExpandUserPath(addressPath);
    m_addressPath = expandedPath;
    if ((m_pRichEdit != nullptr) && !IsEditing()) {
        //编辑状态下不覆盖用户输入
        m_pRichEdit->SetTextNoEvent(expandedPath);
    }
    if (m_pBarBox != nullptr) {
        bool bChanged = UpdateAddressBarControls(expandedPath, true);
        if (bChanged) {
            AddPathHistory(expandedPath);
        }
    }
}

DString AddressBar::GetAddressPath() const
{
    return m_addressPath.c_str();
}

DString AddressBar::GetPreviousAddressPath() const
{
    return m_prevShowAddressPath.c_str();
}

void AddressBar::ShowCollapsedPathMenu()
{
    if (m_collapsedSegments.empty() || (m_pCollapseBtn == nullptr) || (GetWindow() == nullptr)) {
        return;
    }
    UiRect rcCollapse = m_pCollapseBtn->GetPos();
    GetWindow()->ClientToScreen(rcCollapse);

    Menu* pMenu = new Menu(GetWindow(), m_pCollapseBtn);
    pMenu->SetSkinFolder(DString(DUILIB_PUBLIC_RES_DIR) + DString(_T("/address_bar/")));
    std::vector<PathSegment> collapsed = m_collapsedSegments;
    pMenu->AttachMenuItemActivated([this, collapsed](const DString& /*menuName*/, int32_t /*nMenuLevel*/,
                                                     const DString& /*itemName*/, size_t nItemIndex) {
        if (nItemIndex < collapsed.size()) {
            OnClickedSubPath(collapsed[nItemIndex].m_fullPath);
        }
    });
    pMenu->ShowMenu(_T("address_bar_menu.xml"),
                    UiPoint(rcCollapse.left, rcCollapse.bottom),
                    MenuPopupPosType::LEFT_TOP_N);
    std::vector<MenuItem*> menuItems;
    int32_t nMaxItemWidth = 0;
    //菜单项的最小宽度（DIP，按DPI换算为布局使用的尺寸）
    const int32_t nMinItemWidth = Dpi().GetScaleInt(120);
    for (const PathSegment& segment : collapsed) {
        MenuItem* pMenuItem = new MenuItem(GetWindow());
        pMenuItem->SetClass(_T("menu_element"));

        //菜单项内的文本必须由Label控件显示（直接SetText时，菜单项没有子控件，宽度会为0）
        Label* pLabel = new Label(GetWindow());
        pLabel->SetClass(_T("menu_text"));
        pLabel->SetText(segment.m_displayName);
        pLabel->SetMouseEnabled(false);
        pLabel->SetKeyboardEnabled(false);
        pMenuItem->AddItem(pLabel);

        //按文本实际宽度计算菜单项宽度，避免固定宽度导致较长的目录名显示不完整
        //（"menu_element"的宽度是auto，动态创建时无法计算出宽度，会导致菜单宽度为0，所以必须显式设置）
        //EstimateText返回的是布局使用的尺寸（已按DPI缩放），因此设置时无需再做DPI缩放
        const UiSize szText = pLabel->EstimateText(UiSize(INT32_MAX, INT32_MAX));
        const UiPadding rcItemPadding = pMenuItem->GetControlPadding();
        int32_t nItemWidth = szText.cx + rcItemPadding.left + rcItemPadding.right;
        nItemWidth = std::max(nItemWidth, nMinItemWidth);
        pMenuItem->SetFixedWidth(UiFixedInt(nItemWidth), true, false);
        nMaxItemWidth = std::max(nMaxItemWidth, nItemWidth);

        menuItems.push_back(pMenuItem);
        pMenu->AddMenuItem(pMenuItem);
    }
    //所有菜单项统一使用最大宽度，保证各菜单项宽度一致、文字对齐
    for (MenuItem* pMenuItem : menuItems) {
        pMenuItem->SetFixedWidth(UiFixedInt(nMaxItemWidth), true, false);
    }
    //菜单项是显示后动态添加的，需要重新计算菜单大小与位置
    pMenu->RefreshMenuLayout();
}

void AddressBar::ShowPathHistoryMenu()
{
    if (!m_bHistoryEnabled || m_pathHistory.empty() || (GetWindow() == nullptr)) {
        return;
    }
    UiRect rcBtn(0, 0, 0, 0);
    if (m_pHistoryBtn != nullptr) {
        rcBtn = m_pHistoryBtn->GetPos();
        GetWindow()->ClientToScreen(rcBtn);
    }

    Menu* pMenu = new Menu(GetWindow(), m_pHistoryBtn);
    pMenu->SetSkinFolder(DString(DUILIB_PUBLIC_RES_DIR) + DString(_T("/address_bar/")));
    std::vector<UiString> history = m_pathHistory;
    pMenu->AttachMenuItemActivated([this, history](const DString& /*menuName*/, int32_t /*nMenuLevel*/,
                                                   const DString& /*itemName*/, size_t nItemIndex) {
        if (nItemIndex < history.size()) {
            OnClickedSubPath(history[nItemIndex].c_str());
        }
    });
    pMenu->ShowMenu(_T("address_bar_menu.xml"),
                    UiPoint(rcBtn.left, rcBtn.bottom),
                    MenuPopupPosType::RIGHT_TOP);

    //最多显示最近使用的10条历史路径
    const int32_t nMaxCount = 10;
    const int32_t nMinItemWidth = Dpi().GetScaleInt(120);
    int32_t nMaxItemWidth = 0;
    std::vector<MenuItem*> menuItems;
    for (size_t i = 0; (i < m_pathHistory.size()) && ((int32_t)i < nMaxCount); ++i) {
        MenuItem* pMenuItem = new MenuItem(GetWindow());
        pMenuItem->SetClass(_T("menu_element"));
        Label* pLabel = new Label(GetWindow());
        pLabel->SetClass(_T("menu_text"));
        pLabel->SetText(m_pathHistory[i].c_str());
        pLabel->SetMouseEnabled(false);
        pLabel->SetKeyboardEnabled(false);
        pMenuItem->AddItem(pLabel);

        const UiSize szText = pLabel->EstimateText(UiSize(INT32_MAX, INT32_MAX));
        const UiPadding rcItemPadding = pMenuItem->GetControlPadding();
        int32_t nItemWidth = std::max(szText.cx + rcItemPadding.left + rcItemPadding.right, nMinItemWidth);
        pMenuItem->SetFixedWidth(UiFixedInt(nItemWidth), true, false);
        nMaxItemWidth = std::max(nMaxItemWidth, nItemWidth);

        menuItems.push_back(pMenuItem);
        pMenu->AddMenuItem(pMenuItem);
    }
    for (MenuItem* pMenuItem : menuItems) {
        pMenuItem->SetFixedWidth(UiFixedInt(nMaxItemWidth), true, false);
    }
    pMenu->RefreshMenuLayout();
}

void AddressBar::ShowDefaultContextMenu(const UiPoint& ptMouse)
{
    if (GetWindow() == nullptr) {
        return;
    }
    UiPoint ptScreen = ptMouse;
    GetWindow()->ClientToScreen(ptScreen);

    DString clickedPath = m_clickedAddressPath.c_str();
    if (clickedPath.empty()) {
        clickedPath = GetAddressPath();
    }

    Menu* pMenu = new Menu(GetWindow(), this);
    pMenu->SetSkinFolder(DString(DUILIB_PUBLIC_RES_DIR) + DString(_T("/address_bar/")));
    pMenu->AttachMenuItemActivated([this, clickedPath](const DString& /*menuName*/, int32_t /*nMenuLevel*/,
                                                       const DString& /*itemName*/, size_t nItemIndex) {
        if (nItemIndex == 0) {
            //复制完整路径
            Clipboard::SetClipboardText(StringConvert::TToUTF8(clickedPath));
        }
        else if (nItemIndex == 1) {
            //在文件资源管理器中显示（Windows平台）
#ifdef DUILIB_BUILD_FOR_WIN
            DString params = _T("/select,\"");
            params += clickedPath;
            params += _T("\"");
            ::ShellExecuteW(nullptr, _T("open"), _T("explorer.exe"), params.c_str(), nullptr, SW_SHOWNORMAL);
#endif
        }
        else if (nItemIndex == 2) {
            //打开上级目录
            FilePath parentPath = FilePath(clickedPath).GetParentPath();
            if (!parentPath.IsEmpty() && parentPath.IsExistsDirectory()) {
                OnClickedSubPath(parentPath.ToString());
            }
        }
    });
    pMenu->ShowMenu(_T("address_bar_menu.xml"), ptScreen, MenuPopupPosType::LEFT_TOP_N);

    const int32_t nMinItemWidth = Dpi().GetScaleInt(120);
    const DString itemTexts[] = { _T("STRID_ADDRESS_BAR_COPY_PATH"),
                                  _T("STRID_ADDRESS_BAR_REVEAL"),
                                  _T("STRID_ADDRESS_BAR_OPEN_PARENT") };
    int32_t nMaxItemWidth = 0;
    std::vector<MenuItem*> menuItems;
    for (const DString& textId : itemTexts) {
        MenuItem* pMenuItem = new MenuItem(GetWindow());
        pMenuItem->SetClass(_T("menu_element"));
        Label* pLabel = new Label(GetWindow());
        pLabel->SetClass(_T("menu_text"));
        pLabel->SetTextId(textId);
        pLabel->SetMouseEnabled(false);
        pLabel->SetKeyboardEnabled(false);
        pMenuItem->AddItem(pLabel);

        const UiSize szText = pLabel->EstimateText(UiSize(INT32_MAX, INT32_MAX));
        const UiPadding rcItemPadding = pMenuItem->GetControlPadding();
        int32_t nItemWidth = std::max(szText.cx + rcItemPadding.left + rcItemPadding.right, nMinItemWidth);
        pMenuItem->SetFixedWidth(UiFixedInt(nItemWidth), true, false);
        nMaxItemWidth = std::max(nMaxItemWidth, nItemWidth);

        menuItems.push_back(pMenuItem);
        pMenu->AddMenuItem(pMenuItem);
    }
    for (MenuItem* pMenuItem : menuItems) {
        pMenuItem->SetFixedWidth(UiFixedInt(nMaxItemWidth), true, false);
    }
    pMenu->RefreshMenuLayout();
}

bool AddressBar::HandleEditKeyDown(const EventArgs& msg)
{
    //编辑态下同样支持 Alt+Up 上一级、F6/Ctrl+L 全选（Home/End 留给编辑框移动光标）
    if (HandleNavigationKeyDown(msg)) {
        return true;
    }
    if (msg.vkCode == kVK_TAB) {
        //Tab补全：浮层打开时使用当前选中项（未导航时默认第一项），否则按当前输入重新生成候选并补全第一项
        size_t nIndex = Box::InvalidIndex;
        if ((m_pFlyout != nullptr) && m_pFlyout->IsOpen()) {
            nIndex = m_pFlyout->GetSelectedIndex();
        }
        else {
            const DString editText = (m_pRichEdit != nullptr) ? m_pRichEdit->GetText() : DString();
            BuildAutoCompleteCandidates(editText);
            nIndex = 0;
        }
        if ((nIndex == Box::InvalidIndex) || (nIndex >= m_candidates.size())) {
            nIndex = 0;
        }
        if (nIndex < m_candidates.size()) {
            DString completedPath = m_candidates[nIndex].c_str();
            //补全结果是已存在的目录时，追加路径分隔符，便于继续输入下一级路径
            FilePath completedFilePath(completedPath);
            if (completedFilePath.IsExistsDirectory()) {
                completedFilePath.FormatPathAsDirectory();
                completedPath = completedFilePath.ToString();
            }
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetTextNoEvent(completedPath);
                m_pRichEdit->SetSel(m_pRichEdit->GetTextLength(), m_pRichEdit->GetTextLength());
            }
            UpdateAutoCompletePopup();
        }
        else if (GetWindow() != nullptr) {
            //没有可补全的候选项：保持Tab的默认行为（切换到下一个控件）
            GetWindow()->SetNextTabControl(true);
        }
        return true;
    }
    if ((m_pFlyout == nullptr) || !m_pFlyout->IsOpen()) {
        return false;
    }
    if (msg.vkCode == kVK_DOWN) {
        m_bUserNavigated = true;
        m_pFlyout->MoveSelection(1);
        return true;
    }
    if (msg.vkCode == kVK_UP) {
        m_bUserNavigated = true;
        m_pFlyout->MoveSelection(-1);
        return true;
    }
    if (msg.vkCode == kVK_ESCAPE) {
        CloseAutoCompletePopup();
        return true;
    }
    if (msg.vkCode == kVK_RETURN) {
        if (m_bUserNavigated) {
            const size_t nIndex = m_pFlyout->GetSelectedIndex();
            if (nIndex != Box::InvalidIndex && nIndex < m_candidates.size()) {
                if (m_pRichEdit != nullptr) {
                    m_pRichEdit->SetTextNoEvent(m_candidates[nIndex].c_str());
                    m_pRichEdit->SetSel(m_pRichEdit->GetTextLength(), m_pRichEdit->GetTextLength());
                }
                CloseAutoCompletePopup();
                bool bAccepted = CommitAddressPath(m_candidates[nIndex].c_str(), true);
                SendEvent(kEventReturn);
                if (bAccepted && (GetPreviousAddressPath() != GetAddressPath())) {
                    SendEvent(kEventPathChanged);
                }
                return true;
            }
        }
        return false;
    }
    return false;
}

void AddressBar::UpdateAutoCompletePopup()
{
    if (m_bUpdatingPopup) {
        return;
    }
    m_bUpdatingPopup = true;
    m_bUserNavigated = false;
    const DString editText = (m_pRichEdit != nullptr) ? m_pRichEdit->GetText() : DString();
    BuildAutoCompleteCandidates(editText);
    const int32_t nHighlightChars = static_cast<int32_t>(editText.size());
    if (m_candidates.empty()) {
        CloseAutoCompletePopup();
    }
    else if ((m_pFlyout != nullptr) && m_pFlyout->IsOpen()) {
        m_pFlyout->SetHighlightChars(nHighlightChars);
        m_pFlyout->UpdateItems(m_candidates, 0);
    }
    else {
        //宽度与地址栏客户区一致（GetPos为物理像素）
        int32_t nWidth = GetPos().Width();
        if (nWidth > 0) {
            Window* pOwnerWindow = GetWindow();
            if (pOwnerWindow != nullptr) {
                nWidth = pOwnerWindow->Dpi().GetDipSizeFromClientSize(nWidth);
            }
            AutoCompleteFlyout* pFlyout = new AutoCompleteFlyout(GetWindow());
            pFlyout->SetHighlightChars(nHighlightChars);
            pFlyout->AttachItemSelect(UiBind(this, [this](size_t nIndex) {
                if (nIndex < m_candidates.size()) {
                    if (m_pRichEdit != nullptr) {
                        m_pRichEdit->SetTextNoEvent(m_candidates[nIndex].c_str());
                        m_pRichEdit->SetSel(m_pRichEdit->GetTextLength(), m_pRichEdit->GetTextLength());
                    }
                    CloseAutoCompletePopup();
                    bool bAccepted = CommitAddressPath(m_candidates[nIndex].c_str(), true);
                    SendEvent(kEventReturn);
                    if (bAccepted && (GetPreviousAddressPath() != GetAddressPath())) {
                        SendEvent(kEventPathChanged);
                    }
                }
                }, std::placeholders::_1));
            pFlyout->AttachClosed(UiBind(this, [this, pFlyout](Flyout::CloseReason /*reason*/) {
                if (m_pFlyout == pFlyout) {
                    m_pFlyout = nullptr;
                }
                }, std::placeholders::_1));
            if (pFlyout->ShowAt(this, m_candidates, 0, nWidth, Flyout::Placement::Bottom)) {
                m_pFlyout = pFlyout;
            }
        }
    }
    m_bUpdatingPopup = false;
}

void AddressBar::CloseAutoCompletePopup()
{
    if (m_pFlyout != nullptr) {
        AutoCompleteFlyout* pFlyout = m_pFlyout;
        m_pFlyout = nullptr;
        pFlyout->Dismiss();
    }
    m_bUserNavigated = false;
}

void AddressBar::BuildAutoCompleteCandidates(const DString& input)
{
    m_candidates.clear();
    if (input.empty()) {
        return;
    }
    //前缀匹配（Windows路径不区分大小写）
    auto MatchPrefix = [](const DString& text, const DString& prefix) {
        if (text.size() < prefix.size()) {
            return false;
        }
#ifdef DUILIB_BUILD_FOR_WIN
        return _tcsnicmp(text.c_str(), prefix.c_str(), prefix.size()) == 0;
#else
        return text.compare(0, prefix.size(), prefix.c_str()) == 0;
#endif
    };
    auto AddCandidate = [this](const DString& path) {
        const DString candidate = path;
        for (const UiString& exist : m_candidates) {
            if (exist.c_str() == candidate) {
                return;
            }
        }
        m_candidates.push_back(candidate.c_str());
    };

    //1. 历史记录匹配（最多10条）
    if (m_bHistoryEnabled) {
        int32_t nHistoryCount = 0;
        for (const UiString& historyPath : m_pathHistory) {
            if (nHistoryCount >= 10) {
                break;
            }
            const DString historyText = historyPath.c_str();
            if (MatchPrefix(historyText, input)) {
                AddCandidate(historyText);
                ++nHistoryCount;
            }
        }
    }

    //2. 文件系统补全：枚举“目录部分”下的同名子目录/文件（最多10条）
    const size_t nSep = input.find_last_of(_T("\\/"));
    if ((nSep == DString::npos) || (nSep == 0)) {
        return;
    }
    const DString dirPart = input.substr(0, nSep + 1);
    const DString namePart = input.substr(nSep + 1);
    std::error_code ec;
    const std::filesystem::path dirPath(dirPart.c_str());
    if (!std::filesystem::is_directory(dirPath, ec)) {
        return;
    }
    std::vector<DString> dirNames;
    std::vector<DString> fileNames;
    const auto options = std::filesystem::directory_options::skip_permission_denied;
    std::filesystem::directory_iterator it(dirPath, options, ec);
    const std::filesystem::directory_iterator end;
    for (; !ec && (it != end); it.increment(ec)) {
        DString name = it->path().filename().native();
        if (name.empty() || !MatchPrefix(name, namePart)) {
            continue;
        }
        std::error_code typeEc;
        if (it->is_directory(typeEc)) {
            dirNames.push_back(name);
        }
        else {
            fileNames.push_back(name);
        }
        if ((dirNames.size() + fileNames.size()) >= 200) {
            //目录项过多时限制扫描数量，避免影响输入流畅度
            break;
        }
    }
    std::sort(dirNames.begin(), dirNames.end());
    std::sort(fileNames.begin(), fileNames.end());
    int32_t nFileSystemCount = 0;
    for (const DString& name : dirNames) {
        if (nFileSystemCount >= 10) {
            break;
        }
        AddCandidate(dirPart + name);
        ++nFileSystemCount;
    }
    for (const DString& name : fileNames) {
        if (nFileSystemCount >= 10) {
            break;
        }
        AddCandidate(dirPart + name);
        ++nFileSystemCount;
    }
}

bool AddressBar::OnDropData(const EventArgs& msg)
{
    if (!m_bEnableDropNavigation || (msg.lParam == 0)) {
        return false;
    }
    std::vector<DString> fileList;
#ifdef DUILIB_BUILD_FOR_WIN
    if (msg.wParam == kControlDropTypeWindows) {
        const ControlDropData_Windows* pData = (const ControlDropData_Windows*)msg.lParam;
        if (pData != nullptr) {
            fileList = pData->m_fileList;
        }
    }
#endif
#ifdef DUILIB_BUILD_FOR_SDL
    if (fileList.empty() && (msg.wParam == kControlDropTypeSDL)) {
        const ControlDropData_SDL* pData = (const ControlDropData_SDL*)msg.lParam;
        if (pData != nullptr) {
            fileList = pData->m_fileList;
        }
    }
#endif
    if (fileList.empty()) {
        return false;
    }

    //多文件拖放：优先导航到第一个存在的目录；没有目录时，导航到第一个存在文件的父目录
    DString dropDirectory;
    for (const DString& filePath : fileList) {
        FilePath path(filePath);
        if (path.IsExistsDirectory()) {
            dropDirectory = path.ToString();
            break;
        }
    }
    if (dropDirectory.empty()) {
        for (const DString& filePath : fileList) {
            FilePath path(filePath);
            if (!path.IsExistsPath()) {
                continue;
            }
            FilePath parentPath = path.GetParentPath();
            if (parentPath.IsExistsDirectory()) {
                dropDirectory = parentPath.ToString();
                break;
            }
        }
    }
    if (dropDirectory.empty()) {
        return false;
    }

    ShowAddressEdit(true);
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetTextNoEvent(dropDirectory);
    }
    bool bAccepted = CommitAddressPath(dropDirectory, true);
    SendEvent(kEventReturn);
    if (bAccepted && (GetPreviousAddressPath() != GetAddressPath())) {
        SendEvent(kEventPathChanged);
    }
    return true;
}

}//namespace ui
