#include "MainForm.h"

bool MainForm::OnNewFile()
{
    if (!ConfirmDiscardChanges()) {
        return false;
    }
    if (m_pRichEdit != nullptr) {
        m_bLoadingDocument = true;
        m_pRichEdit->SetText(DString());
        m_bLoadingDocument = false;
        m_pRichEdit->SetModify(false);
        m_pRichEdit->SetFocus();
        m_pRichEdit->SetSel(0, 0);
    }
    m_filePath.Clear();
    m_bSampleDocument = false;
    m_encodingText = _T("UTF-8");
    m_statusMessage.clear();
    UpdateSaveStatus();
    UpdateStatusBar();
    UpdateEditButtonStatus();
    return true;
}

bool MainForm::OnOpenFile()
{
    if (!ConfirmDiscardChanges()) {
        return false;
    }

    std::vector<ui::FileDialog::FileType> fileTypes;
    fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_ALL_FILES")), _T("*.*")});
    fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_TXT_FILES")), _T("*.txt") });
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    //仅富文本模式支持RTF文件，避免选择后无法打开
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsRichText()) {
        fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_RTF_FILES")), _T("*.rtf") });
    }
#endif

    DString defaultExt;
    //默认选中"所有文件"（索引从0开始），避免默认只显示txt导致其他类型的文件看不到
    int32_t nFileTypeIndex = 0;
    DString fileName;
    if (!m_filePath.IsEmpty() && !m_bSampleDocument) {
        fileName = m_filePath.GetFileName();
    }

    ui::FilePath filePath;
    ui::FileDialog openFileDlg;
    //初始的目录：当前文件所在的目录，或者上一次使用的目录
    ui::FilePath defaultLocation = m_filePath.GetParentPath();
    if (defaultLocation.IsEmpty() || m_bSampleDocument) {
        defaultLocation = m_lastFileDir;
    }
    if (defaultLocation.IsEmpty() || !defaultLocation.IsExistsDirectory()) {
        //默认目录不存在时，使用程序所在目录（避免文件对话框使用无效目录）
        defaultLocation = ui::FilePathUtil::GetCurrentModuleDirectory();
    }
    if (openFileDlg.BrowseForFile(this, filePath, true, fileTypes, nFileTypeIndex, defaultExt, fileName, defaultLocation)) {
        if (LoadFile(filePath)) {
            m_filePath = filePath;
            m_bSampleDocument = false;
            m_lastFileDir = filePath.GetParentPath();
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetModify(false);
            }
            m_statusMessage.clear();
            UpdateSaveStatus();
            UpdateStatusBar();
            UpdateEditButtonStatus();
            UpdateFontStatus();
            UpdateRowSpacingEnabled();
        }
        else {
            ShowErrorMessage(_T("STRID_RICHEDIT_OPEN_FAILED_TEXT"), _T("STRID_RICHEDIT_OPEN_FAILED_TITLE"));
        }
    }
    return true;
}

bool MainForm::OnSaveFile()
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    if (m_bSampleDocument || m_filePath.IsEmpty()) {
        //示例文档或未命名文档，转成另存为
        return OnSaveAsFile();
    }
    if (!m_pRichEdit->GetModify()) {
        return true;
    }
    if (!SaveFile(m_filePath)) {
        ShowErrorMessage(_T("STRID_RICHEDIT_SAVE_FAILED_TEXT"), _T("STRID_RICHEDIT_SAVE_FAILED_TITLE"));
        return false;
    }
    m_pRichEdit->SetModify(false);
    m_statusMessage.clear();
    UpdateSaveStatus();
    UpdateStatusBar();
    return true;
}

