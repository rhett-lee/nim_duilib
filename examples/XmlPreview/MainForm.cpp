#include "MainForm.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#include <oleidl.h>
#endif
#include <vector>

namespace
{
/** 获取拖放数据中的文件列表与鼠标所在点的客户区坐标
* @return 返回 false 表示该拖放数据不是文件拖放（比如拖入的是文本）
*/
bool GetDropFileData(ui::ControlDropType dropType, void* pDropData,
                     std::vector<DString>& fileList, ui::UiPoint& pt)
{
    if (pDropData == nullptr) {
        return false;
    }
    if (dropType == ui::kControlDropTypeWindows) {
        const ui::ControlDropData_Windows* pData = (const ui::ControlDropData_Windows*)pDropData;
        fileList = pData->m_fileList;
        pt = ui::UiPoint(pData->m_ptClientX, pData->m_ptClientY);
        return true;
    }
    if (dropType == ui::kControlDropTypeSDL) {
        const ui::ControlDropData_SDL* pData = (const ui::ControlDropData_SDL*)pDropData;
        if (pData->m_bTextData) {
            return false;
        }
        fileList = pData->m_fileList;
        pt = ui::UiPoint(pData->m_ptClientX, pData->m_ptClientY);
        return true;
    }
    return false;
}

/** 标记拖放事件已经处理（截获该事件，不再派发给界面中的其他控件）
*/
void MarkDropHandled(ui::ControlDropType dropType, void* pDropData)
{
    if (pDropData == nullptr) {
        return;
    }
    if (dropType == ui::kControlDropTypeWindows) {
        ui::ControlDropData_Windows* pData = (ui::ControlDropData_Windows*)pDropData;
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        pData->m_dwEffect = DROPEFFECT_COPY;
#endif
        pData->m_hResult = 0; //S_OK
        pData->m_bHandled = true;
    }
    else if (dropType == ui::kControlDropTypeSDL) {
        ui::ControlDropData_SDL* pData = (ui::ControlDropData_SDL*)pDropData;
        pData->m_bHandled = true;
    }
}

} //namespace

MainForm::MainForm() :
    m_pXmlBox(nullptr),
    m_pXmlFilePathLabel(nullptr),
    m_pXmlLoadResult(nullptr),
    m_pXmlFileError(nullptr),
    m_pRefreshInterval(nullptr),
    m_pCheckAutoRefresh(nullptr),
    m_nIntervalSeconds(-1),
    m_timerId(0),
    m_bLoadSuccess(false),
    m_bHasXmlFile(false),
    m_sLastError(),
    m_bDragOverPreview(false)
{
}

MainForm::~MainForm()
{
    StopRefreshTimer();
}

DString MainForm::GetSkinFolder()
{
    return _T("xml_preview");
}

DString MainForm::GetSkinFile()
{
    return _T("xml_preview.xml");
}

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    InitControls();
    if (m_pXmlBox == nullptr) {
        return;
    }

    InitEvents();
    InitAutoRefresh();

    //初始化已加载XML文件的状态（初始加载发生在回调注册之前，需通过错误信息判断结果）
    ui::FilePath xmlFilePath = m_pXmlBox->GetXmlFileFullPath();
    if (xmlFilePath.IsEmpty()) {
        xmlFilePath = m_pXmlBox->GetXmlFilePath();
    }
    DString lastError = m_pXmlBox->GetLastError();
    if (!xmlFilePath.IsEmpty()) {
        m_bHasXmlFile = true;
        m_xmlFilePath = xmlFilePath;
        if (lastError.empty()) {
            m_bLoadSuccess = true;
            m_sLastError.clear();
        }
        else {
            m_bLoadSuccess = false;
            m_sLastError = FormatLoadError(m_xmlFilePath, lastError);
        }
        UpdatePathText();
        UpdateStatusText();
        if (m_bLoadSuccess) {
            UpdateFileDataFromDisk();
        }
    }
    else {
        m_bHasXmlFile = false;
        m_bLoadSuccess = false;
        UpdatePathText();
        UpdateStatusText();
    }
}

bool MainForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();
    //状态文本由代码设置，需要按当前语言重新刷新
    UpdateStatusText();
    return bRet;
}

