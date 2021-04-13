

#include "ThreadSync1toN.h"

#include <iostream>
#include <cassert>

ThreadSync1toN::ThreadSync1toN(unsigned int workerCount) :
    workerCount{workerCount},
    state{ThreadSyncState::WorkersStarting},
    doneThreads{workerCount} {}

void ThreadSync1toN::mainThreadDone() {
    assert(doneThreads == workerCount);
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
    doneThreads++;
    //std::cout << "Done: " << doneThreads << " of " << workerCount << std::endl;
    if (doneThreads == workerCount) {
        state = ThreadSyncState::Main;
        switch_to_main_cv.notify_one();
    }
}

void ThreadSync1toN::workerThreadWait() {
    std::unique_lock<std::mutex> lock(context_switch_mutex);
    while (state != ThreadSyncState::WorkersStarting) switch_to_workers_cv.wait(lock); // wait until all other threads are done
    doneThreads--;
    if (doneThreads == 0) {
        state = ThreadSyncState::WorkersEnding; // if i am the last thread to pick up work again, let's start the ending phase
        switch_to_closing_cv.notify_all();
    }
}
