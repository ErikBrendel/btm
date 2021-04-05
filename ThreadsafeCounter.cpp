#include "ThreadsafeCounter.h"

unsigned int ThreadsafeCounter::get() {
    std::scoped_lock<std::mutex> lock(counter_mutex);
    return counter;
}

unsigned int ThreadsafeCounter::incrementAndGet() {
    std::scoped_lock<std::mutex> lock(counter_mutex);
    counter++;
    return counter;
}

unsigned int ThreadsafeCounter::getAndIncrement() {
    std::scoped_lock<std::mutex> lock(counter_mutex);
    unsigned int value = counter;
    counter++;
    return value;
}

void ThreadsafeCounter::increment() {
    std::scoped_lock<std::mutex> lock(counter_mutex);
    counter++;
}

void ThreadsafeCounter::reset() {
    {
        std::scoped_lock<std::mutex> lock(counter_mutex);
        counter = 0;
    }
    counter_reset_cv.notify_all();
}

void ThreadsafeCounter::waitUntilReset() {
    std::unique_lock<std::mutex> lock(counter_mutex);
    while (counter != 0) {
        counter_reset_cv.wait(lock);
    }
}
