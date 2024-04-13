// Copyright (c) 2024 Francesco Cavaliere
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.


#include <catch2/catch.hpp>
#include <thread>

#include "../src/utils/Chrono.hpp"

namespace cft {
TEST_CASE("Chrono restart returns elapsed time in specified unit", "[Chrono]") {
    Chrono<> chrono;
    std::this_thread::sleep_for(msec(100));
    REQUIRE(chrono.restart<msec>() >= 100);
}

TEST_CASE("Chrono elapsed returns elapsed time in specified unit", "[Chrono]") {
    Chrono<> chrono;
    std::this_thread::sleep_for(msec(200));
    REQUIRE(chrono.elapsed<sec>() >= 0.2);
}

TEST_CASE("Chrono restart reset timer", "[Chrono]") {
    Chrono<> chrono;
    std::this_thread::sleep_for(msec(100));
    REQUIRE(chrono.restart<msec>() >= 100);
    std::this_thread::sleep_for(msec(100));
    REQUIRE(chrono.restart<msec>() >= 100);
}

TEST_CASE("Chrono elapsed return monotonic timestamps", "[Chrono]") {
    Chrono<> chrono;
    auto     t1 = chrono.elapsed<msec>();
    auto     t2 = chrono.elapsed<msec>();
    std::this_thread::sleep_for(msec(100));
    auto t3 = chrono.elapsed<msec>();
    REQUIRE(t2 >= t1);
    REQUIRE(t3 >= t2 + 100);
}


}  // namespace cft