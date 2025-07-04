// SPDX-FileCopyrightText: 2023 Gottlieb+Freitag <info@gottliebtfreitag.de>
// SPDX-License-Identifier: CC0-1.0
#include <clice/clice.h>

#include <iostream>

namespace {
auto cliHelp = clice::Argument {
    .args   = "--help",
    .desc   = "prints the help page",
    .cb     = [](){ std::cout << clice::generateHelp(); exit(0); },
    .tags   = {"ignore-required"},
};


void generateCWL(std::vector<std::string> const& commands) {
    auto toolInfo = clice::generateCWL(commands);
    toolInfo.metaInfo = {
        .version        = "0.0.1",
        .name           = "clice-demo",
        .docurl         = "example.cpp",
        .category       = "demo",
        .description    = "demonstration tool of clice options",
        .executableName = clice::argv0,
};
    std::cout << convertToCWL(toolInfo) << "\n";
    exit(0);
}

auto cliCWL = clice::Argument {
    .args   = "--cwl-description",
    .id     = "<command>...",
    .desc   = "prints a cwl-description of a certain subcommand",
    .value  = std::vector<std::string>{},
    .cb     = generateCWL,
    .tags   = {"ignore-required"},
};

auto cliAdd = clice::Argument {
    .args    = "add",
    .symlink = true, // allows access via symlink to clice-demo-add
    .desc    = "adds some stuff",
    .cb      = []() { std::cout << "command add executed\n"; }
};
auto cliVerbose = clice::Argument {
    .parent = &cliAdd,
    .args   = {"-v", "--verbose"},
    .desc   = "detailed description of what is happening",
};
auto cliNbr = clice::Argument {
    .args   = "--nbr",
    .id     = "<nbr>",
    .desc   = "setting some nbr",
    .value  = 5,
};
auto cliInts = clice::Argument {
    .args   = "--ints",
    .id     = "<ints>...",
    .desc   = "a list of numbers",
    .value  = std::vector<int>{},
};
auto cliAuto = clice::Argument {
    .args   = "--auto",
    .id     = "<int>",
    .desc   = "value depending on cliNbr +1",
    .value  = []() { return *cliNbr+1; },
    .cb     = [](auto const&) {
        std::cout << "done\n";
    }
};
auto cliAuto2 = clice::Argument {
    .args   = "--auto2",
    .id     = "<int>",
    .desc   = "same as --auto, but with lower priority",
    .value  = []() { return *cliNbr+1; },
    .cb     = [](auto const&) {
        std::cout << "auto2 done, after auto\n";
    },
    .cb_priority = 200,
};

auto cliAuto3 = clice::Argument {
    .args   = "--auto3",
    .id     = "<int>",
    .desc   = "same as --auto, but with higher priority",
    .value  = []() { return *cliNbr+1; },
    .cb     = [](auto const&) {
        std::cout << "auto3 done, before auto\n";
    },
    .cb_priority = 50,
};

auto cliBasicTypes = clice::Argument {
    .args = "basic",
    .desc = "basic values like bool, ints and doubles",
};
auto cliBTFlag = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--flag",
    .desc   = "flag value",
};
auto cliBTBool = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--bool",
    .id     = "<bool>",
    .desc   = "bool value",
    .value  = false,
};
auto cliBTUInt8 = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--uint8",
    .id     = "<int>",
    .desc   = "uint8 value",
    .value  = uint8_t{},
};
auto cliBTInt8 = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--int8",
    .id     = "<int>",
    .desc   = "int8 value",
    .value  = int8_t{},
};
auto cliBTUInt16 = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--uint16",
    .id     = "<int>",
    .desc   = "uint16 value",
    .value  = uint16_t{},
};
auto cliBTInt16 = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--int16",
    .id     = "<int>",
    .desc   = "int16 value",
    .value  = int16_t{},
};
auto cliBTUInt32 = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--uint32",
    .id     = "<int>",
    .desc   = "uint32 value",
    .value  = uint32_t{},
};
auto cliBTInt32 = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--int32",
    .id     = "<int>",
    .desc   = "int32 value",
    .value  = int32_t{},
};
auto cliBTUInt64 = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--uint64",
    .id     = "<int>",
    .desc   = "uint64 value",
    .value  = uint64_t{},
};
auto cliBTInt64 = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--int64",
    .id     = "<int>",
    .desc   = "int64 value",
    .value  = int64_t{},
};
auto cliBTChar = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--char",
    .id     = "<char>",
    .desc   = "char value",
    .value  = char{'A'},
};
auto cliBTString = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--string",
    .id     = "<word>",
    .desc   = "string value",
    .value  = std::string{},
};
auto cliBTMappedBool = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--mapped_bool",
    .id     = "<qual>",
    .desc   = R"(takes "good" and "bad" as input)",
    .value  = bool{},
    .mapping = {{{"good", true}, {"bad", false}}},
};
auto cliBTSuffix = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--timeout",
    .desc   = "a timeout in seconds",
    .value  = double{4},
    .suffix = "s",
};

