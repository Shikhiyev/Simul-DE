# SimulIDE Windows Setup Script
# This script downloads dependencies (like libelf) required for building SimulIDE on Windows.

$ROOT = Resolve-Path "$PSScriptRoot\.."
$THIRDPARTY = "$ROOT\3rdparty"
$LIBELF_DIR = "$THIRDPARTY\libelf"

Write-Host "=== SimulIDE Windows Setup ===" -ForegroundColor Cyan

# 1. Create directories
if (-not (Test-Path $LIBELF_DIR)) {
    New-Item -ItemType Directory -Force -Path $LIBELF_DIR | Out-Null
    New-Item -ItemType Directory -Force -Path "$LIBELF_DIR\include" | Out-Null
    New-Item -ItemType Directory -Force -Path "$LIBELF_DIR\lib" | Out-Null
}

# 2. Download libelf-win32
# Using a reliable mirror or pre-built binaries if possible. 
# For now, we point the user to download it or provide a Direct Link if we have one.
# Since we want it to be automated, let's try to get it from a common location.

$LIBELF_ZIP = "$env:TEMP\libelf-win32.zip"
$URL = "https://github.com/vovkos/libelf-win32/archive/refs/heads/master.zip"

Write-Host "Downloading libelf-win32 source..." -ForegroundColor Yellow
Invoke-WebRequest -Uri $URL -OutFile $LIBELF_ZIP

Write-Host "Extracting..." -ForegroundColor Yellow
$EXTRACT_PATH = "$env:TEMP\libelf-win32-temp"
if (Test-Path $EXTRACT_PATH) { Remove-Item -Recurse -Force $EXTRACT_PATH }
Expand-Archive -Path $LIBELF_ZIP -DestinationPath $EXTRACT_PATH

# libelf-win32 is a source-only distribution usually, but it has a .pro file!
# We can actually include it in our build!

Write-Host "Setting up libelf in 3rdparty..." -ForegroundColor Green
Copy-Item -Recurse -Force "$EXTRACT_PATH\libelf-win32-master\lib\*" "$LIBELF_DIR\include"
# libelf-win32 usually has headers in 'lib' folder

# Clean up
Remove-Item $LIBELF_ZIP
Remove-Item -Recurse $EXTRACT_PATH

Write-Host "Setup complete. You can now open SimulIDE.pro in Qt Creator." -ForegroundColor Green
Write-Host "Make sure to run 'qmake' after opening the project." -ForegroundColor Yellow
