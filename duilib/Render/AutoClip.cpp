// AutoClip.cpp
// -----------------------------------------------------------------------------
// AutoClip 类的实现。
// 本文件实现的是一个 RAII 风格的"裁剪区域作用域守卫"：
//   - 构造时，根据用户传入的参数，调用 IRender::SetClip 或
//     IRender::SetRoundClip 设置一个新的裁剪区域；
//   - 析构时，如果确实设置过裁剪区域，则调用 IRender::ClearClip
//     将渲染状态恢复到设置前的水平。
//
// 这样可以让上层调用者通过简单的局部变量声明来"自动"管理裁剪区域，
// 避免忘记清理或提前 return 时造成裁剪区域泄漏的问题。
// -----------------------------------------------------------------------------

#include "AutoClip.h"
#include "duilib/Render/IRender.h"

namespace ui
{

// 构造一个矩形裁剪守卫。
// 行为说明：
//   1. 默认初始化成员：m_pRender = nullptr，m_bClip = false；
//   2. 仅当 bClip 为 true 时，才真正尝试对 IRender 设置裁剪区域；
//   3. 启用裁剪时要求 pRender 必须非空，使用 ASSERT 在 Debug 下提示，
//      Release 下即使 pRender 为空也不会崩溃（析构时已做空指针保护）。
AutoClip::AutoClip(IRender* pRender, const UiRect& rc, bool bClip)
{
    // 先将成员初始化为安全状态，防止对象在 bClip=false 路径下被错误使用
    m_pRender = nullptr;
    m_bClip = false;
    m_nClipState = -1;

    if (bClip) {
        // 记录"本次构造确实设置了裁剪区域"，析构时据此决定是否需要 ClearClip
        m_bClip = bClip;
        // Debug 断言：启用裁剪时必须传入有效的渲染对象指针
        ASSERT(pRender != nullptr);

        // 保存渲染对象指针，供析构时调用 ClearClip
        m_pRender = pRender;

        // 二次判空：在 Release 编译下避免对空指针解引用
        if (m_pRender != nullptr) {
            // 设置矩形裁剪区域，内部会与当前裁剪区取交集并保存旧状态
            m_nClipState = m_pRender->SetClip(rc);
        }
    }
}

// 构造一个圆角矩形裁剪守卫。
// 行为与矩形版本一致，只是最终调用的是 SetRoundClip。
AutoClip::AutoClip(IRender* pRender, const UiRect& rcRound,
                   float fRoundWidth, float fRoundHeight, bool bClip)
{
    // 同样先将成员重置为安全状态
    m_pRender = nullptr;
    m_bClip = false;
    m_nClipState = -1;

    if (bClip) {
        m_bClip = bClip;
        ASSERT(pRender != nullptr);

        m_pRender = pRender;

        if (m_pRender != nullptr) {
            // 设置圆角矩形裁剪区域，内部会与当前裁剪区取交集并保存旧状态
            m_nClipState = m_pRender->SetRoundClip(rcRound, fRoundWidth, fRoundHeight);
        }
    }
}

// 析构函数：仅当确实设置过裁剪且渲染对象有效时，才清除裁剪区域。
// 这样做的好处：
//   1. 与构造时"对称"——有 SetClip/SetRoundClip 就有 ClearClip；
//   2. 对 bClip=false 的"空操作"守卫，析构时不会产生任何副作用；
//   3. 通过 m_pRender != nullptr 的检查，保证即使在 Release 下也不会
//      对空指针调用 ClearClip，从而避免崩溃。
AutoClip::~AutoClip()
{
    if (m_bClip && (m_pRender != nullptr)) {
        // 清除当前裁剪区域，并恢复至最近一次保存的状态
        m_pRender->ClearClip(m_nClipState);
    }
}

} // namespace ui
