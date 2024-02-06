#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include "../include/parsing.hpp"

namespace cft {
namespace {
    template <class T>
    std::vector<T> span_to_vector(Span<T*> const& span) {
        std::vector<T> vec;
        vec.reserve(span.size());
        for (auto const elem : span)
            vec.push_back(elem);
        return vec;
    }
}

TEST_CASE("test_parse_scp_instance") {
    auto inst = parse_scp_instance("../instances/scp/scp41.txt");

    REQUIRE(inst.nrows == 200);
    REQUIRE(inst.cols.size() == 1000);

    REQUIRE(inst.cols[0].size() == 8);
    REQUIRE_THAT(
        span_to_vector<ridx_t>(inst.cols[0]),
        Catch::Matchers::UnorderedEquals(std::vector<ridx_t>{17, 31, 74, 75, 106, 189, 195, 198}));

    REQUIRE(inst.cols.size() == inst.costs.size());
    REQUIRE(std::fabs(inst.costs[0] - 1.0) < 0.01);

    REQUIRE(inst.cols.size() == inst.solcosts.size());
    for (real_t c : inst.solcosts)
        REQUIRE(std::fabs(c - limits<real_t>::max()) < 0.01);

    REQUIRE(inst.warmstart.empty());
}

TEST_CASE("test_parse_rail_instance") {
    auto inst = parse_rail_instance("../instances/rail/rail507");

    REQUIRE(inst.nrows == 507);
    REQUIRE(inst.cols.size() == 63009);

    REQUIRE(inst.cols[0].size() == 7);
    REQUIRE_THAT(
        span_to_vector<ridx_t>(inst.cols[0]),
        Catch::Matchers::UnorderedEquals(std::vector<ridx_t>{41, 42, 43, 317, 318, 421, 422}));

    REQUIRE(inst.cols.size() == inst.costs.size());
    REQUIRE(std::fabs(inst.costs[0] - 2.0) < 0.01);

    REQUIRE(inst.cols.size() == inst.solcosts.size());
    for (real_t c : inst.solcosts)
        REQUIRE(std::fabs(c - limits<real_t>::max()) < 0.01);

    REQUIRE(inst.warmstart.empty());
}

TEST_CASE("test_parse_cvrp_instance") {
    auto inst = parse_cvrp_instance("../instances/cvrp/X-n819-k171.vrp_seed-0_z159081.000000.scp");

    REQUIRE(inst.nrows == 818);
    REQUIRE(inst.cols.size() == 121467);

    REQUIRE(inst.cols[0].size() == 4);
    REQUIRE_THAT(span_to_vector<ridx_t>(inst.cols[0]),
                 Catch::Matchers::UnorderedEquals(std::vector<ridx_t>{27, 329, 384, 650}));

    REQUIRE(inst.cols.size() == inst.costs.size());
    REQUIRE(std::fabs(inst.costs[0] - 871.0) < 0.01);

    REQUIRE(inst.cols.size() == inst.solcosts.size());
    REQUIRE(std::fabs(inst.solcosts[0] - 703.0) < 0.01);

    REQUIRE(inst.warmstart.empty());
}

}