void MainForm::InitControls()
{
    m_pXmlBox = dynamic_cast<ui::XmlBox*>(FindControl(_T("xml_box_test")));
    m_pXmlFilePathLabel = dynamic_cast<ui::Label*>(FindControl(_T("xml_file_path")));
    m_pXmlLoadResult = dynamic_cast<ui::Label*>(FindControl(_T("xml_file_load_result")));
    m_pXmlFileError = dynamic_cast<ui::Label*>(FindControl(_T("xml_file_error")));
    m_pRefreshInterval = dynamic_cast<ui::RichEdit*>(FindControl(_T("auto_refresh_interval")));
    m_pCheckAutoRefresh = dynamic_cast<ui::CheckBox*>(FindControl(_T("checkbox_auto_refresh")));
}

void MainForm::InitEvents()
{
    //XML加载完成的通知：更新路径、状态与错误信息
    m_pXmlBox->AddLoadXmlCallback([this](const ui::FilePath& xmlPath, bool bSuccess) {
        if (bSuccess) {
            ui::FilePath fullPath = m_pXmlBox->GetXmlFileFullPath();
            if (fullPath.IsEmpty()) {
                fullPath = xmlPath;
            }
            m_bHasXmlFile = true;
            m_bLoadSuccess = true;
            m_xmlFilePath = fullPath;
            m_sLastError.clear();
            UpdatePathText();
            UpdateStatusText();
            UpdateFileDataFromDisk();
        }
        else {
            m_bHasXmlFile = true;
            m_bLoadSuccess = false;
            m_xmlFilePath = xmlPath;
            DString errorText = m_pXmlBox->GetLastError();
            if (errorText.empty()) {
                errorText = ui::StringUtil::Printf(_T("Failed to load: %s"), xmlPath.ToString().c_str());
            }
            m_sLastError = FormatLoadError(xmlPath, errorText);
            UpdatePathText();
            UpdateStatusText();
        }
    });

    if (ui::Button* pBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_browse_xml_file")))) {
        pBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            BrowseXmlFile();
            return true;
        });
    }
    if (ui::Button* pBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_reload_xml_file")))) {
        pBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (!m_xmlFilePath.IsEmpty()) {
                m_pXmlBox->SetXmlFilePath(m_xmlFilePath);
            }
            else {
                BrowseXmlFile();
            }
            return true;
        });
    }
    if (ui::Button* pBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_clear_xml_file")))) {
        pBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ClearPreview();
            return true;
        });
    }

    //拖入XML文件时加载
    m_pXmlBox->SetEnableDragDrop(true);
    m_pXmlBox->SetEnableDropFile(true);
    m_pXmlBox->SetDropFileTypes(_T(".xml"));
    m_pXmlBox->AttachDropData([this](const ui::EventArgs& args) {
        ui::FilePath xmlFilePath;
        if (args.wParam == ui::kControlDropTypeWindows) {
            const ui::ControlDropData_Windows* dropData = (const ui::ControlDropData_Windows*)args.lParam;
            if (dropData != nullptr) {
                GetDropXmlFile(dropData->m_fileList, xmlFilePath);
            }
        }
        else if (args.wParam == ui::kControlDropTypeSDL) {
            const ui::ControlDropData_SDL* dropData = (const ui::ControlDropData_SDL*)args.lParam;
            if (dropData != nullptr) {
                GetDropXmlFile(dropData->m_fileList, xmlFilePath);
            }
        }
        if (!xmlFilePath.IsEmpty()) {
            m_pXmlBox->SetXmlFilePath(xmlFilePath);
        }
        return true;
    });
}

void MainForm::InitAutoRefresh()
{
    if (m_pCheckAutoRefresh != nullptr) {
        m_pCheckAutoRefresh->Selected(false, false);
        m_pCheckAutoRefresh->AttachSelect([this](const ui::EventArgs& /*args*/) {
            EnableRefreshInterval(true);
            ApplyAutoRefresh();
            return true;
        });
        m_pCheckAutoRefresh->AttachUnSelect([this](const ui::EventArgs& /*args*/) {
            EnableRefreshInterval(false);
            StopRefreshTimer();
            return true;
        });
    }

    //未勾选自动刷新时，间隔输入框不可编辑
    EnableRefreshInterval(false);

    if (m_pRefreshInterval != nullptr) {
        m_pRefreshInterval->AttachTextChanged([this](const ui::EventArgs& /*args*/) {
            //仅在自动刷新启用时，间隔变化才重新设置定时器
            if ((m_pCheckAutoRefresh != nullptr) && m_pCheckAutoRefresh->IsSelected()) {
                ApplyAutoRefresh();
            }
            return true;
        });
    }
}

