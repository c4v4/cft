#include <catch2/catch.hpp>

#include "core/parsing.hpp"

namespace cft {
namespace local { namespace {
    template <class T>
    std::vector<T> span_to_vector(Span<T*> const& span) {
        std::vector<T> vec;
        vec.reserve(span.size());
        for (auto const elem : span)
            vec.push_back(elem);
        return vec;
    }
}  // namespace
}  // namespace local

TEST_CASE("test_parse_scp_instance") {
    auto inst = parse_scp_instance("../instances/scp/scp41.txt");

    REQUIRE(rsize(inst.rows) == 200);
    REQUIRE(csize(inst.cols) == 1000);

    REQUIRE(inst.cols[0].size() == 8);
    REQUIRE_THAT(
        local::span_to_vector<ridx_t>(inst.cols[0]),
        Catch::Matchers::UnorderedEquals(std::vector<ridx_t>{17, 31, 74, 75, 106, 189, 195, 198}));

    REQUIRE(csize(inst.cols) == csize(inst.costs));
    REQUIRE(std::fabs(inst.costs[0] - 1.0) < 0.01);

    REQUIRE(csize(inst.cols) == csize(inst.solcosts));
    for (real_t c : inst.solcosts)
        REQUIRE(std::fabs(c - limits<real_t>::max()) < 0.01);
}

TEST_CASE("test_parse_rail_instance") {
    auto inst = parse_rail_instance("../instances/rail/rail507");

    REQUIRE(rsize(inst.rows) == 507);
    REQUIRE(csize(inst.cols) == 63009);

    REQUIRE(inst.cols[0].size() == 7);
    REQUIRE_THAT(
        local::span_to_vector<ridx_t>(inst.cols[0]),
        Catch::Matchers::UnorderedEquals(std::vector<ridx_t>{41, 42, 43, 317, 318, 421, 422}));

    REQUIRE(csize(inst.cols) == csize(inst.costs));
    REQUIRE(std::fabs(inst.costs[0] - 2.0) < 0.01);

    REQUIRE(csize(inst.cols) == csize(inst.solcosts));
    for (real_t c : inst.solcosts)
        REQUIRE(std::fabs(c - limits<real_t>::max()) < 0.01);
}

TEST_CASE("test_parse_cvrp_instance") {
    auto  fdata = parse_cvrp_instance("../instances/cvrp/X-n536-k96_z95480_cplex95479.scp");
    auto& inst  = fdata.inst;
    REQUIRE(rsize(inst.rows) == 535);
    REQUIRE(csize(inst.cols) == 127262);

    REQUIRE(inst.cols[0].size() == 1);
    REQUIRE(inst.cols[1].size() == 4);
    REQUIRE_THAT(local::span_to_vector<ridx_t>(inst.cols[1]),
                 Catch::Matchers::UnorderedEquals(std::vector<ridx_t>{486, 526, 320, 239}));

    REQUIRE(csize(inst.cols) == csize(inst.costs));
    REQUIRE(std::fabs(inst.costs[1] - 787.0) < 0.01);

    REQUIRE(csize(inst.cols) == csize(inst.solcosts));
    REQUIRE(std::fabs(inst.solcosts[0] - 96162.0) < 0.01);

    REQUIRE(fdata.warmstart.empty());
}

}  // namespace cft