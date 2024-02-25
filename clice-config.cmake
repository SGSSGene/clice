# SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 3.12)
if (TARGET clice::clice)
    return()
endif()

CPMaddPackage("gh:fmtlib/fmt#10.2.1")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/clice ${CMAKE_CURRENT_BINARY_DIR}/clice)
