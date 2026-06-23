#include "RichEditDropTarget_Windows.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Control/RichEdit2.h"
#include "duilib/Control/RichEditDropTargetHelper.h"
#include "duilib/Core/ControlDropTargetImpl_Windows.h"
#include "duilib/Core/ControlDropTargetUtils.h"
#include "duilib/Core/DpiManager.h"

#include <oleidl.h>
#include <shellapi.h>

namespace ui 
{
RichEditDropTarget_Windows::RichEditDropTarget_Windows(RichEdit2* pRichEdit) :
    m_pRichEdit(pRichEdit),
    m_pDataObj(nullptr),
    m_nStartChar(0),
    m_nEndChar(0)
{
}

RichEditDropTarget_Windows::~RichEditDropTarget_Windows()
{
    ClearDragStatus();
}

void RichEditDropTarget_Windows::ClearDragStatus()
{
    if (m_pDataObj != nullptr) {
        ((IDataObject*)m_pDataObj)->Release();
        m_pDataObj = nullptr;
    }
    m_dropTextList.clear();
    m_dropFileList.clear();
}

int32_t RichEditDropTarget_Windows::DragEnter(void* pDataObj, uint32_t /*grfKeyState*/, const UiPoint& /*pt*/, uint32_t* /*pdwEffect*/)
{
    HRESULT hr = S_FALSE;
    if ((m_pRichEdit == nullptr) || m_pRichEdit->IsReadOnly() || !m_pRichEdit->IsEnabled() || m_pRichEdit->IsPasswordMode()) {
        return hr;
    }
    IDataObject* pDataObject = (IDataObject*)pDataObj;
    if (pDataObject != nullptr) {
        pDataObject->AddRef();
    }
    ClearDragStatus();    
    m_pDataObj = pDataObject;

    if (pDataObj != nullptr) {
        ControlDropTargetImpl_Windows::ParseWindowsDataObject(pDataObj, m_dropTextList, m_dropFileList);
    }

    m_pRichEdit->GetSel(m_nStartChar, m_nEndChar);
    m_scrollPos = m_pRichEdit->GetScrollPos();

    if (!m_dropFileList.empty()) {
        if (!m_pRichEdit->IsEnableDropFile()) {
            //不支持文件拖放操作
            return hr;
        }
        //支持文件拖放操作，直接返回
        return S_OK;
    }
    else if (!m_dropTextList.empty()) {
        //支持文本拖放操作，直接返回
        return S_OK;
    }
    return hr;
}

int32_t RichEditDropTarget_Windows::DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect)
{
    HRESULT hr = S_FALSE;
    if ((m_pRichEdit == nullptr) || m_pRichEdit->IsReadOnly() || !m_pRichEdit->IsEnabled() || m_pRichEdit->IsPasswordMode()) {
        return hr;
    }
    if (m_dropFileList.empty() && m_dropTextList.empty()) {
        return hr;
    }

    if (!m_dropFileList.empty()) {
        if (!m_pRichEdit->IsEnableDropFile()) {
            //不支持文件拖放操作
            return hr;
        }
        //支持文件拖放操作，判断是否满足过滤条件
        DString fileTypes = m_pRichEdit->GetDropFileTypes();
        if (!ControlDropTargetUtils::IsFilteredFileTypes(fileTypes, m_dropFileList)) {
            //文件类型不满足过滤条件
            return hr;
        }
        //支持文件拖放操作，直接返回
        return S_OK;
    }

    UiPoint clientPt = pt;
    m_pRichEdit->ScreenToClient(clientPt);
    if (!CheckDropText(clientPt)) {
        //拖入文本操作：进行有效性判断
        return hr;
    }

    if (!m_pRichEdit->IsFocused()) {
        //必须设置为焦点控件，否则CharFromPos会失败
        m_pRichEdit->SetFocus();
    }
    int32_t pos = m_pRichEdit->CharFromPos(clientPt);
    if (pos >= 0) {
        UiPoint charPt = m_pRichEdit->PosFromChar(pos);
        m_pRichEdit->SetCaretPos(charPt.x, charPt.y);
        m_pRichEdit->ShowCaret(true);

        //检查是否需要滚动
        CheckTextScroll(clientPt);

        if (pdwEffect != nullptr) {
            //操作：按住Control键时，是复制，否则移动
            if (grfKeyState & MK_CONTROL) {
                *pdwEffect = DROPEFFECT_COPY;
            }
            else {
                *pdwEffect = DROPEFFECT_MOVE;
            }
        }
        return S_OK;
    }
    return hr;
}