enum class MyEnumType {Foo, Bar};
auto cliBTEnum = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--enum",
    .desc   = "takes \"foo\" and \"bar\" as input",
    .value  = MyEnumType::Foo,
    .mapping = {{{"foo", MyEnumType::Foo}, {"bar", MyEnumType::Bar}}},
};

auto cliBTInputPath = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--input",
    .desc   = "input path",
    .value  = std::filesystem::path{},
};
auto cliBTOutputPath = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--output",
    .desc   = "output path",
    .value  = std::filesystem::path{},
};
auto cliBTInputFile = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--file",
    .desc   = "file path",
    .value  = std::filesystem::path{},
    .tags   = {"short: FILE"},
};

auto cliBTVectorInt = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = "--vector_int",
    .desc   = "vector of ints",
    .value  = std::vector<int>{},
};

auto cliBTOption = clice::Argument {
    .parent = &cliBasicTypes,
    .args   = {"--option1", "--option2"},
    .desc   = "option can be set in two different ways",
    .value  = int{},
};

auto cliSingleDash = clice::Argument {
    .args = "single_dash",
    .desc   = "single dash grouping",
};
auto cliSD_a = clice::Argument {
    .parent = &cliSingleDash,
    .args   = "-a",
    .desc   = "some a",
};
auto cliSD_b = clice::Argument {
    .parent = &cliSingleDash,
    .args   = "-b",
    .desc   = "some b",
};

auto cliRequired = clice::Argument {
    .args = "required_child",
    .desc   = "this option has a required child option",
};

auto cliRequiredOpt1 = clice::Argument {
    .parent = &cliRequired,
    .args   = "--opt1",
    .desc   = "this option is required",
    .value  = std::string{},
    .tags   = {"required"},
};
auto cliRequiredOpt2 = clice::Argument {
    .parent = &cliRequired,
    .args  = "--opt2",
    .desc  = "this option is not required",
    .value = std::string{},
};

auto cliAlwaysRequired = clice::Argument {
    .args  = "always_required",
    .desc  = "this option is always required",
    .value = size_t{},
    .tags  = {"required"},
};

auto cliCompletion = clice::Argument {
    .args = "completion",
    .desc = "section allowing to try different completion types",
};
auto cliCompletionStaticString = clice::Argument {
    .parent = &cliCompletion,
    .args   = "--str1",
    .desc   = "static completion",
    .value = std::string{},
    .completion = []() -> std::vector<std::string> { return {"foo", "bar", "faa"}; },
};

auto cliSingleTrailingArgument = clice::Argument {
    .id = "<input1>",
    .desc   = "single trailing argument",
    .value = std::string{},
};

auto cliSingleTrailingArgumentPos2 = clice::Argument {
    .parent = &cliSingleTrailingArgument,
    .id = "<input2>",
    .desc   = "single trailing argument at position 2",
    .value = std::string{},
    .tags = {"required"}
};

auto cliMultiTrailingArguments = clice::Argument {
    .id    = "<inputs>...",
    .desc  = "multiple trailing arguments",
    .value = std::vector<std::string>{},
};

auto cliEnvVariable = clice::Argument {
    .args = {"--variable"},
    .env   = {"VARIABLE"},
    .id    = "Name",
    .desc  = "a variable that can also be set via environment variable",
    .value = std::string{}
};

auto cliEnvVariable2 = clice::Argument {
    .env   = {"VARIABLE2"},
    .id    = "Name",
    .desc  = "a variable that can also be set as an environment variable",
    .value = std::string{}
};
}

