#ifndef UI_RENDER_BITMAP_ALPHA_H_
#define UI_RENDER_BITMAP_ALPHA_H_

#include "duilib/Core/UiRect.h"

namespace ui
{

/** 对位图数据的Alpha值进行保存和恢复

    使用场景：使用 GDI/GDI+ 等不支持 Alpha 通道的 API 绘制位图后，Alpha 通道会丢失或被破坏；
    使用本类可以在绘制前把指定区域的 Alpha 清除为固定值，绘制完成后再恢复原来的 Alpha 值。

    像素格式要求：
    - 每个像素为 32 位、通道数为 4，内存布局为 BGRA 或 RGBA（Alpha 始终位于第 4 个字节）；
    - 位图按行连续存储即可，本类只按行偏移访问，不关心位图是自上而下还是自下而上存储；
    - 所有矩形的坐标都使用位图坐标，超出位图范围的部分会被裁剪。
*/
class DUILIB_API BitmapAlpha
{
public:
    /** 构造函数
    * @param [in] pPixel 位图像素数据的起始地址，必须有效，且至少包含 nWidth * nHeight * nChannels 字节
    * @param [in] nWidth 位图宽度（像素），必须大于 0
    * @param [in] nHeight 位图高度（像素），必须大于 0
    * @param [in] nChannels 每个像素的通道数，当前仅支持 4（BGRA/RGBA）
    */
    BitmapAlpha(uint8_t* pPixel, int32_t nWidth, int32_t nHeight, int32_t nChannels);
    ~BitmapAlpha();

public:
    /** 将矩形区域内的图像Alpha设定为指定值alpha(0 - 255)，同时清除图像数据，也设置为alpha
    * 说明：该函数按字节覆盖整个区域，RGB 与 Alpha 通道都会被设置为 alpha，
    *       调用后区域内不会保留原来的图像内容；通常用于绘制前清空区域，
    *       绘制完成后再通过 RestoreAlpha 恢复 Alpha 通道。
    * @param [in] rcDirty 需要清除的矩形区域（位图坐标，超出位图范围的部分会被裁剪）
    * @param [in] alpha 需要设定的值（0 - 255），传 0 表示完全透明
    */
    void ClearAlpha(const UiRect& rcDirty, uint8_t alpha) const;

    /** 恢复矩形区域内的图像Alpha值（按 ClearAlpha 时使用的 alpha 值恢复）
    * 恢复规则（只修改 Alpha 通道，RGB 通道保持不变）：
    * - alpha 为 0 时：Alpha 为 0 的像素恢复为 255（不透明），其余像素保持原值；
    * - alpha 不为 0 时：Alpha 等于 alpha 的像素恢复为 0，Alpha 为 0 的像素恢复为 255，其余像素保持原值。
    * 说明：该函数适用于 GDI 等 API 渲染位图导致 Alpha 通道丢失的情况，用于把 Alpha 通道补回来；
    *       如果渲染位图时通过 GDI+、AlphaBlend 等 API 设置了半透明的 Alpha 通道，
    *       则可能无法正确修正 Alpha 通道。
    * @param [in] rcDirty 需要恢复的矩形区域（位图坐标，超出位图范围的部分会被裁剪）
    * @param [in] rcShadowPadding 阴影边距（分别对应矩形的左/右/上/下边距的Padding值），
    *                             在 rcDirty 裁剪后的区域内再扣除该边距，边距区域不做恢复
    * @param [in] alpha 需要恢复的Alpha值，必须与调用 ClearAlpha 时传入的 alpha 值相同
    */
    void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) const;

    /** 恢复矩形区域内的图像Alpha值（无条件把区域内所有像素的Alpha设为255）
    * 说明：与上一个重载不同，该函数不区分像素当前的 Alpha 值，统一置为 255（不透明），
    *       RGB 通道保持不变；适用于区域内所有像素都需要恢复为完全不透明的场景。
    * @param [in] rcDirty 需要恢复的矩形区域（位图坐标，超出位图范围的部分会被裁剪）
    * @param [in] rcShadowPadding 阴影边距（分别对应矩形的左/右/上/下边距的Padding值），
    *                             在 rcDirty 裁剪后的区域内再扣除该边距，边距区域不做恢复
    */
    void RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding) const;

private:
    /** 图像数据
    */
    uint8_t* m_pPixel;

    /** 图像宽度
    */
    int32_t m_nWidth;

    /** 图像高度
    */
    int32_t m_nHeight;

    /** 通道数，目前只支持4（ARGB）
    */
    int32_t m_nChannels;
};
} // namespace ui

#endif // UI_RENDER_BITMAP_ALPHA_H_