int32_t RichEditDropTarget_Windows::DragLeave(void)
{
    if (m_pRichEdit != nullptr) {
        //取消拖动时，恢复原来的滚动位置
        m_pRichEdit->SetSel(m_nStartChar, m_nEndChar);
        m_pRichEdit->SetScrollPos(m_scrollPos);
    }
    ClearDragStatus();
    return S_OK;
}

int32_t RichEditDropTarget_Windows::Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect)
{
    ASSERT(m_pDataObj == pDataObj);
    if (m_pDataObj != pDataObj) {
        return S_FALSE;
    }

    HRESULT hr = S_FALSE;
    if ((m_pRichEdit == nullptr) || m_pRichEdit->IsReadOnly() || !m_pRichEdit->IsEnabled() || m_pRichEdit->IsPasswordMode()) {
        ClearDragStatus();
        return hr;
    }
    if (m_dropFileList.empty() && m_dropTextList.empty()) {
        ClearDragStatus();
        return hr;
    }

    if (!m_dropFileList.empty()) {
        //拖入文件操作
        if (!m_pRichEdit->IsEnableDropFile()) {
            ClearDragStatus();
			//不支持文件拖放操作
            return hr;
        }
        //支持文件拖放操作，判断是否满足过滤条件
        DString fileTypes = m_pRichEdit->GetDropFileTypes();
        if (!ControlDropTargetUtils::IsFilteredFileTypes(fileTypes, m_dropFileList)) {
            ClearDragStatus();
			//文件类型不满足过滤条件
            return hr;
        }

        //移除不支持的文件
        std::vector<DString> filteredFileList = m_dropFileList;
        ControlDropTargetUtils::RemoveUnsupportedFiles(filteredFileList, fileTypes);
        if (filteredFileList.empty()) {
            ClearDragStatus();
            return hr;
        }

        ControlDropData_Windows data;
        data.m_pDataObj = m_pDataObj;
        data.m_ptClientX = 0;
        data.m_ptClientY = 0;
        data.m_dwEffect = (pdwEffect != nullptr) ? *pdwEffect : 0;
        data.m_hResult = S_OK;
        data.m_bHandled = false;
        data.m_fileList = filteredFileList;

        EventArgs msg;
        msg.SetSender(m_pRichEdit);
        msg.eventType = EventType::kEventDropData;
        msg.vkCode = VirtualKeyCode::kVK_None;
        msg.wParam = kControlDropTypeWindows;
        msg.lParam = (LPARAM)&data;
        msg.modifierKey = 0;
        msg.eventData = 0;

        m_pRichEdit->SendEventMsg(msg);
        if (pdwEffect != nullptr) {
            *pdwEffect = data.m_dwEffect;
        }
        ClearDragStatus();
        return data.m_hResult;
    }

    UiPoint clientPt = pt;
    m_pRichEdit->ScreenToClient(clientPt);
    if (!CheckDropText(clientPt)) {
        //校验拖入的文本
        ClearDragStatus();
        return hr;
    }

    int32_t nDropPos = m_pRichEdit->CharFromPos(clientPt);
    if ((nDropPos >= 0) && !m_dropTextList.empty()) {
        DString dropText;
        for (size_t i = 0; i < m_dropTextList.size(); ++i) {
            if (i > 0) {
                dropText += _T("\n");
            }
            dropText += m_dropTextList[i];
        }
        m_pRichEdit->SetDropTextPosition(nDropPos);
        m_pRichEdit->SetSel(nDropPos, nDropPos);
        m_pRichEdit->ReplaceSel(dropText, true);
        hr = S_OK;
    }
    if (pdwEffect != nullptr) {
        //操作：按住Control键时，是复制，否则移动
        if (grfKeyState & MK_CONTROL) {
            *pdwEffect = DROPEFFECT_COPY;
        }
        else {
            *pdwEffect = DROPEFFECT_MOVE;
        }
    }
    ClearDragStatus();
    return hr;
}

bool RichEditDropTarget_Windows::CheckDropText(const UiPoint& clientPt) const
{
    RichEditDropTargetHelper dropTargetHelper(m_pRichEdit, m_dropTextList);
    return dropTargetHelper.CheckDropText(clientPt);
}

void RichEditDropTarget_Windows::CheckTextScroll(const UiPoint& clientPt)
{
    RichEditDropTargetHelper dropTargetHelper(m_pRichEdit, m_dropTextList);
    dropTargetHelper.CheckTextScroll(clientPt);
}

} // namespace ui

#endif // defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
