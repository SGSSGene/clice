// SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
// SPDX-License-Identifier: ISC
#pragma once

#include "Argument.h"
#include "generateHelp.h"
#include "printCompletion.h"

#include <cassert>
#include <cstdlib>
#include <fmt/format.h>
#include <iostream>
#include <list>
#include <map>
#include <span>

namespace clice {

namespace {
inline void makeCompletionSuggestion(std::vector<ArgumentBase*> const& activeBases, std::string_view arg) {
    // single completion
    if (activeBases.size() and activeBases.back()->fromString and activeBases.back()->completion and (arg.empty() || arg[0] != '-')) {
        fmt::print("{}", *activeBases.back()->completion);
        return;
    }
    if (activeBases.size() and activeBases.back()->fromString and activeBases.back()->completion_fn and (arg.empty() || arg[0] != '-')) {
        for (auto c : activeBases.back()->completion_fn()) {
            fmt::print("{}\n", c);
        }
        return;
    }

    auto options = std::vector<std::tuple<std::string, std::string>>{};
    for (auto bases : activeBases) {
        for (auto arg : bases->children) {
            if (!arg->args.empty()) {
                options.emplace_back(arg->args[0], arg->desc);
            }
        }
    }
    for (auto arg : Register::getInstance().arguments) {
        if (!arg->args.empty()) {
            options.emplace_back(arg->args[0], arg->desc);
        }
    }
    size_t longestWord = {};
    for (auto const& [arg, desc] : options) {
        longestWord = std::max(longestWord, arg.size());
    }
    //!TODO maybe we also want to show descriptions?

    std::ranges::sort(options);
    for (auto const& [arg, desc] : options) {
        fmt::print("{}\n", arg);
    }
}
}

auto parse(int argc, char const* const* argv, bool allowDashCombi = false) -> std::optional<std::string>;

inline auto parseSingleDash(int _argc, char const* const* _argv) -> std::optional<std::string> {
    auto args   = std::list<std::string>{};
    auto argptr = std::vector<char const*>{};
    bool allTrailing{false};
    for (int i{0}; i < _argc; ++i) {
        auto view = std::string_view{_argv[i]};
        if (allTrailing
            || view.size() <= 2
            || view[0] != '-'
            || view[1] == '-'
        ) {
            argptr.push_back(_argv[i]);
            if (view == "--") {
                allTrailing = true;
            }
        } else {
            for (size_t j{1}; j < view.size(); ++j) {
                args.push_back(std::string{"-"} + view[j]);
                argptr.push_back(args.back().c_str());
            }
        }
    }
    return parse(argptr.size(), argptr.data(), false);
}


// creates a string like "-i, --input"
inline auto createParameterStrList(std::vector<std::string> const& args) -> std::string {
    auto param = std::string{};
    for (auto const& a : args) {
        param += a + ", ";
    }
    if (param.size() > 1) {
        param.pop_back();
        param.pop_back();
    }
    return param;
}


/**
 * allowDashCombi: allows flags like "-a -b" be combined to "-ab"
 */
inline auto parse(int argc, char const* const* argv, bool allowDashCombi) -> std::optional<std::string> {
    if (allowDashCombi) {
        return parseSingleDash(argc, argv);
    }

    assert(argc > 0);
    clice::argv0 = argv[0];

    // check for symlink (only root arguments are considered)
    auto redirectedArguments = std::vector<char const*>{};
    for (auto arg : Register::getInstance().arguments) {
        if (arg->symlink and std::filesystem::path{argv0}.filename().string().ends_with("-" + arg->args[0])) {
            redirectedArguments.push_back(argv[0]);
            redirectedArguments.push_back(arg->args[0].c_str());
            for (auto i{1}; i < argc; ++i) {
                redirectedArguments.push_back(argv[i]);
            }
            redirectedArguments.push_back(nullptr);
            argc = redirectedArguments.size()-1;
            argv = redirectedArguments.data();
        }
    }

    if (auto gen = std::getenv("CLICE_GENERATE_COMPLETION"); gen != nullptr) {
        printCompletion(gen);
        exit(0);
    }

    // check environment variables first
    {
        using CB = std::function<void(ArgumentBase&, std::string)>;
        auto visitAllArguments = std::function<void(std::vector<ArgumentBase*> const&, CB const&)>{};
        visitAllArguments = [&](auto const& args, auto const& cb) {
            for (auto arg : args) {
                for (auto const& e : arg->env) {
                    cb(*arg, e);
                }
                for (auto child : arg->children) {
                    visitAllArguments(child->children, cb);
                }
            }
        };
        visitAllArguments(Register::getInstance().arguments, [&](ArgumentBase& arg, std::string env) {
            if (auto ptr = std::getenv(env.c_str()); ptr) {
                arg.init();
                arg.fromString(std::string_view{ptr});
            }
        });
    }


    // parse argc/argv


    auto activeBases = std::vector<ArgumentBase*>{}; // current commands whos sub arguments are of interest;

    auto completion = std::getenv("CLICE_COMPLETION") != nullptr;

    auto findRootArg = [&](std::string_view str) -> ArgumentBase* {
        for (auto arg : Register::getInstance().arguments) {
            auto iter = std::find(arg->args.begin(), arg->args.end(), str);
            if (iter != arg->args.end() and arg->init) {
                return arg;
            }
        }
        return nullptr;
    };

    auto findActiveArg = [&](std::string_view str, ArgumentBase* base) -> ArgumentBase* {
        for (auto arg : base->children) {
            auto iter = std::find(arg->args.begin(), arg->args.end(), str);
            if (iter != arg->args.end() and arg->init) {
                return arg;
            }
        }
        return nullptr;
    };


    bool allTrailing = false;
    for (int i{1}; i < argc; ++i) {
        // make suggestion about next possible tokens
        if (completion and i+1 == argc) {
            makeCompletionSuggestion(activeBases, std::string_view{argv[i]});
            continue;
        }
        if (std::string_view{argv[i]} == "--" and !allTrailing) {
            allTrailing = true;
            continue;
        }

        [&]() {
            // walk up the arguments, until one active argument has a child with fitting parameter
            for (size_t j{0}; j < activeBases.size(); ++j) {
                auto const& base = activeBases[activeBases.size()-j-1];
                if (((argv[i][0] != '-' or allTrailing or !base->tags.contains("multi")) and base->fromString) and (!base->tags.contains("multi") || base->args.size()>0 || allTrailing)) {
                    base->fromString(argv[i]);
                    return;
                }
                if (auto arg = findActiveArg(argv[i], base); arg) {
                    arg->init();
                    activeBases.push_back(arg);
                    return;
                }
                if (!base->tags.contains("multi") && base->fromString) {
                    auto param = std::string{};
                    for (auto const& a : base->args) {
                        param += a + ", ";
                    }
                    param.pop_back(); param.pop_back();
                    throw std::runtime_error{"option \"" + param + "\" is missing a value (1)"};
                }
            }
            auto arg = findRootArg(argv[i]);
            if (arg) {
                arg->init();
                activeBases.push_back(arg);
                return;
            }
            // check if an cli option without arguments exists
            // first walk up active arguments
            for (size_t j{0}; j < activeBases.size(); ++j) {
                auto const& base = activeBases[activeBases.size()-j-1];
                for (auto arg : base->children) {
                    if (arg->args.empty() && arg->init) {
                        arg->init();
                        if (!arg->tags.contains("multi")) arg->init = nullptr;
                        activeBases.push_back(arg);
                        arg->fromString(argv[i]);
                        return;
                    }
                }
            }

            // second check root arguments
            for (auto arg : Register::getInstance().arguments) {
                if (arg->args.empty()) {
                    if (arg->init) {
                        arg->init();
                        if (!arg->tags.contains("multi")) arg->init = nullptr;

                        activeBases.push_back(arg);
                        arg->fromString(argv[i]);
                        return;
                    }
                }
            }

            throw std::runtime_error{std::string{"unexpected cli argument \""} + argv[i] + "\""};
        }();
    }
    if (completion) {
        exit(0);
    }

    // check if all active arguments got parameters
    for (size_t j{0}; j < activeBases.size(); ++j) {
        auto const& base = activeBases[activeBases.size()-j-1];
        if (!base->tags.contains("multi") && base->fromString) {
            auto param = createParameterStrList(base->args);
            throw std::runtime_error{"option " + base->id + "\"" + param + "\" is missing a value (2)"};
        }
        for (auto child : base->children) {
            if (child->tags.contains("required")) {
                if (std::ranges::find(activeBases, child) == activeBases.end()) {
                    auto option = createParameterStrList(base->args);
                    auto suboption = createParameterStrList(child->args);
                    throw std::runtime_error{"option " + child->id + "\"" + suboption + "\" is required (enforced by \"" + option + "\")"};
                }
            }
        }
    }

    // create list of all triggers according to priority
    auto triggers = std::map<size_t, std::vector<std::tuple<clice::ArgumentBase*, std::function<void()>>>>{};
    auto f = std::function<void(std::vector<clice::ArgumentBase*>)>{};
    f = [&](auto const& args) {
        for (auto arg : args) {
            if (arg->cb) {
                triggers[arg->cb_priority].emplace_back(arg, [=]() {
                    arg->cb();
                });
            }
            f(arg->children);
        }
    };
    f(clice::Register::getInstance().arguments);

    // trigger all calls that have a "ignore-required" tag
    {
        bool only_ignore = true;
        for (auto const& [level, cbs] : triggers) {
            for (auto const& [arg, cb] : cbs) {
                if (!only_ignore && arg->tags.contains("ignore-required")) {
                    throw std::runtime_error{"option " + arg->id + " could not run, since a higher priority option is missing a \"ignore-required\" tag"};
                }
                if (!arg->tags.contains("ignore-required")) {
                    // do not execute any further callback
                    only_ignore = false;
                }
                if (only_ignore) {
                    cb();
                }
            }
        }
    }

    // check if all top level arguments got parameters
    for (auto base : Register::getInstance().arguments) {
        if (base->tags.contains("required")) {
            if (std::ranges::find(activeBases, base) == activeBases.end()) {
                auto option = createParameterStrList(base->args);
                throw std::runtime_error{"option " + base->id + " \"" + option + "\" is a required parameter"};
            }
        }
    }


    // call triggers in priority level order
    for (auto const& [level, cbs] : triggers) {
        for (auto const& [arg, cb] : cbs) {
            if (!arg->tags.contains("ignore-required")) {
                cb();
            }
        }
    }

    return std::nullopt;
}

struct Parse {
    std::tuple<int, char const* const*> args;
    std::string desc;            // description of the tool
    bool allowDashCombi{false};  // allows to combine "-a -b" into "-ab"
    bool helpOpt{false};         // automatically registers --help option
    bool catchExceptions{false}; // catches exception and prints them
    std::function<void()> run;   // function to run
};

inline void parse(Parse const& parse) {
    auto f = [&]() {
        auto [argc, argv] = parse.args;
        if (auto failed = clice::parse(argc, argv, parse.allowDashCombi); failed) {
            std::cerr << "parsing failed: " << *failed << "\n";
            std::exit(1);
        }

        if (parse.run) {
            parse.run();
        }
    };

    auto wrappedWithHelp = [&](auto cb) {
        auto cliHelp    = clice::Argument{ .args   = {"-h", "--help"},
                                           .desc   = "prints the help page",
                                           .cb     = [&](){
                                                if (parse.desc.size()) {
                                                    std::cout << parse.desc << "\n\n";
                                                }
                                                std::cout << generateHelp(); exit(0);
                                            },
                                           .tags   = {"ignore-required"},
        };
        cb();

    };

    if (parse.catchExceptions) {
        try {
            if (parse.helpOpt) {
                wrappedWithHelp(f);
            } else {
                f();
            }
        } catch(std::exception const& e) {
            std::cerr << "error: " << e.what() << "\n";
            std::exit(1);
        } catch(...) {
            std::cerr << "unknown exception was thrown\n";
            std::exit(1);
        }
    } else {
        if (parse.helpOpt) {
            wrappedWithHelp(f);
        } else {
            f();
        }
    }
}
}
