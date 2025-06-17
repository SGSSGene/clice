// SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
// SPDX-License-Identifier: ISC
#pragma once

#include "Argument.h"

#include <cassert>
#include <fmt/format.h>
#include <fmt/ranges.h>

namespace clice {

inline auto typeToString(ArgumentBase const& arg) -> std::string {
    for (auto t : arg.tags) {
        if (t.starts_with("short: ")) {
            if (arg.tags.contains("multi")) {
                return fmt::format("[{}]...", t.substr(7));
            }
            return t.substr(7);
        }
    }
    if (arg.type_index == std::type_index(typeid(std::nullptr_t))) {
        //!Nothing to do, this is a flag and doesn't take any parameters
        return "";
    } else if (arg.mapping) {
        return fmt::format("[{}]", fmt::join(*arg.mapping, "|"));
    } else if (arg.type_index == std::type_index(typeid(char))) {
        return "CHAR";
    } else if (arg.type_index == std::type_index(typeid(bool))) {
        return "[true|false]";
    } else if (arg.type_index == std::type_index(typeid(int8_t))) {
        return "INT8";
    } else if (arg.type_index == std::type_index(typeid(uint8_t))) {
        return "UINT8";
    } else if (arg.type_index == std::type_index(typeid(int16_t))) {
        return "INT16";
    } else if (arg.type_index == std::type_index(typeid(uint16_t))) {
        return "UINT16";
    } else if (arg.type_index == std::type_index(typeid(int32_t))) {
        return "INT32";
    } else if (arg.type_index == std::type_index(typeid(uint32_t))) {
        return "UINT32";
    } else if (arg.type_index == std::type_index(typeid(int64_t))) {
        return "INT64";
    } else if (arg.type_index == std::type_index(typeid(uint64_t))) {
        return "UINT64";
    } else if (arg.type_index == std::type_index(typeid(float))) {
        return "FLOAT";
    } else if (arg.type_index == std::type_index(typeid(double))) {
        return "DOUBLE";
    } else if (arg.type_index == std::type_index(typeid(std::string))) {
        return "STRING";
    } else if (arg.type_index == std::type_index(typeid(std::filesystem::path))) {
        return "PATH";
    } else if (arg.type_index == std::type_index(typeid(std::vector<bool>))) {
        return "[true|false]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<int8_t>))) {
        return "[INT8]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<uint8_t>))) {
        return "[UINT8]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<int16_t>))) {
        return "[INT16]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<uint16_t>))) {
        return "[UINT16]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<int32_t>))) {
        return "[INT32]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<uint32_t>))) {
        return "[UINT32]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<int64_t>))) {
        return "[INT64]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<uint64_t>))) {
        return "[UINT64]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<float>))) {
        return "[FLOAT]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<double>))) {
        return "[DOUBLE]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<std::string>))) {
        return "[STRING]...";
    } else if (arg.type_index == std::type_index(typeid(std::vector<std::filesystem::path>))) {
        return "[PATH]...";
    }
    return "_unknown_";
}

inline auto generatePartialSynopsis(ArgumentBase const& arg) -> std::string {
    auto ret = fmt::format("{}", fmt::join(arg.args, "|"));


    for (auto child : arg.children) {
        ret += " " + generatePartialSynopsis(*child);
    }
    if (arg.id.empty()) {
        auto typeAsString = typeToString(arg);
        ret += typeAsString.empty()?"":(" " + typeAsString);
    } else {
        ret += " " + arg.id;
    }

    // remove spaaces at the beginning
    while (!ret.empty() && ret.front() == ' ') {
        ret.erase(ret.begin());
    }
    // suround parameter with [...] if it is an optional type
    if (!arg.tags.contains("required")) {
        ret = "[" + ret + "]";
    }
    return  ret;
}
inline auto generateSynopsis() -> std::string {
    auto ret = argv0;
    for (auto const& arg : Register::getInstance().arguments) {
        ret += " " + generatePartialSynopsis(*arg);
    }
    return ret;
}

inline auto generateSplitSynopsis() -> std::string {
    auto ret = std::string{};
    auto bases = std::vector<ArgumentBase const*>{};
    for (auto const& arg : Register::getInstance().arguments) {
        if (!arg->args.empty() and arg->args[0][0] != '-') {
            bases.emplace_back(arg);
        } else {
            ret += " " + generatePartialSynopsis(*arg);
        }
    }
    if (ret.size() > 0) {
        ret = fmt::format("{} {}\n", argv0, ret);
    }
    for (auto arg : bases) {
        auto s = generatePartialSynopsis(*arg);
        assert(s.size() >= 2);
        s.pop_back();
        s.erase(s.begin());
        ret = fmt::format("{}{} {}\n", ret, argv0, s);
    }
    return ret;
}

inline auto generateHelp() -> std::string {
    auto ret = std::string{};

    ret = fmt::format("Usage:\n");
    ret += generateSynopsis() + "\n\n";
    ret += fmt::format("Subcommand usage:\n");
    ret = ret + generateSplitSynopsis();

    auto& args = clice::Register::getInstance().arguments;

    auto f = std::function<void(std::vector<clice::ArgumentBase*>, std::string)>{};

    // Find longest word (for correct indentation)
    size_t longestWord{};
    f = [&](auto const& args, std::string ind) {
        for (auto arg : args) {
            auto typeAsString = typeToString(*arg);
            if (!arg->id.empty()) {
                typeAsString = arg->id;
            }

            auto argstr = fmt::format("{}{} {}", ind, fmt::join(arg->args, ", "), typeAsString);
            longestWord = std::max(longestWord, argstr.size());
        }

        for (auto arg : args) {
            if (!arg->args.empty()) continue;
            f(arg->children, ind + "  ");
        }

        for (auto arg : args) {
            if (arg->args.empty() or arg->args[0][0] == '-') continue;
            f(arg->children, ind + "  ");
        }
        for (auto arg : args) {
            if (arg->args.empty() or arg->args[0][0] != '-') continue;
            f(arg->children, ind + "  ");
        }
    };
    f(args, "");

    if (longestWord > 0) {
        ret += "\n\nOptions:\n";
    }

    f = [&](auto const& args, std::string ind) {

        for (auto arg : args) {
            if (!arg->args.empty()) continue;
            auto typeAsString = typeToString(*arg);
            if (!arg->id.empty()) {
                typeAsString = arg->id;
            }

            auto tagstr = [&]() -> std::string {
                if (arg->tags.contains("required")) return "(required)";
                auto defaultValue = arg->toString();
                if (!defaultValue) return "";
                return fmt::format("(default: {})", *defaultValue);
            }();

            ret = ret + fmt::format("{:<{}} - {} {}\n", typeAsString, longestWord, arg->desc, tagstr);
            f(arg->children, ind);
        }

        for (auto arg : args) {
            if (arg->args.empty() or arg->args[0][0] == '-') continue;
            auto typeAsString = typeToString(*arg);
            if (!arg->id.empty()) {
                typeAsString = arg->id;
            }
            auto argstr = fmt::format("{}{} {}", ind, fmt::join(arg->args, ", "), typeAsString);
            auto tagstr = [&]() -> std::string {
                if (arg->tags.contains("required")) return "(required)";
                auto defaultValue = arg->toString();
                if (!defaultValue) return "";
                return fmt::format("(default: {})", *defaultValue);
            }();

            ret = ret + fmt::format("{:<{}} - {} {}\n", argstr, longestWord, arg->desc, tagstr);
            f(arg->children, ind + "  ");
        }
        for (auto arg : args) {
            if (arg->args.empty() or arg->args[0][0] != '-') continue;
            auto typeAsString = typeToString(*arg);
            if (!arg->id.empty()) {
                typeAsString = arg->id;
            }

            auto argstr = fmt::format("{}{} {}", ind, fmt::join(arg->args, ", "), typeAsString);
            auto tagstr = [&]() -> std::string {
                if (arg->tags.contains("required")) return "(required)";
                auto defaultValue = arg->toString();
                if (!defaultValue) return "";
                return fmt::format("(default: {})", *defaultValue);
            }();
            ret = ret + fmt::format("{:<{}} - {} {}\n", argstr, longestWord, arg->desc, tagstr);
            if (!arg->env.empty()) {
                ret = ret + fmt::format("{:<{}}   environment variable {}", "", longestWord, fmt::join(arg->env, ", "));
            }

            f(arg->children, ind + "  ");
        }
    };
    f(args, "");

    // print environment variables
    {
        // collect all argument bases with environment variables
        auto bases = std::vector<ArgumentBase*>{};
        auto visitAllArguments = std::function<void(std::vector<ArgumentBase*> const&)>{};
        visitAllArguments = [&](auto const& args) {
            for (auto arg : args) {
                if (arg->env.size()) {
                    bases.push_back(arg);
                }
                for (auto child : arg->children) {
                    visitAllArguments(child->children);
                }
            }
        };
        visitAllArguments(Register::getInstance().arguments);

        if (bases.size()) {
            ret += "\n\nEnvironment Variables:\n";

            for (auto arg : bases) {
                std::string env;
                auto env_str = fmt::format("{}", fmt::join(arg->env, ", "));
                if (arg->args.empty()) {
                    auto tagstr = [&]() -> std::string {
                        if (arg->tags.contains("required")) return "(required)";
                        auto defaultValue = arg->toString();
                        if (!defaultValue) return "";
                        return fmt::format("(default: {})", *defaultValue);
                    }();

                    ret = ret + fmt::format("{:<{}} - {} {}\n", env_str, longestWord, arg->desc, tagstr);
                } else {
                    auto arg_str = fmt::format("{}", fmt::join(arg->args, ", "));
                    ret = ret + fmt::format("{:<{}} - same as {}\n", env_str, longestWord, arg_str);
                }
            }
        }
    }

    return ret;
}
}
