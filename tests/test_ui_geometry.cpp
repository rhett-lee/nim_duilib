// duilib/Core 基础几何类型的单元测试
//
// 覆盖 UiPoint / UiPointF / UiSize / UiSize16 / UiSize64 /
// UiRect / UiRectF / UiMargin / UiPadding 的构造、有效性校验、
// 偏移、膨胀/收缩、并集/交集、包含判断与比较运算。
// 这些类都是纯头文件实现，不依赖窗口与渲染环境。

#include "duilib/Core/UiTypes.h"
#include <functional>
#include <iostream>
#include <string>

using namespace ui;

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const std::function<std::string()>& message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message() << std::endl;
    }
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

#define CHECK_INT(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual " == " #expected) + ": expected " + \
               std::to_string(static_cast<int64_t>(expected)) + ", actual " + \
               std::to_string(static_cast<int64_t>(actual)); \
    })

#define CHECK_FLOAT(actual, expected, eps) \
    Check(std::fabs((double)(actual) - (double)(expected)) <= (eps), [&]() { \
        return std::string(#actual " ≈ " #expected) + ": expected " + \
               std::to_string((double)(expected)) + ", actual " + \
               std::to_string((double)(actual)); \
    })

/** UiPoint：构造、赋值、偏移、比较 */
void TestUiPoint()
{
    UiPoint pt;
    CHECK(pt.IsZero());
    CHECK_INT(pt.X(), 0);
    CHECK_INT(pt.Y(), 0);

    UiPoint pt2(10, -20);
    CHECK_INT(pt2.X(), 10);
    CHECK_INT(pt2.Y(), -20);
    CHECK(!pt2.IsZero());

    pt2.SetX(3);
    pt2.SetY(4);
    CHECK(pt2.Equals(3, 4));
    pt2.SetXY(5, 6);
    CHECK(pt2.Equals(UiPoint(5, 6)));
    CHECK(pt2 == UiPoint(5, 6));
    CHECK(pt2 != UiPoint(5, 7));

    pt2.Offset(2, -1);
    CHECK(pt2 == UiPoint(7, 5));
    pt2.Offset(UiPoint(-7, -5));
    CHECK(pt2.IsZero());
}

/** UiPointF：浮点坐标与从 UiPoint 构造 */
void TestUiPointF()
{
    UiPointF pt;
    CHECK(pt.IsZero());

    UiPointF pt2(1.5f, -2.5f);
    CHECK_FLOAT(pt2.X(), 1.5, 1e-6);
    CHECK_FLOAT(pt2.Y(), -2.5, 1e-6);
    CHECK(!pt2.IsZero());

    pt2.Offset(0.5f, 2.5f);
    CHECK(pt2.Equals(2.0f, 0.0f));
    pt2.Offset(UiPointF(-2.0f, 0.0f));
    CHECK(pt2.IsZero());

    pt2.SetXY(3.0f, 4.0f);
    CHECK(pt2 == UiPointF(3.0f, 4.0f));
    CHECK(pt2 != UiPointF(3.0f, 5.0f));

    UiPointF pt3 = UiPointF::MakeFromPoint(UiPoint(12, -34));
    CHECK_FLOAT(pt3.X(), 12.0, 1e-6);
    CHECK_FLOAT(pt3.Y(), -34.0, 1e-6);
}

/** UiSize / UiSize16 / UiSize64：构造、校验、清空与比较 */
void TestUiSize()
{
    UiSize size;
    CHECK(size.IsEmpty());
    CHECK(size.IsValid());
    CHECK_INT(size.Width(), 0);
    CHECK_INT(size.Height(), 0);

    UiSize size2(30, 40);
    CHECK_INT(size2.Width(), 30);
    CHECK_INT(size2.Height(), 40);
    CHECK(!size2.IsEmpty());

    size2.SetWidth(10);
    size2.SetHeight(20);
    CHECK(size2.Equals(10, 20));
    size2.SetWidthHeight(50, 60);
    CHECK(size2.Equals(UiSize(50, 60)));
    CHECK(size2 == UiSize(50, 60));
    CHECK(size2 != UiSize(50, 61));

    //拷贝构造与赋值
    UiSize size3(size2);
    CHECK(size3 == size2);
    UiSize size4;
    size4 = size2;
    CHECK(size4 == size2);

    size4.Clear();
    CHECK(size4.IsEmpty());

    //负数校验
    UiSize invalid(-1, -2);
    CHECK(!invalid.IsValid());
    invalid.Validate();
    CHECK(invalid.IsValid());
    CHECK_INT(invalid.Width(), 0);
    CHECK_INT(invalid.Height(), 0);

    //16 位版本
    UiSize16 size16(32000, 100);
    CHECK_INT(size16.Width(), 32000);
    CHECK_INT(size16.Height(), 100);
    UiSize16 invalid16(-1, 5);
    CHECK(!invalid16.IsValid());
    invalid16.Validate();
    CHECK_INT(invalid16.Width(), 0);
    CHECK_INT(invalid16.Height(), 5);
    CHECK(invalid16 == UiSize16(0, 5));
    CHECK(invalid16 != UiSize16(0, 6));
    invalid16.SetWidthHeight(7, 8);
    CHECK(invalid16.Equals(7, 8));

    //64 位版本
    UiSize64 size64(5000000000LL, -6000000000LL);
    CHECK_INT(size64.Width(), 5000000000LL);
    CHECK_INT(size64.Height(), -6000000000LL);
    CHECK(!size64.IsEmpty());
    size64.SetWidth(1);
    size64.SetHeight(2);
    CHECK(size64.Equals(UiSize64(1, 2)));
    CHECK(size64 == UiSize64(1, 2));
    CHECK(size64 != UiSize64(1, 3));
    size64.Clear();
    CHECK(size64.IsEmpty());
}

/** UiRect：尺寸、中心点、校验、偏移、膨胀与收缩 */
void TestUiRectBasics()
{
    UiRect rect;
    CHECK(rect.IsZero());
    CHECK(rect.IsEmpty());

    UiRect rect2(10, 20, 50, 80);
    CHECK_INT(rect2.Left(), 10);
    CHECK_INT(rect2.Top(), 20);
    CHECK_INT(rect2.Right(), 50);
    CHECK_INT(rect2.Bottom(), 80);
    CHECK_INT(rect2.Width(), 40);
    CHECK_INT(rect2.Height(), 60);
    CHECK_INT(rect2.CenterX(), 30);
    CHECK_INT(rect2.CenterY(), 50);
    CHECK(rect2.Center() == UiPoint(30, 50));
    CHECK(!rect2.IsEmpty());
    CHECK(rect2 != UiRect(10, 20, 51, 80));

    rect2.Clear();
    CHECK(rect2.IsZero());

    //宽高非法时，Validate 将其修正为空矩形
    UiRect invalid(100, 200, 50, 100);
    CHECK(invalid.IsEmpty());
    invalid.Validate();
    CHECK_INT(invalid.Width(), 0);
    CHECK_INT(invalid.Height(), 0);
    CHECK(invalid == UiRect(100, 200, 100, 200));

    //中心点使用 64 位中间值，避免极大坐标相加溢出
    UiRect big(INT32_MAX - 10, INT32_MAX - 6, INT32_MAX, INT32_MAX);
    CHECK_INT(big.CenterX(), INT32_MAX - 5);
    CHECK_INT(big.CenterY(), INT32_MAX - 3);
}

/** UiRect 的偏移、Inflate / Deflate 各种重载 */
void TestUiRectTransform()
{
    UiRect rect(10, 20, 50, 80);
    rect.Offset(5, -5);
    CHECK(rect == UiRect(15, 15, 55, 75));

    rect.Offset(UiPoint(-5, 5));
    CHECK(rect == UiRect(10, 20, 50, 80));

    //Inflate(dx, dy)：四边各扩大 dx/dy
    rect.Inflate(2, 3);
    CHECK(rect == UiRect(8, 17, 52, 83));
    rect.Deflate(2, 3);
    CHECK(rect == UiRect(10, 20, 50, 80));

    //Inflate(UiMargin)
    rect.Inflate(UiMargin(1, 2, 3, 4));
    CHECK(rect == UiRect(9, 18, 53, 84));
    rect.Deflate(UiMargin(1, 2, 3, 4));
    CHECK(rect == UiRect(10, 20, 50, 80));

    //Inflate(UiPadding)
    rect.Inflate(UiPadding(4, 3, 2, 1));
    CHECK(rect == UiRect(6, 17, 52, 81));
    rect.Deflate(UiPadding(4, 3, 2, 1));
    CHECK(rect == UiRect(10, 20, 50, 80));

    //四边独立的重载
    rect.Inflate(1, 2, 3, 4);
    CHECK(rect == UiRect(9, 18, 53, 84));
    rect.Deflate(1, 2, 3, 4);
    CHECK(rect == UiRect(10, 20, 50, 80));
}

/** UiRect 的并集与交集 */
void TestUiRectBoolean()
{
    //并集：两个非空矩形
    UiRect r1(0, 0, 10, 10);
    UiRect r2(5, 5, 20, 30);
    CHECK(r1.Union(r2));
    CHECK(r1 == UiRect(0, 0, 20, 30));

    //并集：参数为空矩形，结果保持自身
    UiRect r3(1, 2, 3, 4);
    CHECK(r3.Union(UiRect()));
    CHECK(r3 == UiRect(1, 2, 3, 4));

    //并集：自身为空矩形，结果等于参数
    UiRect r4;
    CHECK(r4.Union(UiRect(7, 8, 9, 10)));
    CHECK(r4 == UiRect(7, 8, 9, 10));

    //并集：两者都为空
    UiRect r5;
    CHECK(!r5.Union(UiRect()));

    //交集：重叠
    UiRect a(0, 0, 10, 10);
    CHECK(a.Intersect(UiRect(5, 5, 20, 20)));
    CHECK(a == UiRect(5, 5, 10, 10));

    //交集：不相交，结果为空且返回 false
    UiRect b(0, 0, 10, 10);
    CHECK(!b.Intersect(UiRect(20, 20, 30, 30)));
    CHECK(b.IsEmpty());

    //静态交集：输出新矩形
    UiRect c;
    CHECK(UiRect::Intersect(c, UiRect(0, 0, 10, 10), UiRect(-5, 5, 5, 20)));
    CHECK(c == UiRect(0, 5, 5, 10));

    UiRect d(1, 1, 2, 2);
    CHECK(!UiRect::Intersect(d, UiRect(0, 0, 10, 10), UiRect(20, 20, 30, 30)));
    CHECK(d.IsEmpty());

    //相切（边界重合）视为不相交
    UiRect e(0, 0, 10, 10);
    CHECK(!e.Intersect(UiRect(10, 0, 20, 10)));
}

/** UiRect 的包含判断 */
void TestUiRectContains()
{
    const UiRect rect(10, 20, 50, 80);
    CHECK(rect.ContainsPt(UiPoint(10, 20)));
    CHECK(rect.ContainsPt(49, 79));
    CHECK(!rect.ContainsPt(UiPoint(50, 40))); //right 为开区间
    CHECK(!rect.ContainsPt(40, 80));          //bottom 为开区间
    CHECK(!rect.ContainsPt(UiPoint(9, 20)));

    //矩形包含
    CHECK(rect.ContainsRect(UiRect(20, 30, 40, 60)));
    CHECK(rect.ContainsRect(rect));
    CHECK(!rect.ContainsRect(UiRect(20, 30, 60, 60)));
    CHECK(!rect.ContainsRect(UiRect())); //空矩形不被包含
    UiRect empty;
    CHECK(!empty.ContainsRect(UiRect()));
    CHECK(!empty.ContainsRect(rect));
}

/** UiRectF：浮点矩形的相应语义 */
void TestUiRectF()
{
    UiRectF rect;
    CHECK(rect.IsEmpty());
    CHECK(rect.IsZero());

    UiRectF rect2(10, 20, 50, 80);
    CHECK_FLOAT(rect2.Width(), 40.0, 1e-6);
    CHECK_FLOAT(rect2.Height(), 60.0, 1e-6);
    CHECK_FLOAT(rect2.CenterX(), 30.0, 1e-6);
    CHECK_FLOAT(rect2.CenterY(), 50.0, 1e-6);
    UiPointF center = rect2.Center();
    CHECK_FLOAT(center.X(), 30.0, 1e-6);
    CHECK_FLOAT(center.Y(), 50.0, 1e-6);

    UiRectF rect3(0.5f, 1.5f, 2.5f, 3.5f);
    CHECK(rect3 == UiRectF(0.5f, 1.5f, 2.5f, 3.5f));
    CHECK(rect3 != UiRectF(0.5f, 1.5f, 2.5f, 3.6f));

    //校验
    UiRectF invalid(10.0f, 10.0f, 5.0f, 5.0f);
    CHECK(invalid.IsEmpty());
    invalid.Validate();
    CHECK_FLOAT(invalid.Width(), 0.0, 1e-6);
    CHECK_FLOAT(invalid.Height(), 0.0, 1e-6);

    //偏移与膨胀/收缩
    rect2.Offset(1.5f, -1.5f);
    CHECK(rect2 == UiRectF(11.5f, 18.5f, 51.5f, 78.5f));
    rect2.Offset(UiPoint(-1, 1));
    CHECK(rect2 == UiRectF(10.5f, 19.5f, 50.5f, 79.5f));
    rect2.Inflate(0.5f, 0.5f);
    CHECK(rect2 == UiRectF(10.0f, 19.0f, 51.0f, 80.0f));
    rect2.Deflate(0.5f, 0.5f);
    CHECK(rect2 == UiRectF(10.5f, 19.5f, 50.5f, 79.5f));
    rect2.Inflate(UiMargin(1, 1, 1, 1));
    CHECK(rect2 == UiRectF(9.5f, 18.5f, 51.5f, 80.5f));
    rect2.Deflate(UiPadding(1, 1, 1, 1));
    CHECK(rect2 == UiRectF(10.5f, 19.5f, 50.5f, 79.5f));
    rect2.Inflate(0.5f, 0.5f, 0.5f, 0.5f);
    CHECK(rect2 == UiRectF(10.0f, 19.0f, 51.0f, 80.0f));
    rect2.Deflate(0.5f, 0.5f, 0.5f, 0.5f);
    CHECK(rect2 == UiRectF(10.5f, 19.5f, 50.5f, 79.5f));

    //并集/交集
    UiRectF u(0.0f, 0.0f, 10.0f, 10.0f);
    CHECK(u.Union(UiRectF(5.0f, 5.0f, 20.0f, 30.0f)));
    CHECK(u == UiRectF(0.0f, 0.0f, 20.0f, 30.0f));
    CHECK(u.Union(UiRectF())); //与空矩形取并集，结果仍非空
    CHECK(u == UiRectF(0.0f, 0.0f, 20.0f, 30.0f));

    UiRectF i(0.0f, 0.0f, 10.0f, 10.0f);
    CHECK(i.Intersect(UiRectF(5.0f, 5.0f, 20.0f, 20.0f)));
    CHECK(i == UiRectF(5.0f, 5.0f, 10.0f, 10.0f));
    UiRectF out;
    CHECK(UiRectF::Intersect(out, UiRectF(0.0f, 0.0f, 10.0f, 10.0f), UiRectF(-5.0f, 5.0f, 5.0f, 20.0f)));
    CHECK(out == UiRectF(0.0f, 5.0f, 5.0f, 10.0f));

    //包含判断
    CHECK(u.ContainsPt(0.0f, 0.0f));
    CHECK(u.ContainsPt(19.5f, 29.5f));
    CHECK(!u.ContainsPt(20.0f, 0.0f));
    CHECK(!u.ContainsPt(UiPoint(0, 30)));
    CHECK(u.ContainsRect(UiRectF(1.0f, 1.0f, 19.0f, 29.0f)));
    CHECK(!u.ContainsRect(UiRectF(1.0f, 1.0f, 21.0f, 29.0f)));
    CHECK(!u.ContainsRect(UiRectF()));

    //从 UiRect 构造
    UiRectF fromRect = UiRectF::MakeFromRect(UiRect(1, 2, 3, 4));
    CHECK(fromRect == UiRectF(1.0f, 2.0f, 3.0f, 4.0f));
}

/** UiMargin / UiPadding 及其 16 位版本 */
void TestMarginPadding()
{
    UiMargin margin;
    CHECK(margin.IsEmpty());
    CHECK(margin == UiMargin());
    CHECK(margin != UiMargin(1, 0, 0, 0));

    UiMargin margin2(-1, 2, -3, 4);
    CHECK(!margin2.IsEmpty());
    CHECK(margin2 == UiMargin(-1, 2, -3, 4));
    margin2.Validate();
    CHECK(margin2 == UiMargin(0, 2, 0, 4));

    margin2.Clear();
    CHECK(margin2.IsEmpty());

    UiPadding padding;
    CHECK(padding.IsEmpty());
    UiPadding padding2(5, -6, 7, -8);
    CHECK(!padding2.IsEmpty());
    padding2.Validate();
    CHECK(padding2 == UiPadding(5, 0, 7, 0));
    CHECK(padding2 != UiPadding(5, 0, 7, 1));
    padding2.Clear();
    CHECK(padding2.IsEmpty());

    UiMargin16 margin16(1, 2, 3, 4);
    CHECK_INT(margin16.left, 1);
    CHECK_INT(margin16.top, 2);
    CHECK_INT(margin16.right, 3);
    CHECK_INT(margin16.bottom, 4);

    UiPadding16 padding16(9, 8, 7, 6);
    CHECK_INT(padding16.left, 9);
    CHECK_INT(padding16.top, 8);
    CHECK_INT(padding16.right, 7);
    CHECK_INT(padding16.bottom, 6);
}

} // namespace

int main()
{
    TestUiPoint();
    TestUiPointF();
    TestUiSize();
    TestUiRectBasics();
    TestUiRectTransform();
    TestUiRectBoolean();
    TestUiRectContains();
    TestUiRectF();
    TestMarginPadding();

    std::cout << "ui geometry tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
