# nim_duilib AI toolchain - global unregister script (Windows)
# Usage: cd nim_duilib && .claude\unregister.bat
#    or: pwsh .claude\unregister.ps1

$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ClaudeHome = Join-Path $env:USERPROFILE ".claude"
$GlobalSkillsDir = Join-Path $ClaudeHome "skills"

Write-Host "=== nim_duilib AI toolchain unregister ===" -ForegroundColor Cyan

if (-not (Test-Path $GlobalSkillsDir)) {
    Write-Host ""
    Write-Host "No global skills directory found ($GlobalSkillsDir). Nothing to remove."
    exit 0
}

# Remove /nim-init
$NimInitDir = Join-Path $GlobalSkillsDir "nim-init"
if (Test-Path $NimInitDir) {
    Remove-Item -Path $NimInitDir -Recurse -Force -Confirm:$false
    Write-Host "  - removed /nim-init" -ForegroundColor Yellow
}

# Remove all nim-duilib-* global skills
$SkillDirs = Get-ChildItem -Path $GlobalSkillsDir -Directory -Filter "nim-duilib-*"
foreach ($dir in $SkillDirs) {
    Remove-Item -Path $dir.FullName -Recurse -Force -Confirm:$false
    Write-Host "  - removed /$($dir.Name)" -ForegroundColor Yellow
}

# Remove legacy skill names used by older versions (no nim-duilib- prefix).
# These would otherwise be left behind forever after a rename.
$LegacySkillNames = @("nim-create-window", "nim-xml-layout", "nim-add-control", "nim-event-handler")
foreach ($name in $LegacySkillNames) {
    $dir = Join-Path $GlobalSkillsDir $name
    if (Test-Path $dir) {
        Remove-Item -Path $dir -Recurse -Force -Confirm:$false
        Write-Host "  - removed legacy /$name" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "All nim_duilib global skills removed." -ForegroundColor Green
Write-Host ""
Write-Host "Note: per-project files (.claude/docs/, CLAUDE.md) are not removed." -ForegroundColor Gray
Write-Host "Clean them manually if needed." -ForegroundColor Gray
