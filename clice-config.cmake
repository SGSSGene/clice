cmake_minimum_required (VERSION 3.12)
if (TARGET ivio::ivio)
    return()
endif()

add_library(clice INTERFACE)
add_library(clice::clice ALIAS clice)
target_include_directories(clice INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src)
