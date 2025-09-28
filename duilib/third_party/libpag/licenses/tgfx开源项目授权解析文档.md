# tgfx 开源项目授权解析文档
## 一、tgfx 主项目授权概况
tgfx 主项目采用 **BSD 3-Clause License** 授权，仅第三方组件遵循其他开源协议。腾讯对该项目中的部分软件进行了修改（称为“Tencent Modifications”），所有腾讯修改内容的版权归腾讯所有（Copyright (C) 2025 Tencent）。

### BSD 3-Clause License 核心条款
| 条款类别 | 具体要求 |
|----------|----------|
| 再分发条件 | 1. 源代码形式再分发：必须保留原版权声明、本协议条款列表及以下免责声明；<br>2. 二进制形式再分发：必须在随附的文档和/或其他材料中复制原版权声明、本协议条款列表及免责声明；<br>3. 商标使用限制：未经版权持有者或贡献者的特定书面许可，不得使用其名称为基于本软件的产品背书或推广。 |
| 免责声明 | 本软件由版权持有者和贡献者按“现状（AS IS）”提供，不承担任何明示或暗示的担保，包括但不限于适销性、特定用途适用性的担保。 |
| 责任限制 | 在任何情况下，版权持有者或贡献者不对因使用本软件产生的直接、间接、偶然、特殊、示例性或 consequential 损害（包括但不限于替代商品或服务的采购成本、数据或利润损失、业务中断等）承担责任，无论责任基于合同、严格责任还是侵权（包括过失），即使已告知可能发生此类损害。 |


## 二、第三方组件授权明细
### 1. freetype
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright (C) 2006-2022 by David Turner, Robert Wilhelm, and Werner Lemberg |
| 遵循协议 | The FreeType Project LICENSE |
| 核心授权条款 | - **无担保**：Freetype 项目按“现状”提供，不承担任何明示或暗示担保，包括适销性和特定用途适用性，作者或版权持有者不对使用或无法使用该项目导致的损害负责。<br>- **再分发要求**：<br>  1. 源代码再分发需保留协议文件（`FTL.TXT`）且不修改，原文件的版权声明需完整保留，修改文件需在随附文档中明确标注；<br>  2. 二进制再分发需在分发文档中声明软件部分基于 Freetype 团队的工作，建议在文档中添加 Freetype 官网链接（非强制）；<br>- **商标与推广限制**：未经书面许可，Freetype 作者、贡献者与使用者均不得使用对方名称进行商业、广告或推广活动；建议在文档或广告材料中使用“FreeType Project”“FreeType Engine”等指定表述指代该软件。<br>- **版权归属**：Freetype 项目版权归 David Turner、Robert Wilhelm 和 Werner Lemberg 所有（1996-2000 年），除非文件另有明确说明，否则本协议适用于所有原分布文件。 |
| 额外说明 | 建议在使用时添加信用声明：“Portions of this software are copyright © <year> The FreeType Project (www.freetype.org). All rights reserved.”，其中 `<year>` 需替换为实际使用的 Freetype 版本对应的年份。 |


### 2. libpng
| 项目 | 内容 |
|------|------|
| 版权信息 | - 版本 1.0.7（2000 年 7 月 1 日）至 1.6.33（2017 年 9 月 28 日）：Copyright (c) 2000-2002, 2004, 2006-2017 Glenn Randers-Pehrson；<br>- 版本 0.97（1998 年 1 月）至 1.0.6（2000 年 3 月 20 日）：Copyright (c) 1998-2000 Glenn Randers-Pehrson；<br>- 版本 0.89（1996 年 6 月）至 0.96（1997 年 5 月）：Copyright (c) 1996-1997 Andreas Dilger；<br>- 版本 0.5（1995 年 5 月）至 0.88（1996 年 1 月）：Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc. |
| 遵循协议 | libpng license |
| 核心授权条款 | - **免责声明**：PNG 参考库按“现状”提供，贡献者和 Group 42, Inc. 不承担任何明示或暗示担保，包括适销性和特定用途适用性，也不承担因使用该库导致的直接、间接、偶然、特殊、示例性或 consequential 损害责任，即使已告知可能发生此类损害。<br>- **使用与再分发权限**：允许免费使用、复制、修改和分发源代码或其部分内容，用于任何目的，但需遵守以下限制：<br>  1. 不得歪曲源代码的来源；<br>  2. 修改版本需明确标注为修改版，不得冒充原版源代码；<br>  3. 版权声明不得从任何源代码或修改版源代码分发中移除或修改。<br>- **商业使用鼓励**：明确允许并鼓励将该源代码作为支持 PNG 文件格式的组件用于商业产品，使用时无需付费，虽不强制要求致谢，但建议在产品中体现。 |
| 额外说明 | - 商标：“libpng”未在任何司法管辖区注册为商标，但版权持有者主张在承认普通法商标的司法管辖区享有“普通法商标保护”；<br>- OSI 认证：libpng 是 OSI 认证的开源软件，但 OSI 未涉及 1.0.7 版本新增的免责声明；<br>- 出口管制：版权持有者认为 libpng 的出口管制分类编号（ECCN）为 EAR99，属于开源公开软件，不含加密内容，不受出口管制或国际武器贸易条例（ITAR）约束。 |


