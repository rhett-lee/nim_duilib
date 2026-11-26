// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_BROWSER_BYTES_WRITE_HANDLER_H_
#define CEF_TESTS_CEFCLIENT_BROWSER_BYTES_WRITE_HANDLER_H_
#pragma once

#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_CEF

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/base/cef_lock.h"
#include "include/cef_stream.h"
#pragma warning (pop)

namespace client {

/** 内存写数据流处理器（对应 CefWriteHandler 接口）
*   把写入的数据保存在一块可自动扩容的内存缓冲区中，常用于把网页内容
*   抓取为内存数据（例如 CEF 的 zip 压缩、网页快照等场景）。
*/
class BytesWriteHandler : public CefWriteHandler {
 public:
  /** 构造函数
  * @param [in] grow 缓冲区每次扩容的字节数（初始容量也等于该值）
  */
  explicit BytesWriteHandler(size_t grow);
  ~BytesWriteHandler() override;

  /** 向缓冲区写入数据（空间不足时自动扩容）
  * @param [in] ptr 要写入的数据
  * @param [in] size 单个数据块的字节数
  * @param [in] n 数据块的数量
  * @return 返回实际写入的数据块数量，写入失败返回 0
  */
  size_t Write(const void* ptr, size_t size, size_t n) override;

  /** 移动当前读写位置
  * @param [in] offset 相对于 whence 的偏移量
  * @param [in] whence 基准位置（SEEK_CUR/SEEK_END/SEEK_SET）
  * @return 返回 0 表示成功，-1 表示失败
  */
  int Seek(int64_t offset, int whence) override;

  /** 获取当前读写位置
  * @return 返回当前读写位置的字节偏移
  */
  int64_t Tell() override;

  /** 刷新缓冲区（内存流无需刷新，直接返回 0）
  * @return 返回 0
  */
  int Flush() override;
  bool MayBlock() override { return false; }

  /** 获取内存缓冲区的首地址
  * @return 返回内存缓冲区指针（不转移所有权）
  */
  void* GetData() { return data_; }

  /** 获取当前已写入的数据长度
  * @return 返回已写入的字节数
  */
  int64_t GetDataSize() { return offset_; }

 private:
  /** 扩展缓冲区容量
  * @param [in] size 本次需要新增的最小字节数
  * @return 返回扩容后的容量，扩容失败返回 0
  */
  size_t Grow(size_t size);

  size_t grow_;
  void* data_;
  int64_t datasize_;
  int64_t offset_;

  base::Lock lock_;

  // CEF 引用计数与拷贝控制的宏实现
  IMPLEMENT_REFCOUNTING(BytesWriteHandler);
  DISALLOW_COPY_AND_ASSIGN(BytesWriteHandler);
};

}  // namespace client

#endif //DUILIB_BUILD_FOR_CEF

#endif  // CEF_TESTS_CEFCLIENT_BROWSER_BYTES_WRITE_HANDLER_H_
