#include "AddressBar.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Control/Button.h"
#include "duilib/Utils/FilePath.h"
#include <numeric>

#ifdef DUILIB_BUILD_FOR_WIN
    #include "duilib/Utils/DiskUtils_Windows.h"
#endif

namespace ui
{

AddressBar::AddressBar(Window* pWindow):
    HBox(pWindow),
    m_pRichEdit(nullptr),
    m_pBarBox(nullptr),
    m_bEnableTooltip(true),
    m_bReturnUpdateUI(true),
    m_bEscUpdateUI(true),
    m_bKillFocusUpdateUI(true),
    m_bUpdatingUI(false),
    m_editClass(_T("address_bar_edit")),
    m_editClearBtnClass(_T("rich_edit_clear_btn")),
    m_subPathHBoxClass(_T("address_bar_sub_path_hbox")),
    m_subPathBtnClass(_T("address_bar_sub_path_button")),
    m_subPathRootClass(_T("address_bar_sub_path_root")),
    m_pathSeparatorClass(_T("address_bar_path_separator"))
{
}

DString AddressBar::GetType() const { return DUI_CTR_ADDRESS_BAR; }

void AddressBar::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("path_tooltip")) {
        SetEnablePathTooltip(strValue == _T("true"));
    }
    else if (strName == _T("return_update_ui")) {
        SetReturnUpdateUI(strValue == _T("true"));
    }
    else if (strName == _T("esc_update_ui")) {
        SetEscUpdateUI(strValue == _T("true"));
    }
    else if (strName == _T("kill_focus_update_ui")) {
        SetKillFocusUpdateUI(strValue == _T("true"));
    }
    else if (strName == _T("rich_edit_class")) {
        SetRichEditClass(strValue);
    }
    else if (strName == _T("rich_edit_clear_btn_class")) {
        SetRichEditClearBtnClass(strValue);
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
    else if (strName == _T("path_separator_class")) {
        SetPathSeparatorClass(strValue);
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

void AddressBar::SetRichEditClearBtnClass(const DString& clearBtnClass)
{
    m_editClearBtnClass = clearBtnClass;
}

DString AddressBar::GetRichEditClearBtnClass() const
{
    return m_editClearBtnClass.c_str();
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

void AddressBar::SetPathSeparatorClass(const DString& pathSeparatorClass)
{
    m_pathSeparatorClass = pathSeparatorClass;
}

DString AddressBar::GetPathSeparatorClass() const
{
    return m_pathSeparatorClass.c_str();
}

void AddressBar::OnSetVisible(bool bChanged)
{
    BaseClass::OnSetVisible(bChanged);
    if (IsVisible()) {
        ShowAddressEdit(false);
        UpdateAddressBarControlsStatus();
    }
}

void AddressBar::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
    //更新子控件的状态
    UpdateAddressBarControlsStatus();
}

void AddressBar::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //地址栏的基本结构
    //<AddressBar>
    //   <RichEdit/>
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
        m_pRichEdit = new RichEdit(GetWindow());
        AddItem(m_pRichEdit);
        m_pRichEdit->SetClass(GetRichEditClearBtnClass());
        m_pRichEdit->SetClass(GetRichEditClass());

        m_pRichEdit->AttachSetFocus([this](const EventArgs&) {
            OnAddressBarSetFocus(m_pRichEdit);
            return true;
            });
        m_pRichEdit->AttachKillFocus([this](const EventArgs& args) {
            ShowAddressEdit(false);
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
    }

    if (m_pBarBox == nullptr) {
        m_pBarBox = new HBox(GetWindow());
        AddItem(m_pBarBox);
        m_pBarBox->SetPadding(UiPadding(0, 0, 16, 0), true);

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

    if (!m_addressPath.empty()) {
        SetAddressPath(m_addressPath.c_str());
    }
}

void AddressBar::ShowAddressEdit(bool bShow)
{
    if (m_pBarBox != nullptr) {
        m_pBarBox->SetVisible(!bShow);
    }
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetVisible(bShow);
        if (bShow) {
            m_pRichEdit->SetFocus();
        }        
    }
}

bool AddressBar::AddSubPath(const DString& displayName, const DString& filePath)
{
    ASSERT(!filePath.empty());
    if (filePath.empty()) {
        return false;
    }

    ASSERT(m_pBarBox != nullptr);
    if (m_pBarBox == nullptr) {
        return false;
    }

    HBox* pBox = new HBox(GetWindow());
    m_pBarBox->AddItem(pBox);
    pBox->SetClass(GetSubPathHBoxClass());
    pBox->SetNoFocus();

    Button* pDisplayNameBtn = new Button(GetWindow());
    pBox->AddItem(pDisplayNameBtn);    
    if (filePath == _T("/")) {
        //根目录
        pDisplayNameBtn->SetClass(GetSubPathBtnClass());
        pDisplayNameBtn->SetClass(GetSubPathRootClass());
    }
    else {
        pDisplayNameBtn->SetClass(GetSubPathBtnClass());
        pDisplayNameBtn->SetText(displayName.empty() ? filePath : displayName);
    }
    if (m_bEnableTooltip) {
        pDisplayNameBtn->SetToolTipText(filePath);
    }
    pDisplayNameBtn->SetNoFocus();

    pDisplayNameBtn->AttachClick([this, filePath](const EventArgs&) {
        OnClickedSubPath(filePath);
        return true;
        });

    Button* pArrowBtn = new Button(GetWindow());
    pBox->AddItem(pArrowBtn);
    pArrowBtn->SetClass(GetPathSeparatorClass());
    pArrowBtn->SetNoFocus();
    return true;
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

void AddressBar::OnAddressBarKillFocus(Control* pNewFocus)
{
    if ((pNewFocus != this) && (pNewFocus != m_pRichEdit) && (pNewFocus != m_pBarBox)) {
        bool bChanged = false;
        auto flag = GetWeakFlag();
        if (m_bKillFocusUpdateUI) {
            if (!m_bUpdatingUI) {
                DString addressPath;
                if (m_pRichEdit != nullptr) {
                    addressPath = m_pRichEdit->GetText();
                }
                bChanged = UpdateAddressBarControls(addressPath);
            }
        }
        else {
            //恢复
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetText(GetAddressPath());
            }
        }
        SendEvent(kEventKillFocus, (WPARAM)pNewFocus);
        if (bChanged && !flag.expired()) {
            SendEvent(kEventPathChanged);
        }
    }
}

void AddressBar::OnAddressBarReturn()
{
    DString addressPath;
    if (m_pRichEdit != nullptr) {
        addressPath = m_pRichEdit->GetText();
    }
    bool bChanged = false;
    auto flag = GetWeakFlag();
    if (m_bReturnUpdateUI) {
        m_bUpdatingUI = true;
        bChanged = UpdateAddressBarControls(addressPath);
        ShowAddressEdit(false);
        m_bUpdatingUI = false;
    }    
    SendEvent(kEventReturn);
    if (bChanged && !flag.expired()) {
        SendEvent(kEventPathChanged);
    }
}

void AddressBar::OnAddressBarEsc()
{
    if (m_bEscUpdateUI) {
        m_bUpdatingUI = true;
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetText(GetAddressPath());
        }
        ShowAddressEdit(false);
        m_bUpdatingUI = false;
    }
    SendEvent(kEventEsc);
}

void AddressBar::SetAddressPath(const DString& addressPath)
{
    m_addressPath = addressPath;
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetText(addressPath);
    }
    UpdateAddressBarControls(addressPath);
}

