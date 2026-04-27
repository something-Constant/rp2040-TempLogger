@echo off
setlocal enabledelayedexpansion
echo ========================================
echo Copying firmware to Raspberry Pi Pico
echo ========================================
echo.

set FOUND=0
set UF2_FILE="Generated_Debug\Temp_Logger_Debug_0.0.0.uf2"
set Pico_Name="RPI-RP2"

:: Check if UF2 file exists
if not exist "%UF2_FILE%" (
    echo ERROR: UF2 file not found: %UF2_FILE%
    echo Run 'cmake --build build' first
    pause
    exit /b 1
)

:notfound
:: Find Pico drive (look for drive with RPI-RP2 label)
for %%D in (A B D I J K L M N O Q R T U V W X Y Z) do (
    if exist %%D:\ (
        if exist %%D:\INFO_UF2.TXT (
            set PICO_DRIVE=%%D:
            set FOUND=1
            goto :found
        )
    )
)

goto :notfound


:found
if %FOUND% equ 1 (
    echo Pico drive found at %PICO_DRIVE%
    echo Copying %UF2_FILE%...
    copy /Y "%UF2_FILE%" "%PICO_DRIVE%\"
    if !errorlevel! equ 0 (
        echo.
        echo ========================================
        echo SUCCESS! Firmware copied to Pico
        echo ========================================
    ) else (
        echo ERROR: Failed to copy file
    )
) else (
    echo ERROR: Pico not found in bootloader mode
    echo.
    echo To put Pico in bootloader mode:
    echo 1. Hold the BOOTSEL button on the Pico
    echo 2. Connect USB cable
    echo 3. Release BOOTSEL button
    echo 4. Wait for RPI-RP2 drive to appear
    echo 5. Run this script again
)