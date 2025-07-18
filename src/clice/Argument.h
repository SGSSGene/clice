// SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
// SPDX-License-Identifier: ISC
#pragma once

#include "parseString.h"

#include <algorithm>
#include <any>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_set>
#include <utility>
#include <vector>

namespace clice {

inline std::string argv0; // Parser will fill this

struct ArgumentBase {
    ArgumentBase*                           parent{};
    std::vector<std::string>                args;
    std::vector<std::string>                env;
    std::string                             id;
    std::string                             desc;
    std::optional<std::vector<std::string>> mapping{};
    std::unordered_set<std::string>         tags;
    std::optional<std::string>              completion{};
    std::function<std::vector<std::string>()> completion_fn;
    std::vector<ArgumentBase*>              children;  // child parameters
    bool                                    symlink{};  // a symlink for example to "slix-env" should actually call "slix env"
    std::type_index                         type_index;

    std::function<void()> init;
    std::function<void(std::string_view)>       fromString;
    std::function<std::optional<std::string>()> toString;
    std::function<void()> cb;
    size_t                cb_priority;

    ArgumentBase() = delete;
    ArgumentBase(ArgumentBase* parent, std::type_index idx);
    virtual ~ArgumentBase();
    ArgumentBase(ArgumentBase const&) = delete;
    ArgumentBase(ArgumentBase&&) = delete;
    auto operator=(ArgumentBase const&) -> ArgumentBase& = delete;
    auto operator=(ArgumentBase&&) -> ArgumentBase& = delete;

    void validateOrThrowInvariant() const;
};

struct Register {
    std::vector<ArgumentBase*> arguments;

    static auto getInstance() -> Register& {
        static Register instance;
        return instance;
    }
};

inline ArgumentBase::ArgumentBase(ArgumentBase* parent, std::type_index idx)
    : parent{parent}
    , type_index{idx}
{
    if (parent) {
        parent->children.push_back(this);
    } else {
        // check this child does not exists yet
        Register::getInstance().arguments.push_back(this);
    }
}

inline ArgumentBase::~ArgumentBase() {
    if (parent) {
        auto& children = parent->children;
        children.erase(std::remove(children.begin(), children.end(), this), children.end());
    } else {
        auto& children = Register::getInstance().arguments;
        children.erase(std::remove(children.begin(), children.end(), this), children.end());
    }
}

inline void ArgumentBase::validateOrThrowInvariant() const {
    auto const& self = *this;
    auto checkAgainstOther = [&](ArgumentBase const* child) {
        if (&self == child) return;
        for (auto const& s : args) {
            for (auto const& s2 : child->args) {
                if (s == s2) {
                    throw std::runtime_error{"two options register the same option/flag \"" + s + "\""};
                }
            }
        }
    };
    if (parent) {
        for (auto const& child : parent->children) {
            checkAgainstOther(child);
        }
    } else {
        for (auto const& child : Register::getInstance().arguments) {
            checkAgainstOther(child);
        }
    }
}

struct ListOfStrings : std::vector<std::string> {
    ListOfStrings() {}
    ListOfStrings(char const* str) {
        emplace_back(str);
    }
    ListOfStrings(std::initializer_list<char const*> list) {
        for (auto l : list) {
            emplace_back(l);
        }
    }
};

template <typename S>
constexpr bool HasPushBack = requires {
    typename S::value_type;
    { std::declval<S>().push_back(std::declval<typename S::value_type>()) };
};

template <typename T = std::nullptr_t, typename CBType = std::function<void()>, typename ...TParents>
struct Argument {
    Argument<TParents...>*     parent{};
    ListOfStrings              args{};
    ListOfStrings              env{};
    std::string                id{}; // some identification, like <threadNbr>
    bool                       symlink{};
    std::string                desc{};
    bool                       isSet{};   // (not for the user)
    T                          value{};
    std::optional<std::string> suffix{};  // require a suffix like "b" (bytes) or "s" (seconds)
    mutable std::any           anyType{}; // used if T is a callback (not for the user)
    std::function<std::vector<std::string>()> completion{};
    CBType                                            cb{};
    size_t                                            cb_priority{100}; // lower priorities will be triggered before larger ones
    std::optional<std::unordered_map<std::string, T>> mapping{};
    std::unordered_set<std::string>                   tags{};  // known tags "required"

