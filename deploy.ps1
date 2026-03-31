# deploy.ps1
# Builds the project, bundles Qt DLLs, and generates a Setup .exe via NSIS.
# Run from the project root: .\deploy.ps1
#
# Parameters:
#   -QtPath    Path to your Qt installation (MSVC 64-bit)
#   -BuildDir  CMake build directory (default: build-release)
#   -SkipNSIS  Skip installer generation (just produce the raw Release folder)

param(
    [string]$QtPath   = "C:\Qt\6.7.0\msvc2019_64",
    [string]$BuildDir = "build-release",
    [switch]$SkipNSIS = $false
)

$ErrorActionPreference = "Stop"
$AppVersion = "1.0.0"

Write-Host ""
Write-Host "=== FFmpeg Converter — Windows Release Build ===" -ForegroundColor Cyan
Write-Host ""

# -----------------------------------------------------------------------
# Helper: abort with a clear message
# -----------------------------------------------------------------------
function Fail([string]$msg) {
    Write-Host ""
    Write-Host "ERROR: $msg" -ForegroundColor Red
    Write-Host ""
    exit 1
}

# -----------------------------------------------------------------------
# 0. Pre-flight checks
# -----------------------------------------------------------------------
Write-Host "[0/5] Checking prerequisites..." -ForegroundColor Yellow

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Fail "cmake not found. Install from https://cmake.org/"
}

$windeployqt = Join-Path $QtPath "bin\windeployqt.exe"
if (-not (Test-Path $windeployqt)) {
    Fail "windeployqt not found at: $windeployqt`nCheck your -QtPath parameter."
}

$ffmpegBundled = "ffmpeg\ffmpeg.exe"
if (-not (Test-Path $ffmpegBundled)) {
    Fail "Bundled ffmpeg.exe not found at: $ffmpegBundled`nDownload from https://www.gyan.dev/ffmpeg/builds/ and place it in the ffmpeg\ folder."
}

if (-not $SkipNSIS) {
    $makensis = Get-Command makensis -ErrorAction SilentlyContinue
    if (-not $makensis) {
        # Also check the default NSIS install location
        $nsisDefault = "C:\Program Files (x86)\NSIS\makensis.exe"
        if (Test-Path $nsisDefault) {
            $makensis = $nsisDefault
        } else {
            Fail "makensis not found.`nInstall NSIS from https://nsis.sourceforge.io/ or via:`n  winget install NSIS.NSIS`nOr run with -SkipNSIS to skip installer generation."
        }
    } else {
        $makensis = $makensis.Source
    }
}

Write-Host "  Qt:        $QtPath" -ForegroundColor DarkGray
Write-Host "  BuildDir:  $BuildDir" -ForegroundColor DarkGray
if (-not $SkipNSIS) {
    Write-Host "  NSIS:      $makensis" -ForegroundColor DarkGray
}

# -----------------------------------------------------------------------
# 1. CMake configure
# -----------------------------------------------------------------------
Write-Host ""
Write-Host "[1/5] Configuring with CMake..." -ForegroundColor Yellow

cmake -S . -B $BuildDir `
      -DCMAKE_PREFIX_PATH="$QtPath" `
      -DCMAKE_BUILD_TYPE=Release

if ($LASTEXITCODE -ne 0) { Fail "CMake configure failed." }

# -----------------------------------------------------------------------
# 2. CMake build
# -----------------------------------------------------------------------
Write-Host ""
Write-Host "[2/5] Building (Release)..." -ForegroundColor Yellow

cmake --build $BuildDir --config Release --parallel

if ($LASTEXITCODE -ne 0) { Fail "Build failed." }

# -----------------------------------------------------------------------
# 3. windeployqt — copy all Qt DLLs next to the .exe
# -----------------------------------------------------------------------
Write-Host ""
Write-Host "[3/5] Running windeployqt..." -ForegroundColor Yellow

$exePath    = Join-Path $BuildDir "Release\FFmpegWrapper.exe"
$releaseDir = Join-Path $BuildDir "Release"

& $windeployqt `
    --release `
    --no-translations `
    --no-system-d3d-compiler `
    --no-opengl-sw `
    $exePath

if ($LASTEXITCODE -ne 0) { Fail "windeployqt failed." }

# -----------------------------------------------------------------------
# 4. Also keep a plain zip for users who prefer that
# -----------------------------------------------------------------------
Write-Host ""
Write-Host "[4/5] Creating portable zip..." -ForegroundColor Yellow

$zipName = "FFmpegConverter-v$AppVersion-win64-portable.zip"
if (Test-Path $zipName) { Remove-Item $zipName }
Compress-Archive -Path "$releaseDir\*" -DestinationPath $zipName
Write-Host "  -> $zipName" -ForegroundColor DarkGray

# -----------------------------------------------------------------------
# 5. NSIS — generate the Setup .exe
# -----------------------------------------------------------------------
if ($SkipNSIS) {
    Write-Host ""
    Write-Host "[5/5] Skipping NSIS (--SkipNSIS flag set)." -ForegroundColor DarkGray
} else {
    Write-Host ""
    Write-Host "[5/5] Generating installer with NSIS..." -ForegroundColor Yellow

    # Generate welcome.bmp if it doesn't exist yet
    $welcomeBmp = "installer\welcome.bmp"
    if (-not (Test-Path $welcomeBmp)) {
        Write-Host "  Generating welcome.bmp..." -ForegroundColor DarkGray
        & powershell -ExecutionPolicy Bypass -File "installer\generate-welcome-bmp.ps1"
    }

    # Run makensis
    & $makensis "installer\installer.nsi"

    if ($LASTEXITCODE -ne 0) { Fail "NSIS failed. Check the output above for details." }

    $setupExe = "installer\FFmpegConverter-Setup-$AppVersion.exe"
    if (Test-Path $setupExe) {
        # Move the setup exe to the project root for easy access
        $dest = "FFmpegConverter-Setup-v$AppVersion.exe"
        Move-Item $setupExe $dest -Force
        Write-Host "  -> $dest" -ForegroundColor DarkGray
    }
}

# -----------------------------------------------------------------------
# Done
# -----------------------------------------------------------------------
Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host " Build complete!" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

if (-not $SkipNSIS) {
    $setupFile = "FFmpegConverter-Setup-v$AppVersion.exe"
    if (Test-Path $setupFile) {
        $sizeMB = [math]::Round((Get-Item $setupFile).Length / 1MB, 1)
        Write-Host " Installer:  $setupFile  ($sizeMB MB)" -ForegroundColor Green
    }
}

$zipFile = "FFmpegConverter-v$AppVersion-win64-portable.zip"
if (Test-Path $zipFile) {
    $sizeMB = [math]::Round((Get-Item $zipFile).Length / 1MB, 1)
    Write-Host " Portable:   $zipFile  ($sizeMB MB)" -ForegroundColor Green
}

Write-Host ""
Write-Host " Installed files:" -ForegroundColor DarkGray
Get-ChildItem $releaseDir | Select-Object Name, @{N="Size";E={
    if ($_.PSIsContainer) { "(folder)" }
    else { "$([math]::Round($_.Length/1KB))  KB" }
}} | Format-Table -AutoSize
