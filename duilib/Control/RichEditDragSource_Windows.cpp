#include "RichEditDragSource_Windows.h"

#if defined(DUILIB_BUILD_FOR_WIN)

#include <shellapi.h>

namespace ui 
{

////////////////////////////////////////////////////////////////////////////////
// RichEditDragSource_Windows

RichEditDragSource_Windows* RichEditDragSource_Windows::Create()
{
    return new RichEditDragSource_Windows(nullptr);
}

RichEditDragSource_Windows::RichEditDragSource_Windows(RichEdit2* pRichEdit)
    : m_pRichEdit(pRichEdit)
    , m_refCount(0)
{
}

HRESULT RichEditDragSource_Windows::QueryInterface(REFIID iid, void** object)
{
    *object = nullptr;
    if (IsEqualIID(iid, IID_IUnknown)) {
        IUnknown* obj = this;
        *object = obj;
    }
    else if (IsEqualIID(iid, IID_IDropSource)) {
        IDropSource* obj = this;
        *object = obj;
    }
    else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG RichEditDragSource_Windows::AddRef()
{
    return ++m_refCount;
}

ULONG RichEditDragSource_Windows::Release()
{
    if (--m_refCount == 0) {
        delete this;
        return 0U;
    }
    return m_refCount;
}

HRESULT RichEditDragSource_Windows::GiveFeedback(DWORD /*dwEffect*/)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}

HRESULT RichEditDragSource_Windows::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    if (fEscapePressed) {
        return DRAGDROP_S_CANCEL;
    }

