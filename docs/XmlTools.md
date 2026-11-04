简体中文 | [English](XmlTools.en.md)

## XML 界面编辑辅助工具

nim_duilib 的界面主要靠手写 XML（`bin/resources/themes/default/<皮肤目录>/*.xml`）。为了减少“改一行 XML、跑一次程序”的循环，仓库提供了三个辅助手段：

| 工具 | 解决的问题 | 使用方式 |
| :--- | :--- | :--- |
| 解析错误定位 | XML 在哪个文件、哪一行、哪一列出错，原因是什么 | 框架内置（`WindowBuilder`/`XmlBox`），错误信息直接显示在 XmlPreview 窗口 |
| XmlPreview 示例程序 | 改完 XML 立刻看到界面效果 | 运行 `examples/XmlPreview`，浏览或直接拖入 XML 文件 |
| [lint_xml.py](Tools/lint_xml.py) | 节点名拼错、`class` 位置不对、`name` 重复、颜色/字体/`text_id`/`Include`/图片等资源引用错误 | `python docs/Tools/lint_xml.py` |
| [gen_xml_schema.py](Tools/gen_xml_schema.py) | 编辑器中的节点名/属性名补全与属性悬浮说明 | 生成 `docs/schema/nim_duilib.xsd` 后关联到编辑器 |

## 一、解析错误的定位信息

XML 解析失败时，框架会给出“文件路径 + 行号 + 列号 + 失败原因”，格式如下：

```
XML parse failed: <文件路径>, line <行号>, column <列号>: <失败原因>
```

行号、列号都从 1 开始计数，列号按字符计数（UTF-8 编码时一个中文字符算一列）。获取方式：

| 接口 | 说明 |
| :--- | :--- |
| `WindowBuilder::GetLastError()` | 解析窗口/布局 XML 失败时的最后一次错误信息 |
| `GlobalManager::Instance().GetLastXmlPreviewError()` | 从内存数据创建预览（`CreateBoxForXmlPreview`）失败时的错误信息 |
| `XmlBox::GetLastError()` | 预览容器加载 XML 失败时的错误信息（内部已包含上述两种来源） |

除接口返回值外，错误信息也会写入日志（`ui::LogUtil::OutputLine`），便于在 Release 版排查。

示例：在自己的程序中使用 XmlBox 做局部预览，并显示失败原因：

```cpp
ui::XmlBox* pXmlBox = dynamic_cast<ui::XmlBox*>(FindControl(_T("xml_box_test")));
if (pXmlBox != nullptr) {
    pXmlBox->AddLoadXmlCallback([](const ui::FilePath& xmlPath, bool bSuccess) {
        if (!bSuccess) {
            //错误信息包含文件路径、行号、列号和原因
            DString errMsg = pXmlBox->GetLastError();
        }
    });
    pXmlBox->SetXmlFilePath(ui::FilePath(_T("controls/controls.xml")));
}
```

## 二、XmlPreview 示例程序

`examples/XmlPreview` 是“XML 界面效果预览”的示例程序（见 [Examples.md](Examples.md) 第 21 节），编译后的程序在 `bin/` 目录中。

主要功能：

* **浏览文件**：点击“打开XML文件”按钮选择任意 XML 文件；
* **拖入文件**：直接把 `.xml` 文件拖到预览区即可加载。预览区内的控件来自被预览的 XML（它们本身没有拖放目标，会导致鼠标显示“禁止拖入”），因此示例在**窗口级**统一处理拖放事件（重写 `OnDropEnterMsg/OnDropOverMsg/OnDropMsg/OnDropLeaveMsg`）：落点在 `XmlBox` 矩形内且拖入的是 `.xml` 文件时，设置 `m_dwEffect = DROPEFFECT_COPY`、`m_bHandled = true` 截获事件，并把预览区边框切换为强调色作为拖入提示；同时仍为 `XmlBox` 设置了控件级拖放（`SetEnableDragDrop`、`SetEnableDropFile`、`SetDropFileTypes(_T(".xml"))`）作为兜底；
* **自动刷新**：勾选“自动刷新”后，按设定秒数（1~60）定时检测文件内容，内容变化时重新加载，适合“编辑器保存 → 预览自动更新”的工作流；
* **清空预览**：点击“清空预览”恢复空白状态，同时取消“自动刷新”；
* **重新加载**：点击“重新加载”按当前路径重新解析，无需重新选择文件；
* **结果显示**：顶部信息卡片显示文件完整路径与状态徽标（加载成功/加载失败/等待加载）；加载失败时在下方展开显示错误详情（解析错误包含 `line ..., column ...`），文本过长时自动省略，鼠标悬浮可看到完整信息（Label 的 `path_ellipsis="true"`、`auto_tooltip="true"`）。

注意事项：

