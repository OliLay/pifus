add_executable(pifus_bb ${CMAKE_CURRENT_LIST_DIR}/src/bytebuffer.c)
target_include_directories(pifus_bb PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pifus_bb pifus)
