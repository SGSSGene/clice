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

# Bash/Zsh completion
Just run `eval "$(CLICE_GENERATE_COMPLETION=$$ ./clice-demo)"` and enjoy
tab-completion when running `./clice-demo` programs.
