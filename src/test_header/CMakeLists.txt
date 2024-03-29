# SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
# SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required (VERSION 3.14)

# Check every header compiles on its own.
# Validates that headers have all required includes.

# Path to the library relative from this CMakeLists.txt file
set(library_path "../clice")

set(target_name "test_header")

file(GLOB_RECURSE header_files LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_LIST_DIR}/.. ${library_path}/*.h)
set(files)
foreach (header ${header_files})
    string (REPLACE "/" "__" test_name "${header}")
    set(cpp_file test_header_${target_name}_${test_name}.cpp)
    configure_file(test.cpp.in ${cpp_file})
    set(files ${files} ${cpp_file})
endforeach()
add_library(${target_name} ${files})
target_link_libraries(${target_name} clice::clice)
