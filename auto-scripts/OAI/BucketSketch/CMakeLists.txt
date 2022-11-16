cmake_minimum_required(VERSION 3.10)
project(elastic)


set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finput-charset=GBK -fexec-charset=UTF-8 -std=c++14 -msse2 -mssse3 -msse4.1 -msse4.2 -mavx -march=native -fno-asynchronous-unwind-tables")
#include_directories(./src/CPU/common/)
#include_directories(./src/CPU/elastic/)
#add_executable(BOBHash src/CPU/common/BOBHash32.cpp)
file(GLOB SOURCES
        src/CPU/common/BOBHash32.h						//*号对应具体的文件名
        src/CPU/common/BOBHash32.cpp
        )
add_executable(main src/CPU/main.cpp ${SOURCE_FILES} ${SOURCES})


