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
    auto inst = Instance();
    REQUIRE_NOTHROW(inst = parse_scp_instance("../instances/scp/scp41.txt"));

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
    auto inst = Instance();
    REQUIRE_NOTHROW(inst = parse_rail_instance("../instances/rail/rail507"));

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

TEST_CASE("test_parse_mps_instance") {
    auto inst = Instance();
    REQUIRE_NOTHROW(inst = parse_mps_instance("../instances/mps/ramos3.mps"));

    REQUIRE(rsize(inst.rows) == 2187);
    REQUIRE(csize(inst.cols) == 2187);

    REQUIRE(inst.cols[0].size() == 15);
    REQUIRE_THAT(
        local::span_to_vector<ridx_t>(inst.cols[0]),
        Catch::Matchers::UnorderedEquals(
            std::vector<ridx_t>{0, 9, 10, 11, 12, 15, 18, 36, 63, 90, 171, 252, 495, 738, 1467}));

    REQUIRE(csize(inst.cols) == csize(inst.costs));
    REQUIRE(std::fabs(inst.costs[0] - 1.0) < 0.01);

    REQUIRE(csize(inst.cols) == csize(inst.solcosts));
    for (real_t c : inst.solcosts)
        REQUIRE(std::fabs(c - limits<real_t>::max()) < 0.01);
}

}  // namespace cft