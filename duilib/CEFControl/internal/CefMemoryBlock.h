#ifndef UI_CEF_CONTROL_MEMORY_BLOCK_H_
#define UI_CEF_CONTROL_MEMORY_BLOCK_H_

#include "duilib/duilib_config.h"
#include <mutex>

namespace ui {

class IRender;
class UiRect;

/** 位图数据内存块的封装
 */
class CefMemoryBlock
{
public:
    CefMemoryBlock();
    ~CefMemoryBlock();

    /** 初始化位图的数据
    */
    bool Init(const void* buffer, int32_t width, int32_t height);

    /** 将位图数据绘制到Render
    */
    void PaintData(IRender* pRender, const UiRect& rcPaint, int32_t left, int32_t top);

    /** 判断内存块是否可以使用
    */
    bool IsValid() const;

private:
    /** 删除位图数据并初始化变量
    */
    void Clear();

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

    /** 数据多线程同步锁
    */
    std::mutex m_memMutex;
};

} //namespace ui

#endif //UI_CEF_CONTROL_MEMORY_BLOCK_H_
