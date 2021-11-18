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

class ThreadSync1toN {
private:
    unsigned int workerCount;
    ThreadSyncState state;
    unsigned int runningThreads;

    std::mutex context_switch_mutex;
    std::condition_variable switch_to_state_cv[3];

public:
    explicit ThreadSync1toN(unsigned int workerCount);

    void mainThreadWait();
    void mainThreadDone();
    void workerThreadWait();
    void workerThreadDone();

private:
    void waitForState(ThreadSyncState expectedState, std::unique_lock<std::mutex>& lock);
    void switchToState(ThreadSyncState newState);
};
