#pragma once

#include "Argument.h"

#include <fmt/format.h>

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
    if (arg.type_index == std::type_index(typeid(nullptr_t))) {
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


    for (auto child : arg.arguments) {
        ret += " " + generatePartialSynopsis(*child);
    }
    auto typeAsString = typeToString(arg);
    ret += typeAsString.empty()?"":(" " + typeAsString);
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

    ret = generateSynopsis() + "\n\n";
    ret = generateSplitSynopsis() + "\n\n";

    auto& args = clice::Register::getInstance().arguments;

    auto f = std::function<void(std::vector<clice::ArgumentBase*>, std::string)>{};

    // Find longest word (for correct indentation)
    size_t longestWord{};
    f = [&](auto const& args, std::string ind) {
        for (auto arg : args) {
            auto typeAsString = typeToString(*arg);

            auto argstr = fmt::format("{}{} {}", ind, fmt::join(arg->args, ", "), typeAsString);
            longestWord = std::max(longestWord, argstr.size());
        }

        for (auto arg : args) {
            if (arg->args.empty() or arg->args[0][0] == '-') continue;
            f(arg->arguments, ind + "  ");
        }
        for (auto arg : args) {
            if (arg->args.empty() or arg->args[0][0] != '-') continue;
            f(arg->arguments, ind + "  ");
        }
    };
    f(args, "");

    f = [&](auto const& args, std::string ind) {

        for (auto arg : args) {
            if (arg->args.empty() or arg->args[0][0] == '-') continue;
            auto typeAsString = typeToString(*arg);

            auto argstr = fmt::format("{}{} {}", ind, fmt::join(arg->args, ", "), typeAsString);
            auto tagstr = [&]() -> std::string {
                if (arg->tags.contains("required")) return "(required)";
                auto defaultValue = arg->toString();
                if (!defaultValue) return "";
                return fmt::format("(default: {})", *defaultValue);
            }();

            ret = ret + fmt::format("{:<{}} - {} {}\n", argstr, longestWord, arg->desc, tagstr);
            f(arg->arguments, ind + "  ");
        }
        for (auto arg : args) {
            if (arg->args.empty() or arg->args[0][0] != '-') continue;
            auto typeAsString = typeToString(*arg);

            auto argstr = fmt::format("{}{} {}", ind, fmt::join(arg->args, ", "), typeAsString);
            auto tagstr = [&]() -> std::string {
                if (arg->tags.contains("required")) return "(required)";
                auto defaultValue = arg->toString();
                if (!defaultValue) return "";
                return fmt::format("(default: {})", *defaultValue);
            }();
            ret = ret + fmt::format("{:<{}} - {} {}\n", argstr, longestWord, arg->desc, tagstr);
            f(arg->arguments, ind + "  ");
        }
    };
    f(args, "");
    return ret;
}
}
