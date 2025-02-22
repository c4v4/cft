// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS

#include <doctest/doctest.h>

#include "utils/custom_types.hpp"

// NOTE1: DO NOT MIX DIFFERENT CUSTOM TYPES IN THE SAME BINARY (I.E.: DO NOT LINK TOGETHER
//        TRANSLATION UNITS THAT USE DIFFERENT CUSTOM TYPES)
// NOTE2: This is an advanced feature that requires a good understanding of the C++ type system.


// This test file serves as an example of how we can customize the basic types (cidx_t, ridx_t, and
// real_t) used in the algorithm. We can replace these types with more advanced ones that provide
// additional features, such as preventing implicit casts or applying custom checks during
// mathematical operations. However, it is important to note that these custom types must still be
// implicitly convertible to the native type they represent, in order to allow conversions to array
// indexes, for example.

// To demonstrate how to customize basic types, we provide an example called `TaggedScalar`.
// `TaggedScalar` wraps a native type and it is uniquelly identified by a second template parameter
// called "TAG".
// It prohibits implicit conversions between a `TaggedScalar` and any other type, including other
// `TaggedScalar` with a different tag. Comparison and basic operations with the corresponding
// native type are also deleted. The only implicit conversion allowed is to the respective native
// type for compatibility with existing functions. Even constructing a `TaggedScalar` from its
// native type must be explicit.
#include "TaggedScalar.hpp"

// Before including `cft.hpp`, we need to define macros for the custom types we want to use in order
// to customize the entire algorithm. In this case, we are defining all three types.
#define CFT_CIDX_TYPE ext::TaggedScalar<int32_t, struct COL>
#define CFT_RIDX_TYPE ext::TaggedScalar<int16_t, struct ROW>
#define CFT_REAL_TYPE ext::TaggedScalar<float, struct REAL>

// In order to enable operations like printing, parsing from files, random value generation, and
// other low-level operations that might be defined only for the C++ numeric types, we need to
// define the conversion to the corresponding C++ native type. This can be achieved by partially
// specializing the `native` template struct. Since we are using the same template (`TaggedScalar`)
// for cidx_t, ridx_t, and real_t, we can define a single partial specialization that matches any
// instantiation of `TaggedScalar`.
namespace cft {
template <typename T, typename TAG>
struct native<ext::TaggedScalar<T, TAG>> {
    using type = T;
};
}  // namespace cft

// Now we can include the CFT headers, which will automatically adapt to our custom type
// definitions.
#include "algorithms/Refinement.hpp"
#include "core/cft.hpp"
#include "test_utils.hpp"

namespace cft {

TEST_CASE("Custom types, whole algorithm run test") {
    auto env       = Environment();
    env.time_limit = 10.0;
    env.heur_iters = 100;
    env.verbose    = 2;
    auto init_sol  = Solution();
    init_sol.idxs  = std::vector<cidx_t>{0_C, 1_C, 2_C, 3_C, 4_C, 5_C, 6_C, 7_C, 8_C, 9_C};
    init_sol.cost  = 1000.0_F;

    for (int n = 0; n < 20; ++n) {
        auto inst = make_easy_inst(n, 100_C);
        auto res  = run(env, inst, init_sol);
        CHECK((res.sol.cost <= 1000.0_F));  // Trivial bad solution has 1000 cost
        CHECK((res.sol.cost >= as_real(res.sol.idxs.size())));  // Min col cost is 1.0
        if (abs(res.sol.cost - 1000.0_F) < 1e-6_F)
            DOCTEST_CHECK_EQ(res.sol.idxs, init_sol.idxs);
        CFT_IF_DEBUG(CHECK_NOTHROW(check_inst_solution(inst, res.sol)));
    }
}

}  // namespace cft
