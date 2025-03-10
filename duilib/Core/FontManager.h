#ifndef UI_CORE_FONTMANAGER_H_
#define UI_CORE_FONTMANAGER_H_

#include "duilib/Core/UiFont.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace ui 
{
class IFont;
class DpiManager;

/** 字体大小信息
*/
struct FontSizeInfo
{
    DString fontSizeName;   //字体大小的显示名称，比如"五号"
    float fFontSize = 0;    //单位：像素，未做DPI自适应
    float fDpiFontSize = 0; //单位：像素，已做DPI自适应
};

/** 字体管理器
*/
class UILIB_API FontManager
{
public:
    FontManager();
    ~FontManager();
    FontManager(const FontManager&) = delete;
    FontManager& operator = (const FontManager&) = delete;

public:
    /** 添加一个字体信息，字体大小未经DPI处理
     * @param [in] fontId 指定字体的ID标记
     * @param [in] fontInfo 字体属性信息, 字体大小为原始大小，未经DPI缩放
     * @param [in] bDefault 是否设置为默认字体
     */
    bool AddFont(const DString& fontId, const UiFont& fontInfo, bool bDefault);

    /** 获取字体接口, 如果通过fontId找不到字体接口，那么会继续查找m_defaultFontId字体接口
    * @param [in] fontId 字体ID
    * @param [in] dpi DPI缩放管理器，用于对字体大小进行缩放
    * @return 成功返回字体接口，外部调用不需要释放资源；如果失败则返回nullptr
    */
    IFont* GetIFont(const DString& fontId, const DpiManager& dpi);

    /** 获取字体接口, 如果通过fontId找不到字体接口，那么会继续查找m_defaultFontId字体接口
    * @param [in] fontId 字体ID
    * @param [in] nZoomPercent 字体大小缩放百分比，用于对字体大小进行缩放，举例：100代表100%，200代表200%
    * @return 成功返回字体接口，外部调用不需要释放资源；如果失败则返回nullptr
    */
    IFont* GetIFont(const DString& fontId, uint32_t nZoomPercent);

    /** 是否包含该字体ID
    * @param [in] fontId 指定字体的ID标记
    */
    bool HasFontId(const DString& fontId) const;

    /** 删除一个字体ID
    * @param [in] fontId 指定字体的ID标记
    */
    bool RemoveFontId(const DString& fontId);

    /** 删除一个字体缓存，以释放内存
    * @param [in] fontId 字体ID
    * @param [in] nZoomPercent 字体大小缩放百分比，用于对字体大小进行缩放，举例：100代表100%，200代表200%
    */
    bool RemoveIFont(const DString& fontId, uint32_t nZoomPercent);

    /** 删除所有字体, 不包含已经加载的字体文件
     */
    void RemoveAllFonts();

    /** 获取默认字体ID
    */
    const DString& GetDefaultFontId() const;

    /** 设置默认字体列表
    * @param [in] defaultFontFamilyNames 字体列表，不同字体用逗号分割，比如："Microsoft YaHei,SimSun"
    */
    void SetDefaultFontFamilyNames(const DString& defaultFontFamilyNames);

public:
    /** @brief 添加一个字体文件, 添加后可以按照正常字体使用
      * @param[in] strFontFile 字体文件名, 相对路径，字体文件的保存路径是目录："<资源路径>\font\"
      * @param[in] strFontDesc 字体描述信息
      * @return 无返回值
      */
    bool AddFontFile(const DString& strFontFile, const DString& strFontDesc);

    /** @brief 清理所有添加的字体文件
      * @return 无返回值
      */
    void RemoveAllFontFiles();

public:
    /** 获取可用的字体名称列表
    * @param [out] fontNameList 返回可用的字体名称列表
    */
    void GetFontNameList(std::vector<DString>& fontNameList) const;

    /** 获取字体大小列表
    * @param [in] dpi DPI缩放管理器，用于对字体大小进行缩放
    * @param [out] fontSizeList 字体大小信息
    */
    void GetFontSizeList(const DpiManager& dpi, std::vector<FontSizeInfo>& fontSizeList) const;

private:
    /** 获取DPI缩放后实际的字体ID
    */
    DString GetDpiFontId(const DString& fontId, uint32_t nZoomPercent) const;

private:
    /** 自定义字体数据：Key时FontID，Value是字体描述信息
    */
    std::unordered_map<DString, UiFont> m_fontIdMap;

    /** 自定义字体信息：Key是FontId
    */
    std::unordered_map<DString, IFont*> m_fontMap;

    /** 默认字体ID
    */
    DString m_defaultFontId;

    /** 默认字体列表
    */
    std::vector<DString> m_defaultFontFamilyNames;

    /** 默认字体列表是否已经完成初始化
    */
    bool m_bDefaultFontInited;
};

}
#endif //UI_CORE_FONTMANAGER_H_
