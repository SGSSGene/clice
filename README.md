<!-- SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de> -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# clice
A CLI argument parser for c++20. It is the spiritual successor of sargparse. It shares many of the goals
and design principles.


```c++
#include <clice/clice.h>
#include <iostream>

auto cliAdd     = clice::Argument{ .args   = {"add"},
                                   .desc   = "adds some stuff",
                                 };
auto cliVerbose = clice::Argument{ .parent = &cliAdd,
                                   .args   = {"--verbose", "-v"},
                                   .desc   = "detailed description of what is happening",
                                  };

auto cliRequired = clice::Argument{ .args   = {"-r", "--required"},
                                    .desc   = "some thing required",
                                    .value  = size_t{0},
                                    .tags   = {"required"}
                                   };

int main(int argc, char** argv) {
    if (auto failed = clice::parse(argc, argv); failed) {
        std::cerr << "parsing failed: " << *failed << "\n";
        return 1;
    }

    std::cout << cliAdd << "\n";
    std::cout << "  " << cliVerbose << "\n";
    std::cout << "required: " << *cliRequired << "\n";
    return 0;
}
```

Or extrem reduced main, including, parsing, help generation and exception catching
```c++
#include <clice/clice.h>

namespace {
auto cliVerbose = clice::Argument{ .args   = {"-v", "--verbose"},
                                   .desc   = "detailed description of what is happening",
};
auto cliNbr     = clice::Argument{ .args   = {"-n", "--nbr"},
                                   .id     = "<nbr>",
                                   .desc   = "setting some nbr",
                                   .value  = 5,
};
}
int main(int argc, char** argv) {
    clice::parse({
        .args = {argc, argv},
        .desc = "Tool that does amazing things",
        .allowDashCombi  = true, // default false, -a -b -> -ab
        .helpOpt         = true, // default false, registers a --help option and generates help page
        .catchExceptions = true, // default false, catches exceptions and prints them to the command line and exists with code 1
        .run = [&]() {
            std::cout << "verbose: " << cliVerbose << "\n";
            std::cout << "nbr: " << *cliNbr << "\n";
        }
    });
    return 0;
}```

# Bash/Zsh completion
Just run `eval "$(CLICE_GENERATE_COMPLETION=$$ ./clice-demo)"` and enjoy
tab-completion when running `./clice-demo` programs.
