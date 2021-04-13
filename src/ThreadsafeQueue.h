#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadsafeQueue {
public:
    std::queue<T> container;
    std::mutex queue_access_mutex;
    std::condition_variable queue_access_wake_up;
public:
    void add(const T& element);
    void add(T&& element);

    template<typename... _Args>
    decltype(auto)
    emplace(_Args&&... args);
    T get();
    [[nodiscard]] bool isEmpty();
    [[nodiscard]] unsigned int size();
};


#include "ThreadsafeQueue.inl"
