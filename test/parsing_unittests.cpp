// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <stdexcept>

#include "core/cft.hpp"
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
    REQUIRE_NOTHROW(inst = parse_scp_instance("../../instances/scp/scp41.txt"));

    CHECK(rsize(inst.rows) == 200_R);
    CHECK(csize(inst.cols) == 1000_C);

    CHECK(inst.cols[0].size() == 8);
    std::vector<ridx_t> sorted_cols = local::span_to_vector<ridx_t>(inst.cols[0]);
    std::sort(sorted_cols.begin(), sorted_cols.end());
    CHECK(sorted_cols == std::vector<ridx_t>{17_R, 31_R, 74_R, 75_R, 106_R, 189_R, 195_R, 198_R});

    CHECK(csize(inst.cols) == csize(inst.costs));
    CHECK(abs(inst.costs[0] - 1.0_F) < 0.01_F);
}

TEST_CASE("test_parse_rail_instance") {
    auto inst = Instance();
    REQUIRE_NOTHROW(inst = parse_rail_instance("../../instances/rail/rail507"));

    CHECK(rsize(inst.rows) == 507_R);
    CHECK(csize(inst.cols) == 63009_C);

    CHECK(inst.cols[0].size() == 7);
    std::sort(inst.cols[0].begin(), inst.cols[0].end());
    CHECK(local::span_to_vector<ridx_t>(inst.cols[0]) ==
          std::vector<ridx_t>{41_R, 42_R, 43_R, 317_R, 318_R, 421_R, 422_R});

    CHECK(csize(inst.cols) == csize(inst.costs));
    CHECK(abs(inst.costs[0] - 2.0_F) < 0.01_F);
}

TEST_CASE("test_parse_cvrp_instance") {
    auto  fdata = parse_cvrp_instance("../../instances/cvrp/X-n536-k96_z95480_cplex95479.scp");
    auto& inst  = fdata.inst;
    CHECK(rsize(inst.rows) == 535_R);
    CHECK(csize(inst.cols) == 127262_C);

    CHECK(rsize(inst.cols[0]) == 1_R);
    CHECK(rsize(inst.cols[1]) == 4_R);
    CHECK(local::span_to_vector<ridx_t>(inst.cols[1]) ==
          std::vector<ridx_t>{486_R, 526_R, 320_R, 239_R});

    CHECK(csize(inst.cols) == csize(inst.costs));
    CHECK(abs(inst.costs[1] - 787.0_F) < 0.01_F);

    CHECK(!fdata.init_sol.idxs.empty());
}

TEST_CASE("test_parse_mps_instance") {
    auto inst = Instance();
    REQUIRE_NOTHROW(inst = parse_mps_instance("../../instances/mps/ramos3.mps"));

    CHECK(rsize(inst.rows) == 2187_R);
    CHECK(csize(inst.cols) == 2187_C);

    CHECK(rsize(inst.cols[0]) == 15_R);
    CHECK(local::span_to_vector<ridx_t>(inst.cols[0]) == std::vector<ridx_t>{0_R,
                                                                             9_R,
                                                                             10_R,
                                                                             11_R,
                                                                             12_R,
                                                                             15_R,
                                                                             18_R,
                                                                             36_R,
                                                                             63_R,
                                                                             90_R,
                                                                             171_R,
                                                                             252_R,
                                                                             495_R,
                                                                             738_R,
                                                                             1467_R});

    CHECK(csize(inst.cols) == csize(inst.costs));
    CHECK(abs(inst.costs[0] - 1.0_F) < 0.01_F);
}

