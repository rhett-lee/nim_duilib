#ifndef UI_CORE_FONTMANAGER_H_
#define UI_CORE_FONTMANAGER_H_

#include "duilib/Core/UiFont.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace ui 
{
class IFont;
class IFallbackFontMgr;
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
class DUILIB_API FontManager
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

    /** 字体回退管理器（当支持的字体无法显示字符时，会查询回退字体管理器，以正确显示文字）
    */
    IFallbackFontMgr* GetFallbackFontMgr() const;

public:
    /** 获取默认字体ID
    */
    const DString& GetDefaultFontId() const;

    /** 设置默认字体列表
    * @param [in] defaultFontFamilyNames 字体列表，不同字体用逗号分割，比如："Microsoft YaHei,SimSun"
    */
    void SetDefaultFontFamilyNames(const DString& defaultFontFamilyNames);

    /** 设置默认回退字体列表，用于显示Emoji字符/扩展汉字（2个UTF16字符表示的汉字）等
    * @param [in] fallbackFontFamilyNames 回退字体列表，不同字体用逗号分割，比如："Segoe UI Emoji,Noto Color Emoji, MingLiU-ExtB"
    */
    void SetFallbackFontFamilyNames(const DString& fallbackFontFamilyNames);

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
    /** 清除字体的缓存（切换主题后，立即生效）
    */
    void ClearFontCache();

public:
    /** 获取可用的字体名称列表
    * @param [out] fontNameList 返回可用的字体名称列表
    */
    void GetFontNameList(std::vector<DString>& fontNameList) const;

    /** 设置默认的字体大小列表（可以覆盖内置的默认列表），字体大小值未进行DPI缩放
    */
    void SetFontSizeList(const std::vector<FontSizeInfo>& fontSizeList);

    /** 获取默认的字体大小列表，字体大小值未进行DPI缩放
    * @param [out] fontSizeList 字体大小信息
    */
    void GetFontSizeList(std::vector<FontSizeInfo>& fontSizeList) const;

    /** 获取默认的字体大小列表(执行DPI缩放)
    * @param [in] dpi DPI缩放管理器，用于对字体大小进行缩放
    * @param [in,out] fontSizeList 字体大小信息
    */
    void GetDpiFontSizeList(const DpiManager& dpi, std::vector<FontSizeInfo>& fontSizeList) const;

    /** 对字体列表执行DPI缩放
    * @param [in,out] fontSizeList 字体大小信息
    * @param [in] dpi DPI缩放管理器，用于对字体大小进行缩放
    */
    void DpiScaleFontSizeList(std::vector<FontSizeInfo>& fontSizeList, const DpiManager& dpi) const;

private:
    /** 获取DPI缩放后实际的字体ID
    */
    DString GetDpiFontId(const DString& fontId, uint32_t nZoomPercent) const;

    /** 初始化默认字体
    */
    void InitDefaultFont();

    /** 一个IFont字体数据被移除了
    */
    void OnIFontDataRemoved(IFont* pIFont);

private:
    /** 自定义字体数据：Key时FontID，Value是字体描述信息
    */
    std::unordered_map<DString, UiFont> m_fontIdMap;

    /** 自定义字体信息：Key是FontId
    */
    std::unordered_map<DString, IFont*> m_fontMap;

    /** 回退字体管理器
    */
    class FallbackFontMgrImpl;
    friend class FallbackFontMgrImpl;
    std::unique_ptr<FallbackFontMgrImpl> m_pFallbackFontMgr;

    /** 回退字体信息：Key是IFont*
    */
    std::unordered_map<const IFont*, std::vector<IFont*>> m_fallbackFontMap;

    /** 默认字体ID
    */
    DString m_defaultFontId;

    /** 默认字体列表
    */
    std::vector<DString> m_defaultFontFamilyNames;

    /** 默认的字体大小列表
    */
    std::vector<FontSizeInfo> m_fontSizeList;

    /** 回退字体列表
    */
    std::vector<DString> m_fallbackFontFamilyNames;

    /** 默认字体列表是否已经完成初始化
    */
    bool m_bDefaultFontInited;

    /** 回退字体列表是否已经完成初始化
    */
    bool m_bFallbackFontInited;
};

}
#endif //UI_CORE_FONTMANAGER_H_
