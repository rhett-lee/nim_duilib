# 验收清单：Button / CheckBox / Option 演示页

- 规格：[spec.md](./spec.md)
- 任务：[tasks.md](./tasks.md)

## 功能

- [x] 示例入口按钮可打开独立演示窗口（入口按钮已绑定 `ShowButtonCheckBoxWindow()`）
- [x] 按钮演示覆盖：主要/次要/图片/危险/灰色/描边/链接/禁用
- [x] 按钮演示覆盖：三种尺寸、图标+文字、纯图标、危险图标
- [x] CheckBox 演示覆盖：`checkbox_1`、`checkbox_2`、禁用、自定义图标
- [x] CheckBox 三态演示：全选 / 半选 / 取消三个按钮均生效
- [x] `checkbox_toggle_1` / `checkbox_toggle_2` 开关样式正常
- [x] Option 演示：两种样式、同组互斥、跨组互不影响、禁用项不可点
- [x] 综合表单：开关 + 复选框 + 单选 + 保存/取消
- [x] 事件回显：点击次数、勾选/取消、Option 选中、三态结果均实时更新

## 资源

- [x] 图标按分类放入 `public/button`、`public/checkbox`，未新增资源变量
- [x] 12 个 SVG 全部可加载（启动冒烟无 `Image File Not Found!` 断言）
- [x] SVG 仅用 `fill`，无 `stroke`
- [x] 所有颜色使用语义色名，未写死主题相关色值（图标原色仅作为替换源）

## 构建与回归

- [x] Debug|x64 编译通过（退出码 0）
- [x] `text_id` 与中英文语言文件一一对应（61 个）
- [x] `test.xml` 原有布局与控件未受影响
- [x] 未修改 `duilib/Control` 下任何控件代码

## 待人工确认

- [ ] 7 张卡片的视觉细节（间距、图标与文字不重叠、深浅主题观感）
