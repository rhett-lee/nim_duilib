#ifndef DUILIB_VERSION_H_
#define DUILIB_VERSION_H_

// Duilib 版本宏定义（全部带 DUILIB_ 前缀，统一规范，版本号数字最大值为65535）
// 后面两个数字目前使用是4位年，2位月2位日，实际使用时，可以根据自己的情况来定义规则
// 注意：BUILD 按 MMDD 书写但不要写前导零（0105 在 C/RC 中会被当作八进制）
#define DUILIB_VER_MAJOR    1
#define DUILIB_VER_MINOR    0
#define DUILIB_VER_PATCH    2026
#define DUILIB_VER_BUILD    1023

#endif //DUILIB_VERSION_H_
