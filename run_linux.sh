#!/bin/bash

# Rhee Creatives Linux v1.0 - One-Click Launcher (Linux/macOS)
# Optimzed for Extreme Performance Edition 2026.01.19

echo "=================================================================="
echo "      RHEE CREATIVES LINUX v1.0 - EXTREME PERFORMANCE LAUNCHER"
echo "=================================================================="

# 1. Check if QEMU is installed
if ! command -v qemu-system-i386 &> /dev/null; then
    echo " [ERROR] QEMU is not installed. Please install qemu-system-i386."
    exit 1
fi

# 2. Check and unzip disk image if needed
if [ ! -f "hd_oldlinux.img" ]; then
    echo " [..] Unzipping Hard Disk Image..."
    if [ -f "hd_oldlinux.img.zip" ]; then
        unzip -o hd_oldlinux.img.zip
        echo " [OK] Disk Image Prepared."
    else
        echo " [ERROR] hd_oldlinux.img.zip not found!"
        exit 1
    fi
else
    echo " [OK] Disk Image Found."
fi

# 3. Check Kernel Image
if [ ! -f "Image" ]; then
    echo " [..] Kernel Image not found. Attempting build with Docker..."
    if command -v docker &> /dev/null; then
        docker build -t linux-0.01-build .
        docker run --rm -v $(pwd):/linux-0.01 linux-0.01-build make
        
        # Permission fix for Docker created files
        if [ -f "Image" ]; then
             # Try to fix permissions if we can (might fail without sudo, but worth a try)
             chmod 644 Image
             echo " [OK] Build Successful."
        else
             echo " [ERROR] Build Failed."
             exit 1
        fi
    else
        echo " [ERROR] Kernel Image missing and Docker not available for build."
        exit 1
    fi
fi

# 4. Launch
echo " [..] Launching Rhee Creatives Linux..."
echo "------------------------------------------------------------------"
qemu-system-i386 -drive format=raw,file=Image,index=0,if=floppy -boot a -hdb hd_oldlinux.img -m 8 -machine pc-i440fx-2.5
echo "------------------------------------------------------------------"
echo " [OK] Session Terminated."