TEST_CASE("test wrong parser") {
    CHECK_THROWS(parse_scp_instance("../../instances/rail/rail507"));
    CHECK_THROWS(parse_scp_instance("../../instances/cvrp/X-n536-k96_z95480_cplex95479.scp"));
    CHECK_THROWS(parse_scp_instance("../../instances/mps/ramos3.mps"));

    CHECK_THROWS(parse_rail_instance("../../instances/scp/scp41.txt"));
    CHECK_THROWS(parse_rail_instance("../../instances/cvrp/X-n536-k96_z95480_cplex95479.scp"));
    CHECK_THROWS(parse_rail_instance("../../instances/mps/ramos3.mps"));

    CHECK_THROWS(parse_cvrp_instance("../../instances/scp/scp41.txt"));
    CHECK_THROWS(parse_cvrp_instance("../../instances/rail/rail507"));
    CHECK_THROWS(parse_cvrp_instance("../../instances/mps/ramos3.mps"));

    CHECK_THROWS(parse_mps_instance("../../instances/scp/scp41.txt"));
    CHECK_THROWS(parse_mps_instance("../../instances/rail/rail507"));
    CHECK_THROWS(parse_mps_instance("../../instances/cvrp/X-n536-k96_z95480_cplex95479.scp"));

    CHECK_THROWS(parse_scp_instance("../../instances/src/main.cpp"));
    CHECK_THROWS(parse_rail_instance("../../instances/src/main.cpp"));
    CHECK_THROWS(parse_cvrp_instance("../../instances/src/main.cpp"));
    CHECK_THROWS(parse_mps_instance("../../instances/src/main.cpp"));
}

TEST_CASE("write_solution and read_solution") {
    auto path = std::string("test_solution.txt");
    auto wsol = cft::Solution();
    wsol.cost = 101.5_F;
    wsol.idxs = {1_C, 2_C, 3_C, 4_C, 5_C, 6_C, 7_C, 8_C, 9_C};

    cft::write_solution(path, wsol);
    cft::Solution sol = cft::parse_solution(path);

    CHECK((101.49_F < sol.cost && sol.cost < 101.51_F));
    CHECK(sol.idxs == std::vector<cft::cidx_t>{1_C, 2_C, 3_C, 4_C, 5_C, 6_C, 7_C, 8_C, 9_C});

    std::remove(path.c_str());
}

TEST_CASE("Read solution: Invalid solution file") {
    auto path = std::string("test_solution.txt");
    auto file = std::ofstream(path);
    CHECK(file.is_open());
    fmt::print(file, "101.5 1 2 3# 4 5 Z 7 8 9\n");
    file.close();

    CHECK_THROWS(cft::parse_solution(path));
    std::remove(path.c_str());
}

TEST_CASE("Write solution: Invalid file path") {
    auto path = std::string("/invalid/path/test_solution.txt");
    auto sol  = cft::Solution();
    sol.cost  = 10.5_F;
    sol.idxs  = {1_C, 2_C, 3_C};

    CHECK_THROWS_AS(cft::write_solution(path, sol), std::exception);
}

TEST_CASE("Parsing SCP instance") {
    auto sol = Solution();
    sol.cost = 429.0_F;
    sol.idxs = std::vector<cidx_t>{
        2_C,   47_C,  121_C, 432_C, 42_C,  0_C,  80_C, 145_C, 88_C,  90_C, 27_C, 9_C,   69_C, 11_C,
        1_C,   152_C, 274_C, 123_C, 137_C, 16_C, 65_C, 43_C,  77_C,  76_C, 53_C, 143_C, 24_C, 193_C,
        10_C,  46_C,  106_C, 17_C,  7_C,   62_C, 82_C, 45_C,  14_C,  74_C, 19_C, 49_C,  5_C,  20_C,
        28_C,  22_C,  4_C,   120_C, 102_C, 21_C, 51_C, 25_C,  84_C,  12_C, 68_C, 48_C,  13_C, 8_C,
        115_C, 58_C,  85_C,  57_C,  119_C, 15_C, 70_C, 93_C,  142_C, 109_C};
    auto env         = cft::Environment();
    env.initsol_path = "./scp41_429.sol";
    env.parser       = CFT_SCP_PARSER;
    env.inst_path    = "../../instances/scp/scp41.txt";

    write_solution(env.initsol_path, sol);
    auto fdata = parse_inst_and_initsol(env);

    CHECK(rsize(fdata.inst.rows) == 200_R);
    CHECK(csize(fdata.inst.cols) == 1000_C);
    CHECK((428.99_F < fdata.init_sol.cost && fdata.init_sol.cost < 429.01_F));
    CHECK(sol.idxs == fdata.init_sol.idxs);
    std::remove(env.initsol_path.c_str());
}

