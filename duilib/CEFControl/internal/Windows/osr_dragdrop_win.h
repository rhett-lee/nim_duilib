// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_OSR_DRAGDROP_WIN_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_OSR_DRAGDROP_WIN_H_
#pragma once

// 使用 CMake 生成工程时，如果使用了受支持的 Visual Studio 版本，
// CEF_USE_ATL 的值会被自动定义。可在 CMake 命令行中传入 -DUSE_ATL=OFF
// 来禁用 ATL。如需手动启用 ATL 支持，请取消下面这一行的注释。
// #define CEF_USE_ATL 1
#include "duilib/duilib_defs.h"

#ifdef DUILIB_BUILD_FOR_CEF

#include <objidl.h>
#include <stdio.h>

#include "duilib/CEFControl/internal/osr_dragdrop_events.h"

namespace client {

// 宏：为指定接口生成 QueryInterface 的标准实现（支持 IUnknown 与指定接口的查询）
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

// 宏：引用计数的标准实现（AddRef/Release 与 ref_count_ 成员变量）
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

/** 离屏渲染模式下，接收拖动目标的实现（对应 IDropTarget 接口）
*/
class DropTargetWin : public IDropTarget {
 public:
  // IDropTarget 接口实现：
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
  /** 获取关联的窗口句柄
  * @return 返回窗口句柄
  */
  HWND GetHWND() { return hWnd_; };
  /** 构造函数
  * @param [in] hWnd 目标窗口句柄
  * @param [in] browser_handler 拖动事件的处理接口
  */
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

/** 开始一次拖放操作（离屏渲染模式下由 StartDragging 调用）
* @param [in] drag_data 拖动的数据
* @param [in] allowed_ops 允许的拖动操作类型
* @param [in] x 拖动起点的 x 坐标（视图坐标）
* @param [in] y 拖动起点的 y 坐标（视图坐标）
* @return 返回实际使用的拖动操作类型
*/
CefBrowserHost::DragOperationsMask OsrStartDragging(CefRefPtr<CefDragData> drag_data,
                                                    CefRenderHandler::DragOperationsMask allowed_ops,
                                                    int x, int y);

/** 拖动源的实现（对应 IDropSource 接口）
*/
class DropSourceWin : public IDropSource {
 public:
  /** 创建拖动源对象
  * @return 返回新创建的拖动源对象
  */
  static DropSourceWin* Create();

  // IDropSource 接口实现：
  HRESULT __stdcall GiveFeedback(DWORD dwEffect) override;

  /** 查询是否继续拖动或取消拖动
  * @param [in] fEscapePressed 是否按下了 Esc 键
  * @param [in] grfKeyState 当前鼠标按键的状态
  * @return 返回 S_OK 表示继续拖动；DRAGDROP_S_DROP 表示执行放下；
  *         DRAGDROP_S_CANCEL 表示取消拖动
  */
  HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed,
                                      DWORD grfKeyState) override;

  DEFAULT_QUERY_INTERFACE(IDropSource)
  IUNKNOWN_IMPLEMENTATION

 protected:
  explicit DropSourceWin() : ref_count_(0) {}
  virtual ~DropSourceWin() = default;
};

/** FORMATETC 的枚举器实现（对应 IEnumFORMATETC 接口）
*/
class DragEnumFormatEtc : public IEnumFORMATETC {
 public:
  /** 创建 FORMATETC 枚举器
  * @param [in] cfmt FORMATETC 的数量
  * @param [in] afmt FORMATETC 数组
  * @param [out] ppEnumFormatEtc 返回创建的枚举器接口
  * @return 返回 S_OK 表示成功，否则为错误码
  */
  static HRESULT CreateEnumFormatEtc(UINT cfmt,
                                     FORMATETC* afmt,
                                     IEnumFORMATETC** ppEnumFormatEtc);

  //
  // IEnumFORMATETC 接口的成员函数
  //
  HRESULT __stdcall Next(ULONG celt,
                         FORMATETC* pFormatEtc,
                         ULONG* pceltFetched) override;
  HRESULT __stdcall Skip(ULONG celt) override;
  HRESULT __stdcall Reset() override;
  HRESULT __stdcall Clone(IEnumFORMATETC** ppEnumFormatEtc) override;

  //
  // 构造 / 析构
  //
  DragEnumFormatEtc(FORMATETC* pFormatEtc, int nNumFormats);
  virtual ~DragEnumFormatEtc();

  /** 深拷贝一个 FORMATETC 结构（目标结构需要调用方自行释放资源）
  * @param [out] dest 目标结构
  * @param [in] source 源结构
  */
  static void DeepCopyFormatEtc(FORMATETC* dest, FORMATETC* source);

  DEFAULT_QUERY_INTERFACE(IEnumFORMATETC)
  IUNKNOWN_IMPLEMENTATION

 private:
  // 数据成员：m_nIndex 当前枚举索引；m_nNumFormats 的 FORMATETC 数量；m_pFormatEtc 的 FORMATETC 数组
  ULONG m_nIndex;           // current enumerator index
  ULONG m_nNumFormats;      // number of FORMATETC members
  FORMATETC* m_pFormatEtc;  // array of FORMATETC objects
};

/** 拖动数据的实现（对应 IDataObject 接口）
*/
class DataObjectWin : public IDataObject {
 public:
  /** 创建拖动数据对象
  * @param [in] fmtetc FORMATETC 数组
  * @param [in] stgmed STGMEDIUM 数组
  * @param [in] count FORMATETC 的数量
  * @return 返回新创建的拖动数据对象，失败时返回 nullptr
  */
  static DataObjectWin* Create(FORMATETC* fmtetc, STGMEDIUM* stgmed, int count);

  // IDataObject 接口的成员函数
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

  /** 复制一个全局内存句柄（用于返回数据时避免共享同一块内存）
  * @param [in] hMem 要复制的全局内存句柄
  * @return 返回复制后的全局内存句柄，失败时返回 nullptr
  */
  static HGLOBAL DupGlobalMem(HGLOBAL hMem);

  /** 查找指定格式在内部数组中的索引
  * @param [in] pFormatEtc 要查找的格式
  * @return 返回匹配的索引，未找到返回 -1
  */
  int LookupFormatEtc(FORMATETC* pFormatEtc);

  /** 构造函数（受保护，请使用 Create 创建对象）
  * @param [in] fmtetc FORMATETC 数组
  * @param [in] stgmed STGMEDIUM 数组
  * @param [in] count FORMATETC 的数量
  */
  explicit DataObjectWin(FORMATETC* fmtetc, STGMEDIUM* stgmed, int count);
  virtual ~DataObjectWin();
};

}  // namespace client

#endif //DUILIB_BUILD_FOR_CEF

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_OSR_DRAGDROP_WIN_H_
