
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