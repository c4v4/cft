#ifndef CAV_INCLUDE_EXPECTED_HPPCFT_
#define CAV_INCLUDE_EXPECTED_HPPCFT_

#include <cassert>
#include <type_traits>

#include "core/cft.hpp"

namespace cft {

template <typename T, typename E>
struct Expected {
    static_assert(std::is_enum<E>::value, "E must be an enum class");
    static_assert(!std::is_same<T, E>::value, "T and E must be different types");
    using value_type = T;
    using error_type = E;

    union {
        error_type error;
        value_type value;
    };

    bool has_value;

    Expected() = delete;  // Still aggregate in c++11

    ~Expected() {
        if (has_value)
            value.~value_type();
    }
};

template <typename T, typename E>
CFT_NODISCARD static Expected<T, E> make_expected(E err) {
    return Expected<T, E>{err, false};
}

template <typename T, typename E>
CFT_NODISCARD static Expected<T, E> make_expected(T&& val) {
    auto res      = Expected<T, E>{};
    res.value     = std::move(val);
    res.has_value = true;
    return res;
}

template <typename T, typename E>
CFT_NODISCARD static Expected<T, E> make_expected(T const& val) {
    auto res      = Expected<T, E>{};
    res.value     = val;
    res.has_value = true;
    return res;
}

}  // namespace cft

#endif /* CAV_INCLUDE_EXPECTED_HPPCFT_ */
