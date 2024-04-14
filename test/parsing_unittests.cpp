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
    REQUIRE(std::fabs(inst.costs[0] - 1.0_F) < 0.01_F);
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
    REQUIRE(std::fabs(inst.costs[0] - 2.0_F) < 0.01_F);
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
    REQUIRE(std::fabs(inst.costs[1] - 787.0_F) < 0.01_F);

    REQUIRE(!fdata.init_sol.idxs.empty());
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
    REQUIRE(std::fabs(inst.costs[0] - 1.0_F) < 0.01_F);
}

TEST_CASE("test wrong parser") {
    REQUIRE_THROWS(parse_scp_instance("../instances/rail/rail507"));
    REQUIRE_THROWS(parse_scp_instance("../instances/cvrp/X-n536-k96_z95480_cplex95479.scp"));
    REQUIRE_THROWS(parse_scp_instance("../instances/mps/ramos3.mps"));

    REQUIRE_THROWS(parse_rail_instance("../instances/scp/scp41.txt"));
    REQUIRE_THROWS(parse_rail_instance("../instances/cvrp/X-n536-k96_z95480_cplex95479.scp"));
    REQUIRE_THROWS(parse_rail_instance("../instances/mps/ramos3.mps"));

    REQUIRE_THROWS(parse_cvrp_instance("../instances/scp/scp41.txt"));
    REQUIRE_THROWS(parse_cvrp_instance("../instances/rail/rail507"));
    REQUIRE_THROWS(parse_cvrp_instance("../instances/mps/ramos3.mps"));

    REQUIRE_THROWS(parse_mps_instance("../instances/scp/scp41.txt"));
    REQUIRE_THROWS(parse_mps_instance("../instances/rail/rail507"));
    REQUIRE_THROWS(parse_mps_instance("../instances/cvrp/X-n536-k96_z95480_cplex95479.scp"));
}

TEST_CASE("Test parse_solution", "[parsing]") {
    SECTION("write_solution and read_solution") {
        std::string   path = "test_solution.txt";
        cft::Solution wsol;
        wsol.cost = 101.5;
        wsol.idxs = {1, 2, 3, 4, 5, 6, 7, 8, 9};

        cft::write_solution(path, wsol);
        cft::Solution sol = cft::parse_solution(path);

        REQUIRE(sol.cost == 101.5);
        REQUIRE(sol.idxs == std::vector<cft::cidx_t>{1, 2, 3, 4, 5, 6, 7, 8, 9});

        std::remove(path.c_str());
    }

    SECTION("Read solution: Invalid solution file") {
        std::string   path = "test_solution.txt";
        std::ofstream file(path);
        REQUIRE(file.is_open());
        fmt::print(file, "101.5 1 2 3# 4 5 Z 7 8 9\n");
        file.close();

        REQUIRE_THROWS(cft::parse_solution(path));
        std::remove(path.c_str());
    }

    SECTION("Write solution: Invalid file path") {
        std::string   path = "/invalid/path/test_solution.txt";
        cft::Solution sol;
        sol.cost = 10.5;
        sol.idxs = {1, 2, 3};

        REQUIRE_THROWS_AS(cft::write_solution(path, sol), std::exception);
    }
}


}  // namespace cft