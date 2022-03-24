add_executable(pifus_baseline_reader ${CMAKE_CURRENT_LIST_DIR}/src/baseline_reader.c)
target_link_libraries(pifus_baseline_reader pifus pthread)
