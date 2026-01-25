# Rhee Creatives Linux v1.0 - Extreme Performance Edition
# Rhee Creatives Linux v1.0 - 익스트림 퍼포먼스 에디션

> **Pure. Potent. Permanent.** - The highly optimized, modded version of original Linux 0.01.
> **순수함. 강력함. 영원함.** - 원본 리눅스 0.01의 고도로 최적화되고 개조된 버전입니다.

![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)
![Version](https://img.shields.io/badge/Version-v1.0%20Extreme-red.svg)
![Build](https://img.shields.io/badge/Build-Optimized-brightgreen.svg)
![Author](https://img.shields.io/badge/Author-Rheehose-purple.svg)

## 🌟 Introduction / 소개
**Rhee Creatives Linux v1.0 - Extreme Performance Edition** is a heavily modified ("Ma-Gae-Jo") version of the historic Linux 0.01 kernel.
**Rhee Creatives Linux v1.0 - 익스트림 퍼포먼스 에디션**은 역사적인 리눅스 0.01 커널을 대대적으로 개조("마개조")한 버전입니다.

We have engineered this kernel to push the limits of retro-computing performance, featuring real-time benchmarking, enhanced system reporting, and a dramatic boot sequence that screams power.
우리는 레트로 컴퓨팅 성능의 한계를 뛰어넘기 위해 이 커널을 설계했으며, 실시간 벤치마킹, 향상된 시스템 보고, 그리고 강력함을 외치는 극적인 부팅 시퀀스를 특징으로 합니다.

## 🚀 Key Features (Extreme Edition) / 주요 기능 (익스트림 에디션)
- **Hyper-Fast Boot Sequence**: Optimized initialization routines.
  - **초고속 부팅 시퀀스**: 최적화된 초기화 루틴.
- **Real-Time Benchmarking**: Integrated CPU Integer Benchmark runs on every boot.
  - **실시간 벤치마킹**: 부팅 시마다 실행되는 통합 CPU 정수 벤치마크.
- **Enhanced Diagnostics**: KB-level memory reporting and efficiency tracking (100% No Leaks).
  - **향상된 진단**: KB 수준의 메모리 보고 및 효율성 추적 (100% 누수 없음).
- **Visual Overhaul**: Custom system banners and "Panic" screens for extreme feedback.
  - **비주얼 오버홀**: 극적인 피드백을 위한 커스텀 시스템 배너 및 "패닉" 화면.
- **Spider-Web Architecture**: Robust, defensive coding with system integrity checks.
  - **거미줄 아키텍처**: 시스템 무결성 검사가 포함된 강력하고 방어적인 코딩.

## 🛠️ One-Click Run (Easy Start) / 원클릭 실행 (쉬운 시작)

We provide one-click scripts to get you running in seconds. You need **QEMU** installed.
몇 초 안에 실행할 수 있는 원클릭 스크립트를 제공합니다. **QEMU**가 설치되어 있어야 합니다.

### Linux / macOS
```bash
./run_linux.sh
```
*(Make sure to give it execution permission: `chmod +x run_linux.sh`)*
*(실행 권한을 부여했는지 확인하세요: `chmod +x run_linux.sh`)*

### Windows
Double-click `run_windows.bat`.
`run_windows.bat` 파일을 더블 클릭하세요.

---

## 🏗️ Building from Source / 소스로 빌드하기

If you want to compile the kernel yourself, we recommend using Docker to ensure a consistent environment (GCC 4.x toolchain).
커널을 직접 컴파일하고 싶다면, 일관된 환경(GCC 4.x 툴체인)을 보장하기 위해 Docker 사용을 권장합니다.

```sh
# 1. Build the compiler image / 컴파일러 이미지 빌드
docker build -t linux-0.01-build .

# 2. Compile the kernel (creates 'Image' file) / 커널 컴파일 ('Image' 파일 생성)
docker run --rm -v $(pwd):/linux-0.01 linux-0.01-build make
```

## 📜 License / 라이선스
This project is licensed under the **Apache License, Version 2.0**.
See [LICENSE](LICENSE) file for details.
이 프로젝트는 **Apache License, Version 2.0**에 따라 라이선스가 부여됩니다.
자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

### Credits / 크레딧
- **Original Kernel (v0.01)**: Linus Torvalds (1991)
  - **원본 커널 (v0.01)**: 리누스 토르발즈 (1991)
- **Modern GCC Port**: isoux & mapopa
  - **현대적 GCC 포트**: isoux & mapopa
- **Extreme Performance Edition**: Rheehose (Rhee Creative) 2008-2026
  - **익스트림 퍼포먼스 에디션**: Rheehose (Rhee Creative) 2008-2026
