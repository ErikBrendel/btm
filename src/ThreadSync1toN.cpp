#include "ThreadSync1toN.h"

#include <cassert>

ThreadSync1toN::ThreadSync1toN(unsigned int workerCount) :
    workerCount{workerCount},
    state{ThreadSyncState::WorkersStarting},
    runningThreads{0} {}

void ThreadSync1toN::mainThreadWait() {
    std::unique_lock<std::mutex> lock(context_switch_mutex);
    waitForState(ThreadSyncState::Main, lock);
    assert(runningThreads == 0);
}

void ThreadSync1toN::mainThreadDone() {
    assert(runningThreads == 0);
    std::scoped_lock<std::mutex> lock(context_switch_mutex);
    switchToState(ThreadSyncState::WorkersStarting);
}

void ThreadSync1toN::workerThreadWait() {
    std::unique_lock<std::mutex> lock(context_switch_mutex);
    waitForState(ThreadSyncState::WorkersStarting, lock);
    runningThreads++;
    if (runningThreads == workerCount) { // if I am the last thread to pick up work again, let's start the ending phase
        switchToState(ThreadSyncState::WorkersEnding);
    }
}

void ThreadSync1toN::workerThreadDone() {
    std::unique_lock<std::mutex> lock(context_switch_mutex);
    waitForState(ThreadSyncState::WorkersEnding, lock); // wait til all other threads have started
    runningThreads--;
    if (runningThreads == 0) { // If I am the last thread to finish working, hand control over to the main thread again
        switchToState(ThreadSyncState::Main);
    }
}

void ThreadSync1toN::waitForState(ThreadSyncState expectedState, std::unique_lock<std::mutex>& lock) {
    while (state != expectedState) {
        switch_to_state_cv[expectedState].wait(lock);
    }
}

void ThreadSync1toN::switchToState(ThreadSyncState newState) {
    state = newState;
    switch_to_state_cv[newState].notify_all();
}
