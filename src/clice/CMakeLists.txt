# SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 3.25)

add_library(clice INTERFACE)
add_library(clice::clice ALIAS clice)
target_compile_features(clice INTERFACE cxx_std_20)


option (CLICE_USE_TDL "Enables tool_description_lib(TDL) to be supported by CLICE (enables CWL features)" ${PROJECT_IS_TOP_LEVEL})
target_link_libraries(clice INTERFACE fmt::fmt)


if (CLICE_USE_TDL)
    target_link_libraries(clice INTERFACE tdl::tdl fmt::fmt)
    target_compile_definitions(clice INTERFACE CLICE_USE_TDL)
endif ()
target_include_directories(clice
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
    $<INSTALL_INTERFACE:include>
)
include(GNUInstallDirs)
install (TARGETS clice)
install (DIRECTORY src/clice TYPE INCLUDE FILES_MATCHING PATTERN "*.h")
install (FILES clice-config.cmake DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/)
