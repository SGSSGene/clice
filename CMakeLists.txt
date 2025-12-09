# SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 3.25)

project(clice LANGUAGES CXX
            DESCRIPTION "clice -- command line interface argument parsing")

# check if this is the main project
if (${PROJECT_IS_TOP_LEVEL})
    if (WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /EHsc")
    elseif (APPLE)
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
    endif ()
endif ()

include(cmake/CPM.cmake)
CPMAddPackage("gh:SGSSGene/cpmpack#58e562be2a2a288686c0db3d1b6a869503b6f9eb")
loadCPMPack("${CMAKE_CURRENT_SOURCE_DIR}/cpmpack.json")

if (CLICE_USE_TDL)
    target_link_libraries(clice INTERFACE tdl::tdl)
    target_compile_definitions(clice INTERFACE CLICE_USE_TDL)
endif ()


if (${PROJECT_IS_TOP_LEVEL})
    add_subdirectory(src/test_header "")
endif ()
