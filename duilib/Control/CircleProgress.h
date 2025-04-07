/** @file CircleProgress.h
* @brief 环型进度条控件，圆环中间可以有文本（如85%）
* @copyright (c) 2019-2022, NetEase Inc. All rights reserved
* @author Xuhuajie
* @date 2019/8/14
*/

#ifndef UI_CONTROL_CIRCLEPROGRESS_H_
#define UI_CONTROL_CIRCLEPROGRESS_H_

#include "duilib/Control/Progress.h"

namespace ui
{

class UILIB_API CircleProgress : public Progress
{
    typedef Progress BaseClass;
public:
    explicit CircleProgress(Window* pWindow);
    virtual ~CircleProgress() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void ClearImageCache() override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 设置环型进度条，默认为普通进度条
    * @param [in] bCircular 为 true 时设置为环型进度条，false 时设置为父级进度条样式，默认为 true
    */
    void SetCircular(bool bCircular = true);

    /** 设置递增方向
    * @param [in] bClockwise 为 true 时设置为顺时针，false 时设置为逆时针，默认为 true
    */
    void SetClockwiseRotation(bool bClockwise = true);

    /** 设置圆环宽度
    * @param [in] fCircleWidth 宽度数值
    * @param [in] bNeedDpiScale 是否支持DPI缩放
    */
    void SetCircleWidth(float fCircleWidth, bool bNeedDpiScale);

    /** 获取圆环宽度
    */
    float GetCircleWidth() const;

    /** 设置进度条背景颜色
    * @param [in] strColor要设置的背景颜色字符串，该字符串必须在 global.xml 中存在
    */
    void SetBackgroudColor(const DString& strColor);

    /** 设置进度条前景颜色
    * @param [in] strColor要设置的前景颜色字符串，该字符串必须在 global.xml 中存在
    */
    void SetForegroudColor(const DString& strColor);

    /** 设置进度条前景渐变颜色，与 SetForegroudColor 同时使用，可以不设置,则无渐变效果
    * @param [in] strColor要设置的前景渐变颜色字符串，该字符串必须在 global.xml 中存在
    */
    void SetCircleGradientColor(const DString& strColor);

    /** 设置进度指示移动图标
    * @param [in] sIndicatorImage 要设置的图片
    */
    void SetIndicator(const DString& sIndicatorImage);

protected:
    bool m_bCircular;
    bool m_bClockwise;
    float m_fCircleWidth;
    UiColor m_dwBackgroundColor;
    UiColor m_dwForegroundColor;
    UiColor m_dwGradientColor;
    Image* m_pIndicatorImage;
};

} // namespace ui

#endif // UI_CONTROL_CIRCLEPROGRESS_H_
