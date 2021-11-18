#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

enum ThreadSyncState {
    Main = 0,
    WorkersStarting = 1,
    WorkersEnding = 2,
};

//template <typename T1, typename T2>
class ThreadSync1toN {
private:
    unsigned int workerCount;
    ThreadSyncState state;
    unsigned int runningThreads;

    std::mutex context_switch_mutex;
    std::condition_variable switch_to_main_cv;
    std::condition_variable switch_to_workers_cv;
    std::condition_variable switch_to_closing_cv;
public:
    explicit ThreadSync1toN(unsigned int workerCount);

    void mainThreadDone();
    void mainThreadWait();
    void workerThreadDone();
    void workerThreadWait();
};
