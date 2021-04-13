cmake_minimum_required(VERSION 3.16)
project(btm)

set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

add_executable(btm src/util.cpp src/main.cpp src/Btm.cpp src/CountVectorizer.cpp src/SortedLimitedList.inl src/SortedLimitedList.h src/ParallelBtm.cpp src/ParallelBtm.h src/SyncParallelBtm.cpp src/SyncParallelBtm.h src/ThreadsafeQueue.inl src/ThreadsafeQueue.h src/ThreadsafeCounter.cpp src/ThreadsafeCounter.h src/ThreadSync1toN.cpp src/ThreadSync1toN.h src/argparse.h)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(btm PRIVATE Threads::Threads)
