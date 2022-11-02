cmake_minimum_required(VERSION 3.10)
project(mobileum_ex_6 C)

set(CMAKE_C_STANDARD 99)

add_executable(sample src/sample.c)
add_executable(test_dlist src_test/test_DList.c)
add_executable(test_buddy_allocator src_test/test_BuddyAllocator.c)
