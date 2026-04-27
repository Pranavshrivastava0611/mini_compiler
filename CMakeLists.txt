cmake_minimum_required(VERSION 3.16)
project(vm_lang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find threading library (needed by httplib)
find_package(Threads REQUIRED)

add_executable(vm_lang backend/main.cpp)
target_include_directories(vm_lang PRIVATE backend vendor)
target_link_libraries(vm_lang Threads::Threads)