void MainForm::BrowseXmlFile()
{
    std::vector<ui::FileDialog::FileType> fileTypes;
    ui::FileDialog::FileType xmlFileType;
    xmlFileType.szName = _T("XML Files");
    xmlFileType.szExt = _T("*.xml");
    fileTypes.push_back(xmlFileType);

    ui::FileDialog fileDialog;
    ui::FilePath xmlFilePath;
    if (fileDialog.BrowseForFile(this, xmlFilePath, true, fileTypes) && !xmlFilePath.IsEmpty()) {
        m_pXmlBox->SetXmlFilePath(xmlFilePath);
    }
}

void MainForm::OnDropEnterMsg(ui::ControlDropType dropType, void* pDropData)
{
    ui::FilePath xmlFilePath;
    if (IsDropInPreviewArea(dropType, pDropData, xmlFilePath)) {
        SetPreviewDropHighlight(true);
        MarkDropHandled(dropType, pDropData);
    }
    else {
        SetPreviewDropHighlight(false);
    }
}

void MainForm::OnDropOverMsg(ui::ControlDropType dropType, void* pDropData)
{
    //拖过事件的判断逻辑与拖入事件相同
    OnDropEnterMsg(dropType, pDropData);
}

void MainForm::OnDropMsg(ui::ControlDropType dropType, void* pDropData)
{
    ui::FilePath xmlFilePath;
    if (!IsDropInPreviewArea(dropType, pDropData, xmlFilePath)) {
        SetPreviewDropHighlight(false);
        return;
    }
    SetPreviewDropHighlight(false);
    MarkDropHandled(dropType, pDropData);
    if (m_pXmlBox != nullptr) {
        m_pXmlBox->SetXmlFilePath(xmlFilePath);
    }
}

void MainForm::OnDropLeaveMsg()
{
    SetPreviewDropHighlight(false);
}

bool MainForm::IsDropInPreviewArea(ui::ControlDropType dropType, void* pDropData,
                                   ui::FilePath& xmlFilePath) const
{
    xmlFilePath.Clear();
    if (m_pXmlBox == nullptr) {
        return false;
    }
    std::vector<DString> fileList;
    ui::UiPoint pt;
    if (!GetDropFileData(dropType, pDropData, fileList, pt)) {
        return false;
    }
    if (!m_pXmlBox->GetRect().ContainsPt(pt)) {
        return false;
    }
    return GetDropXmlFile(fileList, xmlFilePath);
}

bool MainForm::GetDropXmlFile(const std::vector<DString>& fileList, ui::FilePath& xmlFilePath) const
{
    xmlFilePath.Clear();
    for (const DString& fileName : fileList) {
        ui::FilePath filePath(fileName);
        if (filePath.IsEmpty()) {
            continue;
        }
        if (ui::StringUtil::IsEqualNoCase(filePath.GetFileExtension().c_str(), _T(".xml"))) {
            xmlFilePath = filePath;
            return true;
        }
    }
    return false;
}

void MainForm::SetPreviewDropHighlight(bool bHighlight)
{
    if (m_bDragOverPreview == bHighlight) {
        return;
    }
    m_bDragOverPreview = bHighlight;
    if (m_pXmlBox != nullptr) {
        m_pXmlBox->SetBorderColor(ui::kControlStateNormal,
            bHighlight ? DString(_T("color_accent")) : DString(_T("border_split_level1")));
        m_pXmlBox->Invalidate();
    }
}

void MainForm::ClearPreview()
{
    if (m_pXmlBox == nullptr) {
        return;
    }
    //清空后没有可刷新的文件，停止自动刷新
    if ((m_pCheckAutoRefresh != nullptr) && m_pCheckAutoRefresh->IsSelected()) {
        m_pCheckAutoRefresh->Selected(false, true);
    }
    m_pXmlBox->ClearXmlBox();

    m_xmlFilePath.Clear();
    m_xmlFileData.clear();
    m_bHasXmlFile = false;
    m_bLoadSuccess = false;
    m_sLastError.clear();

    UpdatePathText();
    UpdateStatusText();
}

void MainForm::UpdatePathText()
{
    if (m_pXmlFilePathLabel != nullptr) {
        m_pXmlFilePathLabel->SetText(m_xmlFilePath.ToString());
    }
}

