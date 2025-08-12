#ifndef UI_CEF_CONTROL_MEMORY_BLOCK_H_
#define UI_CEF_CONTROL_MEMORY_BLOCK_H_

#include "duilib/duilib_config.h"
#include <mutex>
#include <vector>

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
    bool Init(const void* buffer, const std::vector<UiRect>& dirtyRectList, int32_t width, int32_t height);

    /** 将位图数据绘制到Render
    */
    void PaintData(IRender* pRender, const UiRect& rc);

    /** 判断内存块是否可以使用
    */
    bool IsValid() const;

    /** 将网页数据绘制到Render，大小和网页保持一致
    */
    bool MakeImageSnapshot(IRender* pRender);

    /** 获取内存位图的宽度
    */
    int32_t GetWidth() const;

    /** 获取内存位图的高度
    */
    int32_t GetHeight() const;

private:
    /** 删除位图数据并初始化变量
    */
    void Clear();

    /** 获取内存位图数据指针，可用于填充位图数据
    */
    uint8_t* GetBits() const;

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
