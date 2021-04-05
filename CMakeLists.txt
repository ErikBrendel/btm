cmake_minimum_required(VERSION 3.16)
project(btm)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(btm util.cpp main.cpp Btm.cpp CountVectorizer.cpp SortedLimitedList.inl SortedLimitedList.h ParallelBtm.cpp ParallelBtm.h SyncParallelBtm.cpp SyncParallelBtm.h ThreadsafeQueue.inl ThreadsafeQueue.h ThreadsafeCounter.cpp ThreadsafeCounter.h ThreadSync1toN.cpp ThreadSync1toN.h argparse.h)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(btm PRIVATE Threads::Threads)
