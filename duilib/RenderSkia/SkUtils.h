/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef UI_RENDER_SKIA_SK_UTILS_H_
#define UI_RENDER_SKIA_SK_UTILS_H_

#include "include/core/SkTypes.h"

//该文件原始文件的出处：skia/chrome_67/src/core/SkUtils.h
//基于原始文件，有修改，以兼容最新版本的skia代码（2023-06-25）
//原始文件从后续版本就删除了。

namespace ui
{

#define kMaxBytesInUTF8Sequence     4

#ifdef SK_DEBUG
    int SkUTF8_LeadByteToCount(unsigned c);
#else
    #define SkUTF8_LeadByteToCount(c)   ((((0xE5 << 24) >> ((unsigned)c >> 4 << 1)) & 3) + 1)
#endif

inline int SkUTF8_CountUTF8Bytes(const char utf8[]) {
    SkASSERT(utf8);
    return SkUTF8_LeadByteToCount(*(const uint8_t*)utf8);
}

/**
 * @returns -1  iff invalid UTF8 byte,
 *           0  iff UTF8 continuation byte,
 *           1  iff ASCII byte,
 *           2  iff leading byte of 2-byte sequence,
 *           3  iff leading byte of 3-byte sequence, and
 *           4  iff leading byte of 4-byte sequence.
 *
 * I.e.: if return value > 0, then gives length of sequence.
*/
int SkUTF8_ByteType(uint8_t c);
bool SkUTF8_TypeIsValidLeadingByte(int type);
bool SkUTF8_ByteIsContinuation(uint8_t c);

int SkUTF8_CountUnichars(const char utf8[]);

/** These functions are safe: invalid sequences will return -1; */
int SkUTF8_CountUnichars(const void* utf8, size_t byteLength);
int SkUTF16_CountUnichars(const void* utf16, size_t byteLength);
int SkUTF32_CountUnichars(const void* utf32, size_t byteLength);

/** This function is safe: invalid UTF8 sequences will return -1
 *  When -1 is returned, ptr is unchanged.
 *  Precondition: *ptr < end;
 */
SkUnichar SkUTF8_NextUnicharWithError(const char** ptr, const char* end);

/** this version replaces invalid utf-8 sequences with code point U+FFFD. */
inline SkUnichar SkUTF8_NextUnichar(const char** ptr, const char* end) {
    SkUnichar val = SkUTF8_NextUnicharWithError(ptr, end);
    if (val < 0) {
        *ptr = end;
        return 0xFFFD;  // REPLACEMENT CHARACTER
    }
    return val;
}

SkUnichar   SkUTF8_ToUnichar(const char utf8[]);
SkUnichar   SkUTF8_NextUnichar(const char**);
SkUnichar   SkUTF8_PrevUnichar(const char**);

/** Return the number of bytes need to convert a unichar
    into a utf8 sequence. Will be 1..kMaxBytesInUTF8Sequence,
    or 0 if uni is illegal.
*/
size_t      SkUTF8_FromUnichar(SkUnichar uni, char utf8[] = nullptr);

///////////////////////////////////////////////////////////////////////////////

#define SkUTF16_IsHighSurrogate(c)  (((c) & 0xFC00) == 0xD800)
#define SkUTF16_IsLowSurrogate(c)   (((c) & 0xFC00) == 0xDC00)

int SkUTF16_CountUnichars(const uint16_t utf16[]);
// returns the current unichar and then moves past it (*p++)
SkUnichar SkUTF16_NextUnichar(const uint16_t**);
// this guy backs up to the previus unichar value, and returns it (*--p)
SkUnichar SkUTF16_PrevUnichar(const uint16_t**);
size_t SkUTF16_FromUnichar(SkUnichar uni, uint16_t utf16[] = nullptr);

size_t SkUTF16_ToUTF8(const uint16_t utf16[], int numberOf16BitValues,
                      char utf8[] = nullptr);

inline bool SkUnichar_IsVariationSelector(SkUnichar uni) {
/*  The 'true' ranges are:
 *      0x180B  <= uni <=  0x180D
 *      0xFE00  <= uni <=  0xFE0F
 *      0xE0100 <= uni <= 0xE01EF
 */
    if (uni < 0x180B || uni > 0xE01EF) {
        return false;
    }
    if ((uni > 0x180D && uni < 0xFE00) || (uni > 0xFE0F && uni < 0xE0100)) {
        return false;
    }
    return true;
}

namespace SkHexadecimalDigits {
    extern const char gUpper[16];  // 0-9A-F
    extern const char gLower[16];  // 0-9a-f
}

}// namespace ui

#endif //UI_RENDER_SKIA_SK_UTILS_H_
