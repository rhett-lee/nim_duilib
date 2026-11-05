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
#include "duilib/Utils/StringUtil.h"
#include <algorithm>
#include <numeric>
#ifdef DUILIB_BUILD_FOR_WIN
    #include "duilib/Utils/DiskUtils_Windows.h"
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
            if (m_clickedAddressPath.empty()) {
                m_clickedAddressPath = m_addressPath;
                for (size_t index = 0; (index < m_subPathBoxes.size()) && (index < m_pathSegments.size()); ++index) {
                    Control* pBox = m_subPathBoxes[index];
                    if ((pBox != nullptr) && pBox->IsVisible() && pBox->GetRect().ContainsPt(msg.ptMouse)) {
                        m_clickedAddressPath = m_pathSegments[index].m_fullPath;
                        break;
                    }
                }
            }
            //返回true表示继续派发该事件，使外部绑定的右键菜单处理函数也能收到事件
            return true;
            });

    //支持拖放目录到地址栏进行导航
    SetAttribute(_T("enable_drag_drop"), _T("true"));
    AttachEvent(kEventDropData, [this](const EventArgs& args) {
        return OnDropData(args);
        }, 0);

    //初始化地址栏显示内容（空路径时显示“此电脑”等占位文本）
    m_pRichEdit->SetTextNoEvent(m_addressPath.c_str());
    UpdateAddressBarControls(m_addressPath.c_str(), true);
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
        pDisplayNameBtn->AttachClick([this, nSegmentIndex](const EventArgs&) {
            if (nSegmentIndex < m_pathSegments.size()) {
                OnClickedSubPath(m_pathSegments[nSegmentIndex].m_fullPath);
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
        //键盘：左右方向键在面包屑之间移动焦点，回车激活
        pDisplayNameBtn->AttachEvent(kEventKeyDown, [this, nSegmentIndex](const EventArgs& msg) {
            if ((msg.vkCode == kVK_LEFT) || (msg.vkCode == kVK_RIGHT)) {
                MoveSubPathFocus(nSegmentIndex, msg.vkCode == kVK_LEFT);
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

    FilePath filePath(addressPath);
    filePath.NormalizeFilePath();
    filePath.TrimRightPathSeparator();

    std::vector<FilePath> pathList;
    filePath.GetParentPathList(pathList);
    if (!pathList.empty()) {
        if (pathList.back() != filePath) {
            pathList.push_back(filePath);
        }
    }
    else {
        pathList.push_back(filePath);
    }

    for (size_t i = 0; i < pathList.size(); ++i) {
        const FilePath& subPath = pathList[i];
        PathSegment segment;
        segment.m_fullPath = subPath.ToString();
        segment.m_displayName = subPath.GetFileName();
        segment.m_bRoot = (i == 0);
        segment.m_bDrive = false;
        segment.m_bComputer = false;

        if (segment.m_displayName.empty()) {
            //根目录：Windows盘符、UNC根、Unix根
            segment.m_displayName = subPath.ToString();
            while (!segment.m_displayName.empty() &&
                   ((segment.m_displayName.back() == FilePath::GetPathSeparator()) ||
                    (segment.m_displayName.back() == _T(':'))) &&
                   (segment.m_displayName.size() > 1)) {
                if ((segment.m_displayName.back() == _T(':')) && (segment.m_displayName.size() <= 2)) {
                    break;
                }
                segment.m_displayName.pop_back();
            }
            if (segment.m_displayName.empty()) {
                segment.m_displayName = FilePath::GetPathSeparator();
            }
#ifdef DUILIB_BUILD_FOR_WIN
            if (!segment.m_displayName.empty() && (segment.m_displayName.back() == _T(':'))) {
                segment.m_bDrive = true;
                DiskUtils::DiskInfo diskInfo;
                DiskUtils::GetLogicalDriveInfo(subPath.ToString(), diskInfo);
                if (!diskInfo.m_volumeName.empty()) {
                    //卷名通常已包含盘符（如“本地磁盘 (C:)”），优先显示卷名
                    segment.m_displayName = diskInfo.m_volumeName;
                }
            }
#endif
        }
        segments.push_back(segment);
    }
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

    const size_t nCount = m_subPathBoxes.size();
    if ((nCount == 0) || (m_pathSegments.empty())) {
        for (HBox* pBox : m_subPathBoxes) {
            pBox->SetVisible(false);
        }
        if (m_pCollapseBtn != nullptr) {
            m_pCollapseBtn->SetVisible(false);
        }
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
        m_collapsedSegments.clear();
        return;
    }

    //宽度不足：保留根段与末尾段，中间段折叠为"…"
    std::vector<bool> showFlags(m_pathSegments.size(), false);
    showFlags[nLastIndex] = true;

    Button* pCollapseBtn = m_pCollapseBtn;
    int32_t nCollapseWidth = 0;
    if (pCollapseBtn != nullptr) {
        nCollapseWidth = pCollapseBtn->EstimateSize(szAvailable).cx.GetInt32();
        UiMargin collapseMargin = pCollapseBtn->GetMargin();
        nCollapseWidth += (collapseMargin.left + collapseMargin.right);
    }

    const int32_t nMinLastWidth = 48;
    int32_t nAvailable = nBarWidth;
    if ((m_pathSegments.size() > 1) && (nAvailable >= widths[0] + nCollapseWidth + nMinLastWidth)) {
        showFlags[0] = true;
        nAvailable -= widths[0];
    }
    //从末尾向前，尽量保留更多的中间段
    if (m_pathSegments.size() > 2) {
        for (size_t i = m_pathSegments.size() - 2; i >= 1; --i) {
            if (nAvailable >= widths[i] + nCollapseWidth + nMinLastWidth) {
                showFlags[i] = true;
                nAvailable -= widths[i];
            }
            else {
                break;
            }
        }
    }

    bool bShowCollapse = false;
    int32_t nLastWidth = widths[nLastIndex];
    if ((m_pathSegments.size() > 1) && (nAvailable >= nCollapseWidth + nMinLastWidth)) {
        bShowCollapse = true;
        if (nLastWidth > nAvailable - nCollapseWidth) {
            nLastWidth = nAvailable - nCollapseWidth;
        }
    }
    else {
        //放不下"…"与末尾段：隐藏根段，末尾段占满可用宽度
        for (size_t i = 0; i < m_pathSegments.size(); ++i) {
            showFlags[i] = (i == (size_t)nLastIndex);
        }
        nLastWidth = std::min(widths[nLastIndex], nBarWidth);
    }

    m_collapsedSegments.clear();
    for (size_t i = 0; i < m_pathSegments.size(); ++i) {
        HBox* pBox = m_subPathBoxes[i];
        pBox->SetVisible(showFlags[i]);
        bool bCurrent = (i == (size_t)nLastIndex);
        UpdateSubPathControls(i, m_pathSegments[i], !bCurrent, bCurrent);
        Button* pNameBtn = dynamic_cast<Button*>(pBox->GetItemAt(0));
        if (pNameBtn != nullptr) {
            pNameBtn->SetMaxWidth(bCurrent ? nLastWidth : INT32_MAX, false);
        }
        if (!showFlags[i] && !bCurrent) {
            m_collapsedSegments.push_back(m_pathSegments[i]);
        }
    }

    if ((pCollapseBtn != nullptr) && bShowCollapse) {
        //把折叠按钮放到第一个显示的中间段（或末尾段）之前
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
}

std::vector<int32_t> AddressBar::AdjustControlsWidth(const std::vector<int32_t>& originalWidths, int32_t totalWidth)
{
    std::vector<int32_t> adjustedWidths;
    if (originalWidths.empty()) {
        return adjustedWidths;
    }
    const int32_t nMinWidth = 32; //最小宽度（物理像素）
    if (totalWidth < (int32_t)originalWidths.size() * nMinWidth) {
        //控件不足，按最小宽度设置
        adjustedWidths.assign(originalWidths.size(), nMinWidth);
        return adjustedWidths;
    }

    // 计算平均宽度
    int32_t avgWidth = totalWidth / static_cast<int32_t>(originalWidths.size());

    // 找出需要调整的控件(大于平均宽度的)
    std::vector<size_t> adjustIndices;
    for (size_t i = 0; i < originalWidths.size(); ++i) {
        if (originalWidths[i] > avgWidth) {
            adjustIndices.push_back(i);
        }
    }

    // 计算需要从大控件中缩减的总宽度
    int32_t reduceTotal = std::accumulate(originalWidths.begin(), originalWidths.end(), 0) - totalWidth;
    if (reduceTotal <= 0) {
        return originalWidths;  // 不需要缩减
    }

    // 计算大控件的原始总宽度
    int32_t largeSum = 0;
    for (auto idx : adjustIndices) {
        largeSum += originalWidths[idx];
    }
    if (largeSum <= 0) {
        return originalWidths;  // 不需要缩减
    }

    // 按比例缩减大控件
    adjustedWidths = originalWidths;
    int32_t reduced = 0;
    for (size_t i = 0; i < adjustIndices.size(); ++i) {
        size_t idx = adjustIndices[i];
        if (i == adjustIndices.size() - 1) {
            // 最后一个控件吸收舍入误差
            adjustedWidths[idx] = originalWidths[idx] - (reduceTotal - reduced);
        }
        else {
            int32_t reduce = (originalWidths[idx] * reduceTotal) / largeSum;
            adjustedWidths[idx] = originalWidths[idx] - reduce;
            reduced += reduce;
        }
    }

    for (int32_t& v : adjustedWidths) {
        //限制最小值，避免出现0或者负值
        if (v < nMinWidth) {
            v = nMinWidth;
        }
    }
    return adjustedWidths;
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

    //预提交：允许业务层校验/规范化路径（例如把文件路径转换为父目录）
    if ((m_pathChangingCallback != nullptr) && !addressPath.empty()) {
        DString newPath = addressPath;
        bool bAccept = m_pathChangingCallback(newPath);
        if (!bAccept) {
            //拒绝提交：恢复编辑框并退出编辑状态，不派发kEventPathChanged
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
    m_bUpdatingUI = false;
    return true;
}

void AddressBar::SetAddressPath(const DString& addressPath)
{
    m_addressPath = addressPath;
    if ((m_pRichEdit != nullptr) && !IsEditing()) {
        //编辑状态下不覆盖用户输入
        m_pRichEdit->SetTextNoEvent(addressPath);
    }
    if (m_pBarBox != nullptr) {
        bool bChanged = UpdateAddressBarControls(addressPath, true);
        if (bChanged) {
            AddPathHistory(addressPath);
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
        nItemWidth = std::max(nItemWidth, 220);
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

bool AddressBar::HandleEditKeyDown(const EventArgs& msg)
{
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
    BuildAutoCompleteCandidates(m_pRichEdit != nullptr ? m_pRichEdit->GetText() : DString());
    if (m_candidates.empty()) {
        CloseAutoCompletePopup();
    }
    else if ((m_pFlyout != nullptr) && m_pFlyout->IsOpen()) {
        m_pFlyout->UpdateItems(m_candidates, 0);
    }
    else {
        //宽度与地址栏客户区一致（GetPos为物理像素）
        int32_t nWidth = GetPos().Width();
        if (nWidth > 0) {
            Window* pOwnerWindow = GetWindow();
            if (pOwnerWindow != nullptr) {
                if (pOwnerWindow->Dpi().HasPixelDensity()) {
                    pOwnerWindow->Dpi().ClientSizeToWindowSize(nWidth);
                }
                else {
                    nWidth = pOwnerWindow->Dpi().GetUnscaleWindowSize(nWidth);
                }
            }
            AutoCompleteFlyout* pFlyout = new AutoCompleteFlyout(GetWindow());
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
    if (input.empty() || !m_bHistoryEnabled) {
        return;
    }
    for (const UiString& historyPath : m_pathHistory) {
        const DString historyText = historyPath.c_str();
        bool bMatch = false;
#ifdef DUILIB_BUILD_FOR_WIN
        //Windows路径不区分大小写
        bMatch = (historyText.size() >= input.size()) &&
                 (_tcsnicmp(historyText.c_str(), input.c_str(), input.size()) == 0);
#else
        bMatch = (historyText.size() >= input.size()) &&
                 (historyText.compare(0, input.size(), input.c_str()) == 0);
#endif
        if (bMatch) {
            m_candidates.push_back(historyPath);
        }
    }
}

bool AddressBar::OnDropData(const EventArgs& msg)
{
    if (!m_bEnableDropNavigation || (msg.lParam == 0)) {
        return false;
    }
    DString filePath;
#ifdef DUILIB_BUILD_FOR_WIN
    if (msg.wParam == kControlDropTypeWindows) {
        const ControlDropData_Windows* pData = (const ControlDropData_Windows*)msg.lParam;
        if ((pData != nullptr) && !pData->m_fileList.empty()) {
            filePath = pData->m_fileList[0].c_str();
        }
    }
#endif
#ifdef DUILIB_BUILD_FOR_SDL
    if (filePath.empty() && (msg.wParam == kControlDropTypeSDL)) {
        const ControlDropData_SDL* pData = (const ControlDropData_SDL*)msg.lParam;
        if ((pData != nullptr) && !pData->m_fileList.empty()) {
            filePath = pData->m_fileList[0].c_str();
        }
    }
#endif
    if (filePath.empty()) {
        return false;
    }
    FilePath dropPath(filePath);
    if (!dropPath.IsExistsPath()) {
        return false;
    }
    if (dropPath.IsExistsDirectory()) {
        ShowAddressEdit(true);
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetTextNoEvent(dropPath.ToString());
        }
        bool bAccepted = CommitAddressPath(dropPath.ToString(), true);
        SendEvent(kEventReturn);
        if (bAccepted && (GetPreviousAddressPath() != GetAddressPath())) {
            SendEvent(kEventPathChanged);
        }
        return true;
    }
    FilePath parentPath = dropPath.GetParentPath();
    if (parentPath.IsExistsDirectory()) {
        bool bAccepted = CommitAddressPath(parentPath.ToString(), true);
        SendEvent(kEventReturn);
        if (bAccepted && (GetPreviousAddressPath() != GetAddressPath())) {
            SendEvent(kEventPathChanged);
        }
        return true;
    }
    return false;
}

}//namespace ui
