include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)

set (LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src/include"
    "${LWIP_DIR}/contrib/"
    "${LWIP_DIR}/contrib/ports/unix/port/include"
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

include(${LWIP_DIR}/src/Filelists.cmake)
include(${LWIP_DIR}/contrib/ports/unix/Filelists.cmake)

add_executable(stack 
                ${CMAKE_CURRENT_LIST_DIR}/src/stack.c 
                ${CMAKE_CURRENT_LIST_DIR}/src/discovery.c 
                ${CMAKE_CURRENT_LIST_DIR}/src/tcp/bind.c 
                ${CMAKE_CURRENT_LIST_DIR}/src/tcp/connect.c 
                ${CMAKE_CURRENT_LIST_DIR}/src/tcp/write.c 
                ${CMAKE_CURRENT_LIST_DIR}/src/tcp/recv.c 
                ${CMAKE_CURRENT_LIST_DIR}/src/tcp/listen.c
                ${CMAKE_CURRENT_LIST_DIR}/src/tcp/accept.c
                ${CMAKE_CURRENT_LIST_DIR}/src/tcp/close.c
                ${CMAKE_CURRENT_LIST_DIR}/src/prio_thread.c
                ${CMAKE_CURRENT_LIST_DIR}/src/list/linked_list.c
                ${NEEDED_SOURCES})

target_include_directories(stack PRIVATE ${LWIP_INCLUDE_DIRS})
target_compile_options(stack PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(stack PRIVATE ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_link_libraries(stack ${LWIP_SANITIZER_LIBS} lwipcontribportunix lwipcore lwipmbedtls)
