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
# Step 1: Register all nim-duilib-* skills as global skills
# ============================================================
echo "[1/2] Registering global skills..."

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
echo "[2/2] Registering /nim-init command..."

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
echo "To update after editing skills: just re-run this script."
