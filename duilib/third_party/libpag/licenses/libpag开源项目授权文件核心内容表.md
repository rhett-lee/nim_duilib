# libpag 开源项目授权文件核心内容表

| 协议类型 | 适用组件 | 版权归属 | 核心权限 | 关键限制/要求 | 第三方授权参考 |
|----------|----------|----------|----------|----------------|----------------|
| Apache License Version 2.0 | 1. libpag 本体（除第三方组件外）<br>2. libavc | 1. libpag：Tencent（2025）<br>2. libavc：原版权 The Android Open Source Project（2015），腾讯修改部分归 Tencent | 1. 永久、全球、非独占、免费的版权许可（复制、修改、公开展示/执行、再授权、分发）<br>2. 专利许可（仅覆盖贡献者必然涉及的专利） | 1. 分发需随附协议副本<br>2. 修改文件需标注修改信息<br>3. 保留原版权/专利/商标声明<br>4. 含“NOTICE”文件需同步归因信息<br>5. 若发起专利诉讼指控侵权，专利许可终止 | <div style="width: 200px">libavc 源码：https://android.googlesource.com/platform/external/libavc</div> |
| BSD 2-Clause License + GNU GPL v2 | lz4（分目录授权）<br>- `lib` 目录：BSD 2-Clause<br>- `programs`/`tests`/`examples` 目录：GPL v2 | Yann Collet（2011-2020） | 1. BSD 2-Clause：允许源码/二进制分发（可修改）<br>2. GPL v2：可修改、分发，但衍生作品需以相同协议授权 | 1. BSD 2-Clause：需保留版权声明、条款列表及免责声明<br>2. GPL v2：分发二进制文件需提供完整源码或3年获取源码承诺，禁止附加限制条款 | - |
| BSD 3-Clause License | tgfx | Tencent（2023） | 允许源码/二进制分发（可修改），权限与 BSD 2-Clause 基本一致 | 1. 保留版权声明、条款列表及免责声明<br>2. 不得使用版权方/贡献者名称为衍生产品背书（需书面许可） | <div style="width: 200px">https://github.com/Tencent/tgfx/blob/main/LICENSE.txt </div>|
| GNU Lesser General Public License (LGPL) v2.1 | ffavc | Tencent（2021） | 1. 允许非开源程序链接该库<br>2. 可修改、分发库文件（需以相同协议授权） | 1. 分发修改后的库需提供完整源码<br>2. 组合使用时需标注库的使用及协议归属，随附协议副本 | <div style="width: 200px">1. ffavc 源码：https://github.com/libpag/ffavc<br>2. 第三方授权：https://github.com/libpag/ffavc/blob/1.0.1/third_party/ffmpeg/LICENSE.md  </div>|
| MIT License | 1. vendor_tools<br>2. rttr | 1. vendor_tools：Dom Chen（2021-present）<br>2. rttr：Axel Menzel（2014-2018） | 无限制使用、复制、修改、合并、发布、分发、再授权 | 需保留版权声明和许可声明 | <div style="width: 200px"> rttr 第三方授权：https://github.com/rttrorg/rttr/tree/v0.9.6/3rd_party  </div>|
| MIT Style License | harfbuzz | Google、Facebook、Mozilla 等多版权方 | 允许任意用途使用（无限制复制、修改、分发） | 1. 保留版权声明<br>2. 包含两段免责条款（无直接/间接损害责任、无担保） | - |
| GNU Lesser General Public License (LGPL) v3 | QT 系列组件：<br>1. QT Quick<br>2. QT Widgets<br>3. QT OpenGL<br>4. QT Core<br>5. QT Multimedia<br>6. QT GUI<br>7. QT Qml | 各组件原作者 | 1. 允许非开源程序链接库文件<br>2. 可修改、分发（衍生作品需以相同协议授权） | 1. 分发组合作品需提供“最小对应源码”（仅库相关部分）<br>2. 标注协议归属，随附协议副本<br>3. 部分组件含 GPL v3 授权 | <div style="width: 200px"> 1. QT Core：https://doc.qt.io/qt-6/qtcore-index.html#licenses-and-attributions<br>2. QT Multimedia：https://doc.qt.io/qt-6/qtmultimedia-index.html#licenses-and-attributions<br>3. QT GUI/QT Qml：https://doc.qt.io/qt-6/qtqml-index.html#licenses-and-attributions </div> |

### 通用条款（所有协议共通）
1. **商标限制**：不授予使用版权方商标、服务标记、产品名称的权利（仅允许合理描述作品来源）；
2. **无担保义务**：组件按“现状”提供，无明示/暗示担保（如适销性、特定用途适用性），用户自行承担使用风险；
3. **责任限制**：除非法律强制或书面约定，贡献者不承担直接/间接/特殊/衍生损害责任（如商誉损失、数据丢失）；
4. **贡献规则**：用户提交代码默认按对应协议授权（除非另有单独协议）。