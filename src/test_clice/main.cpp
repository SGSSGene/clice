// SPDX-FileCopyrightText: 2025 Simon Gene Gottlieb
// SPDX-License-Identifier: CC0-1.0
#include "someheader.h"

#include <clice/clice.h>
#include <catch2/catch_all.hpp>

#include <cstdlib>
#include <string>

template <typename T>
concept dereferencable = requires(T t) {
    { *t };
};

// sets an environment variable for the lifetime of this object
struct ScopedEnv {
    std::string name;
    ScopedEnv(std::string _name, std::string const& value)
        : name{std::move(_name)}
    {
#ifdef _WIN32
        _putenv_s(name.c_str(), value.c_str());
#else
        setenv(name.c_str(), value.c_str(), 1);
#endif
    }
    ~ScopedEnv() {
#ifdef _WIN32
        _putenv_s(name.c_str(), "");
#else
        unsetenv(name.c_str());
#endif
    }
    ScopedEnv(ScopedEnv const&) = delete;
    auto operator=(ScopedEnv const&) -> ScopedEnv& = delete;
};

TEST_CASE("check clice::Argument", "argument") {
    SECTION("test flag") {
        auto cliOpt1 = clice::Argument{ .args   = "--flag1",};

        SECTION("no parse, no option set") {
            CHECK(!cliOpt1);
        }

        SECTION("parse, but no --flag1, no option set") {
            auto args = std::vector<std::string_view>{"app"};
            clice::parse(args);
            CHECK(!cliOpt1);
        }

        SECTION("parse with --flag1 set") {
            auto args = std::vector<std::string_view>{"app", "--flag1"};
            clice::parse(args);
            CHECK(cliOpt1);
        }

        static_assert(!dereferencable<decltype(cliOpt1)>, "check that cliOpt1 can not be dereferenced");
    }

    SECTION("test single value option - int") {
        auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                        .value  = int{7}};

        SECTION("no parse: no option set") {
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == 7); // should have default value
        }

        SECTION("parse: but no --opt1 option set") {
            auto args = std::vector<std::string_view>{"app"};
            clice::parse(args);
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == 7); // should have default value
        }

        SECTION("parse with --opt1 option") {
            auto args = std::vector<std::string_view>{"app", "--opt1", "12"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == 12);
        }

        static_assert(dereferencable<decltype(cliOpt1)>, "check that cliOpt1 can be dereferenced");
    }
    SECTION("test single value option - int -- negative number") {
        auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                        .value  = int{7}};

        SECTION("no parse: no option set") {
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == 7); // should have default value
        }

        SECTION("parse: but no --opt1 option set") {
            auto args = std::vector<std::string_view>{"app"};
            clice::parse(args);
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == 7); // should have default value
        }

        SECTION("parse with --opt1 option") {
            auto args = std::vector<std::string_view>{"app", "--opt1", "-12"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == -12);
        }

        static_assert(dereferencable<decltype(cliOpt1)>, "check that cliOpt1 can be dereferenced");
    }

    SECTION("test single value option - std::string") {
        auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                        .value  = std::string{"foo"}};

        SECTION("no parse: no option set") {
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == "foo"); // should have default value
        }

        SECTION("parse: but no --opt1 option set") {
            auto args = std::vector<std::string_view>{"app"};
            clice::parse(args);
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == "foo"); // should have default value
        }

        SECTION("parse with --opt1 option") {
            auto args = std::vector<std::string_view>{"app", "--opt1", "bar"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == "bar");
        }

        static_assert(dereferencable<decltype(cliOpt1)>, "check that cliOpt1 can be dereferenced");
    }

    SECTION("test multi value option - int") {
        SECTION("parse with --opt1 option - no values") {
            auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                            .value  = std::vector<int>{}};


            auto args = std::vector<std::string_view>{"app", "--opt1"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == std::vector<int>{});
        }

        SECTION("parse with --opt1 option - single value") {
            auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                            .value  = std::vector<int>{}};


            auto args = std::vector<std::string_view>{"app", "--opt1", "3"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == std::vector<int>{3});
        }

        SECTION("parse with --opt1 option - two values") {
            auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                            .value  = std::vector<int>{}};


            auto args = std::vector<std::string_view>{"app", "--opt1", "3", "5"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == std::vector<int>{3, 5});
        }

        SECTION("parse with --opt1 option - many values") {
            auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                            .value  = std::vector<int>{}};


            auto args = std::vector<std::string_view>{"app", "--opt1", "3", "5", "7", "100", "1000", "2", "-7"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == std::vector<int>{3, 5, 7, 100, 1000, 2, -7});
        }
    }

    SECTION("test position argument") {
        auto cliOpt1 = clice::Argument{ .value  = int{7}};

        SECTION("no parse: no option set") {
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == 7); // should have default value
        }

        SECTION("parse: but no --opt1 option set") {
            auto args = std::vector<std::string_view>{"app"};
            clice::parse(args);
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == 7); // should have default value
        }

        SECTION("parse with --opt1 option") {
            auto args = std::vector<std::string_view>{"app", "12"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == 12);
        }

        static_assert(dereferencable<decltype(cliOpt1)>, "check that cliOpt1 can be dereferenced");
    }

    SECTION("test position argument - but it takes also an environment variable") {
        auto cliOpt1 = clice::Argument{ .env   = {"MY_ENV"},
                                        .value = int{7},
        };

        SECTION("no parse: no option set") {
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == 7); // should have default value
        }

        SECTION("parse: but no --opt1 option set") {
            auto args = std::vector<std::string_view>{"app"};
            clice::parse(args);
            CHECK(!cliOpt1);
            CHECK(*cliOpt1 == 7); // should have default value
        }

        SECTION("parse with --opt1 option") {
            auto args = std::vector<std::string_view>{"app", "12"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == 12);
        }

        static_assert(dereferencable<decltype(cliOpt1)>, "check that cliOpt1 can be dereferenced");
    }

    SECTION("flag set via environment variable") {
        auto cliFlag = clice::Argument{ .args = "--flag",
                                        .env  = {"CLICE_TEST_FLAG"},
        };
        auto args = std::vector<std::string_view>{"app"};

        SECTION("env not set") {
            clice::parse(args);
            CHECK(!cliFlag);
        }

        SECTION("true-like values set the flag") {
            for (auto v : {"1", "true", "TRUE", "yes"}) {
                INFO(v);
                auto env = ScopedEnv{"CLICE_TEST_FLAG", v};
                auto cliFlag2 = clice::Argument{ .args = "--flag2",
                                                 .env  = {"CLICE_TEST_FLAG"},
                };
                clice::parse(args);
                CHECK(cliFlag2);
            }
        }

#ifndef _WIN32 // windows can't store empty environment variables
        SECTION("empty value sets the flag") {
            auto env = ScopedEnv{"CLICE_TEST_FLAG", ""};
            clice::parse(args);
            CHECK(cliFlag);
        }
#endif

        SECTION("false-like values leave the flag unset") {
            for (auto v : {"0", "false", "no"}) {
                INFO(v);
                auto env = ScopedEnv{"CLICE_TEST_FLAG", v};
                auto cliFlag2 = clice::Argument{ .args = "--flag2",
                                                 .env  = {"CLICE_TEST_FLAG"},
                };
                clice::parse(args);
                CHECK(!cliFlag2);
            }
        }

        SECTION("invalid value is reported") {
            auto env = ScopedEnv{"CLICE_TEST_FLAG", "maybe"};
            CHECK_THROWS_WITH(clice::parse(args),
                              "environment variable CLICE_TEST_FLAG: invalid boolean specifier \"maybe\"");
        }
    }

    SECTION("env-only flag does not consume positional arguments") {
        auto cliFlag = clice::Argument{ .env = {"CLICE_TEST_FLAG"} };
        auto cliPos  = clice::Argument{ .value = int{0} };

        auto env  = ScopedEnv{"CLICE_TEST_FLAG", "1"};
        auto args = std::vector<std::string_view>{"app", "12"};
        clice::parse(args);
        CHECK(cliFlag);
        CHECK(cliPos);
        CHECK(*cliPos == 12);
    }

    SECTION("splitting arguments") {
        auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                        .value  = std::vector<int>{}};


        auto args = std::vector<std::string_view>{"app", "--opt1", "-15"};
        clice::parse(args, /*.allowDashCombi=*/true);
        CHECK(cliOpt1);
        CHECK(*cliOpt1 == std::vector<int>{-15});
    }

    SECTION("splitting arguments - combined flags") {
        auto cliA = clice::Argument{ .args = "-a" };
        auto cliB = clice::Argument{ .args = "-b" };

        auto args = std::vector<std::string_view>{"app", "-ab"};
        clice::parse(args, /*.allowDashCombi=*/true);
        CHECK(cliA);
        CHECK(cliB);
    }

    SECTION("splitting arguments - single dash option is not split") {
        auto cliA      = clice::Argument{ .args = "-a" };
        auto cliOption = clice::Argument{ .args = "-option" };

        auto args = std::vector<std::string_view>{"app", "-option"};
        clice::parse(args, /*.allowDashCombi=*/true);
        CHECK(!cliA);
        CHECK(cliOption);
    }

    SECTION("splitting arguments - unknown arguments are reported") {
        auto cliA = clice::Argument{ .args = "-a" };

        auto args = std::vector<std::string_view>{"app", "-axyz"};
        CHECK_THROWS_WITH(clice::parse(args, /*.allowDashCombi=*/true),
                          "unexpected cli argument \"-axyz\"");
        CHECK(!cliA);
    }

    SECTION("splitting arguments - split args survive in unprocessedParameters") {
        auto cliA = clice::Argument{ .args = "-a" };
        auto cliB = clice::Argument{ .args = "-b" };

        auto unprocessed = std::vector<std::string_view>{};
        {
            auto args = std::vector<std::string_view>{"app", "--unknown", "-ab"};
            clice::parse(args, /*.allowDashCombi=*/true, &unprocessed);
        }
        // views must still be valid after parse() returned
        CHECK(unprocessed == std::vector<std::string_view>{"--unknown", "-a", "-b"});
        CHECK(!cliA);
        CHECK(!cliB);
    }

    SECTION("arguments with magnitude suffix") {
        auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                        .value  = std::vector<size_t>{}};


        auto args = std::vector<std::string_view>{"app", "--opt1", "15M", "16k", "16ki", "1'024Mi"};
        clice::parse(args);
        CHECK(cliOpt1);
        CHECK(*cliOpt1 == std::vector<size_t>{15*1000*1000, 16*1000, 16*1024, 1024ull * 1024 * 1024});
    }

    SECTION("arguments with magnitude suffix - floats") {
        auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                        .value  = std::vector<double>{}};


        auto args = std::vector<std::string_view>{"app", "--opt1", "15M", "16k", "16ki", "1'024Gi", "10m", "20.2u", "1.25n"};
        clice::parse(args);
        CHECK(cliOpt1);
        auto expected = std::vector<double>{15*1000*1000, 16*1000, 16*1024, 1024ull * 1024 * 1024 * 1024, 0.01, 0.0000202, 0.00000000125};
        REQUIRE(cliOpt1->size() == expected.size());
        for (size_t i{0}; i < expected.size(); ++i) {
            INFO(i);
            INFO(expected[i] - cliOpt1->at(i));
            CHECK(expected[i] == cliOpt1->at(i));
        }
    }

    SECTION("arguments with magnitude suffix - floats") {
        auto cliOpt1 = clice::Argument{ .args   = "--time",
                                        .value  = std::vector<double>{},
                                        .suffix = "s"
                                        };


        auto args = std::vector<std::string_view>{"app", "--time", "15Ms", "16ks", "16kis", "1'024Gis", "10ms", "20.2us", "1.25ns"};
        clice::parse(args);
        CHECK(cliOpt1);
        auto expected = std::vector<double>{15*1000*1000, 16*1000, 16*1024, 1024ull * 1024 * 1024 * 1024, 0.01, 0.0000202, 0.00000000125};
        REQUIRE(cliOpt1->size() == expected.size());
        for (size_t i{0}; i < expected.size(); ++i) {
            INFO(i);
            INFO(expected[i] - cliOpt1->at(i));
            CHECK(expected[i] == cliOpt1->at(i));
        }
    }

    SECTION("floats - exa suffix vs exponent") {
        auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                        .value  = std::vector<double>{}};

        auto args = std::vector<std::string_view>{"app", "--opt1", "1E", "2Ei", "1.5E", "1E3", "1e3", "2E-3", "1E3k", "-.5E"};
        clice::parse(args);
        CHECK(cliOpt1);
        auto expected = std::vector<double>{1e18, 2. * 1024 * 1024 * 1024 * 1024 * 1024 * 1024, 1.5e18, 1e3, 1e3, 2e-3, 1e6, -0.5e18};
        REQUIRE(cliOpt1->size() == expected.size());
        for (size_t i{0}; i < expected.size(); ++i) {
            INFO(i);
            CHECK(expected[i] == cliOpt1->at(i));
        }
    }

    SECTION("floats - invalid values") {
        auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                        .value  = double{}};

        for (auto v : {"abc", ".", "-", "E", "1Ex"}) {
            INFO(v);
            auto args = std::vector<std::string_view>{"app", "--opt1", v};
            CHECK_THROWS(clice::parse(args));
        }
    }

    SECTION("arguments as children") {
        auto cliOptA = clice::Argument {
            .args = "--parent",
        };
        auto cliOptB = clice::Argument {
            .parent = &cliOptA,
            .args   = "--opt",
            .value  = std::string{},
        };


        auto args = std::vector<std::string_view>{"app", "--parent", "--opt", "blub"};
        clice::parse(args);
        CHECK(cliOptA);
        CHECK(cliOptB);
    }

    SECTION("dynamic parameters") {
        auto cliOptA = clice::Argument {
            .args = "--parent",
        };

        auto cliOptLoad = clice::Argument {
            .args = "--load",
            .cb   = [&]() {
                static auto cliOptB = clice::Argument {
                    .parent = &cliOptA,
                    .args   = "--opt",
                    .value  = std::string{},
                };
            }
        };


        auto args = std::vector<std::string_view>{"app", "--load", "--parent", "--opt", "blub"};
        auto unprocessed = std::vector<std::string_view>{};
        unprocessed.emplace_back("app");
        clice::parse(args, /*.allowDashCombi=*/false, &unprocessed);
        CHECK(cliOptA);
        CHECK(cliOptLoad);
    }

}
