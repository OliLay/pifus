cmake_minimum_required(VERSION 3.7)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(NEEDED_SOURCES
   ${CMAKE_CURRENT_LIST_DIR}/default_netif.c
   ${CMAKE_CURRENT_LIST_DIR}/init_nosys.c
   ${LWIP_DIR}/contrib/ports/unix/port/sys_arch.c # somehow needed, else one needs an explicit sys_now() call, else linking fails?!
)

add_subdirectory(benchmark)
