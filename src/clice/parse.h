// SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
// SPDX-License-Identifier: ISC
#pragma once

#include "Argument.h"
#include "generateHelp.h"
#include "printCompletion.h"

#include <array>
#include <cassert>
#include <cstdlib>
#include <fmt/format.h>
#include <iostream>
#include <list>
#include <map>
#include <set>
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

void parse(int argc, char const* const* argv, bool allowDashCombi = false, std::vector<std::string_view>* unprocessedParameters = nullptr);;
void parse(std::span<std::string_view> args, bool allowDashCombi = false, std::vector<std::string_view>* unprocessedParameters = nullptr);

// A fixed/static list of all -a...-z -A....-Z and all other variant -\0 -\n -@ etc.
// This can be used to return a single dash variable;
// Returns a view onto a static "-c" string. The view stays valid for the
// lifetime of the program, so it can safely end up in unprocessedParameters.
inline auto singleDashArg(char c) -> std::string_view {
    static auto const table = [] {
        auto t = std::array<std::array<char, 2>, 256>{};
        for (size_t i{0}; i < t.size(); ++i) {
            t[i] = {'-', static_cast<char>(i)};
        }
        return t;
    }();
    return {table[static_cast<int>(c)].data(), 2};
}

inline void parseSingleDash(std::span<std::string_view> _args, std::vector<std::string_view>* unprocessedParameters) {
    auto argview = std::vector<std::string_view>{};
    bool allTrailing{false};
    for (size_t i{0}; i < _args.size(); ++i) {
        auto view = _args[i];
        if (allTrailing
            || view.size() <= 2
            || view[0] != '-'
            || view[1] == '-'
        ) {
            argview.emplace_back(view);
            if (view == "--") {
                allTrailing = true;
            }
        } else {
            // check if the split arguments would exists before splitting:
            auto splitArgs = std::set<std::string>{};
            for (size_t j{1}; j < view.size(); ++j) {
                splitArgs.insert(std::string{"-"} + view[j]);
            }

            // check each argument, if any option could/would take it
            auto foundArgs = std::set<std::string>{};
            auto visitAllArguments = std::function<void(std::vector<ArgumentBase*> const&)>{};
            visitAllArguments = [&](auto const& args) {
                for (auto arg : args) {
                    for (auto const& e : arg->args) {
                        if (splitArgs.contains(e)) {
                            foundArgs.insert(e);
                        }
                    }
                    visitAllArguments(arg->children);
                }
            };
            visitAllArguments(Register::getInstance().arguments);

            // Only split if all arguments have been found, otherwise pass the
            // argument through unchanged (e.g. negative numbers like "-15",
            // single dash options like "-option" or unknown arguments that
            // should be reported by parse())
            if (foundArgs.size() == splitArgs.size()) {
                for (size_t j{1}; j < view.size(); ++j) {
                    argview.emplace_back(singleDashArg(view[j]));
                }
            } else {
                argview.emplace_back(view);
            }
        }
    }
    parse(argview, false, unprocessedParameters);
}

inline void parseSingleDash(int _argc, char const* const* _argv, std::vector<std::string_view>* unprocessedParameters) {
    auto args = std::vector<std::string_view>{};
    for (int i{0}; i < _argc; ++i) {
        args.emplace_back(_argv[i]);
    }
    parseSingleDash(args, unprocessedParameters);
}


