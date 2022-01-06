add_library(pifus STATIC ${CMAKE_CURRENT_LIST_DIR}/src/pifus.c)

target_include_directories(pifus PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pifus rt)

#set(PIFUS_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include)

add_subdirectory(benchmarks)