TEST_CASE("Parsing RAIL instance") {

    auto sol = Solution();
    sol.cost = 174.0_F;
    sol.idxs = std::vector<cidx_t>{
        610_C,   38909_C, 12694_C, 18537_C, 12495_C, 45716_C, 12391_C, 3041_C,  33082_C, 56641_C,
        581_C,   39708_C, 6595_C,  42314_C, 9261_C,  27079_C, 15198_C, 62403_C, 42657_C, 57117_C,
        56703_C, 41195_C, 55674_C, 20667_C, 19939_C, 6882_C,  6859_C,  20668_C, 7279_C,  6229_C,
        20646_C, 246_C,   33189_C, 17876_C, 12129_C, 43867_C, 11958_C, 245_C,   56964_C, 47176_C,
        55671_C, 10683_C, 11593_C, 49227_C, 3457_C,  5890_C,  10349_C, 25973_C, 48276_C, 60232_C,
        61649_C, 56143_C, 2063_C,  9963_C,  56892_C, 57149_C, 3953_C,  9297_C,  33130_C, 14451_C,
        5234_C,  48476_C, 38220_C, 52534_C, 30826_C, 48922_C, 40632_C, 3154_C,  45479_C, 54870_C,
        11020_C, 53275_C, 23838_C, 55759_C, 18535_C, 15869_C, 14226_C, 47884_C, 38069_C, 3172_C,
        62370_C, 2947_C,  49385_C, 40832_C, 11396_C, 36867_C, 2325_C,  49405_C, 7878_C,  44976_C,
        1971_C,  3258_C,  7802_C,  38381_C, 13297_C, 7556_C,  11194_C, 41084_C, 1392_C,  57163_C,
        38516_C, 33610_C, 57556_C, 1252_C,  351_C,   13211_C, 44684_C, 42200_C, 62293_C, 41743_C,
        17051_C, 11968_C, 41267_C, 50918_C};
    auto env         = cft::Environment();
    env.initsol_path = "./rail507_174.sol";
    env.parser       = CFT_RAIL_PARSER;
    env.inst_path    = "../../instances/rail/rail507";

    write_solution(env.initsol_path, sol);
    auto fdata = parse_inst_and_initsol(env);

    CHECK(rsize(fdata.inst.rows) == 507_R);
    CHECK(csize(fdata.inst.cols) == 63009_C);
    CHECK(abs(fdata.init_sol.cost - 174.0_F) <= 0.01_F);
    CHECK(sol.idxs == fdata.init_sol.idxs);
    std::remove(env.initsol_path.c_str());

    // Add more assertions as needed
}

TEST_CASE("Parsing CVRP instance") {

    auto sol = Solution();
    sol.cost = 95480.0_F;
    sol.idxs = std::vector<cidx_t>{
        708_C,   175_C,  2884_C,  11143_C, 6904_C,  2265_C,  37893_C, 130_C,   588_C,   258_C,
        70340_C, 2540_C, 527_C,   6506_C,  244_C,   88_C,    7121_C,  490_C,   124_C,   13190_C,
        93598_C, 619_C,  5915_C,  66485_C, 9029_C,  18439_C, 3244_C,  2985_C,  9658_C,  1206_C,
        1579_C,  201_C,  9095_C,  32274_C, 9903_C,  371_C,   19321_C, 1655_C,  104_C,   39649_C,
        2420_C,  766_C,  1159_C,  35820_C, 6681_C,  936_C,   1710_C,  984_C,   58871_C, 6746_C,
        187_C,   6394_C, 16272_C, 87034_C, 6333_C,  4394_C,  86_C,    242_C,   830_C,   910_C,
        174_C,   709_C,  2648_C,  478_C,   848_C,   1856_C,  5434_C,  6332_C,  389_C,   3001_C,
        3429_C,  199_C,  1379_C,  2065_C,  31842_C, 2010_C,  11631_C, 6618_C,  17502_C, 12536_C,
        935_C,   1039_C, 42921_C, 3163_C,  610_C,   10701_C, 91996_C, 89575_C, 15298_C, 43017_C,
        1160_C,  240_C,  2888_C,  59641_C, 313_C,   51525_C, 14203_C};
    auto env         = cft::Environment();
    env.initsol_path = "./cvrp536_95480.sol";
    env.parser       = CFT_CVRP_PARSER;
    env.inst_path    = "../../instances/cvrp/X-n536-k96_z95480_cplex95479.scp";

    write_solution(env.initsol_path, sol);
    auto fdata = parse_inst_and_initsol(env);

    CHECK(rsize(fdata.inst.rows) == 535_R);
    CHECK(csize(fdata.inst.cols) == 127262_C);
    CHECK(abs(fdata.init_sol.cost - 95480.0_F) < 0.01_F);
    CHECK(sol.idxs == fdata.init_sol.idxs);
    std::remove(env.initsol_path.c_str());
}

