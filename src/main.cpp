#include <fmt/core.h>

#include "Expected.hpp"
#include "Instance.hpp"
#include "cft.hpp"

enum class ERR { LESS1, INVALID, OUT_OF_RANGE };

struct S {
    int x;

    S(int y)
        : x(y) {
        fmt::print("S({})\n", y);
    }

    ~S() {
        fmt::print("~S({})\n", x);
    }
};

cft::Expected<S, ERR> init_if_ge_1(std::string arg) noexcept {
    try {
        int val = std::stoi(arg);
        if (val >= 1)
            return cft::make_expected<S, ERR>(S{val});
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

    if (maybe_int.has_value)
        fmt::print("The first argument is {}\n", maybe_int.value.x);

    else
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
    return 0;
}
