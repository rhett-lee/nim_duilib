#!/bin/bash
# nim_duilib AI toolchain - global unregister script
# Usage: bash .claude/unregister.sh

set -e

CLAUDE_HOME="${HOME}/.claude"
GLOBAL_SKILLS_DIR="${CLAUDE_HOME}/skills"

echo "=== nim_duilib AI toolchain unregister ==="

# Remove /nim-init
if [ -d "${GLOBAL_SKILLS_DIR}/nim-init" ]; then
    rm -rf "${GLOBAL_SKILLS_DIR}/nim-init"
    echo "  - removed /nim-init"
fi

# Remove all nim-duilib-* global skills
for skill_dir in "${GLOBAL_SKILLS_DIR}"/nim-duilib-*; do
    if [ -d "$skill_dir" ]; then
        echo "  - removed /$(basename "$skill_dir")"
        rm -rf "$skill_dir"
    fi
done

echo ""
echo "All nim_duilib global skills removed."
echo ""
echo "Note: per-project files (.claude/docs/, CLAUDE.md) are not removed."
echo "Clean them manually if needed."
