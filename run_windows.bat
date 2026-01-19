@echo off
REM Rhee Creatives Linux v1.0 - One-Click Launcher (Windows)
REM Optimzed for Extreme Performance Edition 2026.01.19

echo ==================================================================
echo       RHEE CREATIVES LINUX v1.0 - EXTREME PERFORMANCE LAUNCHER
echo ==================================================================

REM 1. Check for QEMU
where qemu-system-i386 >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] QEMU (qemu-system-i386) is not in your PATH.
    echo Please install QEMU and add it to your environment variables.
    pause
    exit /b
)

REM 2. Unzip Image if needed
if not exist hd_oldlinux.img (
    echo [..] Preparing Hard Disk Image...
    if exist hd_oldlinux.img.zip (
        tar -xf hd_oldlinux.img.zip
        echo [OK] Disk Image Extracted.
    ) else (
        echo [ERROR] hd_oldlinux.img.zip not found!
        pause
        exit /b
    )
) else (
    echo [OK] Disk Image Found.
)

REM 3. Check Kernel Image
if not exist Image (
    echo [ERROR] Kernel 'Image' file not found!
    echo Please build the project first (e.g., using WSL or Docker).
    pause
    exit /b
)

REM 4. Launch
echo [..] Launching Rhee Creatives Linux...
echo ------------------------------------------------------------------
qemu-system-i386 -drive format=raw,file=Image,index=0,if=floppy -boot a -hdb hd_oldlinux.img -m 8 -machine pc-i440fx-2.5
echo ------------------------------------------------------------------
echo [OK] Session Terminated.
pause
