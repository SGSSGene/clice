// SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
// SPDX-License-Identifier: CC0-1.0
#include <clice/clice.h>

namespace {
auto cliVerbose = clice::Argument {
    .args   = {"-v", "--verbose"},
    .desc   = "detailed description of what is happening",
};
auto cliNbr = clice::Argument {
    .args   = {"-n", "--nbr"},
    .id     = "<nbr>",
    .desc   = "setting some nbr",
    .value  = 5,
};
}
int main(int argc, char** argv) {
    clice::parse({
        .args            = {argc, argv},
        .desc            = "demonstration of clice::parse function",
        .allowDashCombi  = true, // default false, -a -b -> -ab
        .helpOpt         = true, // default false, registers a --help option and generates help page
        .catchExceptions = true, // default false, catches exceptions and prints them to the command line and exists with code 1
        .run = [&]() {
            std::cout << "verbose: " << cliVerbose << "\n";
            std::cout << "nbr: " << *cliNbr << "\n";
        }
    });
    return 0;
}
