# nim_duilib AI toolchain - global register script
# Usage: cd nim_duilib && .claude\register.bat
#    or: pwsh .claude\register.ps1

$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$DuilibRoot = (Resolve-Path (Join-Path $ScriptDir "..")).Path
$ClaudeHome = Join-Path $env:USERPROFILE ".claude"
$GlobalSkillsDir = Join-Path $ClaudeHome "skills"
$NimInitDir = Join-Path $GlobalSkillsDir "nim-init"

Write-Host "=== nim_duilib AI toolchain register ===" -ForegroundColor Cyan
Write-Host "nim_duilib path: $DuilibRoot"
Write-Host ""

# Validate
if (-not (Test-Path (Join-Path (Join-Path $DuilibRoot "duilib") "duilib.h"))) {
    Write-Host "Error: nim_duilib project not detected. Run this from nim_duilib root." -ForegroundColor Red
    exit 1
}

# Normalize path for use in markdown (forward slashes)
$DuilibRootUnix = $DuilibRoot -replace '\\', '/'

# ============================================================
# Step 0: Clean up stale nim_duilib skills from previous versions.
#         Two sources of zombies:
#           a) 历史曾用过的旧技能名（未带 nim-duilib- 前缀）
#           b) 当前前缀下已改名的技能
#         Both would otherwise linger in $GlobalSkillsDir forever.
# ============================================================
Write-Host "[0/3] Cleaning up stale skills..." -ForegroundColor Yellow
$LegacySkillNames = @("nim-create-window", "nim-xml-layout", "nim-add-control", "nim-event-handler")

$SourceSkillsDir = Join-Path $ScriptDir "skills"
$SkillFiles = Get-ChildItem -Path $SourceSkillsDir -Filter "nim-duilib-*.md" -File
$SkillNames = @($SkillFiles | ForEach-Object { $_.BaseName })

$CandidateNames = @()
$CandidateNames += $LegacySkillNames
if (Test-Path $GlobalSkillsDir) {
    $CandidateNames += @(Get-ChildItem -Path $GlobalSkillsDir -Directory -Filter "nim-duilib-*" |
                         ForEach-Object { $_.Name })
}
$CandidateNames = @($CandidateNames | Sort-Object -Unique)

foreach ($name in $CandidateNames) {
    if ([string]::IsNullOrEmpty($name)) { continue }
    $targetDir = Join-Path $GlobalSkillsDir $name
    if (-not (Test-Path $targetDir)) { continue }
    # 不在本次注册列表中的，就是旧技能 -> 删除
    if ($SkillNames -notcontains $name) {
        Remove-Item -Path $targetDir -Recurse -Force -Confirm:$false
        Write-Host "  - stale: $name"
    }
}

# ============================================================
# Step 1: Register all nim-duilib-* skills as global skills
# ============================================================
Write-Host "[1/3] Registering global skills..." -ForegroundColor Yellow

foreach ($file in $SkillFiles) {
    # Each skill needs its own subdirectory: ~/.claude/skills/<name>/SKILL.md
    $skillName = $file.BaseName  # e.g. "nim-duilib-create-window"
    $targetDir = Join-Path $GlobalSkillsDir $skillName
    if (-not (Test-Path $targetDir)) {
        New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    }
    Copy-Item -Path $file.FullName -Destination (Join-Path $targetDir "SKILL.md") -Force
    Write-Host "  + $skillName"
}

# ============================================================
# Step 2: Register /nim-init command
# ============================================================
Write-Host "[2/3] Registering /nim-init command..." -ForegroundColor Yellow

if (-not (Test-Path $NimInitDir)) {
    New-Item -ItemType Directory -Path $NimInitDir -Force | Out-Null
}

$SkillContent = @"
---
name: nim-init
description: "Initialize nim_duilib AI dev toolkit for current project (copy LLM docs + update CLAUDE.md). Global skills are already available - this command sets up project-specific config."
user-invocable: true
disable-model-invocation: true
---