* 预览 XML 中引用的图片、字体、颜色等都按**当前主题**解析，因此被预览的 XML 建议放在 `bin/resources/themes/default/<皮肤目录>/*.xml` 下（这样 `res_path` 能自动推导出资源目录）；
* 预览容器 `XmlBox` 支持 `res_path` 属性，用于指定相对路径 XML 的图片资源根目录（示例中为 `res_path="controls"`）；
* `XmlBox` 会为预览内容加载独立的 `Class`、`ThemeColor`、`Font`，切换文件时会自动清理上一次加载的临时定义，不会污染全局样式；
* 框架侧同步做了改进：窗口级拖放未被截获时，如果鼠标下最上层的子控件不支持拖放，会继续向上查找父控件（`WindowDropTarget::GetControlDropTarget`），因此给容器控件设置拖放后，其内部子控件所在的区域也能接收拖入的文件。

## 三、lint_xml.py：布局 XML 静态检查

手写 XML 时，很多错误只有在运行期才会暴露（Debug 版触发断言，Release 版静默失效）。`docs/Tools/lint_xml.py` 在编辑阶段就能把这些问题找出来，并给出 `文件:行:列` 位置：

```
bin/resources/themes/default/controls/controls.xml:123:5: [warning] xxx_image 引用的图片不存在：public/xxx.svg（若为运行期动态注册的资源可忽略）
```

### 3.1 检查规则

| 序号 | 规则 | 级别 |
| :--- | :--- | :--- |
| 1 | XML 语法（解析失败时给出 `文件:行:列` 与原因） | error |
| 2 | 未知节点名（对照 `DUI_CTR_*` 宏与特殊节点；自定义控件可用 `--allow-node` 加入白名单） | warning |
| 3 | `class` 属性必须写在所有属性的最前面（否则 class 内的同名属性会覆盖显式属性） | error |
| 4 | 同一个 XML 文件内控件节点的 `name` 不能重复（`FindControl` 依赖 name 唯一） | error |
| 5 | `text_id` 是否存在（对照 `bin/resources/lang/*.txt` 中的全部字符串 ID） | error |
| 6 | `font` 是否存在（对照各主题 `global.xml` 中的 `<Font id="...">`） | error |
| 7 | 颜色值是否合法（语义色名、`Alias` 别名、标准 CSS 色名，或 `#RRGGBB`/`#AARRGGBB`） | error |
| 8 | `<Include>` 的 `src`/`source` 引用的文件是否存在、`count` 是否为正整数 | error |
| 9 | 图片文件是否存在（`bkimage`/`image` 及以 `_image` 结尾的属性，支持 `file='...'` 语法与 `${VAR}` 变量） | warning |

说明：

* 规则 2 与规则 9 报 warning，是因为自定义控件节点、运行期动态注册的图片属于合法的常见用法；需要严格把关时可加 `--strict`，让 warning 也返回失败；
* 规则 9 只检查带图片后缀（`png/jpg/jpeg/gif/bmp/ico/svg/webp/json/pag/lottie`）的路径，纯色、渐变等非文件资源不会误报。

### 3.2 用法

```bash
# 检查整个主题资源目录（默认行为）
python docs/Tools/lint_xml.py

# 检查指定文件或目录
python docs/Tools/lint_xml.py bin/resources/themes/default/layout/layout.xml
python docs/Tools/lint_xml.py bin/resources/themes/default/layout

# 把警告也当作失败（适合接入 CI），或跳过某一类检查
python docs/Tools/lint_xml.py --strict
python docs/Tools/lint_xml.py --no-image
python docs/Tools/lint_xml.py --no-text-id
python docs/Tools/lint_xml.py --no-color

# 指定仓库根目录（默认自动识别脚本所在的仓库）
python docs/Tools/lint_xml.py --repo .

# 允许自定义控件节点名（可重复指定，避免“未知的节点名”警告）
python docs/Tools/lint_xml.py --allow-node RenderTest1 --allow-node ColorPreviewLabel
```

参数一览：

| 参数 | 说明 |
| :--- | :--- |
| `paths` | 要检查的 XML 文件或目录，可写多个；默认检查 `bin/resources/themes` |
| `--repo` | 仓库根目录，默认按脚本位置自动识别 |
| `--strict` | 警告也视为失败（退出码 1） |
| `--no-image` | 跳过图片存在性检查 |
| `--no-text-id` | 跳过多语言 `text_id` 检查 |
| `--no-color` | 跳过颜色值检查 |
| `--allow-node NAME` | 追加自定义控件节点名到白名单（可重复） |

退出码：`0` 表示没有错误（仅有警告也返回 0，除非 `--strict`）；`1` 表示存在错误；`2` 表示路径或仓库根目录无效。

### 3.3 推荐用法

* 提交前跑一次：`python docs/Tools/lint_xml.py --strict --allow-node <自定义节点>`；
* 在 CI 中执行同样的命令，防止新增的 XML 引入资源引用或命名问题；
* 与 XmlPreview 配合：先在 local 检查出明显错误，再用 XmlPreview 看界面效果。

## 四、gen_xml_schema.py：生成 XSD 供编辑器补全与校验

