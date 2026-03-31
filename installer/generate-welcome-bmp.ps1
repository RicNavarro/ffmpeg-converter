# generate-welcome-bmp.ps1
# Generates installer/welcome.bmp (164x314) using only .NET System.Drawing.
# No external tools needed — run once before calling makensis.
#
# Usage: .\installer\generate-welcome-bmp.ps1

Add-Type -AssemblyName System.Drawing

$width  = 164
$height = 314
$outPath = Join-Path $PSScriptRoot "welcome.bmp"

$bmp = New-Object System.Drawing.Bitmap($width, $height)
$g   = [System.Drawing.Graphics]::FromImage($bmp)

$g.SmoothingMode   = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAlias

# --- Background gradient (dark navy to slightly lighter) ---
$gradBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
    [System.Drawing.Point]::new(0, 0),
    [System.Drawing.Point]::new(0, $height),
    [System.Drawing.Color]::FromArgb(255, 15, 20, 40),
    [System.Drawing.Color]::FromArgb(255, 25, 35, 65)
)
$g.FillRectangle($gradBrush, 0, 0, $width, $height)
$gradBrush.Dispose()

# --- Accent bar on the left edge ---
$accentBrush = New-Object System.Drawing.SolidBrush(
    [System.Drawing.Color]::FromArgb(255, 74, 144, 217)
)
$g.FillRectangle($accentBrush, 0, 0, 4, $height)
$accentBrush.Dispose()

# --- Circle icon background ---
$circleBrush = New-Object System.Drawing.SolidBrush(
    [System.Drawing.Color]::FromArgb(60, 74, 144, 217)
)
$g.FillEllipse($circleBrush, 32, 40, 100, 100)
$circleBrush.Dispose()

$circlePen = New-Object System.Drawing.Pen(
    [System.Drawing.Color]::FromArgb(180, 74, 144, 217), 2
)
$g.DrawEllipse($circlePen, 32, 40, 100, 100)
$circlePen.Dispose()

# --- "F" letter in circle (representing FFmpeg) ---
$fontLarge = New-Object System.Drawing.Font("Segoe UI", 42, [System.Drawing.FontStyle]::Bold)
$letterBrush = New-Object System.Drawing.SolidBrush(
    [System.Drawing.Color]::FromArgb(255, 74, 144, 217)
)
$letterRect = New-Object System.Drawing.RectangleF(32, 40, 100, 100)
$sf = New-Object System.Drawing.StringFormat
$sf.Alignment = [System.Drawing.StringAlignment]::Center
$sf.LineAlignment = [System.Drawing.StringAlignment]::Center
$g.DrawString("F", $fontLarge, $letterBrush, $letterRect, $sf)
$fontLarge.Dispose()
$letterBrush.Dispose()

# --- App name ---
$fontTitle = New-Object System.Drawing.Font("Segoe UI", 11, [System.Drawing.FontStyle]::Bold)
$whiteBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
$titleRect = New-Object System.Drawing.RectangleF(10, 155, 144, 30)
$g.DrawString("FFmpeg Converter", $fontTitle, $whiteBrush, $titleRect, $sf)
$fontTitle.Dispose()

# --- Version ---
$fontSub = New-Object System.Drawing.Font("Segoe UI", 8, [System.Drawing.FontStyle]::Regular)
$grayBrush = New-Object System.Drawing.SolidBrush(
    [System.Drawing.Color]::FromArgb(180, 180, 200)
)
$subRect = New-Object System.Drawing.RectangleF(10, 182, 144, 20)
$g.DrawString("Version 1.0.0", $fontSub, $grayBrush, $subRect, $sf)
$fontSub.Dispose()

# --- Tagline at the bottom ---
$fontTag = New-Object System.Drawing.Font("Segoe UI", 7, [System.Drawing.FontStyle]::Italic)
$tagRect  = New-Object System.Drawing.RectangleF(10, 270, 144, 36)
$g.DrawString("Fast media conversion powered by FFmpeg", $fontTag, $grayBrush, $tagRect, $sf)
$fontTag.Dispose()
$grayBrush.Dispose()
$whiteBrush.Dispose()

# --- Horizontal divider line ---
$linePen = New-Object System.Drawing.Pen(
    [System.Drawing.Color]::FromArgb(60, 74, 144, 217), 1
)
$g.DrawLine($linePen, 10, 205, 154, 205)
$g.DrawLine($linePen, 10, 255, 154, 255)
$linePen.Dispose()

$g.Dispose()

# Save as 24-bit BMP (NSIS requires BMP, not PNG)
$bmp.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Bmp)
$bmp.Dispose()

Write-Host "Generated: $outPath" -ForegroundColor Green
