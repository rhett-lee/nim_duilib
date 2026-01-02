#ifndef UI_CONTROL_ICON_CONTROL_H_
#define UI_CONTROL_ICON_CONTROL_H_

#include "duilib/Core/Control.h"

namespace ui
{
class IBitmap;

/** 用于显示图标的控件，适合小图标类的图片资源显示，功能较简单
 *  位图的数据格式为32位的位图(外部处理)：
 *    Windows平台为BGRA: struct BGRA { uint8_t b, g, r, a; };
 *    非Windows平台RGBA: struct RGBA { uint8_t r, g, b, a; };
 *  位图的alpha type为kPremul_SkAlphaType(外部预乘)
 *  线程模型：单线程，只能在UI线程中使用
 */
class IconControl : public Control
{
    typedef Control BaseClass;
public:
    explicit IconControl(Window* pWindow);
    virtual ~IconControl() override;
    IconControl(const IconControl&) = delete;
    IconControl& operator=(const IconControl&) = delete;

    /** 获取控件类型
    */
    virtual DString GetType() const override;

public:
    /** 设置图标的位图数据
    * @param [in] nWidth 宽度
    * @param [in] nHeight 高度
    * @param [in] pPixelBits 位图数据
    * @param [in] nPixelBitsSize 位图数据的长度（按字节）
    */
    bool SetIconData(int32_t nWidth, int32_t nHeight, const uint8_t* pPixelBits, int32_t nPixelBitsSize);

    /** 清除图标数据
    */
    void ClearIconData();

    /** 获取当前是否含有图标数据
    */
    bool HasIconData() const;

protected:
    /** 重写父控件绘制函数
    */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

private:
    /** 用于绘制的位图数据
    */
    std::unique_ptr<IBitmap> m_pBitmap;
};

}//namespace ui

#endif //UI_CONTROL_ICON_CONTROL_H_
