@echo off
REM Rhee Creatives Linux v1.0 - One-Click Launcher (Windows)
REM Rhee Creatives Linux v1.0 - 원클릭 런처 (Windows)
REM Optimzed for Extreme Performance Edition 2026.01.19
REM 익스트림 퍼포먼스 에디션 2026.01.19에 최적화됨

echo ==================================================================
echo       RHEE CREATIVES LINUX v1.0 - EXTREME PERFORMANCE LAUNCHER
echo       RHEE CREATIVES LINUX v1.0 - 익스트림 퍼포먼스 런처
echo ==================================================================

REM 1. Check for QEMU
REM 1. QEMU 확인
where qemu-system-i386 >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] QEMU (qemu-system-i386) is not in your PATH.
    echo [오류] QEMU (qemu-system-i386)가 PATH에 없습니다.
    echo Please install QEMU and add it to your environment variables.
    echo QEMU를 설치하고 환경 변수에 추가해 주세요.
    pause
    exit /b
)

REM 2. Unzip Image if needed
REM 2. 필요한 경우 이미지 압축 해제
if not exist hd_oldlinux.img (
    echo [..] Preparing Hard Disk Image...
    echo [..] 하드 디스크 이미지 준비 중...
    if exist hd_oldlinux.img.zip (
        tar -xf hd_oldlinux.img.zip
        echo [OK] Disk Image Extracted.
        echo [확인] 디스크 이미지 추출됨.
    ) else (
        echo [ERROR] hd_oldlinux.img.zip not found!
        echo [오류] hd_oldlinux.img.zip을 찾을 수 없습니다!
        pause
        exit /b
    )
) else (
    echo [OK] Disk Image Found.
    echo [확인] 디스크 이미지 발견.
)

REM 3. Check Kernel Image
REM 3. 커널 이미지 확인
if not exist Image (
    echo [ERROR] Kernel 'Image' file not found!
    echo [오류] 커널 'Image' 파일을 찾을 수 없습니다!
    echo Please build the project first (e.g., using WSL or Docker).
    echo 먼저 프로젝트를 빌드해 주세요 (예: WSL 또는 Docker 사용).
    pause
    exit /b
)

REM 4. Launch
REM 4. 실행
echo [..] Launching Rhee Creatives Linux...
echo [..] Rhee Creatives Linux 실행 중...
echo ------------------------------------------------------------------
qemu-system-i386 -drive format=raw,file=Image,index=0,if=floppy -boot a -hdb hd_oldlinux.img -m 8 -machine pc-i440fx-2.5
echo ------------------------------------------------------------------
echo [OK] Session Terminated.
echo [확인] 세션 종료됨.
pause
