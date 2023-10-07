// SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
// SPDX-License-Identifier: ISC
#pragma once

#include "Argument.h"

#include <algorithm>
#include <fmt/format.h>
#include <tdl/tdl.h>

#include <iostream>

namespace clice {

inline bool isBoolType(std::type_index type) {
    static auto s = std::unordered_set<std::type_index> {
        std::type_index(typeid(std::nullptr_t)),
        std::type_index(typeid(bool)),
    };
    return s.contains(type);
}

inline bool isIntType(std::type_index type) {
    static auto s = std::unordered_set<std::type_index> {
        std::type_index(typeid(int8_t)),
        std::type_index(typeid(uint8_t)),
        std::type_index(typeid(int16_t)),
        std::type_index(typeid(uint16_t)),
        std::type_index(typeid(int32_t)),
        std::type_index(typeid(uint32_t)),
        std::type_index(typeid(int64_t)),
        std::type_index(typeid(uint64_t)),
    };
    return s.contains(type);
}
inline bool isFloatType(std::type_index type) {
    static auto s = std::unordered_set<std::type_index> {
        std::type_index(typeid(float)),
        std::type_index(typeid(double)),
    };
    return s.contains(type);
}
inline bool isStringType(std::type_index type) {
    static auto s = std::unordered_set<std::type_index> {
        std::type_index(typeid(std::string)),
        std::type_index(typeid(std::filesystem::path)),
    };
    return s.contains(type);
}
inline bool isIntListType(std::type_index type) {
    static auto s = std::unordered_set<std::type_index> {
        std::type_index(typeid(std::vector<uint8_t>)),
        std::type_index(typeid(std::vector<int8_t>)),
        std::type_index(typeid(std::vector<uint16_t>)),
        std::type_index(typeid(std::vector<int16_t>)),
        std::type_index(typeid(std::vector<uint32_t>)),
        std::type_index(typeid(std::vector<int32_t>)),
        std::type_index(typeid(std::vector<uint64_t>)),
        std::type_index(typeid(std::vector<int64_t>)),
    };
    return s.contains(type);
}

inline bool isFloatListType(std::type_index type) {
    static auto s = std::unordered_set<std::type_index> {
        std::type_index(typeid(std::vector<float>)),
        std::type_index(typeid(std::vector<double>)),
    };
    return s.contains(type);
}
inline bool isStringListType(std::type_index type) {
    static auto s = std::unordered_set<std::type_index> {
        std::type_index(typeid(std::vector<std::string>)),
        std::type_index(typeid(std::vector<std::filesystem::path>)),
    };
    return s.contains(type);
}



inline auto generateCWL(std::vector<std::string> subtool) -> tdl::ToolInfo {
    auto info = tdl::ToolInfo{};

    auto f = std::function<tdl::Node::Children(std::vector<clice::ArgumentBase*>)>{};

    f = [&](auto const& args) {
        auto res = tdl::Node::Children{};
        for (auto arg : args) {
            if (subtool.size() > 0) {
                for (auto a : arg->args) {
                    if (subtool[0] == a) {
                        subtool.erase(subtool.begin());
                        auto node = tdl::Node {
                            .name        = a,
                            .description = arg->desc,
                            .tags        = {arg->tags.begin(), arg->tags.end()},
                            .value       = f(arg->arguments),
                        };
                        node.tags.insert("basecommand");
                        res.push_back(node);
                        return res;
                    }
                }
                continue;
            }

            auto node = tdl::Node {
                .name        = arg->args[0],
                .description = arg->desc,
                .tags        = {arg->tags.begin(), arg->tags.end()},
                .value       = tdl::StringValueList{},
            };
            if (node.name.starts_with("--")) {
                node.name = node.name.substr(2);
            } else if (node.name.starts_with("-")) {
                node.name = node.name.substr(1);
            }
            if (node.name == "help") {
                node.name = "tool-help";
            }

            //!TODO a few more tags probably need to be converted here
            if (arg->arguments.size()) {
                node.value = f(arg->arguments);
            } else if (arg->mapping) { //!TODO only works for single values, produces wrong outputs for lists
                node.value = tdl::StringValue{};
            } else if (isBoolType(arg->type_index)) {
                node.value = tdl::BoolValue{};
            } else if (isIntType(arg->type_index)) {
                node.value = tdl::IntValue{};
            } else if (isFloatType(arg->type_index)) {
                node.value = tdl::DoubleValue{};
            } else if (isStringType(arg->type_index)) {
                node.value = tdl::StringValue{};
            } else if (isIntListType(arg->type_index)) {
                node.value = tdl::IntValueList{};
            } else if (isFloatListType(arg->type_index)) {
                node.value = tdl::DoubleValueList{};
            } else if (isStringListType(arg->type_index)) {
                node.value = tdl::StringValueList{};

            } else {
                std::cerr << "unknown on how to convert " + arg->args[0] + " from clice to tdl\n";
                continue;
            }

            res.push_back(node);
            info.cliMapping.push_back({
                .optionIdentifier = arg->args[0],
                .referenceName    = node.name,
            });
        }
        return res;
    };
    info.params = f(clice::Register::getInstance().arguments);

    return info;
}
}
