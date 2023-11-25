# SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 3.8)

project(clice-demo)

# fmindex-collectionunittests
add_executable(${PROJECT_NAME}
    main.cpp
)
target_link_libraries(${PROJECT_NAME}
    clice::clice
    fmt::fmt
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
