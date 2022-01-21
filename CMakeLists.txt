cmake_minimum_required(VERSION 3.7)

project(pifus)

set(API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api)
set(API_INCLUDES ${API_DIR}/include)

set (API_SOURCES 
    ${API_DIR}/src/pifus.c
    ${API_DIR}/src/pifus_socket.c
    ${API_DIR}/src/pifus_shmem.c
    ${API_DIR}/src/data_structures/pifus_operation_ring_buffer.c
    ${API_DIR}/src/data_structures/pifus_tx_ring_buffer.c
    ${API_DIR}/src/data_structures/pifus_operation.c
    ${API_DIR}/src/utils/futex.c)

add_subdirectory(lwip)  # the lwIP stack (modified)
add_subdirectory(api) # the client side API