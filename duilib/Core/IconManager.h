#ifndef UI_CORE_ICONMANAGER_H_
#define UI_CORE_ICONMANAGER_H_

#include "duilib/Core/UiTypes.h"
#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <functional>

namespace ui 
{
class Window;
class ImageLoadAttribute;

/** 图标位图数据
*/
class UILIB_API IconBitmapData
{
public:
    /** 图标位图数据
    */
    std::vector<uint8_t> m_bitmapData;

    /** 位图数据宽度
    */
    int32_t m_nBitmapWidth = 0;

    /** 位图数据宽度高度
    */
    int32_t m_nBitmapHeight = 0;
};

/** 回调函数的原型: 用于接收删除图标事件（该函数需要保证线程安全，因为可能在子线程中调用）
*/
typedef std::function<void (uint32_t nIconId)> RemoveIconEvent;

/** 图标资源管理器（线程安全，适合图标类的小图片资源）
 *  说明：支持Windows的HICON句柄资源，但内部不使用HICON，因为HICON是内核GDI资源，每个进程有上限，约1万个左右，耗尽后该进程就挂了。
 */
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
    DString GetIconString(uint32_t id) const;

    /** 判断是否为ICON的资源字符串
    *@param [in] str 资源字符串，正确形式例如："icon:1"
    */
    bool IsIconString(const DString& str) const;

    /** 从ICON资源字符串中解析图标ID
    *@param [in] str 资源字符串，正确形式例如："icon:1"
    */
    uint32_t GetIconID(const DString& str) const;

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

#ifdef DUILIB_BUILD_FOR_WIN
    /** 添加一个HICON句柄
    * @param [in] hIcon 需要加入的ICON句柄, 加入后句柄资源生命周期由该类管理
    * @return 返回该句柄对应的ID，如果失败则返回0
    */
    uint32_t AddIcon(HICON hIcon);
#endif

    /** 添加一个图标（适合小图标）
    * @param [in] pBitmapData 位图数据的起始地址
    * @param [in] nBitmapDataSize 位图数据的大小
    * @param [in] nBitmapWidth 位图宽度
    * @param [in] nBitmapHeight 位图高度
    * @return 返回该图标对应的ID，如果失败则返回0
    */
    uint32_t AddIcon(const uint8_t* pBitmapData, int32_t nBitmapDataSize, int32_t nBitmapWidth, int32_t nBitmapHeight);

    /** 获取图标位图数据
    * @param [in] id 图标ID（即AddIcon返回的那个ID）
    * @param [out] bitmapData 成功时返回位图数据
    */
    bool GetIconBitmapData(uint32_t id, IconBitmapData& bitmapData) const;

    /** 根据ID删除一个图标资源
    * @param [in] id 需要删除的图标ID（即AddIcon返回的那个ID）
    */
    void RemoveIcon(uint32_t id);

public:
    /** 添加一个图标资源（本地资源路径的相对，例如："public/shadow/test.png"）
    * @param [in] imageString 图片资源字符串, 格式与XML中设置图片的格式相同
    * @return 返回该图标对应的ID，如果失败则返回0
    */
    uint32_t AddIcon(const DString& imageString);

    /** 判断是否为ImageString格式的资源字符串
    *@param [in] id 图标ID（即AddIcon返回的那个ID）
    */
    bool IsImageString(uint32_t id) const;

    /** 获取ICON的图片资源字符串
    * @param [in] id 图标ID（即AddIcon返回的那个ID）
    * @return 返回资源字符串，例如："public/shadow/test.png"
    */
    DString GetImageString(uint32_t id) const;

    /** 设置用于接收删除图标事件的回调函数
    * @param [in] callback 回调函数
    * @return 返回回调对应的ID，可用于移除回调
    */
    uint32_t AttachRemoveIconEvent(RemoveIconEvent callback);

    /** 删除用于接收删除图标事件的回调函数
    * @param [in] callbackID 回调函数的ID，由AttachRemoveIconEvent返回
    */
    void DetachRemoveIconEvent(uint32_t callbackID);

private:
    /** 添加一个图标
    */
    uint32_t AddIconBitmapData(IconBitmapData& bitmapData);

private:
    /** ICON位图资源映射表
    */
    std::map<uint32_t, IconBitmapData> m_iconMap;

    /** ImageString资源映射表
    */
    std::map<uint32_t, UiString> m_imageStringMap;

    /** 数据多线程同步锁
    */
    mutable std::mutex m_iconMutex;

    /** 下一个ID
    */
    uint32_t m_nNextID;

    /** ICON资源字符串前缀
    */
    const DString m_prefix;

    /** 用于接收删除图标事件的回调函数
    */
    std::map<uint32_t, RemoveIconEvent> m_callbackMap;

    /** 下一个回调函数ID
    */
    uint32_t m_nNextCallbackID;
};

} //namespace ui 

#endif //UI_CORE_ICONMANAGER_H_
