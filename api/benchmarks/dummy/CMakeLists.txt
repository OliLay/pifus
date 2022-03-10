add_executable(pifus_dummy_async ${CMAKE_CURRENT_LIST_DIR}/src/dummy_async.c)
target_link_libraries(pifus_dummy_async pifus pthread)

add_executable(pifus_dummy_single ${CMAKE_CURRENT_LIST_DIR}/src/dummy_single.c)
target_link_libraries(pifus_dummy_single pifus pthread)

add_executable(pifus_dummy_sync ${CMAKE_CURRENT_LIST_DIR}/src/dummy_sync.c)
target_link_libraries(pifus_dummy_sync pifus pthread)