    if (!(grfKeyState & MK_LBUTTON)) {
        return DRAGDROP_S_DROP;
    }

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// RichEditDataObject_Windows

RichEditDataObject_Windows* RichEditDataObject_Windows::Create(const DStringW& text)
{
    return new RichEditDataObject_Windows(text);
}

RichEditDataObject_Windows::RichEditDataObject_Windows(const DStringW& text)
    : m_refCount(0)
    , m_nNumFormats(0)
    , m_pFormatEtc(nullptr)
    , m_pStgMedium(nullptr)
{
    AddRef();

    m_nNumFormats = 1;
    m_pFormatEtc = new FORMATETC[m_nNumFormats];
    m_pStgMedium = new STGMEDIUM[m_nNumFormats];

    FORMATETC fmtetc = { 0, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    fmtetc.cfFormat = CF_UNICODETEXT;
    m_pFormatEtc[0] = fmtetc;

    SIZE_T bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL handle = GlobalAlloc(GPTR, static_cast<SIZE_T>(bytes));
    if (handle) {
        memcpy(handle, text.c_str(), bytes);
    }

    m_pStgMedium[0].hGlobal = handle;
    m_pStgMedium[0].tymed = TYMED_HGLOBAL;
    m_pStgMedium[0].pUnkForRelease = nullptr;
}

RichEditDataObject_Windows::~RichEditDataObject_Windows()
{
    if (m_pFormatEtc != nullptr) {
        for (int i = 0; i < m_nNumFormats; i++) {
            if (m_pFormatEtc[i].ptd != nullptr) {
                CoTaskMemFree(m_pFormatEtc[i].ptd);
            }
        }
        delete[] m_pFormatEtc;
        m_pFormatEtc = nullptr;
    }

    if (m_pStgMedium != nullptr) {
        for (int i = 0; i < m_nNumFormats; i++) {
            if (m_pStgMedium[i].hGlobal != nullptr) {
                GlobalFree(m_pStgMedium[i].hGlobal);
            }
        }
        delete[] m_pStgMedium;
        m_pStgMedium = nullptr;
    }
}

HRESULT RichEditDataObject_Windows::QueryInterface(REFIID iid, void** object)
{
    *object = nullptr;
    if (IsEqualIID(iid, IID_IUnknown)) {
        IUnknown* obj = this;
        *object = obj;
    }
    else if (IsEqualIID(iid, IID_IDataObject)) {
        IDataObject* obj = this;
        *object = obj;
    }
    else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG RichEditDataObject_Windows::AddRef()
{
    return ++m_refCount;
}

ULONG RichEditDataObject_Windows::Release()
{
    if (--m_refCount == 0) {
        delete this;
        return 0U;
    }
    return m_refCount;
}

int RichEditDataObject_Windows::LookupFormatEtc(FORMATETC* pFormatEtc)
{
    for (int i = 0; i < m_nNumFormats; i++) {
        if ((m_pFormatEtc[i].tymed & pFormatEtc->tymed) &&
            m_pFormatEtc[i].cfFormat == pFormatEtc->cfFormat &&
            m_pFormatEtc[i].dwAspect == pFormatEtc->dwAspect) {
            return i;
        }
    }
    return -1;
}

HGLOBAL RichEditDataObject_Windows::DupGlobalMem(HGLOBAL hMem)
{
    SIZE_T len = GlobalSize(hMem);
    PVOID source = GlobalLock(hMem);
    PVOID dest = GlobalAlloc(GMEM_FIXED, len);

    memcpy(dest, source, len);
    GlobalUnlock(hMem);
    return (HGLOBAL)dest;
}

HRESULT RichEditDataObject_Windows::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
    int idx = LookupFormatEtc(pFormatEtc);
    if (idx == -1) {
        return DV_E_FORMATETC;
    }

    pMedium->tymed = m_pFormatEtc[idx].tymed;
    pMedium->pUnkForRelease = nullptr;

    switch (m_pFormatEtc[idx].tymed) {
    case TYMED_HGLOBAL:
        pMedium->hGlobal = DupGlobalMem(m_pStgMedium[idx].hGlobal);
        break;
    default:
        return DV_E_FORMATETC;
    }
    return S_OK;
}

HRESULT RichEditDataObject_Windows::GetDataHere(FORMATETC* /*pFormatEtc*/, STGMEDIUM* /*pMedium*/)
{
    return E_NOTIMPL;
}

HRESULT RichEditDataObject_Windows::QueryGetData(FORMATETC* pFormatEtc)
{
    return (LookupFormatEtc(pFormatEtc) == -1) ? DV_E_FORMATETC : S_OK;
}

HRESULT RichEditDataObject_Windows::GetCanonicalFormatEtc(FORMATETC* /*pFormatEtcIn*/, FORMATETC* pFormatEtcOut)
{
    pFormatEtcOut->ptd = nullptr;
    return E_NOTIMPL;
}

HRESULT RichEditDataObject_Windows::SetData(FORMATETC* /*pFormatEtc*/, STGMEDIUM* /*pMedium*/, BOOL /*fRelease*/)
{
    return E_NOTIMPL;
}

HRESULT RichEditDataObject_Windows::EnumFormatEtc(DWORD /*dwDirection*/, IEnumFORMATETC** ppEnumFormatEtc)
{
    return RichEditEnumFormatEtc_Windows::CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);
}

HRESULT RichEditDataObject_Windows::DAdvise(FORMATETC* /*pFormatEtc*/, DWORD /*advf*/, IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/)
{
    return E_NOTIMPL;
}

HRESULT RichEditDataObject_Windows::DUnadvise(DWORD /*dwConnection*/)
{
    return E_NOTIMPL;
}

HRESULT RichEditDataObject_Windows::EnumDAdvise(IEnumSTATDATA** /*ppEnumAdvise*/)
{
    return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////
// RichEditEnumFormatEtc_Windows

HRESULT RichEditEnumFormatEtc_Windows::CreateEnumFormatEtc(UINT cfmt, FORMATETC* afmt, IEnumFORMATETC** ppEnumFormatEtc)
{
    if (cfmt == 0 || afmt == nullptr || ppEnumFormatEtc == nullptr) {
        return E_INVALIDARG;
    }

    *ppEnumFormatEtc = new RichEditEnumFormatEtc_Windows(afmt, cfmt);
    return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
}

RichEditEnumFormatEtc_Windows::RichEditEnumFormatEtc_Windows(FORMATETC* pFormatEtc, int nNumFormats)
    : m_refCount(0)
    , m_nIndex(0)
    , m_nNumFormats(nNumFormats)
    , m_pFormatEtc(nullptr)
{
    AddRef();

    m_pFormatEtc = new FORMATETC[nNumFormats];
    for (int i = 0; i < nNumFormats; i++) {
        DeepCopyFormatEtc(&m_pFormatEtc[i], &pFormatEtc[i]);
    }
}

RichEditEnumFormatEtc_Windows::~RichEditEnumFormatEtc_Windows()
{
    if (m_pFormatEtc != nullptr) {
        for (ULONG i = 0; i < m_nNumFormats; i++) {
            if (m_pFormatEtc[i].ptd != nullptr) {
                CoTaskMemFree(m_pFormatEtc[i].ptd);
            }
        }
        delete[] m_pFormatEtc;
        m_pFormatEtc = nullptr;
    }
}

HRESULT RichEditEnumFormatEtc_Windows::QueryInterface(REFIID iid, void** object)
{
    *object = nullptr;
    if (IsEqualIID(iid, IID_IUnknown)) {
        IUnknown* obj = this;
        *object = obj;
    }
    else if (IsEqualIID(iid, IID_IEnumFORMATETC)) {
        IEnumFORMATETC* obj = this;
        *object = obj;
    }
    else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG RichEditEnumFormatEtc_Windows::AddRef()
{
    return ++m_refCount;
}

ULONG RichEditEnumFormatEtc_Windows::Release()
{
    if (--m_refCount == 0) {
        delete this;
        return 0U;
    }
    return m_refCount;
}

HRESULT RichEditEnumFormatEtc_Windows::Next(ULONG celt, FORMATETC* pFormatEtc, ULONG* pceltFetched)
{
    ULONG copied = 0;

    while (m_nIndex < m_nNumFormats && copied < celt) {
        DeepCopyFormatEtc(&pFormatEtc[copied], &m_pFormatEtc[m_nIndex]);
        copied++;
        m_nIndex++;
    }

    if (pceltFetched != nullptr) {
        *pceltFetched = copied;
    }

    return (copied == celt) ? S_OK : S_FALSE;
}

HRESULT RichEditEnumFormatEtc_Windows::Skip(ULONG celt)
{
    m_nIndex += celt;
    return (m_nIndex <= m_nNumFormats) ? S_OK : S_FALSE;
}

HRESULT RichEditEnumFormatEtc_Windows::Reset()
{
    m_nIndex = 0;
    return S_OK;
}

HRESULT RichEditEnumFormatEtc_Windows::Clone(IEnumFORMATETC** ppEnumFormatEtc)
{
    HRESULT hResult = CreateEnumFormatEtc(static_cast<UINT>(m_nNumFormats), m_pFormatEtc, ppEnumFormatEtc);
    if (hResult == S_OK) {
        reinterpret_cast<RichEditEnumFormatEtc_Windows*>(*ppEnumFormatEtc)->m_nIndex = m_nIndex;
    }
    return hResult;
}

void RichEditEnumFormatEtc_Windows::DeepCopyFormatEtc(FORMATETC* dest, FORMATETC* source)
{
    *dest = *source;
    if (source->ptd != nullptr) {
        dest->ptd = reinterpret_cast<DVTARGETDEVICE*>(CoTaskMemAlloc(sizeof(DVTARGETDEVICE)));
        if (dest->ptd != nullptr) {
            *(dest->ptd) = *(source->ptd);
        }
    }
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN
