#pragma once

// Duilib 版本宏定义（全部带 DUILIB_ 前缀，统一规范，版本号数字最大值为65535）
// 后面两个数字目前使用是4位年，2位月2位日，实际使用时，可以根据自己的情况来定义规则
#define DUILIB_VER_MAJOR          1
#define DUILIB_VER_MINOR          0
#define DUILIB_VER_PATCH          2026
#define DUILIB_VER_BUILD          0416

// 辅助宏：把数字转成字符串
#define STRINGIFY(x)              #x
#define TOSTRING(x)               STRINGIFY(x)

// 拼接版本号（自动生成 "1.0.0.1"格式的版本号）
#define DUILIB_VER_FILEVERSION_STR  TOSTRING(DUILIB_VER_MAJOR) "." \
                                    TOSTRING(DUILIB_VER_MINOR) "." \
                                    TOSTRING(DUILIB_VER_PATCH) "." \
                                    TOSTRING(DUILIB_VER_BUILD)

#define DUILIB_VER_PRODVERSION_STR  DUILIB_VER_FILEVERSION_STR

// 数字版本（给 RC 使用）
#define DUILIB_VER_FILEVERSION    DUILIB_VER_MAJOR,DUILIB_VER_MINOR,DUILIB_VER_PATCH,DUILIB_VER_BUILD
#define DUILIB_VER_PRODVERSION    DUILIB_VER_MAJOR,DUILIB_VER_MINOR,DUILIB_VER_PATCH,DUILIB_VER_BUILD

// 公司/产品信息
#define DUILIB_VER_COMPANY        "https://github.com/rhett-lee/nim_duilib"
#define DUILIB_VER_DESC           "nim_duilib DLL"
#define DUILIB_VER_COPYRIGHT      "Copyright (C) 2026"
#define DUILIB_VER_PRODUCTNAME    "nim_duilib"
