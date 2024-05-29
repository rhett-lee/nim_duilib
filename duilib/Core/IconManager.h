#ifndef UI_CORE_ICONMANAGER_H_
#define UI_CORE_ICONMANAGER_H_

#pragma once

#include "duilib/Core/UiTypes.h"
#include <map>
#include <string>
#include <vector>

#ifdef DUILIB_PLATFORM_WIN

namespace ui 
{
/** HICON句柄管理器
 */
class Window;
class ImageLoadAttribute;
class UILIB_API IconManager
{
public:
    IconManager();
    ~IconManager();
    IconManager(const IconManager&) = delete;
    IconManager& operator = (const IconManager&) = delete;

public:
    /** 获取ICON的资源字符串（可用作为图片文件路径使用）
    * @return 返回资源字符串，例如："icon:1"
    */
    DString GetIconString(HICON hIcon) const;

    /** 获取ICON的资源字符串（可用作为图片文件路径使用）
    * @return 返回资源字符串，例如："icon:1"
    */
    DString GetIconString(uint32_t id) const;

    /** 判断是否为ICON的资源字符串
    *@param [in] str 资源字符串，正确形式例如："icon:1"
    */
    bool IsIconString(const DString& str) const;

    /** 获取资源字符串对应的ICON的图标句柄，如果没有该资源，则返回nullptr
    *@param [in] str 资源字符串，正确形式例如："icon:1"
    */
    HICON GetIcon(const DString& str) const;

    /** 获取ICON资源字符串对应图标的大小
    *@param [in] str 资源字符串，正确形式例如："icon:1"
    *@return 返回图标的大小，如果失败返回空
    */
    UiSize GetIconSize(const DString& str) const;

    /** 将ICON资源字符串对应的HICON句柄，解码为位图数据（32位，ARGB位图）
    * @param [in] str 资源字符串，正确形式例如："icon:1"
    * @param [in] pWindow 绘制关联的窗口，如果为nullptr，则使用默认桌面DC（绘制图标的时候，需要用到窗口关联的设备上下文DC）
    * @param [in] ImageLoadAttribute 图片属性信息，包含指定图标的宽度和高度，按这个尺寸生成图标位图（不同的DPI下，生成的位图大小不同，而ICON图源大小只有一个）
    * @param [in] bEnableDpiScale 是否允许按照DPI对图片大小进行缩放（此为功能开关）
    * @param [out] bitmapData 成功时，返回位图的数据
    * @param [out] bitmapWidth 成功时，返回位图的宽度
    * @param [out] bitmapHeight 成功时，返回位图的高度
    * @param [out] bDpiScaled 返回图片是否做过DPI缩放
    */
    bool LoadIconData(const DString& str,
                      const Window* pWindow,
                      const ImageLoadAttribute& loadAtrribute,
                      bool bEnableDpiScale,
                      std::vector<uint8_t>& bitmapData,
                      uint32_t& bitmapWidth,
                      uint32_t& bitmapHeight,
                      bool& bDpiScaled) const;

public:
    /** 添加一个HICON句柄
    * @param [in] hIcon 需要加入的ICON句柄, 加入后句柄资源生命周期由该类管理
    * @return 返回该句柄对应的ID，如果失败则返回0
    */
    uint32_t AddIcon(HICON hIcon);

    /** 获取句柄对应的ID
    * @param [in] hIcon ICON句柄
    * @return 返回该句柄对应的ID，如果找不到则返回0
    */
    uint32_t GetIconID(HICON hIcon) const;

    /** 根据ID获取句柄
    * @param [in] id ICON句柄ID
    */
    HICON GetIcon(uint32_t id) const;

    /** 删除一个ICON句柄
    * @param [in] hIcon 需要删除的ICON句柄，从容器删除后，该句柄的生命周期由调用方管理
    */
    void RemoveIcon(HICON hIcon);

    /** 根据ID删除一个ICON句柄
    * @param [in] id 需要删除的ICON句柄ID，从容器删除后，该句柄的生命周期由调用方管理
    */
    void RemoveIcon(uint32_t id);

    /** 获取ICON的大小（宽度和高度）
    * @param [in] hIcon ICON句柄
    */
    UiSize GetIconSize(HICON hIcon) const;

private:
    /** ICON句柄资源映射表
    */
    std::map<uint32_t, HICON> m_iconMap;

    /** 下一个ID
    */
    uint32_t m_nNextID;

    /** ICON资源字符串前缀
    */
    DString m_prefix;
};

} //namespace ui 

#endif //DUILIB_PLATFORM_WIN

#endif //UI_CORE_ICONMANAGER_H_
