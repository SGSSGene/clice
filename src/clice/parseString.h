// SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
// SPDX-License-Identifier: ISC
#pragma once

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <locale>
#include <numbers>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
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
        // Format: [+-][0x|0b]digits[suffix], digits may be separated by '
        // Numbers are decimal by default (leading zeros do not mean octal).

        // remove potential ' separator
        str.erase(std::remove(str.begin(), str.end(), '\''), str.end());

        auto outOfRange = [&]() {
            return std::runtime_error{"integer \"" + str + "\" out of range ["
                                      + std::to_string(std::numeric_limits<T>::min()) + ", "
                                      + std::to_string(std::numeric_limits<T>::max()) + "]"};
        };

        char const* iter = str.data();
        char const* end  = str.data() + str.size();

        bool negative = false;
        if (iter != end && (*iter == '+' || *iter == '-')) {
            negative = (*iter == '-');
            ++iter;
        }
        auto base = int{10};
        if (end - iter >= 2 && iter[0] == '0' && (iter[1] == 'x' || iter[1] == 'X')) {
            base = 16;
            iter += 2;
        } else if (end - iter >= 2 && iter[0] == '0' && (iter[1] == 'b' || iter[1] == 'B')) {
            base = 2;
            iter += 2;
        }

        // parse the magnitude (without sign)
        auto magnitude = uint64_t{};
        auto [ptr, ec] = std::from_chars(iter, end, magnitude, base);
        if (ec == std::errc::invalid_argument) {
            throw std::runtime_error{"not a valid integer \"" + str + "\""};
        } else if (ec == std::errc::result_out_of_range) {
            throw outOfRange();
        }

        // if we didn't parse everything check if it has some known suffix
        if (ptr != end) {
            auto factor = parseSuffix<uint64_t>(std::string_view{ptr, end});
            if (!factor) {
                throw std::runtime_error{"unknown integer suffix \"" + str + "\""};
            }
            if (magnitude > std::numeric_limits<uint64_t>::max() / *factor) {
                throw outOfRange();
            }
            magnitude *= *factor;
        }

        // apply sign and check if it fits into T
        auto maxValue = static_cast<uint64_t>(std::numeric_limits<T>::max());
        if (!negative) {
            if (magnitude > maxValue) throw outOfRange();
            return static_cast<T>(magnitude);
        }
        if (magnitude == 0) return T{};
        if constexpr (std::is_unsigned_v<T>) {
            throw outOfRange();
        } else {
            // |min| == max + 1 (two's complement)
            if (magnitude > maxValue + 1) throw outOfRange();
            return static_cast<T>(-static_cast<T>(magnitude - 1) - 1);
        }

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

        // Find the end of the numeric part: [+-]digits[.digits][(e|E)[+-]digits]
        // The exponent is only part of the number if digits follow, otherwise
        // 'E' is the exa suffix (e.g. "1E" or "1Ei").
        auto numberEnd = [&]() -> size_t {
            auto isDigit = [&](size_t i) {
                return i < str.size() && str[i] >= '0' && str[i] <= '9';
            };
            auto isSign = [&](size_t i) {
                return i < str.size() && (str[i] == '+' || str[i] == '-');
            };
            size_t i{0};
            size_t digitCount{0};
            if (isSign(i)) ++i;
            for (; isDigit(i); ++i) ++digitCount;
            if (i < str.size() && str[i] == '.') {
                ++i;
                for (; isDigit(i); ++i) ++digitCount;
            }
            if (digitCount == 0) {
                throw std::runtime_error{std::string{"not a valid floating-point number \""} + str + "\""};
            }
            if (i < str.size() && (str[i] == 'e' || str[i] == 'E')) {
                auto j = i+1;
                if (isSign(j)) ++j;
                if (isDigit(j)) {
                    for (i = j; isDigit(i); ++i) {}
                }
            }
            return i;
        }();

        auto ret = T{};
        auto ss = std::istringstream{str.substr(0, numberEnd)};
        ss.imbue(std::locale::classic());
        if (not (ss >> ret)) {
            throw std::runtime_error{std::string{"not a valid floating-point number \""} + str + "\""};
        }
        // parse floats/doubles and convert if they are angles or have other suffices
        if (numberEnd < str.size()) {
            auto ending = str.substr(numberEnd);
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
