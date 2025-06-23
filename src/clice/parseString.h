// SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
// SPDX-License-Identifier: ISC
#pragma once

#include <algorithm>
#include <filesystem>
#include <numbers>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>

namespace clice {

template <uint64_t maxValue, typename T>
auto parseSuffixHelper(std::string_view str, std::string_view suffix) -> T {
    if (str != suffix) {
        return 1;
    }
    if constexpr (maxValue > uint64_t(std::numeric_limits<T>::max())) {
        throw std::runtime_error{"out of range"};
    } else {
        return maxValue;
    }
}

template <long double maxValue, typename T>
auto parseSuffixHelper_l(std::string_view str, std::string_view suffix) -> T {
    if (str != suffix) {
        return 1;
    }
    if constexpr (maxValue > (long double)(std::numeric_limits<T>::max())) {
        throw std::runtime_error{"out of range"};
    } else {
        return maxValue;
    }
}


template<typename T>
auto parseSuffix(std::string_view suffix) -> std::optional<T> {
    auto ret = T{1};

    if constexpr (std::floating_point<T>) {
        ret = ret
            * parseSuffixHelper_l<1000.L, T>("k", suffix)
            * parseSuffixHelper_l<1024.L, T>("ki", suffix)
            * parseSuffixHelper_l<1000.L*1000, T>("M", suffix)
            * parseSuffixHelper_l<1024.L*1024, T>("Mi", suffix)
            * parseSuffixHelper_l<1000.L*1000*1000, T>("G", suffix)
            * parseSuffixHelper_l<1024.L*1024*1024, T>("Gi", suffix)
            * parseSuffixHelper_l<1000.L*1000*1000*1000, T>("T", suffix)
            * parseSuffixHelper_l<1024.L*1024*1024*1024, T>("Ti", suffix)
            * parseSuffixHelper_l<1000.L*1000*1000*1000*1000, T>("P", suffix)
            * parseSuffixHelper_l<1024.L*1024*1024*1024*1024, T>("Pi", suffix)
            * parseSuffixHelper_l<1000.L*1000*1000*1000*1000*1000, T>("E", suffix)
            * parseSuffixHelper_l<1024.L*1024*1024*1024*1024*1024, T>("Ei", suffix)

            * parseSuffixHelper_l<0.1L, T>("d", suffix)
            * parseSuffixHelper_l<0.01L, T>("c", suffix)
            * parseSuffixHelper_l<0.001L, T>("m", suffix)
            * parseSuffixHelper_l<0.000'001L, T>("u", suffix)
            * parseSuffixHelper_l<0.000'000'001L, T>("n", suffix)
            * parseSuffixHelper_l<0.000'000'000'001L, T>("p", suffix)
        ;
    } else {
        ret = ret
            * parseSuffixHelper<1000, T>("k", suffix)
            * parseSuffixHelper<1024, T>("ki", suffix)
            * parseSuffixHelper<1000*1000, T>("M", suffix)
            * parseSuffixHelper<1024*1024, T>("Mi", suffix)
            * parseSuffixHelper<1000*1000*1000, T>("G", suffix)
            * parseSuffixHelper<1024*1024*1024, T>("Gi", suffix)
            * parseSuffixHelper<1000ull*1000*1000*1000, T>("T", suffix)
            * parseSuffixHelper<1024ull*1024*1024*1024, T>("Ti", suffix)
            * parseSuffixHelper<1000ull*1000*1000*1000*1000, T>("P", suffix)
            * parseSuffixHelper<1024ull*1024*1024*1024*1024, T>("Pi", suffix)
            * parseSuffixHelper<1000ull*1000*1000*1000*1000*1000, T>("E", suffix)
            * parseSuffixHelper<1024ull*1024*1024*1024*1024*1024, T>("Ei", suffix)
        ;
    }
    if (ret == 1) {
        return std::nullopt;
    }
    return {ret};
}

template<typename T>
auto parseFromString(std::string_view _str) -> T {
    auto str = std::string{_str};
    if constexpr (std::is_same_v<T, bool>) {
        std::ranges::transform(str, str.begin(), ::tolower);
        if (str == "true" or str == "1" or str == "yes") {
            return true;
        }
        if (str == "false" or str == "0" or str == "no") {
            return false;
        }
        throw std::runtime_error{std::string{"invalid boolean specifier \""} + str + "\""};
    } else if constexpr (std::is_same_v<T, std::string>) {
        return str;
    } else if constexpr (std::is_same_v<T, std::filesystem::path>) {
        return str;
    } else if constexpr (std::is_same_v<T, char>) {
        if (_str.size() != 1) {
            throw std::runtime_error{std::string{"invalid char specifier, must be exactly one char \""} + str + "\""};
        }
        return _str[0];
    } else if constexpr (std::numeric_limits<T>::is_exact) {
        // parse all integer-like types
        auto ret = T{};

        // remove potential ' separator
        str.erase(std::remove(str.begin(), str.end(), '\''), str.end());

        auto base = int{0};
        char const* strBegin = str.data();
        char const* strEnd   = str.data() + str.size();
        std::size_t nextIdx=0;
        if (str.find("0b") == 0) {
            base = 2;
            strBegin += 2;
        }
        try {
            if constexpr (std::is_unsigned_v<T>) {
                ret = std::stoull(strBegin, &nextIdx, base);
            } else {
                ret = std::stoll(strBegin, &nextIdx, base);
            }
        } catch (std::invalid_argument const&) {
            throw std::runtime_error{std::string{"not a valid integer \""} + str + "\""};
        }
        // if we didn't parse everything check if it has some known suffix
        if (static_cast<int>(nextIdx) != strEnd - strBegin) {
            if constexpr (not std::is_same_v<bool, T>) {
                auto suffix = std::string_view{strBegin + nextIdx};
                auto value = parseSuffix<T>(suffix);
                if (not value) {
                    throw std::runtime_error{std::string{"unknown integer suffix \""} + str + "\""};
                }
                ret *= value.value();
            }
        }
        return ret;

    } else if constexpr (std::is_enum_v<T>) {
        using UT = std::underlying_type_t<T>;
        auto ret = UT{};
        // parse everything else
        auto ss = std::stringstream{str};
        if (not (ss >> ret)) {
            throw std::runtime_error{std::string{"error parsing cli"}};
        }
        return T(ret);
    } else if constexpr (std::floating_point<T>) {
        // remove potential ' separator
        str.erase(std::remove(str.begin(), str.end(), '\''), str.end());

        auto ret = T{};
        auto ss = std::stringstream{str};
        if (not (ss >> ret)) {
            throw std::runtime_error{std::string{"error parsing cli"}};
        }
        // parse floats/doubles and convert if they are angles or have other suffices
        if (not ss.eof()) {
            auto ending = std::string{};
            if (not (ss >> ending)) {
                throw std::runtime_error{std::string{"invalid string \""} + str + "\""};
            }
            if (ending.ends_with("rad")) {
                ending = ending.substr(0, ending.size()-3);
            } else if (ending.ends_with("deg")) {
                ret = ret / 180. * std::numbers::pi;
                ending = ending.substr(0, ending.size()-3);
            } else if (ending.ends_with("pi") or ending.ends_with("π")) {
                ret = ret * std::numbers::pi;
                ending = ending.substr(0, ending.size()-2);
            } else if (ending.ends_with("tau") or ending.ends_with("τ")) {
                ret = ret * 2. * std::numbers::pi;
                if (ending.ends_with("tau")) {
                    ending = ending.substr(0, ending.size()-3);
                } else {
                    ending = ending.substr(0, ending.size()-2);
                }
            }
            if (ending.size()) {
                auto value = parseSuffix<T>(ending);
                if (!value) {
                    throw std::runtime_error{std::string{"unknown floating-point suffix \""} + str + "\""};
                }
                ret = ret * value.value();
            }
        }
        return ret;
    } else {
        // parse everything else
        auto ret = T{};
        auto ss = std::stringstream{str};
        if (not (ss >> ret)) {
            throw std::runtime_error{std::string{"error parsing cli"}};
        }

        if (not ss.eof()) {
            throw std::runtime_error{std::string{"error parsing cli"}};
        }
        return ret;
    }
}

}
