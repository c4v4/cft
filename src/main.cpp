#include <fmt/core.h>

#include <cstdlib>

#include "Expected.hpp"
#include "Instance.hpp"
#include "cft.hpp"

enum class ERR { LESS1, INVALID, OUT_OF_RANGE };

struct S {
    cft::cidx_t x;

    S(cft::cidx_t y)
        : x(y) {
        fmt::print("S({})\n", y);
    }

    ~S() {
        fmt::print("~S({})\n", x);
    }
};

cft::Expected<S, ERR> init_if_ge_1(std::string const& arg) noexcept {
    try {
        cft::cidx_t val = std::stoul(arg);
        if (val >= 1)
            return cft::make_expected<S, ERR>(S(val));
        return cft::make_expected<S, ERR>(ERR::LESS1);

    } catch (std::invalid_argument& e) {
        return cft::make_expected<S, ERR>(ERR::INVALID);

    } catch (std::out_of_range& e) {
        return cft::make_expected<S, ERR>(ERR::OUT_OF_RANGE);
    }
}

int main(int argc, char const** argv) {

    auto args      = cft::make_span(argv, argc);
    auto maybe_int = init_if_ge_1(args[1]);

    if (!maybe_int.has_value)
        switch (maybe_int.error) {
        case ERR::LESS1:
            fmt::print("The first argument is less than 1\n");
            return static_cast<int>(maybe_int.error);
        case ERR::INVALID:
            fmt::print("The first argument is not a valid integer\n");
            return static_cast<int>(maybe_int.error);
        case ERR::OUT_OF_RANGE:
            fmt::print("The first argument is out of range\n");
            return static_cast<int>(maybe_int.error);
        }

    cft::cidx_t idx = maybe_int.value.x;
    fmt::print("The first argument is {}\n", idx);

    return EXIT_SUCCESS;
}
