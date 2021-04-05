#pragma once

#include <mutex>
#include <condition_variable>

class ThreadsafeCounter {
private:
    unsigned int counter = 0;
    std::mutex counter_mutex;
    std::condition_variable counter_reset_cv;
public:
    unsigned int get();
    unsigned int incrementAndGet();
    unsigned int getAndIncrement();
    void increment();
    void reset();
    void waitUntilReset();
};
