#include "ControlDropTargetImpl_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Core/Control.h"
#include "duilib/Core/ControlDropTargetUtils.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringCharset.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePath.h"
#include <oleidl.h>
#include <shellapi.h>

namespace ui 
{
ControlDropTargetImpl_Windows::ControlDropTargetImpl_Windows(Control* pControl):
    m_pControl(pControl),
    m_pDataObj(nullptr)
{
}

ControlDropTargetImpl_Windows::~ControlDropTargetImpl_Windows()
{
    if (m_pDataObj != nullptr) {
        ((IDataObject*)m_pDataObj)->Release();
        m_pDataObj = nullptr;
    }
}

void ControlDropTargetImpl_Windows::ParseWindowsDataObject(void* pDataObj, std::vector<DString>& textList, std::vector<DString>& fileList)
{
    if (pDataObj == nullptr) {
        return;
    }
    IDataObject* data_object = (IDataObject*)pDataObj;
    IEnumFORMATETC* enumFormats = nullptr;
    HRESULT res = data_object->EnumFormatEtc(DATADIR_GET, &enumFormats);
    if (res != S_OK) {
        return;
    }
    enumFormats->Reset();
    const int kCelt = 12;

    std::vector<DString> unicodeTextList;
    std::vector<DString> ansiTextList;

    ULONG celtFetched;
    do {
        celtFetched = kCelt;
        FORMATETC rgelt[kCelt];
        res = enumFormats->Next(kCelt, rgelt, &celtFetched);
        for (unsigned i = 0; i < celtFetched; i++) {
            CLIPFORMAT format = rgelt[i].cfFormat;
            if (!(format == CF_UNICODETEXT || format == CF_TEXT || format == CF_HDROP) ||
                (rgelt[i].tymed != TYMED_HGLOBAL)) {
                continue;
            }
            STGMEDIUM medium;
            if (data_object->GetData(&rgelt[i], &medium) == S_OK) {
                if (!medium.hGlobal) {
                    ::ReleaseStgMedium(&medium);
                    continue;
                }
                void* hGlobal = GlobalLock(medium.hGlobal);
                if (!hGlobal) {
                    ::ReleaseStgMedium(&medium);
                    continue;
                }
                if (format == CF_UNICODETEXT) {
                    unicodeTextList.clear();
                    DStringW text = (std::wstring::value_type*)hGlobal;
                    if (!text.empty()) {
                        if (!text.empty()) {
                            std::list<std::wstring> lines = StringUtil::Split(text, L"\r\n");
                            for (const std::wstring& line : lines) {
                                if (!line.empty()) {
                                    unicodeTextList.push_back(StringConvert::WStringToT(line));
                                }
                            }
                        }
                    }
                }
                else if (format == CF_TEXT) {
                    //编码类型：可能是Ansi或者UTF8等（Edge网页拖出的文本类型是UTF8的,NotePad拖出的文本类型是Ansi的）
                    ansiTextList.clear();                    
                    DStringA rawText = (std::string::value_type*)hGlobal;
                    CharsetType charset = StringCharset::GetDataCharset(rawText.data(), (uint32_t)rawText.size());
                    DString text;
                    if (charset == CharsetType::ANSI) {
                        text = StringConvert::MBCSToT(rawText);
                    }
                    else if (charset == CharsetType::UTF8) {
                        text = StringConvert::UTF8ToT(rawText);
                    }
                    if (!text.empty()) {
                        std::list<DString> lines = StringUtil::Split(text, _T("\r\n"));
                        for (const DString& line : lines) {
                            if (!line.empty()) {
                                ansiTextList.push_back(line);
                            }
                        }
                    }
                }
                else if (format == CF_HDROP) {
                    fileList.clear();
                    HDROP hdrop = (HDROP)hGlobal;
                    const int kMaxFilenameLen = 4096;
                    const unsigned num_files = ::DragQueryFileW(hdrop, 0xffffffff, nullptr, 0);
                    for (unsigned int x = 0; x < num_files; ++x) {
                        wchar_t fileName[kMaxFilenameLen] = {0};
                        if (!::DragQueryFileW(hdrop, x, fileName, kMaxFilenameLen)) {
                            continue;
                        }
                        DStringW fileNameW = fileName;
                        if (!fileNameW.empty()) {
                            fileList.push_back(StringConvert::WStringToT(fileNameW));
                        }
                    }
                }
                if (medium.hGlobal) {
                    ::GlobalUnlock(medium.hGlobal);
                }
                if (format == CF_HDROP) {
                    ::DragFinish((HDROP)hGlobal);
                }
                else {
                    ::ReleaseStgMedium(&medium);
                }
            }
        }
    } while (res == S_OK);
    enumFormats->Release();

    if (!unicodeTextList.empty()) {
        textList.swap(unicodeTextList);
    }
    else if (!ansiTextList.empty()) {
        textList.swap(ansiTextList);
    }
}

void ControlDropTargetImpl_Windows::ClearDragStatus()
{
    if (m_pDataObj != nullptr) {
        ((IDataObject*)m_pDataObj)->Release();
        m_pDataObj = nullptr;
    }
    m_textList.clear();
    m_fileList.clear();
}

int32_t ControlDropTargetImpl_Windows::DragEnter(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect)
{
    IDataObject* pDataObject = (IDataObject*)pDataObj;
    if (pDataObject != nullptr) {
        pDataObject->AddRef();
    }
    ClearDragStatus();
    m_pDataObj = pDataObject;
    if (pDataObj != nullptr) {
        ParseWindowsDataObject(pDataObj, m_textList, m_fileList);
    }

    if (m_pControl != nullptr) {
        ControlDropData_Windows data;
        data.m_pDataObj = pDataObj;
        data.m_grfKeyState = grfKeyState;
        data.m_screenX = pt.x;
        data.m_screenY = pt.y;
        data.m_dwEffect = (pdwEffect != nullptr) ? *pdwEffect : 0;
        data.m_hResult = S_OK;

        data.m_textList = m_textList;
        data.m_fileList = m_fileList;

        EventArgs msg;
        msg.SetSender(m_pControl.get());
        msg.eventType = EventType::kEventDropEnter;
        msg.vkCode = VirtualKeyCode::kVK_None;
        msg.wParam = kControlDropTypeWindows;
        msg.lParam = (LPARAM)&data;
        msg.ptMouse = pt;
        m_pControl->ScreenToClient(msg.ptMouse);
        msg.modifierKey = 0;
        msg.eventData = 0;

        m_pControl->SendEventMsg(msg);
        if (pdwEffect != nullptr) {
            *pdwEffect = data.m_dwEffect;
        }
        return data.m_hResult;
    }
    return S_OK;
}

int32_t ControlDropTargetImpl_Windows::DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect)
{
    if (m_pControl != nullptr) {
        if (!m_fileList.empty()) {
            //当前执行的是拖放文件操作
            if (!m_pControl->IsEnableDropFile()) {
                //不支持文件拖放
                return S_FALSE;
            }
            //支持文件拖放操作，判断是否满足过滤条件
            DString fileTypes = m_pControl->GetDropFileTypes();
            if (!ControlDropTargetUtils::IsFilteredFileTypes(fileTypes, m_fileList)) {
                //文件类型不满足过滤条件
                return S_FALSE;
            }
        }

        ControlDropData_Windows data;
        data.m_pDataObj = m_pDataObj;
        data.m_grfKeyState = grfKeyState;
        data.m_screenX = pt.x;
        data.m_screenY = pt.y;
        data.m_dwEffect = (pdwEffect != nullptr) ? *pdwEffect : 0;
        data.m_hResult = S_OK;

        data.m_textList = m_textList;        
        data.m_fileList = m_fileList;

        EventArgs msg;
        msg.SetSender(m_pControl.get());
        msg.eventType = EventType::kEventDropOver;
        msg.vkCode = VirtualKeyCode::kVK_None;
        msg.wParam = kControlDropTypeWindows;
        msg.lParam = (LPARAM)&data;
        msg.ptMouse = pt;
        m_pControl->ScreenToClient(msg.ptMouse);
        msg.modifierKey = 0;
        msg.eventData = 0;

        m_pControl->SendEventMsg(msg);
        if (pdwEffect != nullptr) {
            *pdwEffect = data.m_dwEffect;
        }
        return data.m_hResult;
    }
    return S_OK;
}

