add_executable(pifus_dummy ${CMAKE_CURRENT_LIST_DIR}/src/dummy.c)
target_link_libraries(pifus_dummy pifus pthread)
