// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_OSR_DRAGDROP_WIN_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_OSR_DRAGDROP_WIN_H_
#pragma once

// When generating projects with CMake the CEF_USE_ATL value will be defined
// automatically if using a supported Visual Studio version. Pass -DUSE_ATL=OFF
// to the CMake command-line to disable use of ATL.
// Uncomment this line to manually enable ATL support.
// #define CEF_USE_ATL 1
#include "duilib/duilib_defs.h"
#include <objidl.h>
#include <stdio.h>

#include "duilib/CEFControl/internal/osr_dragdrop_events.h"

namespace client {

#define DEFAULT_QUERY_INTERFACE(__Class)                                     \
  HRESULT __stdcall QueryInterface(const IID& iid, void** object) override { \
    *object = nullptr;                                                       \
    if (IsEqualIID(iid, IID_IUnknown)) {                                     \
      IUnknown* obj = this;                                                  \
      *object = obj;                                                         \
    } else if (IsEqualIID(iid, IID_##__Class)) {                             \
      __Class* obj = this;                                                   \
      *object = obj;                                                         \
    } else {                                                                 \
      return E_NOINTERFACE;                                                  \
    }                                                                        \
    AddRef();                                                                \
    return S_OK;                                                             \
  }
#define IUNKNOWN_IMPLEMENTATION        \
  ULONG __stdcall AddRef() override {  \
    return ++ref_count_;               \
  }                                    \
  ULONG __stdcall Release() override { \
    if (--ref_count_ == 0) {           \
      delete this;                     \
      return 0U;                       \
    }                                  \
    return ref_count_;                 \
  }                                    \
                                       \
 protected:                            \
  ULONG ref_count_ = 0;

class DropTargetWin : public IDropTarget {
 public:
  // IDropTarget implementation:
  virtual HRESULT __stdcall DragEnter(IDataObject* data_object,
                                      DWORD key_state,
                                      POINTL cursor_position,
                                      DWORD* effect) override;

  virtual HRESULT __stdcall DragOver(DWORD key_state,
                                     POINTL cursor_position,
                                     DWORD* effect) override;

  virtual HRESULT __stdcall DragLeave() override;

  virtual HRESULT __stdcall Drop(IDataObject* data_object,
                                 DWORD key_state,
                                 POINTL cursor_position,
                                 DWORD* effect) override;

  // 用shared_ptr管理生命周期，不用ComPtr
  DEFAULT_QUERY_INTERFACE(IDropTarget)
  virtual ULONG __stdcall AddRef() override { return 1; }
  virtual ULONG __stdcall Release() override { return 1; }

 public:
  HWND GetHWND() { return hWnd_; };
  DropTargetWin(HWND hWnd, OsrDragEvents* browser_handler)
      : hWnd_(hWnd), browser_handler_(browser_handler){ }
  virtual ~DropTargetWin() = default;

private:
    DropTargetWin() = delete;

 private:
  HWND hWnd_ = nullptr;
  OsrDragEvents *browser_handler_ = nullptr;
};
using DropTargetHandle = std::shared_ptr<client::DropTargetWin>;

CefBrowserHost::DragOperationsMask OsrStartDragging(CefRefPtr<CefDragData> drag_data,
                                                    CefRenderHandler::DragOperationsMask allowed_ops,
                                                    int x, int y);

class DropSourceWin : public IDropSource {
 public:
  static DropSourceWin* Create();

  // IDropSource implementation:
  HRESULT __stdcall GiveFeedback(DWORD dwEffect) override;

  HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed,
                                      DWORD grfKeyState) override;

  DEFAULT_QUERY_INTERFACE(IDropSource)
  IUNKNOWN_IMPLEMENTATION

 protected:
  explicit DropSourceWin() : ref_count_(0) {}
  virtual ~DropSourceWin() = default;
};

class DragEnumFormatEtc : public IEnumFORMATETC {
 public:
  static HRESULT CreateEnumFormatEtc(UINT cfmt,
                                     FORMATETC* afmt,
                                     IEnumFORMATETC** ppEnumFormatEtc);

  //
  // IEnumFormatEtc members
  //
  HRESULT __stdcall Next(ULONG celt,
                         FORMATETC* pFormatEtc,
                         ULONG* pceltFetched) override;
  HRESULT __stdcall Skip(ULONG celt) override;
  HRESULT __stdcall Reset() override;
  HRESULT __stdcall Clone(IEnumFORMATETC** ppEnumFormatEtc) override;

  //
  // Construction / Destruction
  //
  DragEnumFormatEtc(FORMATETC* pFormatEtc, int nNumFormats);
  virtual ~DragEnumFormatEtc();

  static void DeepCopyFormatEtc(FORMATETC* dest, FORMATETC* source);

  DEFAULT_QUERY_INTERFACE(IEnumFORMATETC)
  IUNKNOWN_IMPLEMENTATION

 private:
  ULONG m_nIndex;           // current enumerator index
  ULONG m_nNumFormats;      // number of FORMATETC members
  FORMATETC* m_pFormatEtc;  // array of FORMATETC objects
};

class DataObjectWin : public IDataObject {
 public:
  static DataObjectWin* Create(FORMATETC* fmtetc, STGMEDIUM* stgmed, int count);

  // IDataObject memberS
  HRESULT __stdcall GetDataHere(FORMATETC* pFormatEtc,
                                STGMEDIUM* pmedium) override;
  HRESULT __stdcall QueryGetData(FORMATETC* pFormatEtc) override;
  HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC* pFormatEct,
                                          FORMATETC* pFormatEtcOut) override;
  HRESULT __stdcall SetData(FORMATETC* pFormatEtc,
                            STGMEDIUM* pMedium,
                            BOOL fRelease) override;
  HRESULT __stdcall DAdvise(FORMATETC* pFormatEtc,
                            DWORD advf,
                            IAdviseSink*,
                            DWORD*) override;
  HRESULT __stdcall DUnadvise(DWORD dwConnection) override;
  HRESULT __stdcall EnumDAdvise(IEnumSTATDATA** ppEnumAdvise) override;

  HRESULT __stdcall EnumFormatEtc(DWORD dwDirection,
                                  IEnumFORMATETC** ppEnumFormatEtc) override;
  HRESULT __stdcall GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium) override;

  DEFAULT_QUERY_INTERFACE(IDataObject)
  IUNKNOWN_IMPLEMENTATION

 protected:
  int m_nNumFormats;
  FORMATETC* m_pFormatEtc;
  STGMEDIUM* m_pStgMedium;

  static HGLOBAL DupGlobalMem(HGLOBAL hMem);

  int LookupFormatEtc(FORMATETC* pFormatEtc);

  explicit DataObjectWin(FORMATETC* fmtetc, STGMEDIUM* stgmed, int count);
  virtual ~DataObjectWin() = default;
};

}  // namespace client

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_OSR_DRAGDROP_WIN_H_
