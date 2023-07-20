#ifndef UI_CORE_UITYPES_H_
#define UI_CORE_UITYPES_H_

#pragma once

#include "duilib/Core/UiSize.h"
#include "duilib/Core/UiSize64.h"
#include "duilib/Core/UiPoint.h"
#include "duilib/Core/UiRect.h"
#include "duilib/Core/UiColor.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Core/UiPadding.h"
#include "duilib/Core/UiMargin.h"
#include "duilib/Core/UiFixedInt.h"
#include "duilib/Core/UiEstInt.h"

namespace ui
{
    /** 设置的控件大小
    */
    class UILIB_API UiFixedSize
    {
    public:
        /** 宽度
        */
        UiFixedInt cx;

        /** 高度
        */
        UiFixedInt cy;
    };

    /** 估算的控件大小（相比UiFixedSize，没有Auto类型）
    */
    class UILIB_API UiEstSize
    {
    public:
        /** 宽度
        */
        UiEstInt cx;

        /** 高度
        */
        UiEstInt cy;
    };

    /** 从UiFixedInt生成UiEstInt
    */
    inline UiEstInt MakeEstInt(const UiFixedInt& fixedInt)
    {
        UiEstInt estInt;
        if (fixedInt.IsStretch()) {
            estInt.SetStretch();
        }
        else if (fixedInt.IsInt32()) {
            estInt.SetInt32(fixedInt.GetInt32());
        }
        else {
            estInt.SetInt32(0);
        }
        return estInt;
    }

    /** 从UiFixedSize生成UiEstSize
    */
    inline UiEstSize MakeEstSize(const UiFixedSize& fixedSize)
    {
        UiEstSize estSize;
        estSize.cx = MakeEstInt(fixedSize.cx);
        estSize.cy = MakeEstInt(fixedSize.cy);
        return estSize;
    }

    /** 从UiSize生成UiEstSize
    */
    inline UiEstSize MakeEstSize(const UiSize& size)
    {
        UiEstSize estSize;
        estSize.cx = UiEstInt(size.cx);
        estSize.cy = UiEstInt(size.cy);
        return estSize;
    }

    /** 从UiSize生成UiEstSize
    */
    inline UiSize MakeSize(const UiEstSize& estSize)
    {
        ASSERT(estSize.cx.IsInt32() && estSize.cy.IsInt32());
        UiSize size(estSize.cx.GetInt32(), estSize.cy.GetInt32());
        return size;
    }

    /** 将64位整型值转换位32位整型值
    */
    inline int32_t TruncateToInt32(int64_t x)
    {
        x = x < INT32_MAX ? x : INT32_MAX;
        x = x > INT32_MIN ? x : INT32_MIN;
        return static_cast<int32_t>(x);
    }

}//namespace ui

#endif // UI_CORE_UITYPES_H_
