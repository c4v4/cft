#ifndef SCP_INCLUDE_CLIARGS_HPP_
#define SCP_INCLUDE_CLIARGS_HPP_

#include <fmt/core.h>

#include <optional>
#include <string>

// List of command line tokens.
constexpr char kTokenSeed[] = "--seed";
constexpr char kTokenParser[] = "--parser";

// Parses command line arguments and makes them accessible.
class CliArgs {
public:
    enum class ParserType { CVRP, RAILS, SCP };

    static std::optional<CliArgs> parse(int argc, char** argv) {
        if (argc < 2) {
            // At least the input path is required.
            return std::nullopt;
        }
        return CliArgs(argc, argv);
    }

    const std::string& path() const { return path_; }

    unsigned long seed() const { return seed_; }

    ParserType parser_type() const { return parser_type_; }

private:
    CliArgs(int argc, char** argv) : path_(argv[1]) {
        for (int i = 2; i < argc; i += 2) {
            if (i + 1 >= argc) { fmt::print("Missing value for {}.\n", argv[i]); }

            const std::string key = std::string(argv[i]);
            const std::string value = std::string(argv[i + 1]);

            if (key == kTokenSeed) {
                seed_ = std::stoul(value);
            } else if (key == kTokenParser) {
                if (value == "CVRP") {
                    parser_type_ = ParserType::CVRP;
                } else if (value == "RAILS") {
                    parser_type_ = ParserType::RAILS;
                } else if (value == "SCP") {
                    parser_type_ = ParserType::SCP;
                } else {
                    fmt::print("Warning: Ignored unknown parser type.\n");
                }
            } else {
                fmt::print("Warning: Ignored unknown argument {}.\n", key);
            }
        }
    }

    // Instance path.
    const std::string path_;

    // Random seed.
    unsigned long seed_ = 0;

    // Parser type.
    ParserType parser_type_ = ParserType::CVRP;
};

#endif