#!/bin/bash
# nim_duilib AI toolchain - global register script
# Usage: cd nim_duilib && bash .claude/register.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DUILIB_ROOT="$(dirname "$SCRIPT_DIR")"
CLAUDE_HOME="${HOME}/.claude"
GLOBAL_SKILLS_DIR="${CLAUDE_HOME}/skills"

echo "=== nim_duilib AI toolchain register ==="
echo "nim_duilib path: ${DUILIB_ROOT}"
echo ""

# Validate
if [ ! -f "${DUILIB_ROOT}/duilib/duilib.h" ]; then
    echo "Error: nim_duilib project not detected. Run this from nim_duilib root."
    exit 1
fi

# ============================================================
# Step 0: Clean up stale nim_duilib skills from previous versions.
#         Two sources of zombies:
#           a) 历史曾用过的旧技能名（未带 nim-duilib- 前缀）
#           b) 当前前缀下已改名的技能
#         Both would otherwise linger in ${GLOBAL_SKILLS_DIR} forever.
LEGACY_SKILL_NAMES="nim-create-window nim-xml-layout nim-add-control nim-event-handler"

echo "[0/3] Cleaning up stale skills..."
if [ -d "${GLOBAL_SKILLS_DIR}" ]; then
    # 汇总所有可能需要清理的候选目录
    candidates="${LEGACY_SKILL_NAMES}"
    for skill_dir in "${GLOBAL_SKILLS_DIR}"/nim-duilib-*; do
        [ -d "$skill_dir" ] || continue
        candidates="${candidates} $(basename "$skill_dir")"
    done

    for skill_base in ${candidates}; do
        target_dir="${GLOBAL_SKILLS_DIR}/${skill_base}"
        [ -d "$target_dir" ] || continue

        # 是否属于本次要注册的技能？
        keep=0
        for skill_file in "${SCRIPT_DIR}/skills"/nim-duilib-*.md; do
            [ -f "$skill_file" ] || continue
            if [ "$(basename "$skill_file" .md)" = "$skill_base" ]; then
                keep=1
                break
            fi
        done

        if [ "$keep" -eq 0 ]; then
            rm -rf "$target_dir"
            echo "  - stale: ${skill_base}"
        fi
    done
fi

# ============================================================
# Step 1: Register all nim-duilib-* skills as global skills
# ============================================================
echo "[1/3] Registering global skills..."

for skill_file in "${SCRIPT_DIR}/skills"/nim-duilib-*.md; do
    [ -f "$skill_file" ] || continue
    skill_name="$(basename "$skill_file" .md)"
    target_dir="${GLOBAL_SKILLS_DIR}/${skill_name}"
    mkdir -p "$target_dir"
    cp "$skill_file" "${target_dir}/SKILL.md"
    echo "  + ${skill_name}"
done

# ============================================================
# Step 2: Register /nim-init command
# ============================================================
echo "[2/3] Registering /nim-init command..."

NIM_INIT_DIR="${GLOBAL_SKILLS_DIR}/nim-init"
mkdir -p "$NIM_INIT_DIR"

cat > "${NIM_INIT_DIR}/SKILL.md" << SKILLEOF
---
name: nim-init
description: "Initialize nim_duilib AI dev toolkit for current project (copy LLM docs + update CLAUDE.md). Global skills are already available - this command sets up project-specific config."
user-invocable: true
disable-model-invocation: true
---

# nim_duilib AI Toolkit - Project Init

When the user invokes /nim-init, follow these steps:

## Config
- nim_duilib install path: \`${DUILIB_ROOT}\`
- Source docs: \`${DUILIB_ROOT}/.claude/docs/\`

## Step 1: Copy LLM reference doc
Run this bash command to copy the reference doc to the current project:
\`\`\`bash
mkdir -p .claude/docs
cp "${DUILIB_ROOT}/.claude/docs/nim-duilib-llm-reference.md" .claude/docs/
\`\`\`

## Step 2: Update CLAUDE.md
Check if the current project's CLAUDE.md already contains "nim_duilib UI". If not, append the following block (create CLAUDE.md if it doesn't exist):

\`\`\`markdown

## nim_duilib UI

This project uses [nim_duilib](https://github.com/rhett-lee/nim_duilib) as the UI framework.
Library path: \`${DUILIB_ROOT}\`

- LLM reference: \`.claude/docs/nim-duilib-llm-reference.md\`
- XML layouts: \`bin/resources/themes/default/<skin_folder>/\`
- Global resources (fonts/colors/styles): \`bin/resources/themes/default/global.xml\`
- nim_duilib docs: \`${DUILIB_ROOT}/docs/\`
- nim_duilib examples: \`${DUILIB_ROOT}/examples/\`

### Resource rules (IMPORTANT)
- MUST copy: \`global.xml\` + \`public/\` (shared icons) + your app's own skin directory
- NEVER copy demo directories (basic/, controls/, layout/, chat/, cef/, render/, etc.)
- NEVER copy bin/*.exe, bin/*.dll, bin/bin.zip
- Resource packaging: local files (dev) / ZIP file (release) / embedded EXE (Windows single-file)

### Key patterns
- Window class extends \`ui::WindowImplBase\`, override \`GetSkinFolder()\`/\`GetSkinFile()\`/\`OnInitWindow()\`
- Main thread extends \`ui::FrameworkThread\`, init resources and create window in \`OnInit()\`
- Find control: \`dynamic_cast<ui::Type*>(FindControl(_T("name")))\`
- Bind event: \`control->AttachClick([](const ui::EventArgs&) { return true; });\`
- String type: \`DString\`, literals wrapped with \`_T("...")\`
\`\`\`

## Step 3: Report
Tell the user initialization is complete. Note that nim_duilib skills are already globally available (no per-project copy needed):
- nim-duilib-create-window - Create new window
- nim-duilib-xml-layout - Design XML layout
- nim-duilib-add-control - Add controls
- nim-duilib-event-handler - Event handlers
- nim-duilib-theme - Theme customization
- nim-duilib-resource-pack - Resource packaging / single EXE
SKILLEOF

# ============================================================
# Done
# ============================================================
echo ""
echo "=== Register complete ==="
echo ""
echo "Registered:"
echo "  /nim-init               - project initialization command"
for skill_file in "${SCRIPT_DIR}/skills"/nim-duilib-*.md; do
    [ -f "$skill_file" ] || continue
    echo "  /$(basename "$skill_file" .md)  - global skill"
done
echo ""
echo "Skills are now globally available in ALL projects."
echo "Run /nim-init in a project to set up CLAUDE.md and LLM docs."
echo ""
echo "Installed layout: ${GLOBAL_SKILLS_DIR}/<skill-name>/SKILL.md"
echo "NOTE: Claude Code requires the <name>/SKILL.md layout. The .md files inside"
echo "      this repo's .claude/skills/ are flat sources only - copy them directly"
echo "      into ~/.claude/skills/ WITHOUT running this script and they will NOT load."
echo ""
echo "To update after editing skills: just re-run this script."
echo "To uninstall: bash .claude/unregister.sh   (Windows: .claude\\unregister.bat)"