int32_t ControlDropTargetImpl_Windows::DragLeave(void)
{
    ClearDragStatus();
    if (m_pControl != nullptr) {
        m_pControl->SendEvent(EventType::kEventDropLeave);
    }
    return S_OK;
}

int32_t ControlDropTargetImpl_Windows::Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect)
{
    ASSERT(m_pDataObj == pDataObj);
    if (m_pDataObj != pDataObj) {
        return S_FALSE;
    }

    if (m_pControl != nullptr) {
        if (!m_fileList.empty()) {
            //当前执行的是拖放文件操作
            if (!m_pControl->IsEnableDropFile()) {
                //不支持文件拖放
                ClearDragStatus();
                return S_FALSE;
            }
            //支持文件拖放操作，判断是否满足过滤条件
            DString fileTypes = m_pControl->GetDropFileTypes();
            if (!ControlDropTargetUtils::IsFilteredFileTypes(fileTypes, m_fileList)) {
                //文件类型不满足过滤条件
                ClearDragStatus();
                return S_FALSE;
            }
        }

        ControlDropData_Windows data;
        data.m_pDataObj = m_pDataObj;
        data.m_grfKeyState = grfKeyState;
        data.m_screenX = pt.x;
        data.m_screenY = pt.y;
        data.m_dwEffect = (pdwEffect != nullptr) ? *pdwEffect : 0;
        data.m_hResult = S_OK;

        data.m_textList = m_textList;
        data.m_fileList = m_fileList;

        EventArgs msg;
        msg.SetSender(m_pControl.get());
        msg.eventType = EventType::kEventDropData;
        msg.vkCode = VirtualKeyCode::kVK_None;
        msg.wParam = kControlDropTypeWindows;
        msg.lParam = (LPARAM)&data;
        msg.ptMouse = pt;
        m_pControl->ScreenToClient(msg.ptMouse);
        msg.modifierKey = 0;
        msg.eventData = 0;

        m_pControl->SendEventMsg(msg);
        if (pdwEffect != nullptr) {
            *pdwEffect = data.m_dwEffect;
        }

        ClearDragStatus();
        return data.m_hResult;
    }
    else {
        ClearDragStatus();
        return S_OK;
    }
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
