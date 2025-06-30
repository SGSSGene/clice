# SPDX-FileCopyrightText: 2025 Simon Gene Gottlieb
# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required (VERSION 3.25)

add_executable(test_clice
    main.cpp
    secondtu.cpp
)
target_link_libraries(test_clice PUBLIC
    Catch2::Catch2WithMain
    clice::clice
)
add_test(NAME test_clice COMMAND test_clice)
