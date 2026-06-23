#ifndef UI_RENDER_AUTO_CLIP_H_
#define UI_RENDER_AUTO_CLIP_H_

#include "duilib/Core/UiRect.h"

namespace ui
{

class IRender;

/** @class AutoClip
  * @brief 基于 RAII 机制自动管理 IRender 的裁剪区域（Clip Region）。
  *
  * 本类是一个轻量级的"作用域守卫"(Scope Guard)：
  *   - 构造时，根据传入的参数，对绑定的 IRender 设置一个矩形或圆角矩形裁剪区域；
  *   - 析构时，自动将 IRender 的裁剪区域恢复到设置之前的状态。
  *
  * 使用方式（推荐）：
  * @code
  *   void DrawSomething(IRender* pRender, const UiRect& rc)
  *   {
  *       // 进入此作用域时设置裁剪区域，离开此作用域时自动清除
  *       AutoClip clip(pRender, rc);
  *
  *       // 此处所有绘制操作都会被裁剪到 rc 范围内
  *       pRender->DrawRect(...);
  *       // ...
  *   } // <- clip 在此析构，自动调用 IRender::ClearClip()
  * @endcode
  *
  * 这种设计可以有效避免：
  *   1. 忘记调用 ClearClip() 导致裁剪区域泄漏，影响后续绘制；
  *   2. 在多条提前 return / 异常分支中重复书写 ClearClip() 代码。
  *
  * @note 该类只持有外部传入的 IRender 指针，并不管理其生命周期。
  *       调用方必须保证在 AutoClip 对象存活期间 pRender 一直有效。
  */
class DUILIB_API AutoClip
{
public:
    /** @brief 构造一个矩形裁剪守卫。
      *
      * 当 @p bClip 为 true 时，构造函数会立即调用
      * IRender::SetClip(rc)，将 @p rc 与当前裁剪区取交集作为新的裁剪区域；
      * 当 @p bClip 为 false 时，本对象不会对 IRender 做任何操作，
      * 析构时也不会调用 ClearClip()，相当于一个"空操作"守卫。
      *
      * @param[in] pRender 目标渲染对象指针，必须保证在 AutoClip 生命周期内有效。
      *                    当 @p bClip 为 true 时不允许传 nullptr。
      * @param[in] rc      需要设置的矩形裁剪区域（坐标相对于当前渲染目标的左上角）。
      * @param[in] bClip   是否启用裁剪。默认为 true。
      *                    传 false 可用于通过单个变量控制是否启用裁剪的模板/泛型代码。
      */
    AutoClip(IRender* pRender, const UiRect& rc, bool bClip = true);

    /** @brief 构造一个圆角矩形裁剪守卫。
      *
      * 当 @p bClip 为 true 时，构造函数会立即调用
      * IRender::SetRoundClip(rcRound, fRoundWidth, fRoundHeight)，
      * 将一个圆角矩形与当前裁剪区取交集作为新的裁剪区域；
      * 当 @p bClip 为 false 时，本对象不会对 IRender 做任何操作。
      *
      * @param[in] pRender      目标渲染对象指针，必须保证在 AutoClip 生命周期内有效。
      *                        当 @p bClip 为 true 时不允许传 nullptr。
      * @param[in] rcRound      圆角矩形裁剪区域的外接矩形（坐标相对于当前渲染目标的左上角）。
      * @param[in] fRoundWidth  圆角的水平半径（X 方向圆角半径）。
      * @param[in] fRoundHeight 圆角的垂直半径（Y 方向圆角半径）。
      * @param[in] bClip        是否启用裁剪。默认为 true。
      */
    AutoClip(IRender* pRender, const UiRect& rcRound, float fRoundWidth, float fRoundHeight, bool bClip = true);

    /** @brief 析构函数。
      *
      * 当且仅当构造时实际启用了裁剪（即 m_bClip 为 true）且 m_pRender 有效时，
      * 才会调用 IRender::ClearClip()，将渲染状态恢复到裁剪之前的水平。
      *
      * @note 析构函数不会抛出异常。
      */
    ~AutoClip();

private:
    /** @brief 绑定的渲染对象指针。
      *  仅在启用裁剪时（即 m_bClip == true）才可能为非空。
      *  不持有所有权，生命周期由调用方负责管理。
      */
    IRender* m_pRender;

    /** 裁剪区域的设备上下文标志
    */
    int32_t m_nClipState;

    /** @brief 标记本次构造是否真的设置了裁剪区域。
      *  - true  ：构造时调用了 SetClip/SetRoundClip，析构时需要 ClearClip；
      *  - false ：构造时未设置裁剪（用户传入 bClip=false 或 pRender 非法），
      *            析构时不需要做任何清理。
      */
    bool m_bClip;
};

} // namespace ui

#endif // UI_RENDER_AUTO_CLIP_H_
