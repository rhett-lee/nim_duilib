#ifndef UI_RENDER_SKIA_MATRIX_H_
#define UI_RENDER_SKIA_MATRIX_H_

#include "duilib/Render/IRender.h"

class SkMatrix;

namespace ui 
{

/** Matrix_Skia 是 IMatrix 的 Skia 实现，内部封装 SkMatrix，功能要点如下：
 *  1. 提供平移、缩放、旋转、错切、矩阵连接与求逆等变换操作
 *  2. 支持判断单位矩阵，并对点坐标应用当前变换
 */
class Matrix_Skia : public IMatrix
{
public:
    Matrix_Skia();
    virtual ~Matrix_Skia();

    /// 重写父类方法，请参考父类声明：平移
    virtual void Translate(float offsetX, float offsetY) override;

    /// 重写父类方法，请参考父类声明：缩放
    virtual void Scale(float scaleX, float scaleY) override;
    virtual void Scale(float scaleX, float scaleY, float px, float py) override;

    /// 重写父类方法，请参考父类声明：旋转
    virtual void Rotate(float angle) override;
    virtual void RotateAt(float angle, float px, float py) override;

    /// 重写父类方法，请参考父类声明：错切
    virtual void Skew(float kx, float ky) override;
    virtual void Skew(float kx, float ky, float px, float py) override;

    /// 重写父类方法，请参考父类声明：重置、连接、求逆与点映射
    virtual void Reset() override;
    virtual void Concat(const IMatrix* pMatrix) override;
    virtual bool Invert() override;
    virtual bool IsIdentity() const override;
    virtual UiPointF MapPoint(const UiPointF& pt) const override;

    /** 获取内部的 SkMatrix 对象
    * @return 返回 SkMatrix 指针，不可为 nullptr
    */
    SkMatrix* GetMatrix() { return m_matrix.get(); }

    /** 获取内部的 SkMatrix 对象（只读）
    * @return 返回 SkMatrix 指针，不可为 nullptr
    */
    const SkMatrix* GetMatrix() const { return m_matrix.get(); }

private:
    /** 内部封装的 SkMatrix 对象
    */
    std::unique_ptr<SkMatrix> m_matrix;
};

} // namespace ui

#endif // UI_RENDER_SKIA_MATRIX_H_
