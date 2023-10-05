# SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 3.12)
if (TARGET clice::clice)
    return()
endif()

add_library(clice INTERFACE)
add_library(clice::clice ALIAS clice)

option (CLICE_USE_TDL "Enables tool_description_lib(TDL) to be supported by CLICE (enables CWL features)" OFF)
if (${ROOT_PROJECT})
    set(CLICE_USE_TDL ON)
endif()

if (CLICE_USE_TDL)
    find_package(tdl REQUIRED PATHS lib/tool_description_lib)
    target_link_libraries(clice INTERFACE tdl::tdl)
    target_compile_definitions(clice INTERFACE CLICE_USE_TDL)
endif ()
target_include_directories(clice
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
    $<INSTALL_INTERFACE:include>
)
include(GNUInstallDirs)
install (TARGETS clice)
install (DIRECTORY src/clice TYPE INCLUDE FILES_MATCHING PATTERN "*.h")
install (FILES clice-config.cmake DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/)
