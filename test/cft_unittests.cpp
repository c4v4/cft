// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS

#include <doctest/doctest.h>

#include "core/cft.hpp"

namespace cft {

TEST_CASE("Test CidxAndCost struct") {
    auto c = CidxAndCost{1_C, 2.5_F};
    CHECK(c.idx == 1_C);
    CHECK(c.cost == 2.5_F);
}

TEST_CASE("Test Solution struct") {
    auto s = Solution{{}, 0.0_F};
    CHECK(s.idxs.empty());
    CHECK(s.cost == 0.0_F);
}

TEST_CASE("Test DualState struct") {
    auto d = DualState{{}, 0.0_F};
    CHECK(d.mults.empty());
    CHECK(d.lb == 0.0_F);
}

TEST_CASE("Test CftResult struct") {
    auto r = CftResult{{{}, 1.2_F}, {{}, 2.3_F}};
    CHECK(r.sol.idxs.empty());
    CHECK(r.sol.cost == 1.2_F);
    CHECK(r.dual.mults.empty());
    CHECK(r.dual.lb == 2.3_F);
}

TEST_CASE("Test as_cidx function") {
    int    i = 10;
    cidx_t c = as_cidx(i);
    CHECK(c == cidx_t{10});
}

TEST_CASE("Test as_ridx function") {
    int    i = 5;
    ridx_t r = as_ridx(i);
    CHECK(r == ridx_t{5});
}

TEST_CASE("Test as_real function") {
    double i = 5.55;
    real_t r = as_real(i);
    CHECK(r == real_t{5.55});
}

TEST_CASE("Test user-defined literals") {
    cidx_t c = 5_C;
    ridx_t r = 10_R;
    real_t f = 2.5_F;
    CHECK(c == cidx_t{5});
    CHECK(r == ridx_t{10});
    CHECK(f == real_t{2.5});
}

TEST_CASE("Test csize function") {
    auto   vec  = std::vector<int>{1, 2, 3};
    cidx_t size = csize(vec);
    CHECK(size == 3_C);
}

TEST_CASE("Test rsize function") {
    auto   vec  = std::vector<int>{1, 2, 3};
    ridx_t size = rsize(vec);
    CHECK(size == 3_R);
}

#ifndef NDEBUG

TEST_CASE("Test as_cidx function failure ") {
    if (static_cast<uint64_t>(limits<cidx_t>::max()) < limits<uint64_t>::max())
        CHECK_THROWS_AS(as_cidx(limits<uint64_t>::max()), std::runtime_error);
    if (!std::is_signed<native_t<cidx_t>>::value)
        CHECK_THROWS_AS(as_cidx(-1), std::runtime_error);
}

TEST_CASE("Test as_ridx function failure ") {
    if (static_cast<uint64_t>(limits<ridx_t>::max()) < limits<uint64_t>::max())
        CHECK_THROWS_AS(as_ridx(limits<uint64_t>::max()), std::runtime_error);
    if (!std::is_signed<native_t<ridx_t>>::value)
        CHECK_THROWS_AS(as_ridx(-1), std::runtime_error);
}

TEST_CASE("Test as_real function failure ") {
    if (static_cast<long double>(limits<real_t>::max()) < limits<long double>::max())
        CHECK_THROWS_AS(1e+4932_F, std::runtime_error);
    if (static_cast<long double>(limits<real_t>::max()) < limits<long double>::max())
        CHECK_THROWS_AS(as_real(1e+4932L), std::runtime_error);
}

#endif

}  // namespace cft