void MainForm::UpdateStatusText()
{
    if (m_pXmlLoadResult == nullptr) {
        return;
    }

    DString statusText;
    DString statusColor;
    bool bShowError = false;
    if (!m_bHasXmlFile) {
        statusText = GetLangString(_T("STRID_XMLPREVIEW_STATUS_EMPTY"));
        if (statusText.empty()) {
            statusText = _T("Waiting");
        }
        statusColor = _T("text_muted");
    }
    else if (m_bLoadSuccess) {
        statusText = GetLangString(_T("STRID_XMLPREVIEW_STATUS_SUCCESS"));
        if (statusText.empty()) {
            statusText = _T("Loaded");
        }
        statusColor = _T("color_success");
    }
    else {
        statusText = GetLangString(_T("STRID_XMLPREVIEW_STATUS_FAILED"));
        if (statusText.empty()) {
            statusText = _T("Load failed");
        }
        statusColor = _T("color_error");
        bShowError = true;
    }
    m_pXmlLoadResult->SetText(statusText);
    m_pXmlLoadResult->SetStateTextColor(ui::kControlStateNormal, statusColor);

    if (m_pXmlFileError != nullptr) {
        if (bShowError) {
            m_pXmlFileError->SetText(m_sLastError);
            m_pXmlFileError->SetVisible(true);
        }
        else {
            m_pXmlFileError->SetText(DString());
            m_pXmlFileError->SetVisible(false);
        }
    }
}

void MainForm::UpdateFileDataFromDisk()
{
    m_xmlFileData.clear();
    if (!m_xmlFilePath.IsEmpty()) {
        ui::FileUtil::ReadFileData(m_xmlFilePath, m_xmlFileData);
    }
}

void MainForm::ApplyAutoRefresh()
{
    if ((m_pCheckAutoRefresh == nullptr) || !m_pCheckAutoRefresh->IsSelected()) {
        StopRefreshTimer();
        return;
    }
    StartRefreshTimer(GetRefreshIntervalSeconds());
}

void MainForm::EnableRefreshInterval(bool bEnable)
{
    if (m_pRefreshInterval != nullptr) {
        m_pRefreshInterval->SetEnabled(bEnable);
    }
}

int32_t MainForm::GetRefreshIntervalSeconds()
{
    int64_t nSeconds = 0;
    if (m_pRefreshInterval != nullptr) {
        nSeconds = (int64_t)m_pRefreshInterval->GetTextNumber();
    }
    if (nSeconds < 1) {
        nSeconds = 1;
    }
    if (nSeconds > 60) {
        nSeconds = 60;
    }
    return (int32_t)nSeconds;
}

void MainForm::StartRefreshTimer(int32_t nIntervalSeconds)
{
    if (nIntervalSeconds < 1) {
        nIntervalSeconds = 1;
    }
    if ((m_timerId > 0) && (m_nIntervalSeconds == nIntervalSeconds)) {
        //间隔未变化时无需重建定时器
        return;
    }
    StopRefreshTimer();

    m_nIntervalSeconds = nIntervalSeconds;
    m_timerId = ui::GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), [this]() {
        CheckXmlPreview();
        }, (uint64_t)nIntervalSeconds * 1000);
}

void MainForm::StopRefreshTimer()
{
    if (m_timerId > 0) {
        ui::GlobalManager::Instance().Timer().RemoveTimer(m_timerId);
        m_timerId = 0;
    }
    m_nIntervalSeconds = -1;
}

void MainForm::CheckXmlPreview()
{
    if (m_xmlFilePath.IsEmpty() || (m_pXmlBox == nullptr)) {
        return;
    }

    std::vector<uint8_t> xmlFileData;
    if (!ui::FileUtil::ReadFileData(m_xmlFilePath, xmlFileData)) {
        //文件被删除或不可读时，更新为失败状态
        if (m_bLoadSuccess) {
            m_bLoadSuccess = false;
            m_sLastError = ui::StringUtil::Printf(_T("%s %s"),
                GetLangString(_T("STRID_XMLPREVIEW_FILE_MISSING")).c_str(),
                m_xmlFilePath.ToString().c_str());
            UpdateStatusText();
        }
        return;
    }

    if (xmlFileData != m_xmlFileData) {
        m_xmlFileData = xmlFileData;
        //文件内容变化时重新加载，加载结果由回调统一更新
        m_pXmlBox->SetXmlFilePath(m_xmlFilePath);
    }
}

DString MainForm::FormatLoadError(const ui::FilePath& xmlPath, const DString& errorText) const
{
    //文件本身不存在或不可读时，使用本地化的提示信息
    std::vector<uint8_t> xmlFileData;
    if (!ui::FileUtil::ReadFileData(xmlPath, xmlFileData)) {
        DString missingText = GetLangString(_T("STRID_XMLPREVIEW_FILE_MISSING"));
        if (missingText.empty()) {
            missingText = _T("File not found or unreadable:");
        }
        return ui::StringUtil::Printf(_T("%s %s"), missingText.c_str(), xmlPath.ToString().c_str());
    }
    return errorText;
}

DString MainForm::GetLangString(const DString& stringId) const
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(stringId);
}
