English | [简体中文](XmlTools.md)

> Last synced: 2026-11-02

## XML Interface Editing Helper Tools

nim_duilib interfaces are mainly written by hand in XML (`bin/resources/themes/default/<skin folder>/*.xml`). To shorten the "edit one XML line, run the program once" loop, the repository provides three helpers:

| Tool | Problem it solves | How to use |
| :--- | :--- | :--- |
| Parse error location | Which file, line and column failed, and why | Built into the framework (`WindowBuilder`/`XmlBox`); the message is shown directly in the XmlPreview window |
| XmlPreview sample program | See the UI immediately after saving XML | Run `examples/XmlPreview`, browse for or drop in an XML file |
| [lint_xml.py](Tools/lint_xml.py) | Misspelled node names, wrong `class` position, duplicate `name`, invalid color/font/`text_id`/`Include`/image references | `python docs/Tools/lint_xml.py` |
| [gen_xml_schema.py](Tools/gen_xml_schema.py) | Node/attribute completion and hover documentation in the editor | Generate `docs/schema/nim_duilib.xsd` and associate it with the editor |

## 1. Location Information for Parse Errors

When XML parsing fails, the framework reports "file path + line + column + reason":

```
XML parse failed: <file path>, line <line number>, column <column number>: <failure reason>
```

Line and column numbers start from 1; the column is counted in characters (with UTF-8, one Chinese character counts as one column). Ways to get the message:

| API | Description |
| :--- | :--- |
| `WindowBuilder::GetLastError()` | Last error when parsing a window/layout XML |
| `GlobalManager::Instance().GetLastXmlPreviewError()` | Error when creating a preview from in-memory data (`CreateBoxForXmlPreview`) |
| `XmlBox::GetLastError()` | Error when the preview container fails to load XML (already covers both sources above) |

Besides the API return values, the message is also written to the log (`ui::LogUtil::OutputLine`), which helps when debugging Release builds.

Example: use XmlBox for local preview in your own program and show the failure reason:

```cpp
ui::XmlBox* pXmlBox = dynamic_cast<ui::XmlBox*>(FindControl(_T("xml_box_test")));
if (pXmlBox != nullptr) {
    pXmlBox->AddLoadXmlCallback([](const ui::FilePath& xmlPath, bool bSuccess) {
        if (!bSuccess) {
            //The message contains the file path, line, column and reason
            DString errMsg = pXmlBox->GetLastError();
        }
    });
    pXmlBox->SetXmlFilePath(ui::FilePath(_T("controls/controls.xml")));
}
```

## 2. XmlPreview Sample Program

`examples/XmlPreview` is the "preview XML interface" sample program (see section 21 of [Examples.en.md](Examples.en.md)); the compiled program is placed in the `bin/` directory.

Main features:

* **Browse for a file**: click the "Open XML file" button to choose any XML file;
* **Drop a file**: drop a `.xml` file onto the preview area to load it. The controls inside the preview come from the previewed XML and have no drop target of their own, which makes Windows show the "no-drop" cursor. The sample therefore handles drop events at the **window level** (overriding `OnDropEnterMsg/OnDropOverMsg/OnDropMsg/OnDropLeaveMsg`): when the drop point is inside the `XmlBox` rectangle and the dragged file is `.xml`, it sets `m_dwEffect = DROPEFFECT_COPY` and `m_bHandled = true` to take over the event, and highlights the preview border with the accent color; control-level drop support on `XmlBox` (`SetEnableDragDrop`, `SetEnableDropFile`, `SetDropFileTypes(_T(".xml"))`) is kept as a fallback;
* **Auto refresh**: after enabling "auto refresh", the file content is checked periodically at the configured interval (1~60 seconds) and reloaded when it changes — a good fit for the "save in the editor, preview updates automatically" workflow;
* **Clear preview**: click "Clear preview" to return to the empty state; auto refresh is turned off as well;
* **Reload**: click "Reload" to parse the current path again without choosing the file again;
* **Result display**: the info card shows the full file path and a status badge (Loaded / Load failed / Waiting); on failure the error details are expanded below (parse errors include `line ..., column ...`); long text is ellipsized and the full message appears on hover (`path_ellipsis="true"`, `auto_tooltip="true"` on the Label).

Notes:

