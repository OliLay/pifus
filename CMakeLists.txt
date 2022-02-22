cmake_minimum_required(VERSION 3.7)

project(pifus)

set(API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api)
set(API_INCLUDES ${API_DIR}/include)

set (API_SOURCES 
    ${API_DIR}/src/pifus.c
    ${API_DIR}/src/pifus_socket.c
    ${API_DIR}/src/pifus_shmem.c
    ${API_DIR}/src/pifus_ring_buffer.c
    ${API_DIR}/src/pifus_priority_aware_ring_buffer.c
    ${API_DIR}/src/pifus_byte_buffer.c
    ${API_DIR}/src/pifus_operation.c
    ${API_DIR}/src/pifus_identifiers.c
    ${API_DIR}/src/pifus_ip.c
    ${API_DIR}/src/pifus_qos.c
    ${API_DIR}/src/utils/futex.c
    ${API_DIR}/src/utils/log.c
    )

add_subdirectory(lwip)  # the lwIP stack (modified)
add_subdirectory(api) # the client side API