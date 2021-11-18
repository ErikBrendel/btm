

#include "ThreadSync1toN.h"

#include <iostream>
#include <cassert>

ThreadSync1toN::ThreadSync1toN(unsigned int workerCount) :
    workerCount{workerCount},
    state{ThreadSyncState::WorkersStarting},
    runningThreads{0} {}

void ThreadSync1toN::mainThreadDone() {
    assert(runningThreads == 0);
    std::scoped_lock<std::mutex> lock(context_switch_mutex);
    state = ThreadSyncState::WorkersStarting;
    switch_to_workers_cv.notify_all();
}

void ThreadSync1toN::mainThreadWait() {
    std::unique_lock<std::mutex> lock(context_switch_mutex);
    while (state != ThreadSyncState::Main) switch_to_main_cv.wait(lock);
}

void ThreadSync1toN::workerThreadDone() {
    std::unique_lock<std::mutex> lock(context_switch_mutex);
    while (state == ThreadSyncState::WorkersStarting) switch_to_closing_cv.wait(lock); // wait til all other threads have started
    runningThreads--;
    if (runningThreads == 0) {
        state = ThreadSyncState::Main; // If I am the last thread to finish working, hand it over to the main thread
        switch_to_main_cv.notify_one();
    }
}

void ThreadSync1toN::workerThreadWait() {
    std::unique_lock<std::mutex> lock(context_switch_mutex);
    while (state != ThreadSyncState::WorkersStarting) switch_to_workers_cv.wait(lock); // wait until all other threads are done
    runningThreads++;
    if (runningThreads == workerCount) {
        state = ThreadSyncState::WorkersEnding; // if I am the last thread to pick up work again, let's start the ending phase
        switch_to_closing_cv.notify_all();
    }
}