### 3. 多组件（libwebp、pathKit、skcms、highway）
| 项目 | 内容 |
|------|------|
| 涉及组件及版权 | - libwebp：Copyright (c) 2010, Google Inc. All rights reserved.；<br>- pathKit：Copyright (c) 2011 Google Inc. All rights reserved.；<br>- skcms：Copyright 2018 Google Inc.；<br>- highway：Copyright (c) The Highway Project Authors. All rights reserved. |
| 遵循协议 | BSD 3-Clause License（腾讯对部分软件有修改，修改内容版权归腾讯所有） |
| 核心授权条款 | 与 tgfx 主项目的 BSD 3-Clause License 条款完全一致，具体如下：<br>- 源代码再分发需保留原版权声明、协议条款及免责声明；<br>- 二进制再分发需在随附文档/材料中复制上述声明；<br>- 未经书面许可，不得使用版权持有者或贡献者名称为产品背书或推广；<br>- 软件按“现状”提供，无担保，不承担间接损害责任。 |


### 4. libjpeg-turbo
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright (C) 1991-1996, Thomas G. Lane；Copyright (C) 2017, D. R. Commander |
| 遵循协议 | IJG License（Independent JPEG Group License）及其他第三方组件协议 |
| 核心授权条款 | - **免责声明**：作者不提供任何明示或暗示担保，包括软件的质量、准确性、适销性和特定用途适用性，软件按“现状”提供，使用者自行承担使用风险；<br>- **使用与再分发权限**：允许免费使用、复制、修改和分发软件（或其部分内容）用于任何目的，但需遵守以下条件：<br>  1. 若分发源代码，需包含本 README 文件（含版权和无担保声明，且不得修改），修改文件需在随附文档中明确标注；<br>  2. 若仅分发可执行代码，随附文档需声明“本软件部分基于 Independent JPEG Group 的工作”；<br>  3. 使用者需自行承担使用该软件可能产生的不良后果，作者不承担任何损害责任。<br>- **商标与推广限制**：不得使用 IJG 作者或公司名称进行与该软件或衍生产品相关的广告或宣传，软件仅可称为“Independent JPEG Group's software”；<br>- **商业使用支持**：明确允许并鼓励将该软件作为商业产品的基础，产品供应商需自行承担所有担保或责任主张。 |
| 额外说明 | - 该软件原包含 GIF 读写代码，为避免 Unisys LZW 专利纠纷（现已过期），已移除 GIF 读取支持，GIF 写入功能简化为生成“未压缩 GIF”，可被所有标准 GIF 解码器读取；<br>- 其他第三方组件协议可参考链接：https://github.com/libjpeg-turbo/libjpeg-turbo/blob/2.0.0/LICENSE.md |


### 5. zlib
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright (C) 1995-2017 Jean-loup Gailly and Mark Adler |
| 遵循协议 | Zlib License |
| 核心授权条款 | - **免责声明**：软件按“现状”提供，无任何明示或暗示担保，作者不对使用软件导致的损害承担责任；<br>- **使用与再分发权限**：允许任何人将软件用于任何目的（包括商业应用），可自由修改和再分发，但需遵守以下限制：<br>  1. 不得歪曲软件来源，不得声称自己编写了原版软件，若用于产品，建议在产品文档中致谢（非强制）；<br>  2. 修改版源代码需明确标注为修改版，不得冒充原版软件；<br>  3. 本声明不得从任何源代码分发中移除或修改。 |
| 额外说明 | zlib 库使用的数据格式由 RFC 文档（RFC 1950、RFC 1951、RFC 1952）描述，可分别通过链接查看：http://tools.ietf.org/html/rfc1950（zlib 格式）、rfc1951（deflate 格式）、rfc1952（gzip 格式）。 |