* Images, fonts and colors referenced by the previewed XML are resolved with the **current theme**, so it is recommended to place the previewed XML under `bin/resources/themes/default/<skin folder>/*.xml` (the `res_path` can then be derived automatically);
* The preview container `XmlBox` supports the `res_path` attribute, which specifies the image resource root for relative-path XML (the sample uses `res_path="controls"`);
* `XmlBox` loads its own `Class`, `ThemeColor` and `Font` for the previewed content; when switching files the previous temporary definitions are cleaned up automatically, so the global styles are not polluted;
* The framework was improved as well: when the window-level drop handler does not take over the event and the topmost control under the cursor has no drop target, the lookup now walks up to its parent controls (`WindowDropTarget::GetControlDropTarget`), so a container with drag-drop enabled also accepts files dropped over its children.

## 3. lint_xml.py: Static Checks for Layout XML

Many errors in hand-written XML only surface at run time (assertions in Debug builds, silent failures in Release builds). `docs/Tools/lint_xml.py` finds these problems while editing and reports `file:line:column` positions:

```
bin/resources/themes/default/controls/controls.xml:123:5: [warning] xxx_image 引用的图片不存在：public/xxx.svg（若为运行期动态注册的资源可忽略）
```

(The tool prints its diagnostic messages in Chinese; the location prefix `file:line:column` is language independent.)

### 3.1 Check Rules