TEST_CASE("Parsing MPS instance") {

    auto sol = Solution();
    sol.cost = 194.0_F;
    sol.idxs = std::vector<cidx_t>{
        1101_C, 1690_C, 1937_C, 1995_C, 2182_C, 1409_C, 120_C,  1558_C, 881_C,  801_C,  1231_C,
        592_C,  1303_C, 59_C,   325_C,  1494_C, 1900_C, 1234_C, 1567_C, 2068_C, 223_C,  1348_C,
        651_C,  1006_C, 1380_C, 957_C,  1988_C, 428_C,  890_C,  1647_C, 1698_C, 1471_C, 1264_C,
        1146_C, 236_C,  1705_C, 726_C,  1038_C, 1066_C, 1070_C, 2159_C, 2141_C, 1667_C, 1476_C,
        555_C,  1422_C, 479_C,  42_C,   1255_C, 2063_C, 615_C,  1556_C, 1791_C, 1498_C, 843_C,
        2097_C, 1280_C, 1215_C, 1747_C, 46_C,   575_C,  338_C,  569_C,  1615_C, 1601_C, 200_C,
        1872_C, 469_C,  2084_C, 206_C,  2044_C, 1334_C, 413_C,  666_C,  946_C,  668_C,  1400_C,
        892_C,  1358_C, 718_C,  664_C,  480_C,  307_C,  1187_C, 1051_C, 1071_C, 2134_C, 1712_C,
        538_C,  2120_C, 27_C,   1776_C, 100_C,  1798_C, 367_C,  7_C,    1517_C, 1580_C, 983_C,
        2034_C, 1861_C, 927_C,  924_C,  633_C,  392_C,  1319_C, 1134_C, 2169_C, 1432_C, 1707_C,
        1450_C, 1128_C, 1176_C, 1673_C, 2115_C, 833_C,  993_C,  1965_C, 167_C,  1811_C, 1524_C,
        622_C,  1973_C, 342_C,  44_C,   789_C,  116_C,  542_C,  373_C,  1265_C, 1084_C, 312_C,
        348_C,  765_C,  794_C,  249_C,  1605_C, 1097_C, 411_C,  1875_C, 913_C,  462_C,  1000_C,
        1732_C, 2122_C, 1005_C, 175_C,  1087_C, 91_C,   847_C,  267_C,  1958_C, 2022_C, 451_C,
        1828_C, 1926_C, 1805_C, 135_C,  1194_C, 1365_C, 1885_C, 418_C,  243_C,  210_C,  189_C,
        137_C,  1457_C, 725_C,  1959_C, 13_C,   689_C,  814_C,  311_C,  1949_C, 1817_C, 1734_C,
        608_C,  522_C,  739_C,  77_C,   760_C,  1913_C, 791_C,  955_C,  156_C,  359_C,  1109_C,
        274_C,  697_C,  1850_C, 293_C,  1969_C, 517_C,  868_C};

    auto env         = cft::Environment();
    env.initsol_path = "./ramos3_194.sol";
    env.parser       = CFT_MPS_PARSER;
    env.inst_path    = "../../instances/mps/ramos3.mps";

    write_solution(env.initsol_path, sol);
    auto fdata = parse_inst_and_initsol(env);

    CHECK(rsize(fdata.inst.rows) == 2187_R);
    CHECK(csize(fdata.inst.cols) == 2187_C);
    CHECK(abs(fdata.init_sol.cost - 194.0_F) < 0.01_F);
    CHECK(sol.idxs == fdata.init_sol.idxs);
    std::remove(env.initsol_path.c_str());
}

TEST_CASE("Invalid parser") {
    auto env         = cft::Environment();
    env.initsol_path = "./instance.txt";
    env.parser       = "INVALID";
    env.inst_path    = "./solution.txt";
    CHECK_THROWS_AS(parse_inst_and_initsol(env), std::runtime_error);
}


}  // namespace cft