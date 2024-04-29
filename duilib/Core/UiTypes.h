#ifndef UI_CORE_UITYPES_H_
#define UI_CORE_UITYPES_H_

#pragma once

#include "duilib/Core/UiSize.h"
#include "duilib/Core/UiSize64.h"
#include "duilib/Core/UiPoint.h"
#include "duilib/Core/UiPointF.h"
#include "duilib/Core/UiRect.h"
#include "duilib/Core/UiColor.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Core/UiPadding.h"
#include "duilib/Core/UiMargin.h"
#include "duilib/Core/UiFixedInt.h"
#include "duilib/Core/UiEstInt.h"
#include "duilib/Core/UiString.h"

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

    /** 估算控件大小的结果
    */
    class UILIB_API UiEstResult
    {
    public:
        /** 是否需要重新评估大小
        */
        bool m_bReEstimateSize = true;

        /** 本次估算时的可用矩形大小(这个参数影响会估算结果)
        */
        UiSize m_szAvailable;

        /** 控件的已估算大小（长度和宽度），相当于估算后的缓存值
        */
        UiEstSize m_szEstimateSize;
    };

    /** 从UiFixedInt生成UiEstInt
    */
    inline UiEstInt MakeEstInt(const UiFixedInt& fixedInt)
    {
        UiEstInt estInt;
        if (fixedInt.IsStretch()) {
            estInt.SetStretch(fixedInt.GetStretchPercentValue());
        }
        else if (fixedInt.IsInt32()) {
            estInt.SetInt32(fixedInt.GetInt32());
        }
        else {
            estInt.SetInt32(0);
        }
        return estInt;
    }

    /** 计算拉伸类型的长度值
    */
    inline int32_t CalcStretchValue(const UiEstInt& estInt, int32_t nAvailable)
    {
        if (estInt.IsStretch()) {
            int32_t nStretchValue = estInt.GetStretchPercentValue(); //代表百分比值，取值范围为：(0, 100]
            if ((nStretchValue > 0) && (nStretchValue < 100)) {
                //返回按百分比计算的值, 四舍五入
                return static_cast<int32_t>(nAvailable * nStretchValue / 100.0f + 0.5f);
            }
            else {
                //返回原值
                return nAvailable;
            }
        }
        //其他情况，返回原值(容错)
        return nAvailable;
    }

    /** 计算拉伸类型的长度值
    */
    inline int32_t CalcStretchValue(const UiFixedInt& fixedInt, int32_t nAvailable)
    {
        if (fixedInt.IsStretch()) {
            int32_t nStretchValue = fixedInt.GetStretchPercentValue(); //代表百分比值，取值范围为：(0, 100]
            if ((nStretchValue > 0) && (nStretchValue < 100)) {
                //返回按百分比计算的值, 四舍五入
                return static_cast<int32_t>(nAvailable * nStretchValue / 100.0f + 0.5f);
            }
            else {
                //返回原值
                return nAvailable;
            }
        }
        //其他情况，返回原值(容错)
        return nAvailable;
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
        ASSERT((x >= INT32_MIN) && (x <= INT32_MAX) );
        x = x < INT32_MAX ? x : INT32_MAX;
        x = x > INT32_MIN ? x : INT32_MIN;
        return static_cast<int32_t>(x);
    }

    /** 将32位y有符号整型值转换位16位无符号整型值
    */
    inline uint16_t TruncateToUInt16(uint32_t x)
    {
        ASSERT(x <= UINT16_MAX);
        x = x < UINT16_MAX ? x : UINT16_MAX;
        return static_cast<uint16_t>(x);
    }

    /** 将32位无符号整型值转换位16位无符号整型值
    */
    inline uint16_t TruncateToUInt16(int32_t x)
    {
        ASSERT((x >= 0) && (x <= UINT16_MAX));
        x = x > 0 ? x : 0;
        x = x < UINT16_MAX ? x : UINT16_MAX;
        return static_cast<uint16_t>(x);
    }

    /** 将32位整型值转换位8位整型值
    */
    inline int8_t TruncateToInt8(int32_t x)
    {
        ASSERT((x >= INT8_MIN) && (x <= INT8_MAX));
        x = x < INT8_MAX ? x : INT8_MAX;
        x = x > INT8_MIN ? x : INT8_MIN;
        return static_cast<int8_t>(x);
    }

    /** 将32位整型值转换位8位整型值
    */
    inline uint8_t TruncateToUInt8(int32_t x)
    {
        ASSERT((x >= 0) && (x <= UINT8_MAX));
        x = x > 0 ? x : 0;
        x = x < UINT8_MAX ? x : UINT8_MAX;
        return static_cast<uint8_t>(x);
    }

}//namespace ui

#endif // UI_CORE_UITYPES_H_
