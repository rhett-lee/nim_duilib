# 构建/打包脚本控制台输出双语化（简体中文 / 英文）

> 状态：已实施（2026-10-29，P3-3）。原「控制台一律英文」方案改为双语：
> 简体中文系统显示中文，其他区域（含繁体中文）显示英文。

## 目标

`build/` 下的构建、打包、发布与 SDK 测试脚本，其**运行期控制台输出**
（stdout / stderr、告警、错误、进度、`--help` 文本）支持简体中文与英文两种语言：

- 简体中文区域：输出中文；
- 其他区域（英文、日文、以及 `zh_TW` / `zh_HK` / `zh_MO` / `zh_Hant` 等繁体区域）：
  输出英文，保证在非中文控制台（含仅支持 ASCII 的终端）中不出现乱码或方块。

范围限定为运行期输出；**注释与 docstring 保持中文**（利于维护），包内生成的
`MANIFEST.txt`、`RELEASE-NOTES` 等文件内容保持现有中文不变。

## 语言判定

统一由 `build/i18n.py` 提供，顺序与 `cmake/duilib_i18n.cmake` 一致：

1. `--lang`（`auto` / `zh_CN` / `en`），其次环境变量 `DUILIB_LANG`；
2. `LC_ALL` → `LC_MESSAGES` → `LANG`（仅非 Windows）；
3. 系统区域：Windows 读注册表 `Control Panel\International\LocaleName`，
   其他平台读 `LC_ALL`/`LC_MESSAGES`/`LANG`；
4. 兜底 `en`。

区域标签归一规则 `_normalize_language()`：

| 输入示例 | 结果 |
| :--- | :--- |
| `zh`、`zh_CN`、`zh-CN`、`zh_Hans`、`zh_Hans_CN` | `zh_CN`（中文） |
| `zh_TW`、`zh_HK`、`zh_MO`、`zh_Hant`、`zh_Hant_TW` | `en`（英文） |
| 其他任意标签（`en_US`、`ja_JP`、空） | `en` |

与 `cmake/duilib_i18n.cmake` 的 `^zh([-_]CN|[-_]Hans)?$` 判定保持一致。

## 消息目录

单一维护点：`cmake/i18n/messages.json`

```json
{
  "messages": {
    "pkg.headers_missing": {
      "en": "missing include/duilib/ headers",
      "zh_CN": "缺少 include/duilib/ 头文件"
    }
  }
}
```

- 键名按脚本分组：`pkg.*`（`pack.py`）、`rel.*`（`release.py`）、
  `auto.*`（`autotest.py`）、`sdk.*`（`sdk_package_test.py`）、
  `build.*` / `deps.*` / `doctor.*` / `test.*` / `pack.*`（`build.py` 运行期）、
  `help.*`（`build.py` argparse 文案）、`preflight.*` / `lang.*`（CMake 侧）；
- 脚本中只出现英文 key（`T('pkg.headers_missing')`），中文只在 JSON 中；
- 每个键必须同时提供 `en` 与 `zh_CN`；缺失时 `Messages.t()` 回退 `en`，
  再回退键名本身（便于发现漏配）。

## 脚本接入方式

五个入口统一使用同一套脚手架（`build.py`、`pack.py`、`release.py`、
`autotest.py`、`sdk_package_test.py`）：

```python
from i18n import make_messages, set_console_utf8

MESSAGES = make_messages()


def T(key):
    """取控制台文案：简体中文系统显示中文，其余（含 zh_TW/zh_HK）显示英文。"""
    return MESSAGES.t(key)


def _requested_lang(argv):
    """预扫描 --lang：让 --help 与参数错误也按所选语言输出。"""
    ...
```

要点：

1. **语言必须在 `argparse` 之前就绪**：`main()` 先 `MESSAGES = make_messages(
   _requested_lang(argv))`，再 `set_console_utf8()`，最后 `build_parser()` /
   `parse_args()`；否则 `--help` 与参数错误仍使用导入时的语言，在 ASCII 控制台
   会直接失败。
2. 五个入口都提供 `--lang {auto,zh_CN,en}`（默认 `auto`）。`release.py` 的
   `--lang` 同时挂在主命令与子命令上（子解析器用 `argparse.SUPPRESS` 作默认值），
   因此写在子命令之后也生效，且不会覆盖写在子命令之前的公共选项。
3. 模块级常量不能直接调用 `T()`（导入时语言尚未确定）：`autotest.py` 的
   `PRESETS[...]['description']` 与 `sdk_package_test.py` 的 `SCENARIO_ORDER`
   保存 key 字符串，在打印处再 `T(...)`。
4. `sdk_package_test.py` 调用 `pack.py --verify` 时显式传递
   `--lang MESSAGES.language`，保证子进程与本进程语言一致，负向用例的
   `expected` 文本能够匹配。
5. `set_console_utf8()` 在 Windows 上把控制台切到 UTF-8；非 Windows 直接返回。
   中文模式下仍需 UTF-8 控制台，英文模式输出为纯 ASCII。

## 不参与翻译的内容

- 所有注释、docstring（保持中文）；
- `pack.py` 的 `write_manifest()` 生成的 `MANIFEST.txt` 文本与字段名
  （`版本`/`平台`/`配置`/`工具链`/`包名`/`包含示例`/`是`）；
- `release.py` 生成的 `RELEASE-NOTES.md` 标题 `发布说明`；
- `packaging/cmake/nim_duilibConfig.cmake` 中的中文报错文本
  （`配置为`/`架构为`/`运行库`/`该 nim_duilib SDK 是`）——SDK 消费方按其
  原文匹配；
- `build/*.bat`：中文只出现在 `@REM` 注释里；
- 一次性维护脚本（`apply_*.py`、`fix_*.py`、`write_release_docs.py` 等）。

## 测试

- `build/test_build.py`：`resolve_language` 的 CLI / 环境变量 / 系统区域优先级，
  新增繁体区域 → 英文、简体变体 → 中文用例；`--lang en` 时 argparse 帮助为
  纯 ASCII；`--help` 语言在 argparse 之前生效。
- `build/test_pack.py`、`build/test_autotest.py`、`build/test_sdk_package.py`：
  断言前固定 `MESSAGES = make_messages('zh_CN')`，避免宿主系统语言影响结果；
  另设英文用例断言关键文案为纯 ASCII。
- `python build/build.py test`（`unittest discover -s build`）覆盖全部 236 个用例。

## 验收

1. `PYTHONIOENCODING=ascii` 下，以下命令输出均为纯 ASCII（非 ASCII 字符数为 0）：

   ```bat
   python build/build.py --help --lang en
   python build/pack.py --help --lang en
   python build/pack.py --list --lang en
   python build/release.py --help --lang en
   python build/release.py clean --list --lang en
   python build/autotest.py --help --lang en
   python build/autotest.py --list-presets --lang en
   python build/sdk_package_test.py --help --lang en
   python build/sdk_package_test.py --list-scenarios --lang en
   ```

2. `DUILIB_LANG=zh_CN` / `zh_Hans` 显示中文；`zh_TW` / `zh_HK` / `zh_Hant` 显示英文。
3. `python build/build.py test` 全部通过。
4. 扫描五个脚本的非 docstring 中文字面量，仅剩上文「不参与翻译的内容」。
5. 不提交代码；只保证单元测试与上述命令通过。
