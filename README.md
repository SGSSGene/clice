<!-- SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de> -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# clice
A CLI argument parser for c++20. It is the spiritual successor of sargparse. It shares many of the goals
and design principles.

## The clice::Argument object
```c++
auto mycomplete() -> std::vector<std::string>;
void onAvailable();

auto cliOpt = clice::Argument{ .parent      = &otherOption,          // A pointer to a parent object, if given this parameter is only being evaluated if the parent option was set before
                               .args        = {"-o", "--option"},    // A list of arguments, on how this option can be triggered on the command line
                               .env         = {"MY_ENV"},            // A list of environment variables, that are used (if values are given via command line and CLI, the CLI argument overwrites the env)
                               .id          = "file_nbr",            // For help age (and similar), a name for the value given
                               .desc        = "some thing required", // Description, used for help page (and similar)
                               .value       = size_t{0},             // Value type and default value
                               .completion  = my_complete,           // A function that returns possible values for completion
                               .cb          = onAvailable,           // This function is being triggered at the end of the parsing step, if the value of this option was given
                               .cb_priority = 100,                   // to order multiple arguments, lower value is being run before the others (default 100)
                               .mapping     = std::nullopt,          // A mapping from strings to values, no mapping used if not given (or std::nullopt)
                                                                     // e.g. (for a string to int mapping:
                                                                     // std::unordered_map<std::string, int> {
                                                                     //     {"mode1", 0},
                                                                     //     {"mode2", 1}
                                                                     // };
                               .tags       = {"sometag"},            // List of tags, known tags: "required" (value must be given), "ignore-required" (callback is run even if not all required values are present)

};

auto mycomplete() -> std::vector<std::string> {
    return {"file1", "file2"};
}

void onAvailable() {
    std::cout << "The option -o, --option was set on the command line\n";
}
```

## Examples


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

Or extreme reduced main, including, parsing, help generation and exception catching
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
