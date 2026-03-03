#ifndef UI_CONTROL_TEXT_DRAWER_H_
#define UI_CONTROL_TEXT_DRAWER_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Control/RichText.h"
#include "duilib/Render/IRender.h"

namespace ui
{
/** 文本绘制的实现类，支持普通文本绘制和RichText文本绘制
*/
class TextDrawer
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

private:
    /** 同步文字绘制属性
    * @param [in] uFormat 文字的格式，参见 enum DrawStringFormat 类型定义
    * @param [in] fSpacingMul 行间距倍数: 字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小）
    * @param [in] fSpacingAdd 行间距附加量: 是固定的附加像素值（默认值通常为 0）
    * @param [in] fontId 默认的字体ID
    */
    void UpdateTextDrawProps(uint32_t uFormat, float fSpacingMul, float fSpacingAdd, const DString& fontId);

private:
    /** RichText实现接口
    */
    std::unique_ptr<RichText> m_pRichText;

    /** 原文本内容是否发生变化（当文本内容变化时，需要重新解析）
    */
    bool m_bRichTextChanged;
};

}//namespace ui

#endif //UI_CONTROL_TEXT_DRAWER_H_