bool MainForm::OnSaveAsFile()
{
    bool bRichText = (m_pRichEdit != nullptr) && m_pRichEdit->IsRichText();

    std::vector<ui::FileDialog::FileType> fileTypes;
    fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_ALL_FILES")), _T("*.*") });
    fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_TXT_FILES")), _T("*.txt") });
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    //仅富文本模式支持保存为RTF文件，避免选择后保存失败
    if (bRichText) {
        fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_RTF_FILES")), _T("*.rtf") });
    }
#endif

    DString defaultExt = bRichText ? _T("rtf") : _T("txt");
    int32_t nFileTypeIndex = 1;
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (bRichText) {
        nFileTypeIndex = 2;
    }
#endif
    DString fileName;
    if (!m_filePath.IsEmpty() && !m_bSampleDocument) {
        fileName = m_filePath.GetFileName();
    }
    else {
        fileName = defaultExt == _T("rtf") ? _T("untitled.rtf") : _T("untitled.txt");
    }

    ui::FilePath filePath;
    ui::FileDialog saveFileDlg;
    //初始的目录：当前文件所在的目录，或者上一次使用的目录
    ui::FilePath defaultLocation = m_filePath.GetParentPath();
    if (defaultLocation.IsEmpty() || m_bSampleDocument) {
        defaultLocation = m_lastFileDir;
    }
    if (defaultLocation.IsEmpty() || !defaultLocation.IsExistsDirectory()) {
        //默认目录不存在时，使用程序所在目录（避免文件对话框使用无效目录）
        defaultLocation = ui::FilePathUtil::GetCurrentModuleDirectory();
    }
    if (!saveFileDlg.BrowseForFile(this, filePath, false, fileTypes, nFileTypeIndex, defaultExt, fileName, defaultLocation)) {
        return false;
    }
    if (!SaveFile(filePath)) {
        ShowErrorMessage(_T("STRID_RICHEDIT_SAVE_FAILED_TEXT"), _T("STRID_RICHEDIT_SAVE_FAILED_TITLE"));
        return false;
    }
    m_filePath = filePath;
    m_bSampleDocument = false;
    m_lastFileDir = filePath.GetParentPath();
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetModify(false);
    }
    m_statusMessage.clear();
    UpdateSaveStatus();
    UpdateStatusBar();
    return true;
}

//读取文本文件（自动识别 UTF-8/UTF-16/ANSI 编码），空文件也能正常打开
static bool ReadTextFileContent(const ui::FilePath& filePath, DStringW& text, DString& encodingText)
{
    std::vector<uint8_t> fileData;
    if (!ui::FileUtil::ReadFileData(filePath, fileData)) {
        return false;
    }
    text.clear();
    encodingText = _T("UTF-8");
    if (fileData.empty()) {
        return true;
    }
    ui::CharsetType charsetType = ui::CharsetType::UNKNOWN;
    uint32_t bomSize = 0;
    if (!ui::StringCharset::GetDataAsString((const char*)fileData.data(), (uint32_t)fileData.size(), text, charsetType, bomSize)) {
        return false;
    }
    switch (charsetType) {
    case ui::CharsetType::ANSI:
        encodingText = _T("ANSI");
        break;
    case ui::CharsetType::UTF16_LE:
        encodingText = _T("UTF-16LE");
        break;
    case ui::CharsetType::UTF16_BE:
        encodingText = _T("UTF-16BE");
        break;
    default:
        encodingText = _T("UTF-8");
        break;
    }
    return true;
}

