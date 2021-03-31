cmake_minimum_required(VERSION 3.17)
project(btm)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(btm util.cpp main.cpp btm.cpp CountVectorizer.cpp SortedLimitedList.inl SortedLimitedList.h)