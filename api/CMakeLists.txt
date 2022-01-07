add_library(pifus STATIC 
    ${CMAKE_CURRENT_LIST_DIR}/src/pifus.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pifus_socket.c
)

target_include_directories(pifus PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pifus rt)

add_subdirectory(benchmarks)