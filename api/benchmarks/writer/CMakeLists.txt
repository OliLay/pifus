add_executable(pifus_writer ${CMAKE_CURRENT_LIST_DIR}/src/writer.c)
target_link_libraries(pifus_writer pifus pthread)
