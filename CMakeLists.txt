# SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 3.25)

project(clice LANGUAGES CXX
            DESCRIPTION "clice -- command line interface argument parsing")

include(cmake/CPM.cmake)
CPMAddPackage("gh:SGSSGene/cpm.dependencies@0.0.8")
CPMLoadDependenciesFile("${CMAKE_SOURCE_DIR}/cpm.dependencies")

# check if this is the main project
if (${PROJECT_IS_TOP_LEVEL})
    if (WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /EHsc")
    elseif (APPLE)
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
    endif ()
endif ()

add_subdirectory(src/clice)

if (${PROJECT_IS_TOP_LEVEL})
    enable_testing()

    add_subdirectory(src/test_header)
    add_subdirectory(src/clice-demo)
endif ()