int main(int argc, char** argv) {
    try {
        if (auto failed = clice::parse(argc, argv, /*.allowDashCombi=*/true); failed) {
            std::cerr << "parsing failed: " << *failed << "\n";
            return 1;
        }

        std::cout << cliAdd << "\n";
        std::cout << "  " << cliVerbose << "\n";
        std::cout << cliHelp << "\n";
        std::cout << *cliNbr << "\n";
        std::cout << cliInts << "\n";
        for (auto i : *cliInts) {
            std::cout << " - " << i << "\n";
        }
        std::cout << "auto: " << *cliAuto << " " << *cliAuto2 << " " << *cliAuto3 << "\n";

        std::cout << "\n\nbasic " << cliBasicTypes << "\n";
        std::cout << "  --flag " << cliBTFlag << " " << "\n";
        std::cout << "  --bool " << cliBTBool << " " << *cliBTBool << "\n";
        std::cout << "  --uint8 " << cliBTUInt8 << " " << (int)*cliBTUInt8 << "\n";
        std::cout << "  --int8 " << cliBTInt8 << " " << (int)*cliBTInt8 << "\n";
        std::cout << "  --uint16 " << cliBTUInt16 << " " << *cliBTUInt16 << "\n";
        std::cout << "  --int16 " << cliBTInt16 << " " << *cliBTInt16 << "\n";
        std::cout << "  --uint32 " << cliBTUInt32 << " " << *cliBTUInt32 << "\n";
        std::cout << "  --int32 " << cliBTInt32 << " " << *cliBTInt32 << "\n";
        std::cout << "  --uint64 " << cliBTUInt64 << " " << *cliBTUInt64 << "\n";
        std::cout << "  --int64 " << cliBTInt64 << " " << *cliBTInt64 << "\n";
        std::cout << "  --char " << cliBTChar << " " << *cliBTChar << "\n";
        std::cout << "  --string " << cliBTString << " " << *cliBTString << " has len " << cliBTString->size() << "\n";
        std::cout << "  --mapped_bool " << cliBTMappedBool << " " << *cliBTMappedBool << "\n";
        std::cout << "  --timeout " << cliBTSuffix << " " << *cliBTSuffix << "\n";
        std::cout << "  --enum " << cliBTEnum << " " << (*cliBTEnum==MyEnumType::Foo?"foo":"bar") << "\n";
        std::cout << "  --input " << cliBTInputPath << " " << *cliBTInputPath << "\n";
        std::cout << "  --output " << cliBTOutputPath << " " << *cliBTOutputPath << "\n";
        std::cout << "  --vector_int " << cliBTVectorInt << " " << cliBTVectorInt->size() << "\n";
        std::cout << "  --option1, --option2 " << cliBTOption << " " << *cliBTOption << "\n";
        std::cout << "\n\nsingle dash " << cliSingleDash << "\n";
        std::cout << "  -a " << cliSD_a << "\n";
        std::cout << "  -b " << cliSD_b << "\n";
        for (auto x : *cliBTVectorInt) {
            std::cout << "    " << x << "\n";
        }
        std::cout << "\n\nrequired_child: " << cliRequired << "\n";
        std::cout << "    --opt1: " << cliRequiredOpt1 << " " << *cliRequiredOpt1 << "\n";
        std::cout << "    --opt2: " << cliRequiredOpt2 << " " << *cliRequiredOpt2 << "\n";

        std::cout << "\n\nalways_required: " << cliAlwaysRequired << "\n";

        std::cout << "\n\nsingle trailing argument: " << *cliSingleTrailingArgument << " " << *cliSingleTrailingArgumentPos2 << "\n";
        std::cout << "multiple trailing arguments: " << cliMultiTrailingArguments->size() << "\n";
        for (auto s : *cliMultiTrailingArguments) {
            std::cout << "  - " << s << "\n";
        }

        std::cout << "\n\nEnvironment VARIABLE: " << cliEnvVariable << " " << *cliEnvVariable << "\n";
        std::cout << "\n\nEnvironment VARIABLE: " << cliEnvVariable2 << " " << *cliEnvVariable2 << "\n";

    } catch (std::exception const& e) {
        std::cerr << "error: " << e.what() << "\n";
    }
    return 0;
}
