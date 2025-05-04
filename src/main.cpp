#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include "threadpool.h"
#include "flac_to_mp3.h"
#include "ncm_to_mp3.h"

namespace fs = std::filesystem;

void print_usage() {
    std::cout << "Usage:\n"
              << "  converter -i <input_path> [-o <output_dir>] [-t <threads>]\n"
              << "  input_path 可以是单个文件或目录\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    std::string input;
    std::string outdir;
    size_t nthreads = std::thread::hardware_concurrency();

    // 简单命令行解析
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-i" && i+1 < argc)        input = argv[++i];
        else if (a == "-o" && i+1 < argc)   outdir = argv[++i];
        else if (a == "-t" && i+1 < argc)   nthreads = std::stoul(argv[++i]);
        else {
            std::cerr << "Unknown option: " << a << "\n";
            print_usage();
            return 1;
        }
    }

    if (input.empty()) { print_usage(); return 1; }

    fs::path inp(input);
    if (!fs::exists(inp)) {
        std::cerr << "输入路径不存在: " << input << "\n";
        return 1;
    }
    if (outdir.empty()) {
        // 默认输出到当前目录下 "converted"
        outdir = "converted";
    }
    fs::create_directories(outdir);

    // 收集待转换文件
    std::vector<fs::path> tasks;
    if (fs::is_directory(inp)) {
        for (auto& e : fs::directory_iterator(inp)) {
            if (!e.is_regular_file()) continue;
            auto ext = e.path().extension().string();
            if (ext == ".flac" || ext == ".FLAC" || ext == ".ncm" || ext == ".NCM")
                tasks.push_back(e.path());
        }
    } else {
        tasks.push_back(inp);
    }
    if (tasks.empty()) {
        std::cerr << "没有找到可转换的文件。\n";
        return 1;
    }

    // 启动线程池
    ThreadPool pool(nthreads);
    for (auto& inpath : tasks) {
        pool.enqueue([inpath, outdir]{
            auto ext = inpath.extension().string();
            fs::path outp = fs::path(outdir) / inpath.filename();
            outp.replace_extension(".mp3");

            bool ok = false;
            if (ext == ".flac" || ext == ".FLAC")
                ok = convert_flac_to_mp3(inpath.string(), outp.string());
            else if (ext == ".ncm" || ext == ".NCM")
                ok = convert_ncm_to_mp3(inpath.string(), outp.string());

            std::cout << (ok ? "[OK]   " : "[FAIL] ")
                      << inpath.string() << " → " << outp.string() << "\n";
        });
    }

    // ThreadPool 析构时会 join 所有任务
    return 0;
}