| No. | Rule | Level |
| :--- | :--- | :--- |
| 1 | XML syntax (reports `file:line:column` and the reason on failure) | error |
| 2 | Unknown node name (checked against `DUI_CTR_*` macros and special nodes; custom controls can be whitelisted with `--allow-node`) | warning |
| 3 | The `class` attribute must come before all other attributes (otherwise same-named attributes in the class override the explicit ones) | error |
| 4 | `name` values must be unique within one XML file (for a control node; `FindControl` relies on unique names) | error |
| 5 | `text_id` exists (checked against all string IDs in `bin/resources/lang/*.txt`) | error |
| 6 | `font` exists (checked against `<Font id="...">` in each theme's `global.xml`) | error |
| 7 | Color values are valid (semantic color name, `Alias` alias, standard CSS color name, or `#RRGGBB`/`#AARRGGBB`) | error |
| 8 | The file referenced by `<Include>`'s `src`/`source` exists, and `count` is a positive integer | error |
| 9 | Referenced image files exist (`bkimage`/`image` and attributes ending with `_image`; supports `file='...'` syntax and `${VAR}` variables) | warning |

Notes:

* Rules 2 and 9 report warnings because custom control nodes and images registered dynamically at run time are common, legitimate usages; add `--strict` to treat warnings as failures when you need strict gating;
* Rule 9 only checks paths with an image suffix (`png/jpg/jpeg/gif/bmp/ico/svg/webp/json/pag/lottie`), so solid colors and gradients are not reported as false positives.

### 3.2 Usage

```bash
# Check the whole theme resource directory (default behavior)
python docs/Tools/lint_xml.py

# Check a specific file or directory
python docs/Tools/lint_xml.py bin/resources/themes/default/layout/layout.xml
python docs/Tools/lint_xml.py bin/resources/themes/default/layout

# Treat warnings as failures (suitable for CI), or skip one kind of check
python docs/Tools/lint_xml.py --strict
python docs/Tools/lint_xml.py --no-image
python docs/Tools/lint_xml.py --no-text-id
python docs/Tools/lint_xml.py --no-color

# Specify the repository root (by default it is derived from the script location)
python docs/Tools/lint_xml.py --repo .

# Whitelist custom control node names (repeatable, avoids "unknown node name" warnings)
python docs/Tools/lint_xml.py --allow-node RenderTest1 --allow-node ColorPreviewLabel
```

Arguments:

| Argument | Description |
| :--- | :--- |
| `paths` | XML files or directories to check; multiple values allowed; defaults to `bin/resources/themes` |
| `--repo` | Repository root; derived from the script location by default |
| `--strict` | Treat warnings as failures (exit code 1) |
| `--no-image` | Skip the image existence check |
| `--no-text-id` | Skip the multilingual `text_id` check |
| `--no-color` | Skip the color value check |
| `--allow-node NAME` | Add a custom control node name to the whitelist (repeatable) |

Exit codes: `0` means no errors (warnings alone also return 0 unless `--strict`); `1` means errors were found; `2` means the path or repository root is invalid.

### 3.3 Recommended Practice

* Run before committing: `python docs/Tools/lint_xml.py --strict --allow-node <custom node>`;
* Run the same command in CI to keep newly added XML from introducing resource or naming problems;
* Use it together with XmlPreview: catch obvious problems locally first, then check the visual result in XmlPreview.

## 4. gen_xml_schema.py: Generate an XSD for Completion and Validation

`docs/Tools/gen_xml_schema.py` generates `docs/schema/nim_duilib.xsd` from attribute tables in the documents and node macros in the source code. Once the editor loads this XSD, node names and attribute names are completed in XML, and attribute purposes are shown on hover.

### 4.1 Data Sources

| Data | Source |
| :--- | :--- |
| Node names | The node name table in `docs/XmlNode.md`, the `DUI_CTR_*` macros in `duilib/duilib_defs.h`, and special nodes supported by `WindowBuilder` |
| Attribute names and descriptions | Attribute tables in `docs/Control.md`, `Box.md`, `Window.md`, `Global.md`, `Theme.md`, `ThemeColor.md` |
| Attribute inheritance | Phrases such as "inherits the `X` attributes" and "based on the same template as `X`" in the documents, plus the "inherits X" notes in `docs/XmlNode.md` |
| Rich text tags | `<a>`, `<b>`, `<i>`, `<u>`, `<s>`, `<del>`, `<strike>`, `<ins>`, `<bgcolor>`, `<font>`, `<br>` supported by `WindowBuilder::ParseRichTextXmlNode` |

### 4.2 Generation

```bash
# Regenerate (output defaults to docs/schema/nim_duilib.xsd)
python docs/Tools/gen_xml_schema.py

# Specify the output file or the repository root
python docs/Tools/gen_xml_schema.py --output docs/schema/nim_duilib.xsd
python docs/Tools/gen_xml_schema.py --repo .

# Extra check: whether attribute names appear in the C++ sources (to find documentation typos)
python docs/Tools/gen_xml_schema.py --verify
```

Example of the generated summary (current version):

```
节点：172 个，属性组：118 个，属性名：823 个，无属性的节点：27 个
```

Attribute names reported by `--verify` as "not found in the source code" are usually specification fields that are documented but not implemented yet (for example `version` and `accent_support` in theme files); they are not necessarily typos.

### 4.3 Structure of the XSD

* There is no target namespace, so layout XML without a namespace can be validated directly;
* Every node is declared as a global element, so any node can serve as the XML root (window, `Global`, `ChildWindowTemplate`, `Loading`, etc.);
* Child elements of container nodes all reference the `AnyNode` group (i.e. "any control node can be a child"); rich text nodes reference `RichTextContent` (which additionally allows rich text tags);
* Attributes are deduplicated into `attributeGroup`s by "source class + attribute subset"; derived classes reference the parent attribute groups to express inheritance, which keeps the file size reasonable;
* Every node type carries `<xs:anyAttribute processContents="skip"/>`, so **unknown attributes never cause validation failures** (custom attributes and newly added framework attributes keep working); for strict checks of attribute names, colors, fonts, `text_id` and images, use `lint_xml.py`;
* Attribute descriptions are written into `<xs:annotation><xs:documentation>`, so the editor shows them on hover.

### 4.4 Associating with the Editor

For VS Code (with the Red Hat XML extension), add file associations in `.vscode/settings.json`:

```json
{
    "xml.fileAssociations": [
        { "pattern": "**/resources/themes/**/*.xml", "systemId": "docs/schema/nim_duilib.xsd" }
    ]
}
```

In Visual Studio, specify this XSD in "Properties → Schemas" of an XML file, or copy the XSD into Visual Studio's `Xml/Schemas` directory (applies globally).

Other editors that support XSD (Qt Creator, EmEditor, etc.) work the same way: just associate the XSD as the schema for namespace-less XML.

Notes:

* Custom controls (nodes registered through the control factory) are not declared in the XSD, so the editor reports "unknown node"; to remove the warning, declare them in your own extension XSD and add it to the association list;
* After changing attribute tables in the documents, rerun the generator, otherwise the completion data will be out of sync with the documentation.

## 5. FAQ

**Q: The editor reports an "unknown node name", but the program runs fine.**
A: The node is most likely a custom control. Use `--allow-node NAME` in `lint_xml.py` to whitelist it; for the XSD, declare it in an extension schema.

**Q: lint reports "image not found", but the image is visible at run time.**
A: The image may be registered dynamically at run time (for example from memory or from a zip package); the warning can be ignored, or skip the image check with `--no-image`.

**Q: `text_id` is reported as missing.**
A: Check whether the corresponding string ID is missing from `bin/resources/lang/*.txt`; IDs that exist in the multilingual files but not in the current language file are also reported.

**Q: The XSD does not catch attribute name typos.**
A: The XSD is deliberately lenient (`xs:anyAttribute`); use `lint_xml.py` (rules 1~9) for attribute spelling checks, or extend the XSD with a strict mode later if needed.
