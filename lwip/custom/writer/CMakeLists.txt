include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)

set (LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src/include"
    "${LWIP_DIR}/contrib/"
    "${LWIP_DIR}/contrib/ports/unix/port/include"
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

include(${LWIP_DIR}/src/Filelists.cmake)
#include(${LWIP_DIR}/contrib/Filelists.cmake) TODO: check if needed
include(${LWIP_DIR}/contrib/ports/unix/Filelists.cmake)

add_executable(writer ${CMAKE_CURRENT_LIST_DIR}/src/writer.c ${CMAKE_CURRENT_LIST_DIR}/src/ping.c ${NEEDED_SOURCES})
target_include_directories(writer PRIVATE ${LWIP_INCLUDE_DIRS})
target_compile_options(writer PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(writer PRIVATE ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_link_libraries(writer ${LWIP_SANITIZER_LIBS} lwipcontribportunix lwipcore lwipmbedtls)