    operator bool() const {
        return isSet;
    }

    auto operator*() const -> auto const&
        requires (!std::same_as<T, std::nullptr_t>)
    {
        if constexpr (std::is_invocable_v<T>) {
            using R = std::decay_t<decltype(value())>;
            if (!anyType.has_value()) {
                anyType = value();
            }
            return *std::any_cast<R>(&anyType);
        } else {
            return value;
        }
    }

    auto operator->() const -> auto const* {
        auto const& r = **this;
        return &r;
    }

    struct CTor {
        ArgumentBase arg;
        static auto detectType() -> std::type_index {
            if constexpr (std::is_invocable_v<T>) {
                // Get the type_index of a lambda
                return std::type_index(typeid(std::invoke_result_t<T>));
            }
            return std::type_index(typeid(T));
        };
        CTor(Argument& desc)
            : arg { desc.parent?&desc.parent->storage.arg:nullptr, detectType()}
        {
            arg.args    = desc.args;
            arg.env     = desc.env;
            arg.id      = desc.id;
            arg.symlink = desc.symlink;
            arg.desc    = desc.desc;
            arg.validateOrThrowInvariant();

            if (desc.completion) {
                arg.completion_fn = desc.completion;
            } else {
                if constexpr (std::same_as<std::filesystem::path, T>) {
                    arg.completion = " -f ";
                } else if (desc.mapping) {
                    std::string str;
                    for (auto [key, value] : *desc.mapping) {
                        str += key + "\n";
                    }
                    arg.completion = str;
                }
            }
            if (desc.mapping) {
                auto v = std::vector<std::string>{};
                for (auto const& [key, value] : *desc.mapping) {
                    v.push_back(key);
                }
                arg.mapping = v;
            }
            arg.tags = desc.tags;

            bool isMulti = HasPushBack<T> && !std::same_as<std::string, T> && !std::same_as<std::filesystem::path, T>;
            if (isMulti) {
                arg.tags.insert("multi");
            }
            arg.init = [&]() {
                desc.isSet = true;
                if constexpr (requires() {
                    { desc.cb() };
                }) {
                    arg.cb = desc.cb;
                } else if constexpr (requires() {
                    { desc.cb(*desc) };
                }) {
                    arg.cb = [&]() {
                        desc.cb(*desc);
                    };
                }
                arg.cb_priority = desc.cb_priority;
                if constexpr (std::same_as<std::nullptr_t, T>) {
                } else if constexpr (std::is_arithmetic_v<T>) {
                    arg.fromString = [&](std::string_view s) {
                        if (desc.mapping) {
                            if (!desc.mapping->contains(std::string{s})) {
                                std::string validValues{};
                                for (auto [key, value] : *desc.mapping) {
                                    validValues += key + ", ";
                                }
                                if (validValues.size() > 2) {
                                    validValues.pop_back();
                                    validValues.pop_back();
                                }
                                throw std::runtime_error{"invalid value \"" + std::string{s} + "\". Valid values are: [ " + validValues + " ]"};
                            }
                            desc.value = desc.mapping->at(std::string{s});
                        } else {
                            if (desc.suffix) {
                                if (!s.ends_with(desc.suffix.value())) {
                                    throw std::runtime_error{"expected the suffix \"" + desc.suffix.value() + "\""};
                                }
                                auto t = s.substr(0, s.size() - desc.suffix->size());
                                desc.value = parseFromString<T>(t);
                            } else {
                                desc.value = parseFromString<T>(s);
                            }
                        }
                        arg.fromString = nullptr;
                    };
                } else if constexpr (   std::same_as<std::string, T>
                                     || std::same_as<std::filesystem::path, T>
                                     || std::is_enum_v<T>) {
                    arg.fromString = [&](std::string_view s) {
                        if (desc.mapping) {
                            if (!desc.mapping->contains(std::string{s})) {
                                std::string validValues{};
                                for (auto [key, value] : *desc.mapping) {
                                    validValues += key + ", ";
                                }
                                if (validValues.size() > 2) {
                                    validValues.pop_back();
                                    validValues.pop_back();
                                }
                                throw std::runtime_error{"invalid value \"" + std::string{s} + "\". Valid values are: [ " + validValues + " ]"};
                            }

                            desc.value = desc.mapping->at(std::string{s});
                        } else {
                            desc.value = parseFromString<T>(s);
                        }
                        arg.fromString = nullptr;
                    };
                } else if constexpr (HasPushBack<T>) {
                    arg.fromString = [&](std::string_view s) {
                        if (desc.mapping) {
                            throw std::runtime_error("Type can't use mapping");
                        } else {
                            using value_type = typename T::value_type;
                            if constexpr (std::integral<value_type> || std::floating_point<value_type>) {
                                if (desc.suffix) {
                                    if (!s.ends_with(desc.suffix.value())) {
                                        throw std::runtime_error{"expected the suffix \"" + desc.suffix.value() + "\""};
                                    }
                                    auto t = s.substr(0, s.size() - desc.suffix->size());
                                    desc.value.push_back(parseFromString<value_type>(t));
                                } else {
                                    desc.value.push_back(parseFromString<value_type>(s));
                                }
                            } else {
                                desc.value.push_back(parseFromString<value_type>(s));
                            }
                        }
                    };
                } else if constexpr (std::is_invocable_v<T>) {
                    arg.fromString = [&](std::string_view s) {
                        using RT = std::invoke_result_t<T>;
                        desc.anyType = parseFromString<RT>(s);
                    };
                } else {
                    []<bool type_available = false> {
                        static_assert(type_available, "Type can't be used as a value type in clice::Argument");
                    }();
                }
            };
            arg.toString = [&]() -> std::optional<std::string> {
                auto reverseMapping = [&](auto v) -> std::string {
                    for (auto const& [key, value] : *desc.mapping) {
                        if (value == v) return key;
                    }
                    return "unknown";
                };

                if constexpr (std::same_as<std::nullptr_t, T>) {
                    return std::nullopt;
                } else if constexpr (std::same_as<bool, T>) {
                    if (desc.mapping) return reverseMapping(desc.value);
                    return desc.value?"true":"false";
                } else if constexpr (std::is_arithmetic_v<T>) {
                    if (desc.mapping) return reverseMapping(desc.value);
                    return std::to_string(desc.value);
                } else if constexpr (std::same_as<std::string, T>) {
                    if (desc.mapping) return reverseMapping(desc.value);
                    if (desc.value.empty()) return "\"\"";
                    return desc.value;
                } else if constexpr (std::same_as<std::filesystem::path, T>) {
                    if (desc.mapping) return reverseMapping(desc.value);
                    if (desc.value.string() == "") return "\"\"";
                    return desc.value.string();
                } else if constexpr (std::is_enum_v<T>) {
                    if (desc.mapping) return reverseMapping(desc.value);
                    using UT = std::underlying_type_t<T>;
                    return std::to_string(static_cast<UT>(desc.value));
                } else if constexpr (HasPushBack<T>) {
                    return std::nullopt;
                } else if constexpr (std::is_invocable_v<T>) {
                    return std::nullopt;
                } else {
                    []<bool type_available = false> {
                        static_assert(type_available, "Type can't be used as a value type in clice::Argument");
                    }();
                }
            };

        }
    } storage{*this};
};

}
