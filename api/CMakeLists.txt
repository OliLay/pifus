add_library(pifus STATIC 
    ${CMAKE_CURRENT_LIST_DIR}/src/pifus.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pifus_socket.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pifus_shmem.c
    ${CMAKE_CURRENT_LIST_DIR}/src/data_structures/pifus_ring_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/src/data_structures/pifus_operation.c
    ${CMAKE_CURRENT_LIST_DIR}/src/data_structures/ext/ring_buf.c
)

target_include_directories(pifus PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pifus rt)

add_subdirectory(benchmarks)