# Build helper script for Zephyr programs
# Usage: .\scripts\build.ps1 01_led_blink

param([string]$Program = "01_led_blink")

$env:PATH += ";C:\Users\$env:USERNAME\AppData\Roaming\Python\Python314\Scripts;C:\arm-toolchain\bin"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "gnuarmemb"
$env:GNUARMEMB_TOOLCHAIN_PATH = "C:\arm-toolchain"

Write-Host "Building: $Program" -ForegroundColor Cyan
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
west build -b rpi_pico2/rp2350a/m33 "programs\$Program"

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "BUILD SUCCESS!" -ForegroundColor Green
    Write-Host "UF2 file: build\zephyr\zephyr.uf2" -ForegroundColor Green
    Write-Host ""
    Write-Host "Now: Hold BOOTSEL + plug USB -> copy UF2 to RPI-RP2 drive" -ForegroundColor Yellow
} else {
    Write-Host "BUILD FAILED - check errors above" -ForegroundColor Red
}
