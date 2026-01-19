# Rhee Creatives Linux v1.0 - Extreme Performance Edition
> **Pure. Potent. Permanent.** - The highly optimized, modded version of original Linux 0.01.

![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)
![Version](https://img.shields.io/badge/Version-v1.0%20Exreme-red.svg)
![Build](https://img.shields.io/badge/Build-Optimized-brightgreen.svg)

## 🌟 Introduction
**Rhee Creatives Linux v1.0** is a heavily modified ("Ma-Gae-Jo") version of the historic Linux 0.01 kernel. 
We have engineered this kernel to push the limits of retro-computing performance, featuring real-time benchmarking, enhanced system reporting, and a dramatic boot sequence that screams power.

## 🚀 Key Features (Extreme Edition)
- **Hyper-Fast Boot Sequence**: Optimized initialization routines.
- **Real-Time Benchmarking**: Integrated CPU Integer Benchmark runs on every boot.
- **Enhanced Diagnostics**: KB-level memory reporting and efficiency tracking (100% No Leaks).
- **Visual Overhaul**: Custom system banners and "Panic" screens for extreme feedback.
- **DMA Mode Simulated**: High-speed driver status reporting.

## 🛠️ One-Click Run (Easy Start)

We provide one-click scripts to get you running in seconds. You need **QEMU** installed.

### Linux / macOS
```bash
./run_linux.sh
```
*(Make sure to give it execution permission: `chmod +x run_linux.sh`)*

### Windows
Double-click `run_windows.bat`.

---

## 🏗️ Building from Source

If you want to compile the kernel yourself, we recommend using Docker to ensure a consistent environment (GCC 4.x toolchain).

```sh
# 1. Build the compiler image
docker build -t linux-0.01-build .

# 2. Compile the kernel (creates 'Image' file)
docker run --rm -v $(pwd):/linux-0.01 linux-0.01-build make
```

## 📜 License
This project is licensed under the **Apache License, Version 2.0**.
See [LICENSE](LICENSE) file for details.

Copyright 2026 **Rheehose (Rhee Creatives)**.

---
*Original Linux 0.01 by Linus Torvalds.*
*Modern GCC Port by isoux & mapopa.*
*Extreme Edition Mod by Rhee Creatives.*
