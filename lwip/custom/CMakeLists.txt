cmake_minimum_required(VERSION 3.7)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(NEEDED_SOURCES
   ${CMAKE_CURRENT_LIST_DIR}/default_netif.c
   ${CMAKE_CURRENT_LIST_DIR}/init_nosys.c
)

#add_subdirectory(reader) TODO: add
add_subdirectory(writer)