# 发布 checklist

本清单用于从干净工作区产出一个可复现的版本发布。自动化部分由
`build/release.py` 提供；`git commit`、`tag`、`push` 始终由维护者手动执行。

详细参数与接入说明见 [发布与打包](../Release.md)。

## 1. 准备

- [ ] 确认目标平台与架构（Windows `x64`/`x86`/`arm64`；Linux/macOS/FreeBSD `x64`/`arm64`）。
      arm64 本轮只验证 Windows（macOS / Linux / FreeBSD 无真机，不安排测试）；
      Windows Release/Debug 构建、无 CEF / CEF 142（Release）与 CEF 142（Debug）三个包的
      出包与包测试已完成；DLL 模式、CEF109 与运行验证仍待补（运行验证需 Windows on ARM 设备）。三平台均可随包
      CEF（Windows 142 与 109 的 `windowsarm64` 预编译包、Linux `linuxarm64`、
      macOS `macosarm64`）；执行步骤与结果见 `build/arm64-test-plan.md`。
- [ ] 确认依赖产物已按目标配置完整编译：
  - `duilib` 5 个库（`duilib`、`duilib-cximage`、`duilib-webp`、`duilib-png`、`duilib-zlib`）；
  - Skia 静态库目录；
  - 需要时：SDL3、CEF、示例程序。
- [ ] 确认库目录符合约定：Windows `lib/<arch>/<Config>/`，Unix `lib/<Config>/`。
- [ ] 确认 `dist/` 不参与提交（已在 `.gitignore` 中）。
- [ ] 若本次新增随包二进制组件或第三方资源（字体、图片、网页库），已按
      [许可合规对照表](License-Compliance.md) 的登记流程处理：
      - 以 `python build/pack.py --config <Config> --arch <arch> [特性开关] --list`
        的产物清单逐项对照表格，确认没有未登记项；
      - 在 `licenses/` 补充许可文本，并在中英文两份 `THIRD-PARTY-NOTICES`
        （`.md` / `.en.md`）与对照表各补一行；
      - 在对照表的「更新记录」注明来源（项目/外部素材库/官方分发包）。
- [ ] 若本次启用 libpag（非 SDK 默认配置），已随程序分发
      `duilib/third_party/libpag/licenses/` 下的全部文件（含 `libpag.LICENSE.txt`、
      `tgfx.LICENSE.txt`），并确认未使用 lz4（GPLv2）与 Qt（GPLv3）。
- [ ] （可选）需要对 CEF 覆盖 Chromium 组件声明时，已从与 CEF 主版本一致的
      Chrome/Chromium 复制 `LICENSES.chromium.html` 到包内 `LICENSES/CEF/` 并登记；
      默认不做（理由与体积评估见 [许可合规对照表](License-Compliance.md)）。

## 2. 更新版本与变更记录

- [ ] 修改 `duilib/duilib_version.h` 的四个数字宏；
      `DUILIB_VER_BUILD` 按 `MMDD` 书写，**不补零**。
- [ ] 在 `CHANGELOG.md` 与 `CHANGELOG.en.md` 同时加入相同版本标题
      （`## [<版本>] - <日期>`），并填写变更内容；两份文件的版本标题必须一致。
- [ ] 提交以上改动后再执行预检（`check` 要求工作区无未提交修改）。

## 3. 预检

```
python build/release.py check
```

- [ ] 工作区无已跟踪文件的未提交修改；
- [ ] 版本号可解析；
- [ ] 当前 `v<版本>` tag 不存在，且版本号相对最近的 `v*` tag 严格递增；
- [ ] 中英 CHANGELOG 都包含当前版本条目。

任一项失败时按输出修正，不要跳过。

## 4. 打包

每个目标单独执行，先 `--dry-run` 核对命令与包名，再去掉该参数实际打包。
产物统一输出到 `dist/<版本>/`。

Windows（在已编译 Release/Debug 产物的机器上）：

```
python build/release.py package --config Release --arch x64 --dry-run
python build/release.py package --config Release --arch x64

python build/release.py package --config Debug   --arch x64
python build/release.py package --config Release --arch x64 --with-examples
python build/release.py package --config Release --arch x64 --with-sdl
python build/release.py package --config Release --arch x64 --with-cef
python build/release.py package --config Release --arch x64 --with-cef --cef-version 109
```

Linux / macOS / FreeBSD（先执行对应构建脚本）：

```
python build/release.py package --config Release
python build/release.py package --config Debug
```

- [ ] 按发布计划逐项执行，包名后缀应为
      `[-mingw-gcc|-mingw-llvm][-sdl][-cef|-cef109][-examples][-debug]`，
      同一版本不互相覆盖；MinGW 的 gcc / llvm 包必须分别产出。
- [ ] 抽查打包输出中的告警（如缺少 Skia、CEF 头文件等），确认可接受。

## 5. 产物校验

```
python build/release.py finalize
```

对 `dist/<版本>/` 下每个压缩包执行：

- [ ] 解压后 `SHA256SUMS` 全量校验通过；
- [ ] 存在 `MANIFEST.txt`、`VERSION`、`include/duilib/duilib.h`；
- [ ] `VERSION` 内容与当前版本一致；
- [ ] 包内文档相对链接无断链；
- [ ] 包内 `licenses/` 覆盖所有随包组件（对照
      [许可合规对照表](License-Compliance.md)），可选的 libpag/CEF 附加声明
      按实际是否随包复核；
- [ ] 生成 `dist/<版本>/SHA256SUMS.txt`；
- [ ] 生成 `dist/<版本>/RELEASE-NOTES.md` 与 `RELEASE-NOTES.en.md`。

任一项失败时不会生成发布说明，按输出修正后重跑。

## 6. 提交与打 tag（手动）

```
git add duilib/duilib_version.h CHANGELOG.md CHANGELOG.en.md
git commit -m "release: <版本>"
git tag -a v<版本> -m "release <版本>"
git push origin --tags
```

- [ ] commit 与 tag 已推送到远端；
- [ ] tag 名为 `v<版本>`，与包名中的版本一致。

## 7. 归档与清理

- [ ] 上传 `dist/<版本>/` 下的全部压缩包、`SHA256SUMS.txt` 与中英发布说明。
- [ ] 查看历史产物：

  ```
  python build/release.py clean --list
  ```

- [ ] 需要时手动删除旧版本（脚本拒绝删除当前版本）：

  ```
  python build/release.py clean --version <旧版本> --yes
  ```

`clean` 默认只列出，不删除任何文件。
