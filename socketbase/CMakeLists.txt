cmake_minimum_required(VERSION 2.8)

project(rpcbase)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=gnu++14 -m64")

set(TARGET_NAME "rpcbase")

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include/
    ${CMAKE_CURRENT_LIST_DIR}/src/
    )

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src/ SRC_FILES)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/test SRC_FILES)


add_executable(test ${SRC_FILES})
target_link_libraries(test pthread)
