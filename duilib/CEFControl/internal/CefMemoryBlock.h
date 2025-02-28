#ifndef UI_CEF_CONTROL_MEMORY_BLOCK_H_
#define UI_CEF_CONTROL_MEMORY_BLOCK_H_

#include "duilib/duilib_config.h"

namespace ui {

/** 位图数据内存块的封装
 */
class CefMemoryBlock
{
public:
    CefMemoryBlock();
    ~CefMemoryBlock();

    /** 初始化位图的大小
    */
    bool Init(int32_t width, int32_t height);

    /** 删除位图数据并初始化变量
    */
    void Uninit();

    /** 判断内存块是否可以使用
    */
    bool IsValid() const;

    /** 获取内存位图数据指针，可用于填充位图数据
    */
    uint8_t* GetBits() const;

    /** 获取内存位图的宽度
    */
    int32_t GetWidth() const;

    /** 获取内存位图的高度
    */
    int32_t GetHeight() const;

private:
    /** 内存数据
    */
    uint8_t* m_pBits;

    /** 内存位图的宽度
    */
    int32_t m_nWidth;

    /** 内存位图的高度
    */
    int32_t m_nHeight;
};

} //namespace ui

#endif //UI_CEF_CONTROL_MEMORY_BLOCK_H_
