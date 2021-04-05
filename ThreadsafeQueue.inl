#include "ThreadsafeQueue.h"

// efficiently adding elements: https://stackoverflow.com/a/17644402/4354423

template<typename T>
void ThreadsafeQueue<T>::add(const T &element) {
    std::scoped_lock<std::mutex> lock(queue_access_mutex);
    container.push(element);
}

template<typename T>
void ThreadsafeQueue<T>::add(T &&element) {
    std::scoped_lock<std::mutex> lock(queue_access_mutex);
    container.push(std::move(element));
}

template<typename T>
template<typename... Args>
decltype(auto) ThreadsafeQueue<T>::emplace(Args &&... args) {
    std::scoped_lock<std::mutex> lock(queue_access_mutex);
    return container.emplace(std::forward<Args>(args)...);
}

template<typename T>
T ThreadsafeQueue<T>::get() {
    T result = container.front();
    container.pop();
    return result;
}

template<typename T>
bool ThreadsafeQueue<T>::isEmpty() {
    return container.empty();
}

template<typename T>
unsigned int ThreadsafeQueue<T>::size() {
    return container.size();
}
