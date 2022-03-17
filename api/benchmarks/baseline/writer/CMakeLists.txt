add_executable(pifus_baseline_writer ${CMAKE_CURRENT_LIST_DIR}/src/baseline_writer.c)
target_link_libraries(pifus_baseline_writer pifus pthread)