// a flag is an argument without a value type (it never accepts values)
// which is indicated by its value type being set to std::nullptr_t
inline auto isFlag(ArgumentBase const& arg) -> bool {
    return arg.type_index == std::type_index(typeid(std::nullptr_t));
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

inline void parse(std::span<std::string_view> args, bool allowDashCombi, std::vector<std::string_view>* unprocessedParameters) {
    if (allowDashCombi) {
        parseSingleDash(args, unprocessedParameters);
        return;
    }
    assert(args.size() > 0);

    clice::argv0 = args[0];;

    // check for symlink (only root arguments are considered)
    auto redirectedArguments = std::vector<std::string_view>{};
    for (auto arg : Register::getInstance().arguments) {
        if (arg->symlink and std::filesystem::path{argv0}.filename().string().ends_with("-" + arg->args[0])) {
            redirectedArguments.push_back(args[0]);
            redirectedArguments.push_back(arg->args[0].c_str());
            for (size_t i{1}; i < args.size(); ++i) {
                redirectedArguments.push_back(args[i]);
            }
            args = redirectedArguments;
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
                visitAllArguments(arg->children, cb);
            }
        };
        visitAllArguments(Register::getInstance().arguments, [&](ArgumentBase& arg, std::string env) {
            auto ptr = std::getenv(env.c_str());
            if (!ptr) return;
            auto value = std::string_view{ptr};
            if (isFlag(arg)) {
                // flags take no value: an empty or true-like value sets the flag,
                // a false-like value ("0", "false", "no") leaves it unset
                auto isTrue = value.empty();
                if (!isTrue) {
                    try {
                        isTrue = parseFromString<bool>(value);
                    } catch (std::exception const& e) {
                        throw std::runtime_error{"environment variable " + env + ": " + e.what()};
                    }
                }
                if (isTrue) arg.init();
                return;
            }
            arg.init();
            arg.fromString(value);
        });
    }


    // parse args (argc/argv)
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
    for (size_t i{1}; i < args.size(); ++i) {
        // make suggestion about next possible tokens
        if (completion and i+1 == args.size()) {
            makeCompletionSuggestion(activeBases, std::string_view{args[i]});
            continue;
        }
        // A marking "--" indicates that all args[i] from now on are interpreted as values
        if (args[i] == "--" and !allTrailing) {
            allTrailing = true;
            continue;
        }

        [&]() {
            // walk up the arguments, until one active argument has a child with fitting parameter
            for (size_t j{0}; j < activeBases.size(); ++j) {
                auto const& base = activeBases[activeBases.size()-j-1];
                if (((!args[i].starts_with("-") or allTrailing or !base->tags.contains("multi")) and base->fromString) and (!base->tags.contains("multi") || base->args.size()>0 || allTrailing)) {
                    base->fromString(args[i]);
                    return;
                }
                if (auto arg = findActiveArg(args[i], base); arg) {
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
            auto arg = findRootArg(args[i]);
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
                    if (arg->args.empty() && !isFlag(*arg) && arg->init) {
                        arg->init();
                        if (!arg->tags.contains("multi")) arg->init = nullptr;
                        activeBases.push_back(arg);
                        arg->fromString(args[i]);
                        return;
                    }
                }
            }

            // second check root arguments
            for (auto arg : Register::getInstance().arguments) {
                if (arg->args.empty() && !isFlag(*arg)) {
                    if (arg->init) {
                        arg->init();
                        if (!arg->tags.contains("multi")) arg->init = nullptr;

                        activeBases.push_back(arg);
                        arg->fromString(args[i]);
                        return;
                    }
                }
            }

            // give it to the furthest up activeBase that has multi values
            if (activeBases.size() > 0) {
                auto const& base = activeBases[activeBases.size()-1];
                if (base->tags.contains("multi") && base->fromString
                    && (!base->ignorePrefix || !args[i].starts_with(*base->ignorePrefix))) {
                    base->fromString(args[i]);
                    return;
                }
            }

            if (unprocessedParameters) {
                for (;i < args.size(); ++i) {
                    unprocessedParameters->push_back(args[i]);
                }
            } else {
                throw std::runtime_error{std::string{"unexpected cli argument \""} + std::string{args[i]} + "\""};
            }
        }();
    }
    if (completion) {
        exit(0);
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
}

struct Parse {
    std::tuple<int, char const* const*> args;
    std::string desc;            // description of the tool
    bool allowDashCombi{false};  // allows to combine "-a -b" into "-ab"
    bool helpOpt{false};         // automatically registers --help option
    bool catchExceptions{false}; // catches exception and prints them
    std::function<void()> run{}; // function to run
    std::vector<std::string_view>* unprocessedParameters{};
};
inline void parse(int argc, char const* const* argv, bool allowDashCombi, std::vector<std::string_view>* unprocessedParameters) {
    auto args = std::vector<std::string_view>{};
    for (int i{0}; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    parse(args, allowDashCombi, unprocessedParameters);
}


inline void parse(Parse const& parse) {
    auto f = [&]() {
        auto [argc, argv] = parse.args;
        clice::parse(argc, argv, parse.allowDashCombi, parse.unprocessedParameters);

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
                                           .cb_priority = 5,
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