//保存文本文件：原文件已存在时尽量保持原编码，新文件按 UTF-8 保存；空文本也允许保存
static bool WriteTextFileContent(const ui::FilePath& filePath, const DStringW& text, DString& encodingText)
{
    encodingText = _T("UTF-8");
    ui::CharsetType charsetType = ui::CharsetType::UNKNOWN;
    std::vector<uint8_t> oldFileData;
    if (ui::FileUtil::ReadFileData(filePath, oldFileData) && !oldFileData.empty()) {
        charsetType = ui::StringCharset::GetDataCharsetByBOM((const char*)oldFileData.data(), (uint32_t)oldFileData.size());
        if (charsetType == ui::CharsetType::UNKNOWN) {
            charsetType = ui::StringCharset::GetDataCharset((const char*)oldFileData.data(), (uint32_t)oldFileData.size());
        }
        oldFileData.clear();
    }
    if (charsetType == ui::CharsetType::UTF16_LE) {
        encodingText = _T("UTF-16LE");
        return ui::FileUtil::WriteFileData(filePath, text);
    }
    if (charsetType == ui::CharsetType::ANSI) {
#ifdef DUILIB_BUILD_FOR_WIN
        encodingText = _T("ANSI");
        DStringA textA = ui::StringConvert::UnicodeToMBCS(text);
#else
        DStringA textA = ui::StringConvert::WStringToUTF8(text);
#endif
        return ui::FileUtil::WriteFileData(filePath, textA);
    }
    DStringA textA = ui::StringConvert::WStringToUTF8(text);
    return ui::FileUtil::WriteFileData(filePath, textA);
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

bool MainForm::IsRtfFile(const DString& filePath) const
{
    DString fileExt = ui::FilePathUtil::GetFileExtension(filePath);
    return ui::StringUtil::IsEqualNoCase(fileExt, _T("rtf"));
}

bool MainForm::LoadFile(const ui::FilePath& filePath)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    DString filePathLocal = filePath.NativePath();

    //非富文本模式不支持读取RTF文件，避免把RTF二进制当成文本读取
    if (!m_pRichEdit->IsRichText() && IsRtfFile(filePathLocal)) {
        return false;
    }

    //RTF文件使用RichEdit的流接口读取，可以保留格式
    if (m_pRichEdit->IsRichText() && IsRtfFile(filePathLocal)) {
        HANDLE hFile = ::CreateFile(filePathLocal.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        EDITSTREAM es;
        es.dwCookie = (DWORD_PTR)hFile;
        es.dwError = 0;
        es.pfnCallback = StreamReadCallback;
        m_pRichEdit->StreamIn(SF_RTF, es);
        ::CloseHandle(hFile);
        if ((BOOL)es.dwError) {
            return false;
        }
        m_encodingText = _T("RTF");
        return true;
    }

    //普通文本统一按自动识别的编码读取，避免UTF-8/Unicode文件乱码
    DStringW text;
    if (!ReadTextFileContent(filePath, text, m_encodingText)) {
        return false;
    }
    m_bLoadingDocument = true;
    m_pRichEdit->SetText(text);
    m_bLoadingDocument = false;
    return true;
}

bool MainForm::SaveFile(const ui::FilePath& filePath)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    DString filePathLocal = filePath.NativePath();

    //非富文本模式不支持保存为RTF文件
    if (!m_pRichEdit->IsRichText() && IsRtfFile(filePathLocal)) {
        return false;
    }

    //RTF文件使用RichEdit的流接口保存，可以保留格式
    if (m_pRichEdit->IsRichText() && IsRtfFile(filePathLocal)) {
        HANDLE hFile = ::CreateFile(filePathLocal.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        EDITSTREAM es;
        es.dwCookie = (DWORD_PTR)hFile;
        es.dwError = 0;
        es.pfnCallback = StreamWriteCallback;
        m_pRichEdit->StreamOut(SF_RTF, es);
        ::CloseHandle(hFile);
        if ((BOOL)es.dwError) {
            return false;
        }
        m_encodingText = _T("RTF");
        return true;
    }

    //普通文本统一保存，空文本也允许保存
    if (!WriteTextFileContent(filePath, m_pRichEdit->GetText(), m_encodingText)) {
        return false;
    }
    return true;
}

DWORD MainForm::StreamReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb)
{
    ASSERT(dwCookie != 0);
    ASSERT(pcb != nullptr);

    return !::ReadFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, nullptr);
}

DWORD MainForm::StreamWriteCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb)
{
    ASSERT(dwCookie != 0);
    ASSERT(pcb != nullptr);

    return !::WriteFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, nullptr);
}

#else //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

bool MainForm::LoadFile(const ui::FilePath& filePath)
{
    //打开文件
    if (m_pRichEdit == nullptr) {
        return false;
    }
    //非Windows平台不支持RTF格式，按普通文本读取会得到乱码，直接拒绝
    if (ui::StringUtil::IsEqualNoCase(filePath.GetFileExtension(), _T(".rtf"))) {
        return false;
    }
    DStringW text;
    if (!ReadTextFileContent(filePath, text, m_encodingText)) {
        return false;
    }
    m_bLoadingDocument = true;
    m_pRichEdit->SetText(text);
    m_bLoadingDocument = false;
    return true;
}

bool MainForm::SaveFile(const ui::FilePath& filePath)
{
    //保存
    if (m_pRichEdit == nullptr) {
        return false;
    }
    //非Windows平台不支持RTF格式，避免生成扩展名与实际内容不符的文件
    if (ui::StringUtil::IsEqualNoCase(filePath.GetFileExtension(), _T(".rtf"))) {
        return false;
    }
    return WriteTextFileContent(filePath, m_pRichEdit->GetTextW(), m_encodingText);
}

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

bool MainForm::SaveDocument()
{
    return OnSaveFile();
}
