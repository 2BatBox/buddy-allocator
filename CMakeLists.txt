cmake_minimum_required(VERSION 3.10)
project(mobileum_ex_6 C)

set(CMAKE_C_STANDARD 99)

set(OPTIMIZATION_LEVEL "-O0")
set(DEBUG_LEVEL "-g3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPTIMIZATION_LEVEL} ${DEBUG_LEVEL} -Wall")

add_executable(test_dlist src_test/test_DList.c)
add_executable(test_buddy_allocator src_test/test_BuddyAllocator.c)
