cmake_minimum_required(VERSION 3.7)

project(pifus)

add_subdirectory(lwip)  # the lwIP stack (modified)
add_subdirectory(api) # the client side API