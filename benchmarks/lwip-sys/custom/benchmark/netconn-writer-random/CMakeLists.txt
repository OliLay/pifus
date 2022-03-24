include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)

set (LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src/include"
    "${LWIP_DIR}/contrib/"
    "${LWIP_DIR}/contrib/ports/unix/port/include"
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

include(${LWIP_DIR}/src/Filelists.cmake)
include(${LWIP_DIR}/contrib/ports/unix/Filelists.cmake)

add_executable(netconn_writer_random ${CMAKE_CURRENT_LIST_DIR}/netconn_writer_random.c ${NEEDED_SOURCES})
target_include_directories(netconn_writer_random PRIVATE ${LWIP_INCLUDE_DIRS})
target_compile_options(netconn_writer_random PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(netconn_writer_random PRIVATE ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_link_libraries(netconn_writer_random ${LWIP_SANITIZER_LIBS} lwipcontribportunix lwipcore lwipmbedtls)
