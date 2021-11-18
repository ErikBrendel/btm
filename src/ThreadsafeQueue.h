#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadsafeQueue {
private:
    std::queue<T> container;
    std::mutex queue_access_mutex;
public:
    void add(const T& element);
    void add(T&& element);
    void addAll(const vector<T>& elements);

    template<typename... _Args>
    decltype(auto)
    emplace(_Args&&... args);
    T get();
    [[nodiscard]] bool isEmpty();
    [[nodiscard]] unsigned int size();
};


#include "ThreadsafeQueue.inl"
