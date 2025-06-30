<!-- SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de> -->
<!-- SPDX-License-Identifier: CC0-1.0 -->
# clice
A Command line argument parser for C++20.

When writing a program in C++, command line arguments (CLI) are passed via `argc`/`argv` variables.
These variables must be parsed and converted to provide an easy interface for a program.
Writing a consistent CLI becomes a lot more challenging, the more arguments a program takes.

Clice tries to solve this problem and supports the user to create help pages, provide tab-completion and [CWL](https://www.commonwl.org/) tool-description files.

## Features
- No enforced argument types: `--flag`, `-a`, `--a`, `-option`, `/help`, `add`, `+no-opt`
- Supports flags, single-value options, multi-value options, dependent options, commands and subcommands
- Global option registration
- Parsable tree for argument inspection (could be used to generate a help page or in the future a man page.)

Clice is the spiritual successor of [sargparse](https://github.com/gottliebtfreitag/sargparse) and [CommonOptions](https://github.com/SGSSGene/CommonOptions).
It shares many of the goals and design principles.

## Core principle
At its core is the `clice::Argument` class, which is recommended to be used to create global objects.
Creating these objects should happen using `designated initializers`, making them very readable.
The interface of these objects is inspired by `std::unique_ptr` or `std::optional`.

A simple example of accepting a `-v` or `--verbose` flag:

```c++
auto cliLogLevel = clice::Argument { .args = {"-l", "--log_level"},
                                     .desc = "Logging level. 0: none, 1: errors, 2: all"
                                     .value = size_t{1}
};
```

The object `cliLogLevel` now can be used via `if (cliLogLevel)` to check if this option was set.
To access the value that was set use `size_t logLevel = *cliLogLevel`.
Slightly different from `std::unique_ptr` and `std::optional`, is that dereferencing is always possible.
It will return the value given to `.value` on initialization.

For more details on `clice::Argument` options, see the reference further down.


## The clice::Argument object
```c++
auto mycomplete() -> std::vector<std::string>;
void onAvailable();

auto cliOpt = clice::Argument{ .parent      = &otherOption,          // A pointer to a parent object, if given this parameter is only being evaluated if the parent option was set before
                               .args        = {"-o", "--option"},    // A list of arguments, on how this option can be triggered on the command line
                               .env         = {"MY_ENV"},            // A list of environment variables, that are used (if values are given via command line and CLI, the CLI argument overwrites the env)
                               .id          = "file_nbr",            // For help page (and similar), a name for the value given
                               .desc        = "some thing required", // Description, used for help page (and similar)

                               .value       = size_t{0},             // Value type and default value
                               .suffix      = "s",                   // Default: std::nullopt, enforces some suffix to be attached to this value
                               .completion  = my_complete,           // A function that returns possible values for completion
                               .cb          = onAvailable,           // This function is being triggered at the end of the parsing step, if the value of this option was given
                               .cb_priority = 100,                   // to order multiple arguments, lower value is being run before the others (default 100)
                               .mapping     = std::nullopt,          // A mapping from strings to values, no mapping used if not given (or std::nullopt)
                                                                     // e.g. (for a string to int mapping)
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
}
```

## Bash/Zsh completion
Just run `eval "$(CLICE_GENERATE_COMPLETION=$$ ./clice-demo)"` and enjoy
tab-completion when running `./clice-demo` programs.

## Other projects

There are many other C++ CLI parsers out there. Maybe you should also write your own?

- [argagg](https://github.com/vietjtnguyen/argagg)
- [Argengine](https://github.com/juzzlin/Argengine)
- [argh!](https://github.com/adishavit/argh)
- [argparse](https://github.com/p-ranav/argparse)
- [argparse-cpp](https://github.com/magiruuvelvet/argparse-cpp)
- [args (by pfultz2)](https://github.com/pfultz2/args)
- [args (by taywee)](https://github.com/taywee/args)
- [args-parser](https://github.com/igormironchik/args-parser)
- [az-cli](https://github.com/sergeniously/az-cli) (💤>3 years)
- [ccli](https://github.com/fknfilewalker/ccli)
- [clippson](https://github.com/heavywatal/clippson)
- [clips](https://github.com/9km-cn/clips )(💤5 years)
- [CLI11](https://github.com/CLIUtils/CLI11)
- [CLI](https://www.codesynthesis.com/projects/cli/)
- [cli.cpp](https://github.com/KoltesDigital/cli.cpp) (💤>9 years)
- [clipp](https://github.com/muellan/clipp) (💤>6 years)
- [clicky](https://github.com/auth-xyz/clicky)
- [cmdlime](https://github.com/kamchatka-volcano/cmdlime) (configure outside)
- [cmdr-cxx](https://github.com/hedzr/cmdr-cxx)
- [cpp_cli](https://github.com/TheLandfill/cpp_cli)
- [cpp-commander](https://github.com/yitsushi/cpp-commander)
- [cxxopts](https://github.com/jarro2783/cxxopts)
- [dimcli](https://github.com/gknowles/dimcli)
- [docopt.cpp](https://github.com/docopt/docopt.cpp) (💤>3 years)
- [gethin](https://github.com/MattiasLiljeson/gethin) (💤>6 years)
- [gflags](https://gflags.github.io/gflags/)
- [Lyra](https://github.com/bfgroup/Lyra)
- [option-parser](https://github.com/lukedeo/option-parser) (💤>6 years)
- [opzioni](https://github.com/ggabriel96/opzioni)
- [popl](https://github.com/badaix/popl) (💤>4 years)
- [Program Options (boost)](https://github.com/boostorg/program_options)
- [QCommandLineParser](https://doc.qt.io/archives/qt-5.15/qcommandlineparser.html)
- [RaptorCLI](https://github.com/smegg99/RaptorCLI)
- [Sharg](https://github.com/seqan/sharg-parser)
- [TCLAP](https://tclap.sourceforge.net/)
- [TheLean Mean C++ Option Parser](https://sourceforge.net/projects/optionparser/) (💤>8 years)
- [wahl](https://github.com/dominiklohmann/wahl) (💤>4 years)

💤: projects have no update for more than a year (Status 30. June 2025)
