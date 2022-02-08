add_executable(pifus_reader ${CMAKE_CURRENT_LIST_DIR}/src/reader.c)
target_include_directories(pifus_reader PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(pifus_reader pifus)
