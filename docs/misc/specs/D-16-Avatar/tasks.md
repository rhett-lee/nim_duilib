# Tasks: D-16 Avatar 头像

- [x] T1 新增 `duilib/Control/Avatar.h`：`class Avatar : public Control`、`enum class AvatarShape : uint8_t
      { kCircle = 0, kSquare = 1 }`、属性/API 声明（`size` / `shape` / `corner` / `image` / `icon` /
      `icon_scale` / `text` / `text_id` / `font` / `text_color` / `text_scale` / `color` / `palette`）、
      四个绘制钩子覆写与 `GetEstimateImage()` / `ClearImageCache()` / `OnLanguageChanged()` 覆写。
- [x] T2 新增 `duilib/Control/Avatar.cpp`：属性解析（`size` 预设 small/default/large 与数值、
      `shape` 忽略大小写与 `_`/`-`）、默认固定尺寸 32、方形 `corner` 走 `SetBorderRound()`、
      圆形绘制/裁剪/描边按 `GetRect()` 实时算半径（`min(w,h)/2`，非正方形时为椭圆）、
      图片 cover + 图标 contain + 文字按 `text_scale` 换算字体 zoom 并在超宽时降 zoom / 末尾省略、
      背景色优先级 `color > bkcolor > 调色板(bg_avatar_1..6, hash(text) % 6) > bg_avatar`。
- [x] T3 注册 5 处：`DUI_CTR_AVATAR`（`duilib_defs.h`，紧随 `DUI_CTR_BADGE`）、
      `WindowBuilder.cpp` 工厂项、`duilib.vcxproj`、`duilib.vcxproj.filters`（Filter = `Control`）、
      `duilib.h` 的 `#include "Control/Avatar.h"`。无新增事件，不改 `EventArgs.cpp`。
- [x] T4 主题与资源：`global.xml` 新增 `PATH_AVATAR` 与 10 个 Class（`avatar` / `avatar_small` /
      `avatar_large` / `avatar_xlarge` / `avatar_square` / `avatar_square_large` / `avatar_user` /
      `avatar_user_large` / `avatar_photo` / `avatar_text`）；`color_light` 与 `color_dark` 各新增
      `bg_avatar`、`bg_avatar_1..6`、`text_avatar`（`type: avatar`）；新增
      `public/avatar/avatar_icon_user.svg` 与 `public/avatar/avatar_picture_demo.svg`。
- [x] T5 示例：`examples/controls/AvatarForm.h/.cpp`、`controls/avatar.xml`、`test.xml` 入口、
      `TestForm` 绑定、`controls.vcxproj` / `.filters`、中英 `zh_CN-examples.txt` / `en_US-examples.txt`。
- [x] T6 文档：`Control.md` / `Control.en.md` 新章节、`Summary` 双语、`Differences` 双语（**序号 55**）、
      `README` 双语、`Development-Plan.md`（D-16 行 + P2 清单）、两份 `api-reference.md`（哈希一致）、
      `.claude` 速查、本 spec / tasks / checklist。
- [x] T7 验证：`duilib` + `controls`（Debug|x64）编译通过、`verify_docs.py` 退出码 0、
      `git diff --check` 无空白错误。

> 编译结果与文档校验记录见 `checklist.md` 第 6 节；验收剩下的手工运行项见第 7 节。
