#include "MainForm.h"
#include "FindForm.h"
#include "ReplaceForm.h"
#include "MainThread.h"
#include <ShellApi.h>
#include <commdlg.h>
#include <fstream>

const std::wstring MainForm::kClassName = L"MainForm";

MainForm::MainForm():
	m_pRichEdit(nullptr),
	m_pFindForm(nullptr),
	m_pReplaceForm(nullptr)
{
}

MainForm::~MainForm()
{
}

std::wstring MainForm::GetSkinFolder()
{
	return L"rich_edit";
}

std::wstring MainForm::GetSkinFile()
{
	return L"rich_edit.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
	return kClassName;
}

void MainForm::OnInitWindow()
{
	ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(L"test_url"));
	if (pRichEdit != nullptr) {
		pRichEdit->AttachCustomLinkClick([this, pRichEdit](const ui::EventArgs& args) {
				//点击了超级链接
				if (args.pSender == pRichEdit) {
					const wchar_t* pUrl = (const wchar_t*)args.wParam;
					if (pUrl != nullptr) {
						::ShellExecute(GetHWND(), L"open", pUrl, NULL, NULL, SW_SHOWNORMAL);
					}
				}
				return true;
			});
	}
	m_pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(L"rich_edit"));
	ASSERT(m_pRichEdit != nullptr);
	m_findReplace.SetRichEdit(m_pRichEdit);
	LoadRichEditData();

	//文件操作：打开、保存、另存
	ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(L"open_file"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					this->OnOpenFile();
				}
				return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"save_file"));
	if (pButton != nullptr) {
		m_saveBtnText = pButton->GetText();
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					this->OnSaveFile();
				}
				return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"save_as_file"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					this->OnSaveAsFile();
				}
				return true;
			});
	}

	//编辑操作
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_copy"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Copy();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_cut"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Cut();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_paste"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Paste();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_delete"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Clear();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_sel_all"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->SetSelAll();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_sel_none"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->SetSelNone();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_undo"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Undo();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_redo"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				if (m_pRichEdit != nullptr) {
					m_pRichEdit->Redo();
					UpdateSaveStatus();
				}
			}
			return true;
			});
	}

	//查找操作
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_find_text"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				OnFindText();
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_find_next"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				OnFindNext();
			}
			return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_replace_text"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
			if (args.pSender == pButton) {
				OnReplaceText();
			}
			return true;
			});
	}
}

LRESULT MainForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

LRESULT MainForm::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	LRESULT lResult = __super::OnKeyUp(uMsg, wParam, lParam, bHandled);
	bool bControlDown = ::GetKeyState(VK_CONTROL) < 0;
	bool bShiftDown = ::GetKeyState(VK_SHIFT) < 0;
	if (bControlDown) {
		if ((wParam == 'O') && !bShiftDown){
			//打开
			OnOpenFile();
		}
		else if (wParam == 'S') {
			if (bShiftDown) {
				//另存为
				OnSaveAsFile();				
			}
			else {
				//保存
				OnSaveFile();
			}
		}
		if (!bShiftDown) {
			if (wParam == 'F') {
				//查找
				OnFindText();
			}
			else if (wParam == 'H') {
				//替换
				OnReplaceText();
			}
		}
	}
	if (!bControlDown && !bShiftDown && (::GetKeyState(VK_F3) < 0)) {
		//查找下一个
		OnFindNext();
	}
	return lResult;
}

LRESULT MainForm::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	LRESULT lResult = __super::OnKeyUp(uMsg, wParam, lParam, bHandled);
	UpdateSaveStatus();
	return lResult;
}

void MainForm::UpdateSaveStatus()
{
	if (m_pRichEdit != nullptr) {
		ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(L"save_file"));
		if (m_pRichEdit->GetModify()) {
			if (pButton != nullptr) {
				pButton->SetText(m_saveBtnText + L"*");
			}
		}
		else {
			if (pButton != nullptr) {
				pButton->SetText(m_saveBtnText);
			}
		}
	}
}

void MainForm::LoadRichEditData()
{
	std::streamoff length = 0;
	std::string xml;
	std::wstring controls_xml = ui::GlobalManager::Instance().GetResourcePath() + GetResourcePath() + GetSkinFile();

	std::ifstream ifs(controls_xml.c_str());
	if (ifs.is_open()) {
		ifs.seekg(0, std::ios_base::end);
		length = ifs.tellg();
		ifs.seekg(0, std::ios_base::beg);

		xml.resize(static_cast<unsigned int>(length) + 1);
		ifs.read(&xml[0], length);
		ifs.close();
	}
	std::wstring xmlU;
	ui::StringHelper::MBCSToUnicode(xml.c_str(), xmlU, CP_UTF8);

	if (m_pRichEdit != nullptr) {
		m_pRichEdit->SetText(xmlU);
		m_pRichEdit->SetFocus();
		m_pRichEdit->HomeUp();
		m_pRichEdit->SetModify(false);
		m_filePath = controls_xml;
	}
}

LPCTSTR g_defualtFilter = L"所有文件 (*.*)\0*.*\0"
						  L"文本文件 (*.txt)\0*.txt\0"
						  L"RTF文件 (*.rtf)\0*.rtf\0"
						  L"";