### 6. harfbuzz
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright © 2010-2020 Google, Inc.；Copyright © 2018-2020 Ebrahim Byagowi；Copyright © 2019-2020 Facebook, Inc.；Copyright © 2012 Mozilla Foundation；Copyright © 2011 Codethink Limited；Copyright © 2008,2010 Nokia Corporation and/or its subsidiary(-ies)；Copyright © 2009 Keith Stribley；Copyright © 2009 Martin Hosken and SIL International；Copyright © 2007 Chris Wilson；Copyright © 2005-2006,2020-2021 Behdad Esfahbod；Copyright © 2005 David Turner；Copyright © 2004,2007-2010 Red Hat, Inc.；Copyright © 1998-2004 David Turner and Werner Lemberg |
| 遵循协议 | “Old MIT” license |
| 核心授权条款 | - **使用与再分发权限**：无需书面协议和许可费，可使用、复制、修改和分发软件及其文档用于任何目的，但需在所有软件副本中包含原版权声明及以下两段内容：<br>  1. “IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”<br>  2. “THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.”<br>- **免责与责任**：版权持有者不提供任何担保，软件按“现状”提供，且无维护、支持、更新等义务，也不对使用软件导致的损害承担责任。 |


### 7. libexpat
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright (c) 1998-2000 Thai Open Source Software Center Ltd and Clark Cooper；Copyright (c) 2001-2022 Expat maintainers |
| 遵循协议 | MIT License |
| 核心授权条款 | - **使用权限**：允许免费获取软件及相关文档文件（“软件”）的副本，可无限制地使用、复制、修改、合并、发布、分发、再许可和/或出售软件副本；<br>- **核心条件**：在所有软件副本或重要部分中包含原版权声明和本许可声明；<br>- **免责与责任**：软件按“现状”提供，无任何明示或暗示的担保，包括但不限于适销性、特定用途适用性和不侵权的担保，作者或版权持有者不对因使用或无法使用软件产生的任何索赔、损害或其他责任负责，无论基于合同、侵权或其他法律理论。 |


### 8. swiftshader
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright (c) swiftshader original author and authors |
| 遵循协议 | Apache License Version 2.0 及其他第三方组件协议 |
| 核心授权条款（Apache 2.0） | - **定义**：明确“License”“Licensor”“Legal Entity”“You”“Source”“Object”“Work”“Derivative Works”“Contribution”“Contributor”等术语的含义（与标准 Apache 2.0 协议一致）；<br>- **版权许可**：每个贡献者授予使用者永久、全球、非独占、免费、不可撤销的版权许可，包括复制、准备衍生作品、公开展示、公开执行、再许可和分发作品及衍生作品（源代码或目标代码形式）；<br>- **专利许可**：贡献者授予使用者永久、全球、非独占、免费、不可撤销（本节另有规定除外）的专利许可，仅适用于因贡献者的贡献单独或与作品结合而必然侵权的专利主张；若使用者提起专利诉讼主张作品或贡献侵权，相关专利许可自诉讼提起之日起终止；<br>- **再分发条件**：<br>  1. 向其他接收者分发时需提供本协议副本；<br>  2. 修改文件需添加显著通知说明修改情况；<br>  3. 源代码形式分发衍生作品时需保留原作品的版权、专利、商标和归属声明（无关声明除外）；<br>  4. 若原作品含“NOTICE”文件，衍生作品需在指定位置包含该文件中的归属声明（无关声明除外）；<br>- **贡献提交**：提交贡献默认受本协议约束，除非与授权方签订单独许可协议；<br>- **商标限制**：不授予使用授权方商标的权限，除非为描述作品来源和复制 NOTICE 文件内容所必需；<br>- **免责与责任**：作品按“现状”提供，无担保，贡献者不承担间接损害责任（法律强制或书面约定除外）。 |
| 额外说明 | - 源代码获取链接：https://swiftshader.googlesource.com/SwiftShader；<br>- 其他第三方组件协议可参考链接：https://swiftshader.googlesource.com/SwiftShader/+/refs/heads/master/LICENSE.txt |


### 9. flatbuffers
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright (c) flatbuffers original author and authors |
| 遵循协议 | Apache License Version 2.0 |
| 核心授权条款 | 与 swiftshader 遵循的 Apache 2.0 协议条款完全一致（包括版权许可、专利许可、再分发条件、贡献提交、商标限制、免责与责任等），文档中包含该协议的完整副本。 |


