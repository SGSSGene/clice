# SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 3.25)

project(clice-demo)

add_executable(${PROJECT_NAME}
    main.cpp
)
target_link_libraries(${PROJECT_NAME}
    clice::clice
    fmt::fmt
)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
