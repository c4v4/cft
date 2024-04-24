// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
 
#include <doctest/doctest.h>
#include <thread>

#include "../src/utils/Chrono.hpp"

namespace cft {
TEST_CASE("Chrono restart returns elapsed time in specified unit") {
    Chrono<> chrono;
    std::this_thread::sleep_for(msec(100));
    CHECK(chrono.restart<msec>() >= 100);
}

TEST_CASE("Chrono elapsed returns elapsed time in specified unit") {
    Chrono<> chrono;
    std::this_thread::sleep_for(msec(200));
    CHECK(chrono.elapsed<sec>() >= 0.2);
}

TEST_CASE("Chrono restart reset timer") {
    Chrono<> chrono;
    std::this_thread::sleep_for(msec(100));
    CHECK(chrono.restart<msec>() >= 100);
    std::this_thread::sleep_for(msec(100));
    CHECK(chrono.restart<msec>() >= 100);
}

TEST_CASE("Chrono elapsed return monotonic timestamps") {
    Chrono<> chrono;
    auto     t1 = chrono.elapsed<msec>();
    auto     t2 = chrono.elapsed<msec>();
    std::this_thread::sleep_for(msec(100));
    auto t3 = chrono.elapsed<msec>();
    CHECK(t2 >= t1);
    CHECK(t3 >= t2 + 100);
}


}  // namespace cft