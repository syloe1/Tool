#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

class ThreadPool {
public:
    explicit ThreadPool(size_t n);
    ~ThreadPool();

    // 提交一个任务（无返回值）
    void enqueue(std::function<void()> task);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex               mtx;
    std::condition_variable  cv;
    bool                     stop = false;
};
