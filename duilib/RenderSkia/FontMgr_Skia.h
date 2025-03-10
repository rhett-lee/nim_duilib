#ifndef UI_RENDER_SKIA_FONT_MGR_H_
#define UI_RENDER_SKIA_FONT_MGR_H_

#include "duilib/Render/IRender.h"

/** Skia字体类型
*/
class SkFont;

namespace ui 
{

/** 字体管理器接口的实现
*/
class UILIB_API FontMgr_Skia: public IFontMgr
{
public:
    explicit FontMgr_Skia();
    FontMgr_Skia(const FontMgr_Skia&) = delete;
    FontMgr_Skia& operator=(const FontMgr_Skia&) = delete;
    virtual ~FontMgr_Skia() override;

public:
    /** 获取字体个数
    * @return 返回字体的个数
    */
    virtual uint32_t GetFontCount() const override;

    /** 获取字体名称
    * @param [in] nIndex 字体的下标值：[0, GetFontCount())
    * @param [out] fontName 返回字体名称
    * @return 成功返回true，失败返回false
    */
    virtual bool GetFontName(uint32_t nIndex, DString& fontName) const override;

    /** 判断是否含有该字体
    * @param [int] fontName 字体名称
    * @return 如果含有该字体名称对应的字体返回true，否则返回false
    */
    virtual bool HasFontName(const DString& fontName) const override;

    /** 设置默认字体名称（当需要加载的字体不存在时，使用默认的字体）
    * @param [in] fontName 默认的字体名称
    */
    virtual void SetDefaultFontName(const DString& fontName) override;

    /** 加载指定字体文件
    * @param [in] fontFilePath 字体文件的路径（本地绝对路径）
    * @return 成功返回true，失败返回false
    */
    virtual bool LoadFontFile(const DString& fontFilePath) override;

    /** 加载指定字体数据
    * @param [in] data 字体文件的内存数据
    * @param [in] length 字体文件的内存数据长度
    * @return 成功返回true，失败返回false
    */
    virtual bool LoadFontFileData(const void* data, size_t length) override;

    /** 清除已加载的字体文件
    */
    virtual void ClearFontFiles() override;

    /** 清除字体缓存
    */
    virtual void ClearFontCache() override;

public:
    /** 创建Skia字体
    * @param [in] fontInfo 字体属性
    * @return 成功返回Skia字体指针，需要调用DeleteSkFont删除指针指向的字体
    */
    SkFont* CreateSkFont(const UiFont& fontInfo);

    /** 删除Skia字体
    */
    void DeleteSkFont(SkFont* pSkFont);

private:
    /** 内部实现类
    */
    class TImpl;
    TImpl* m_impl;
};

} // namespace ui

#endif // UI_RENDER_SKIA_FONT_MGR_H_
