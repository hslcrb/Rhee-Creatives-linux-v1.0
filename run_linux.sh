#!/bin/bash

# Rhee Creatives Linux v1.0 - One-Click Launcher (Linux/macOS)
# Rhee Creatives Linux v1.0 - 원클릭 런처 (Linux/macOS)
# Optimized for Extreme Performance Edition 2026.01.19
# 익스트림 퍼포먼스 에디션 2026.01.19에 최적화됨

echo "=================================================================="
echo "      RHEE CREATIVES LINUX v1.0 - EXTREME PERFORMANCE LAUNCHER"
echo "      RHEE CREATIVES LINUX v1.0 - 익스트림 퍼포먼스 런처"
echo "=================================================================="

# 1. Check if QEMU is installed
# 1. QEMU 설치 확인
if ! command -v qemu-system-i386 &> /dev/null; then
    echo " [ERROR] QEMU is not installed. Please install qemu-system-i386."
    echo " [오류] QEMU가 설치되지 않았습니다. qemu-system-i386을 설치해 주세요."
    exit 1
fi

# 2. Check and unzip disk image if needed
# 2. 필요한 경우 디스크 이미지 확인 및 압축 해제
if [ ! -f "hd_oldlinux.img" ]; then
    echo " [..] Unzipping Hard Disk Image..."
    echo " [..] 하드 디스크 이미지 압축 해제 중..."
    if [ -f "hd_oldlinux.img.zip" ]; then
        unzip -o hd_oldlinux.img.zip
        echo " [OK] Disk Image Prepared."
        echo " [확인] 디스크 이미지 준비 완료."
    else
        echo " [ERROR] hd_oldlinux.img.zip not found!"
        echo " [오류] hd_oldlinux.img.zip을 찾을 수 없습니다!"
        exit 1
    fi
else
    echo " [OK] Disk Image Found."
    echo " [확인] 디스크 이미지 발견."
fi

# 3. Check Kernel Image
# 3. 커널 이미지 확인
if [ ! -f "Image" ]; then
    echo " [..] Kernel Image not found. Attempting build with Docker..."
    echo " [..] 커널 이미지를 찾을 수 없습니다. Docker로 빌드를 시도합니다..."
    if command -v docker &> /dev/null; then
        docker build -t linux-0.01-build .
        docker run --rm -v $(pwd):/linux-0.01 linux-0.01-build make
        
        # Permission fix for Docker created files
        # Docker 생성 파일 권한 수정
        if [ -f "Image" ]; then
             # Try to fix permissions if we can (might fail without sudo, but worth a try)
             # 가능하다면 권한 수정을 시도합니다 (sudo 없이 실패할 수 있지만 시도해 볼 가치는 있습니다)
             chmod 644 Image
             echo " [OK] Build Successful."
             echo " [확인] 빌드 성공."
        else
             echo " [ERROR] Build Failed."
             echo " [오류] 빌드 실패."
             exit 1
        fi
    else
        echo " [ERROR] Kernel Image missing and Docker not available for build."
        echo " [오류] 커널 이미지가 없고 빌드에 사용할 수 있는 Docker가 없습니다."
        exit 1
    fi
fi

# 4. Launch
# 4. 실행
echo " [..] Launching Rhee Creatives Linux..."
echo " [..] Rhee Creatives Linux 실행 중..."
echo "------------------------------------------------------------------"
qemu-system-i386 -drive format=raw,file=Image,index=0,if=floppy -boot a -hdb hd_oldlinux.img -m 8 -machine pc-i440fx-2.5
echo "------------------------------------------------------------------"
echo " [OK] Session Terminated."
echo " [확인] 세션 종료됨."
