// SPDX-FileCopyrightText: 2025 Simon Gene Gottlieb
// SPDX-License-Identifier: CC0-1.0
#include <clice/clice.h>

#include <catch2/catch_all.hpp>

template <typename T>
concept dereferencable = requires(T t) {
    { *t };
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

    SECTION("splitting arguments") {
        auto cliOpt1 = clice::Argument{ .args   = "--opt1",
                                        .value  = std::vector<int>{}};


        auto args = std::vector<std::string_view>{"app", "--opt1", "-15"};
        clice::parse(args, /*.allowDashCombi=*/true);
        CHECK(cliOpt1);
        CHECK(*cliOpt1 == std::vector<int>{});
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


}
