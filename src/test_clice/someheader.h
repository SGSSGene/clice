// SPDX-FileCopyrightText: 2025 Simon Gene Gottlieb
// SPDX-License-Identifier: CC0-1.0
#pragma once
#include <clice/clice.h>

inline auto cliToast = clice::Argument {
    .args  = {"--toast"},
    .desc  = "prints a message when given",
    .value = std::string{},
    .cb    = [](auto& value) {
        std::cout << value << "\n";
    }
};
