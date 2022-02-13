add_library(pifus STATIC ${API_SOURCES})

target_include_directories(pifus PUBLIC ${API_INCLUDES})
target_link_libraries(pifus rt)

add_subdirectory(benchmarks)
add_subdirectory(test)