`docs/Tools/gen_xml_schema.py` 根据文档中的属性表和源码中的节点宏，生成 `docs/schema/nim_duilib.xsd`。编辑器加载该 XSD 后，就可以在 XML 中补全节点名、属性名，并在悬浮时看到属性用途。

### 4.1 数据来源

| 数据 | 来源 |
| :--- | :--- |
| 节点名 | `docs/XmlNode.md` 的节点名称表、`duilib/duilib_defs.h` 中的 `DUI_CTR_*` 宏、`WindowBuilder` 支持的特殊节点 |
| 属性名与说明 | `docs/Control.md`、`Box.md`、`Window.md`、`Global.md`、`Theme.md`、`ThemeColor.md` 中的属性表 |
| 属性继承 | 文档中的“继承了 `X` 属性”“与`X`是基于相同模板的类”，以及 `docs/XmlNode.md` 说明列中的“继承X” |
| 富文本标记 | `WindowBuilder::ParseRichTextXmlNode` 支持的 `<a>`、`<b>`、`<i>`、`<u>`、`<s>`、`<del>`、`<strike>`、`<ins>`、`<bgcolor>`、`<font>`、`<br>` |

### 4.2 生成

```bash
# 重新生成（默认输出到 docs/schema/nim_duilib.xsd）
python docs/Tools/gen_xml_schema.py

# 指定输出文件或仓库根目录
python docs/Tools/gen_xml_schema.py --output docs/schema/nim_duilib.xsd
python docs/Tools/gen_xml_schema.py --repo .

# 额外检查：属性名是否在 C++ 源码中出现过（用于发现文档笔误）
python docs/Tools/gen_xml_schema.py --verify
```

生成结果示例（当前版本）：

```
节点：172 个，属性组：118 个，属性名：823 个，无属性的节点：27 个
```

`--verify` 发现的“源码中不存在的属性名”通常是文档里已定义、但尚未实现的规范字段（例如主题文件中的 `version`、`accent_support`），不一定是笔误。

### 4.3 XSD 的结构

* 没有 targetNamespace，因此可以直接校验不带命名空间的布局 XML；
* 每个节点都声明为全局元素，因此任何节点都可以充当 XML 根节点（窗口、`Global`、`ChildWindowTemplate`、`Loading` 等）；
* 容器节点的子元素统一引用 `AnyNode` 分组（即“任意控件节点都可以作为子节点”），富文本节点引用 `RichTextContent`（额外允许富文本标记）；
* 属性按“来源类 + 属性子集”去重为 `attributeGroup`，子类通过引用父类属性组实现继承，避免文件体积膨胀；
* 每个节点类型都带 `<xs:anyAttribute processContents="skip"/>`，即**未知属性不会导致校验失败**（自定义属性、框架新增属性都可正常使用）；属性名、颜色、字体、`text_id`、图片的严格检查请使用 `lint_xml.py`；
* 属性说明写在 `<xs:annotation><xs:documentation>` 中，编辑器悬浮提示即可看到。

### 4.4 关联到编辑器

以 VS Code（Red Hat XML 扩展）为例，在 `.vscode/settings.json` 中添加文件关联：

```json
{
    "xml.fileAssociations": [
        { "pattern": "**/resources/themes/**/*.xml", "systemId": "docs/schema/nim_duilib.xsd" }
    ]
}
```

Visual Studio 中可在 XML 文件的“属性 → 架构（Schemas）”中指定该 XSD，或把 XSD 复制到 Visual Studio 的 `Xml/Schemas` 目录（全局生效）。

其他支持 XSD 的编辑器（Qt Creator、EmEditor 等）同理，只要把该 XSD 指定为无命名空间 XML 的架构文件即可。

注意：

* 自定义控件（通过控件工厂注册的节点）不在 XSD 中声明，编辑器会提示“未知节点”；如需消除提示，可以在自己的扩展 XSD 中补充这些节点，并把它加入关联列表；
* 修改文档中的属性表后需要重新运行生成脚本，否则补全信息会与文档不一致。

## 五、常见问题

**Q：编辑器提示“未知的节点名”，但程序运行正常。**
A：该节点多半是自定义控件。`lint_xml.py` 用 `--allow-node NAME` 加入白名单；XSD 需要在扩展 schema 中声明。

**Q：lint 报“图片不存在”，但运行期能看到图片。**
A：图片可能是在运行期动态注册的（例如内存图片、压缩包内资源），可忽略该 warning；也可以加 `--no-image` 跳过图片检查。

**Q：`text_id` 报不存在。**
A：检查 `bin/resources/lang/*.txt` 中是否缺少对应的字符串 ID；多语言文件中存在、但当前语言文件没有的 ID 也会被报出。

**Q：XSD 校验不到属性名拼写错误。**
A：XSD 有意采用宽松策略（`xs:anyAttribute`），属性名拼写检查请使用 `lint_xml.py`（规则 1~9），或后续按需扩展 XSD 的严格模式。