DString AddressBar::GetAddressPath() const
{
    return m_showAddressPath.c_str();
}

DString AddressBar::GetPreviousAddressPath() const
{
    return m_prevShowAddressPath.c_str();
}

bool AddressBar::UpdateAddressBarControls(const DString& addressPath)
{
    if (m_showAddressPath == addressPath) {
        return false;
    }
    m_prevShowAddressPath = m_showAddressPath;
    m_showAddressPath = addressPath;

    if (m_pBarBox != nullptr) {
        m_pBarBox->RemoveAllItems();
    }
    if (addressPath.empty()) {
        return true;
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

    bool bRoot = true;
    DString displayName;
    for (const FilePath& subPath : pathList) {
        displayName = subPath.GetFileName();
        if (bRoot && displayName.empty()) {            
#ifdef DUILIB_BUILD_FOR_WIN
            //Windows盘符
            DiskUtils::DiskInfo diskInfo;
            DiskUtils::GetLogicalDriveInfo(subPath.ToString(), diskInfo);
            displayName = diskInfo.m_volumeName;
#endif
            if (displayName.empty()) {
                displayName = subPath.ToString();
                if (!displayName.empty() && displayName[displayName.size() - 1] == FilePath::GetPathSeparator()) {
                    displayName = displayName.substr(0, displayName.size() - 1);
                }
            }
        }
        AddSubPath(displayName, subPath.ToString());
    }
    UpdateAddressBarControlsStatus();
    return true;
}

void AddressBar::UpdateAddressBarControlsStatus()
{
    //如果显示宽度不足，需要隐藏部分控件
    if (m_pBarBox == nullptr) {
        return;
    }
    UiRect rc = GetPos();
    UiPadding rcBarPadding = GetPadding();
    rc.Deflate(rcBarPadding);

    std::vector<Control*> pathButtonList;    
    int32_t nKeepWidth = 0; //需要预留的宽度
    UiPadding rcPadding = m_pBarBox->GetPadding();
    UiMargin rcMargin = m_pBarBox->GetMargin();
    nKeepWidth += (rcPadding.left + rcPadding.right);
    nKeepWidth += (rcMargin.left + rcMargin.right);

    size_t nCount = m_pBarBox->GetItemCount();
    for (size_t nItem = 0; nItem < nCount; ++nItem) {
        Control* pControl = m_pBarBox->GetItemAt(nItem);
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }
        Box* pBox = dynamic_cast<Box*>(pControl);
        if (pBox == nullptr) {
            continue;
        }
        rcPadding = pBox->GetPadding();
        rcMargin = pBox->GetMargin();
        nKeepWidth += (rcPadding.left + rcPadding.right);
        nKeepWidth += (rcMargin.left + rcMargin.right);

        Control* pPathButton = pBox->GetItemAt(0);
        if (pPathButton != nullptr) {
            rcPadding = pPathButton->GetPadding();
            rcMargin = pPathButton->GetMargin();
            nKeepWidth += (rcPadding.left + rcPadding.right);
            nKeepWidth += (rcMargin.left + rcMargin.right);
            pathButtonList.push_back(pPathButton);
        }
        Control* pPathSepButton = pBox->GetItemAt(1);
        if (pPathSepButton != nullptr) {
            rcMargin = pPathSepButton->GetMargin();
            nKeepWidth += (rcMargin.left + rcMargin.right);
            nKeepWidth += pPathSepButton->GetWidth();
        }
    }
    std::vector<int32_t> originalWidths; //每个控件占用的实际宽度
    int32_t nPathWidth = nKeepWidth;     //路径控件所占的总宽度
    for (Control* pPathButton : pathButtonList) {
        rcPadding = pPathButton->GetPadding();
        nPathWidth -= (rcPadding.left + rcPadding.right);//因为EstimateSize返回的结果已经包含了内边距，所以需要减去，避免重复计算

        int32_t nEstimateSize = pPathButton->EstimateSize(UiSize(rc.Width(), rc.Height())).cx.GetInt32();
        nPathWidth += nEstimateSize;
        originalWidths.push_back(nEstimateSize);
    }

    bool bUseDefault = true;
    const int32_t nBarWidth = rc.Width();
    if ((nPathWidth > nBarWidth) && (pathButtonList.size() > 1) && (originalWidths.size() == pathButtonList.size())) {
        //第一个控件：根目录控件保持不变
        pathButtonList[0]->SetMaxWidth(originalWidths[0], false);
        nKeepWidth += originalWidths[0];

        pathButtonList.erase(pathButtonList.begin());
        originalWidths.erase(originalWidths.begin());

        //从第二个控件起，动态按比例调整宽度
        int32_t totalWidth = nBarWidth - nKeepWidth; //路径控件可用的总宽度
        std::vector<int32_t> adjustWidthList = AdjustControlsWidth(originalWidths, totalWidth);
        if (adjustWidthList.size() == pathButtonList.size()) {
            bUseDefault = false;
            const size_t nPathCount = pathButtonList.size();
            for (size_t nIndex = 0; nIndex < nPathCount; ++nIndex) {
                pathButtonList[nIndex]->SetMaxWidth(adjustWidthList[nIndex], false);
            }
        }
    }
    if (bUseDefault) {
        //恢复原状
        for (Control* pPathButton : pathButtonList) {
            pPathButton->SetMaxWidth(INT32_MAX, false);
        }
    }
}

std::vector<int32_t> AddressBar::AdjustControlsWidth(const std::vector<int32_t>& originalWidths, int32_t totalWidth)
{
    std::vector<int32_t> adjustedWidths;
    if (originalWidths.empty()) {
        return adjustedWidths;
    }
    const int32_t nMinWidth = 2; //最小宽度
    if (totalWidth < (int32_t)adjustedWidths.size() * nMinWidth) {
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

}//namespace ui
