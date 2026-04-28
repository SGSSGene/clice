// SPDX-FileCopyrightText: 2026 Simon Gene Gottlieb
// SPDX-License-Identifier: CC0-1.0

#include <clice/clice.h>
#include <generator>

struct Module {
/*    static std::generator<int> config(std::string name) {
        clice::Argument cli {
            .args = std::format("--{}", name),
        };
        clice::Argument args {
            .parent = &cli,
            .args   = "--option",
        };

        co_yield 0;
    }
    std::generator<int> x;*/

    std::string name;
    clice::Argument<> cli;
    clice::Argument<std::string> args {
        .parent = &cli,
        .args   = "--option",
        .value  = std::string{}
    };
    Module(std::string _name)
        : name{_name}
        , cli {
            .args = std::format("--{}", name),
        }
    {
    }
    void print() const {
        fmt::print("module '{}' with option '{}'\n", name, *args);
    }
};

auto cliHelp = clice::Argument {
    .args   = "--help",
    .desc   = "prints the help page",
};

std::list<Module> modules;
int main(int argc, char** argv) {
    auto cliLoadModule = clice::Argument {
        .args   = "--module",
        .desc   = "loads a specific module",
        .value  = std::vector<std::string>{},
        .ignorePrefix = "--",
        .cb     = [](auto const& values) {
            for (auto const& v : values) {
                modules.emplace_back(v);
            }
        },
    };
    auto args = std::vector<std::string_view>{};
    for (int i{0}; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    auto unprocessed = std::vector<std::string_view>{};
    unprocessed.emplace_back(argv[0]);
    do {
        if (auto failed = clice::parse(args, /*.allowDashCombi=*/true, &unprocessed); failed) {
            std::cerr << "parsing failed: " << *failed << "\n";
            return 1;
        }
        std::swap(unprocessed, args);
        unprocessed.resize(1);
    } while (args.size() > 1);

    for (auto const& m : modules) {
        m.print();
    }
    if (cliHelp) {
        std::cout << clice::generateHelp();
    }
}
