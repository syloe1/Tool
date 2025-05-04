# Converter

一款轻量级的命令行工具，用于将 FLAC 与网易云 NCM 音频批量转换为 MP3，支持多线程并行转换。

---

## ✨ 功能特性

- ✅ 支持 FLAC → MP3  
- ✅ 支持 NCM → MP3（纯 C++ 实现解密算法）  
- ✅ 多线程并行转换，效率高  
- ✅ 支持单个文件或目录批量输入  
- ✅ 可自定义输出目录、线程数  

---

## 🗂️ 项目目录结构

```text
converter/
├── .gitignore
├── CMakeLists.txt
├── README.md
└── src/
    ├── flac_to_mp3.cpp / .h
    ├── ncm_to_mp3.cpp / .h
    ├── threadpool.cpp / .h
    └── main.cpp
···text 

##  🛠️ 环境依赖
Ubuntu 24.04（WSL 或原生均可）

GCC / Clang（支持 C++17）

CMake ≥ 3.12

FFmpeg（系统命令行版）

OpenSSL 开发库（libssl-dev）

可选依赖（如使用 API 而非命令行）：
libsndfile1-dev

libmp3lame-dev

🚀 快速开始
1. 克隆项目
bash
复制
编辑
git clone https://github.com/<your-username>/converter.git
cd converter
2. 构建项目
bash
复制
编辑
mkdir build && cd build
cmake ..
make -j$(nproc)
编译完成后，converter 可执行文件将生成在 build/ 目录下。

📦 使用示例
bash
复制
编辑
# 将 ~/music 下所有 .flac/.ncm 文件使用 4 个线程转换为 MP3，输出到 ~/mp3_out
./converter -i ~/music -o ~/mp3_out -t 4
📘 使用说明
text
复制
编辑
Usage:
  converter -i <input_path> [-o <output_dir>] [-t <threads>]

参数说明：
  -i   输入路径，可为单个文件或目录  
  -o   输出目录（默认：./converted）  
  -t   线程数（默认：CPU 核心数）  
