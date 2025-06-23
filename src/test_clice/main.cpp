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


            auto args = std::vector<std::string_view>{"app", "--opt1", "3", "5", "7", "100", "1000", "2", "7"};
            clice::parse(args);
            CHECK(cliOpt1);
            CHECK(*cliOpt1 == std::vector<int>{3, 5, 7, 100, 1000, 2, 7});
        }

    }
}
