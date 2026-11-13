#ifndef UI_CONTROL_TEXT_DRAWER_H_
#define UI_CONTROL_TEXT_DRAWER_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Control/RichText.h"
#include "duilib/Render/IRender.h"

namespace ui
{
/** 文本绘制的实现类，支持普通文本绘制和RichText文本绘制
*/
class DUILIB_API TextDrawer
{
public:
    TextDrawer();
    ~TextDrawer();

public:
    /** 计算指定文本字符串的宽度和高度
     * @param [in] pRender 绘制引擎对象
     * @param [in] strText 文字内容
     * @param [in] measureParam 评估相关的参数
     * @param [in] fontId 默认的字体ID
     * @param [in] bRichText true表示文本内容为RichText，false表示文本内容为普通文本
     * @param [in] pOwner 关联的控件接口
     * @return 返回文本字符串的宽度和高度，以矩形表示结果
     */
    UiRect MeasureString(IRender* pRender,
                         const DString& strText,
                         const MeasureStringParam& measureParam,
                         const DString& fontId,
                         bool bRichText,
                         Control* pOwner);

    /** 绘制文字
     * @param [in] pRender 绘制引擎对象 
     * @param [in] strText 文字内容
     * @param [in] drawParam 文字绘制相关的参数
     * @param [in] fontId 默认的字体ID
     * @param [in] bRichText true表示文本内容为RichText，false表示文本内容为普通文本
     * @param [in] pOwner 关联的控件接口
     */
    void DrawString(IRender* pRender,
                    const DString& strText,
                    const DrawStringParam& drawParam,
                    const DString& fontId,
                    bool bRichText,
                    Control* pOwner);

    /** 设置原文本内容发生变化（当文本内容变化时，需要重新解析）
    */
    void SetTextChanged();

    /** 关联控件的窗口发生变化（同步RichText关联的窗口，避免每次测量/绘制都切换窗口）
    * @param [in] pOwner 关联的控件接口
    */
    void OnWindowChanged(Control* pOwner);

    /** 去除富文本标记，返回纯文本
     *  用于纵向文本的降级绘制，以及富文本模式的ToolTip显示
     */
    static DString ToPlainText(const DString& strRichText);

private:
    /** 同步RichText关联的窗口（仅在窗口真正变化时才切换，避免重复解析文本）
    * @param [in] pOwner 关联的控件接口
    */
    void SyncWindow(Control* pOwner);

    /** 获取纵向文本降级显示时使用的纯文本（带缓存，避免每帧解析）
    */
    const DString& GetPlainText(const DString& strText);

    /** 同步文字绘制属性
    * @param [in] uFormat 文字的格式，参见 enum DrawStringFormat 类型定义
    * @param [in] fSpacingMul 行间距倍数: 字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小）
    * @param [in] fSpacingAdd 行间距附加量: 是固定的附加像素值（默认值通常为 0）
    * @param [in] fLetterSpacing 两个相邻字符之间的间隔（像素，已经完成DPI缩放）
    * @param [in] fontId 默认的字体ID
    */
    void UpdateTextDrawProps(uint32_t uFormat, float fSpacingMul, float fSpacingAdd, float fLetterSpacing, const DString& fontId);

private:
    /** RichText模式使用的实现与缓存（仅在需要时创建，普通文本模式不占用这部分内存）
    */
    struct RichTextState;

    /** 获取RichText状态，首次使用时创建
    */
    RichTextState& EnsureRichTextState();

    /** RichText相关状态（延迟创建，普通文本模式为空指针）
    */
    std::unique_ptr<RichTextState> m_pRichTextState;
};

}//namespace ui

#endif //UI_CONTROL_TEXT_DRAWER_H_