# nim_duilib AI Toolkit - Project Init

When the user invokes /nim-init, follow these steps:

## Config
- nim_duilib install path: ``$DuilibRootUnix``
- Source docs: ``$DuilibRootUnix/.claude/docs/``

## Step 1: Copy LLM reference doc
Run this bash command to copy the reference doc to the current project:
``````bash
mkdir -p .claude/docs
cp "$DuilibRootUnix/.claude/docs/nim-duilib-llm-reference.md" .claude/docs/
``````

## Step 2: Update CLAUDE.md
Check if the current project's CLAUDE.md already contains "nim_duilib UI". If not, append the following block (create CLAUDE.md if it doesn't exist):

``````markdown

## nim_duilib UI

This project uses [nim_duilib](https://github.com/rhett-lee/nim_duilib) as the UI framework.
Library path: ``$DuilibRootUnix``

- LLM reference: ``.claude/docs/nim-duilib-llm-reference.md``
- XML layouts: ``bin/resources/themes/default/<skin_folder>/``
- Global resources (fonts/colors/styles): ``bin/resources/themes/default/global.xml``
- nim_duilib docs: ``$DuilibRootUnix/docs/``
- nim_duilib examples: ``$DuilibRootUnix/examples/``

### Resource rules (IMPORTANT)
- MUST copy: ``global.xml`` + ``public/`` (shared icons) + your app's own skin directory
- NEVER copy demo directories (basic/, controls/, layout/, chat/, cef/, render/, etc.)
- NEVER copy bin/*.exe, bin/*.dll, bin/bin.zip
- Resource packaging: local files (dev) / ZIP file (release) / embedded EXE (Windows single-file)

### Key patterns
- Window class extends ``ui::WindowImplBase``, override ``GetSkinFolder()``/``GetSkinFile()``/``OnInitWindow()``
- Main thread extends ``ui::FrameworkThread``, init resources and create window in ``OnInit()``
- Find control: ``dynamic_cast<ui::Type*>(FindControl(_T("name")))``
- Bind event: ``control->AttachClick([](const ui::EventArgs&) { return true; });``
- String type: ``DString``, literals wrapped with ``_T("...")``
``````

## Step 3: Report
Tell the user initialization is complete. Note that nim_duilib skills are already globally available (no per-project copy needed):
- nim-duilib-create-window - Create new window
- nim-duilib-xml-layout - Design XML layout
- nim-duilib-add-control - Add controls
- nim-duilib-event-handler - Event handlers
- nim-duilib-theme - Theme customization
- nim-duilib-resource-pack - Resource packaging / single EXE
"@

$SkillPath = Join-Path $NimInitDir "SKILL.md"
[System.IO.File]::WriteAllText($SkillPath, $SkillContent, [System.Text.UTF8Encoding]::new($false))

# ============================================================
# Done
# ============================================================
Write-Host ""
Write-Host "=== Register complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "Registered:" -ForegroundColor Yellow
Write-Host "  /nim-init               - project initialization command"
foreach ($file in $SkillFiles) {
    Write-Host "  /$($file.BaseName)  - global skill"
}
Write-Host ""
Write-Host "Skills are now globally available in ALL projects." -ForegroundColor Cyan
Write-Host "Run /nim-init in a project to set up CLAUDE.md and LLM docs."
Write-Host ""
Write-Host "Installed layout: $GlobalSkillsDir\<skill-name>\SKILL.md" -ForegroundColor Gray
Write-Host "NOTE: Claude Code requires the <name>\SKILL.md layout. The .md files inside this" -ForegroundColor Gray
Write-Host "      repo's .claude\skills\ are flat sources only - copied without this script they will NOT load." -ForegroundColor Gray
Write-Host ""
Write-Host "To update after editing skills: just re-run this script." -ForegroundColor Gray
Write-Host "To uninstall: .claude\unregister.bat (or pwsh .claude\unregister.ps1)" -ForegroundColor Gray