### 10. angle
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright 2018 The ANGLE Project Authors. All rights reserved. |
| 遵循协议 | BSD 3-Clause License 及其他第三方组件协议 |
| 核心授权条款（BSD 3-Clause） | 与 tgfx 主项目的 BSD 3-Clause License 条款一致，包括：<br>- 源代码再分发保留声明；<br>- 二进制再分发复制声明；<br>- 名称使用需书面许可；<br>- 无担保且不承担间接损害责任。 |
| 额外说明 | 其他第三方组件协议可参考链接：https://github.com/google/angle/tree/main/third_party |


### 11. digestpp
| 项目 | 内容 |
|------|------|
| 版权信息 | 无特定版权持有者，释放至公有领域（public domain） |
| 遵循协议 | 公有领域协议（参考 https://unlicense.org） |
| 核心授权条款 | - **使用权限**：任何人可自由复制、修改、发布、使用、编译、出售或分发该软件（源代码或编译后的二进制形式），用于任何目的（商业或非商业），通过任何方式；<br>- **版权放弃**：在承认版权法的司法管辖区，作者将该软件的所有版权权益奉献给公有领域，放弃对软件的所有现有和未来版权权利；<br>- **免责声明**：软件按“现状”提供，无任何明示或暗示担保，包括适销性、特定用途适用性和不侵权，作者不对因使用软件产生的任何索赔、损害或其他责任负责，无论基于合同、侵权或其他法律理论。 |


### 12. concurrentqueue
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright (c) 2013-2016, Cameron Desrochers. All rights reserved. |
| 遵循协议 | BSD 2-Clause License 及其他第三方组件协议 |
| 核心授权条款（BSD 2-Clause） | - **再分发条件**：<br>  1. 源代码再分发需保留原版权声明、本协议条款列表及免责声明；<br>  2. 二进制再分发需在随附文档和/或其他材料中复制上述声明；<br>- **免责与责任**：软件按“现状”提供，无任何明示或暗示担保，版权持有者或贡献者不对因使用软件产生的直接、间接、偶然、特殊、示例性或 consequential 损害承担责任，无论基于合同、严格责任还是侵权（包括过失）。 |
| 额外说明 | 其他第三方组件协议可参考链接：https://github.com/cameron314/concurrentqueue/tree/master/benchmarks |


### 13. lz4
| 项目 | 内容 |
|------|------|
| 版权信息 | Copyright (c) 2011-2020, Yann Collet. All rights reserved. |
| 遵循协议 | BSD 2-Clause License 及其他第三方组件协议 |
| 核心授权条款（BSD 2-Clause） | 与 concurrentqueue 遵循的 BSD 2-Clause License 条款完全一致，文档中包含该协议的完整副本。 |
| 额外说明 | 其他第三方组件协议可参考链接：https://github.com/lz4/lz4/tree/dev/lib |


## 三、通用合规建议
1. **协议梳理与匹配**：开发前需明确项目中使用的 tgfx 主项目及各第三方组件，建立“组件-协议”映射表，避免遗漏特殊条款（如 GPL 类协议的“传染性”，虽本项目未涉及，但需警惕后续引入组件的协议冲突）。
2. **声明与文档管理**：
   - 保留所有组件的原版权声明、协议文本，按组件分类存放（如在项目的 `LICENSE` 目录下创建 `BSD-3-Clause-tgfx.txt`、`FreeType-LICENSE.txt` 等文件）；
   - 分发软件时，按对应协议要求提供协议副本和必要声明（如 Freetype 的信用声明、libpng 的修改标注）。
3. **商标与名称使用**：严格遵守 BSD 3-Clause、FreeType 等协议中的名称使用限制，未经书面许可，不得使用腾讯、Google、Freetype 团队等版权持有者的名称为产品背书或推广。
4. **修改与贡献管理**：
   - 对组件进行修改时，需按协议要求标注修改信息（如 Apache 2.0 要求的修改通知、libpng 的修改版标注）；
   - 向第三方组件（如 swiftshader、angle）提交贡献时，需遵循其贡献规范，明确许可方式（默认协议或单独协议）。
5. **免责与风险提示**：在软件文档或“关于”界面中添加明确的免责声明，告知用户软件按“现状”提供，无担保，使用者自行承担使用风险。