void MainForm::OnOpenFile()
{
	TCHAR szFileTitle[_MAX_FNAME] = {0,};   // contains file title after return
	TCHAR szFileName[_MAX_PATH] = {0,};     // contains full path name after return

	OPENFILENAME ofn = {0, };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetHWND();

	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = _MAX_FNAME;
	ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;
	ofn.lpstrFilter = g_defualtFilter;
	ofn.hInstance = ::GetModuleHandle(NULL);

	BOOL bRet = ::GetOpenFileName(&ofn);
	if (bRet) {
		if (LoadFile(szFileName)) {
			m_filePath = szFileName;
			if (m_pRichEdit != nullptr) {
				m_pRichEdit->SetModify(false);
				UpdateSaveStatus();
			}
		}
	}
}

void MainForm::OnSaveFile()
{
	if (m_pRichEdit != nullptr) {
		if (m_pRichEdit->GetModify()) {
			if (SaveFile(m_filePath)) {
				m_pRichEdit->SetModify(false);
				UpdateSaveStatus();
			}
		}
	}
}

void MainForm::OnSaveAsFile()
{
	TCHAR szFileTitle[_MAX_FNAME] = { 0, };   // contains file title after return
	TCHAR szFileName[_MAX_PATH] = { 0, };     // contains full path name after return
	wcscpy_s(szFileName, m_filePath.c_str());

	OPENFILENAME ofn = { 0, };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetHWND();

	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = _MAX_FNAME;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLESIZING;
	ofn.lpstrFilter = g_defualtFilter;
	ofn.hInstance = ::GetModuleHandle(NULL);

	BOOL bRet = ::GetSaveFileName(&ofn);
	if (bRet) {
		if (SaveFile(szFileName)) {
			m_filePath = szFileName;
			if (m_pRichEdit != nullptr) {
				m_pRichEdit->SetModify(false);
				UpdateSaveStatus();
			}
		}
	}
}

bool MainForm::LoadFile(const std::wstring& filePath)
{
	if (m_pRichEdit == nullptr) {
		return false;
	}
	HANDLE hFile = ::CreateFile(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	EDITSTREAM es;
	es.dwCookie = (DWORD_PTR)hFile;
	es.dwError = 0;
	es.pfnCallback = StreamReadCallback;
	UINT nFormat = IsRtfFile(filePath) ? SF_RTF : SF_TEXT;
	m_pRichEdit->StreamIn(nFormat, es);
	::CloseHandle(hFile);
	return !(BOOL)es.dwError;
}

bool MainForm::SaveFile(const std::wstring& filePath)
{
	if (m_pRichEdit == nullptr) {
		return false;
	}
	HANDLE hFile = ::CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	EDITSTREAM es;
	es.dwCookie = (DWORD_PTR)hFile;
	es.dwError = 0;
	es.pfnCallback = StreamWriteCallback;
	UINT nFormat = IsRtfFile(filePath) ? SF_RTF : SF_TEXT;
	m_pRichEdit->StreamOut(nFormat, es);
	::CloseHandle(hFile);
	return !(BOOL)es.dwError;
}

bool MainForm::IsRtfFile(const std::wstring& filePath) const
{
	std::wstring fileExt;
	size_t pos = filePath.find_last_of(L".");
	if (pos != std::wstring::npos) {
		fileExt = filePath.substr(pos);
		fileExt = ui::StringHelper::MakeLowerString(fileExt);
	}
	return fileExt == L".rtf";
}

DWORD MainForm::StreamReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb)
{
	ASSERT(dwCookie != 0);
	ASSERT(pcb != NULL);

	return !::ReadFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, NULL);
}

DWORD MainForm::StreamWriteCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb)
{
	ASSERT(dwCookie != 0);
	ASSERT(pcb != NULL);

	return !::WriteFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, NULL);
}

void MainForm::OnFindText()
{
	if (m_pFindForm == nullptr) {
		m_pFindForm = new FindForm(this);
		m_pFindForm->CreateWnd(GetHWND(), FindForm::kClassName, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_LAYERED);
		m_pFindForm->CenterWindow();
		m_pFindForm->ShowWindow();

		m_pFindForm->AttachWindowClose([this](const ui::EventArgs& args) {
				m_pFindForm = nullptr;
				return true;
			});
	}
	else {
		m_pFindForm->ActiveWindow();
	}
}

void MainForm::OnFindNext()
{
	m_findReplace.FindNext();
}

void MainForm::OnReplaceText()
{
	if (m_pReplaceForm == nullptr) {
		m_pReplaceForm = new ReplaceForm(this);
		m_pReplaceForm->CreateWnd(GetHWND(), ReplaceForm::kClassName, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_LAYERED);
		m_pReplaceForm->CenterWindow();
		m_pReplaceForm->ShowWindow();
		m_pReplaceForm->AttachWindowClose([this](const ui::EventArgs& args) {
				m_pReplaceForm = nullptr;
				return true;
			});
	}
	else {
		m_pReplaceForm->ActiveWindow();
	}
}

void MainForm::FindRichText(const std::wstring& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog)
{
	m_findReplace.FindRichText(findText, bFindDown, bMatchCase, bMatchWholeWord, hWndDialog);
}

void MainForm::ReplaceRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog)
{
	if (m_findReplace.ReplaceRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, hWndDialog)) {
		if (m_pRichEdit != nullptr) {
			m_pRichEdit->SetModify(true);
			UpdateSaveStatus();
		}
	}
}

void MainForm::ReplaceAllRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog)
{
	if (m_findReplace.ReplaceAllRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, hWndDialog)) {
		if (m_pRichEdit != nullptr) {
			m_pRichEdit->SetModify(true);
			UpdateSaveStatus();
		}
	}
}

ui::RichEdit* MainForm::GetRichEdit() const
{
	return m_pRichEdit;
}
