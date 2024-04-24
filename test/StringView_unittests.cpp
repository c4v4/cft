// SPDX-FileCopyrightText: 2024 Francesco Cavaliere <francescocava95@gmail.com>
// SPDX-License-Identifier: MIT

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
 
#include <doctest/doctest.h>

#include "utils/StringView.hpp"

namespace cft {

TEST_CASE("test_ctor") {

    auto sv1 = StringView();
    CHECK(sv1.empty());
    CHECK(sv1.size() == 0);
    CHECK(sv1.data() == nullptr);
    CHECK(sv1.begin() == sv1.end());

    char const* cstr = "Hello";
    auto        sv2  = StringView(cstr, cstr + 5);
    CHECK(sv2.size() == 5);
    CHECK(sv2.data() == cstr);
    CHECK(sv2.begin() == cstr);
    CHECK(sv2.end() == cstr + 5);
    CHECK(sv2[0] == 'H');
    CHECK(sv2[1] == 'e');
    CHECK(sv2[2] == 'l');
    CHECK(sv2[3] == 'l');
    CHECK(sv2[4] == 'o');

    auto sv3 = StringView(cstr, 5);
    CHECK(sv3.size() == 5);
    CHECK(sv3.data() == cstr);
    CHECK(sv3.begin() == cstr);
    CHECK(sv3.end() == cstr + 5);
    CHECK(sv3[0] == 'H');
    CHECK(sv3[1] == 'e');
    CHECK(sv3[2] == 'l');
    CHECK(sv3[3] == 'l');
    CHECK(sv3[4] == 'o');

    std::string cxxstr = "Hello, World!";
    auto        sv4    = StringView(cxxstr);
    CHECK(sv4.size() == 13);
    CHECK(sv4.data() == cxxstr.data());
    CHECK(sv4.begin() == cxxstr.data());
    CHECK(sv4.end() == cxxstr.data() + 13);
    CHECK(sv4.get_substr(0, 5) == sv3);

    auto sv5 = StringView(cstr);
    CHECK(sv5 == sv4.get_substr(0, 5));

    char bstr[] = "World!";
    auto sv6    = StringView(bstr, bstr + 6);
    CHECK(sv6.size() == 6);
    CHECK(sv6.data() == bstr);
    CHECK(sv6.begin() == bstr);
    CHECK(sv6.end() == bstr + 6);
    CHECK(sv6 == sv4.get_substr(7, 13));
}

TEST_CASE("test_string_view_prefix_suffix") {
    auto sv = StringView("Hello, World!");
    CHECK(sv.size() == 13);
    CHECK(sv.find_first_true([](char c) { return std::isspace(c); }) == 6);
    CHECK(sv.find_last_true([](char c) { return c == ','; }) == 5);
    CHECK(sv.find_last_true([](char c) { return c == 'H'; }) == 0);
    CHECK(sv.find_first_true([](char c) { return c == '#'; }) == 13);
    CHECK(sv.find_last_true([](char c) { return c == '#'; }) == 13);
    CHECK(sv.remove_prefix(7) == StringView("World!"));
    CHECK(sv.remove_suffix(6) == StringView("Hello,"));
}

TEST_CASE("test_string_view_prefix_suffix_empty") {
    auto sv = StringView();
    CHECK(sv.empty());
    CHECK(sv.size() == 0);
    CHECK(sv.find_first_true([](char c) { return std::isspace(c); }) == 0);
    CHECK(sv.find_last_true([](char c) { return c == ','; }) == 0);
    CHECK(sv.find_last_true([](char c) { return c == 'H'; }) == 0);
    CHECK(sv.find_first_true([](char c) { return c == '#'; }) == 0);
    CHECK(sv.find_last_true([](char c) { return c == '#'; }) == 0);
    CHECK(sv.remove_prefix(0) == StringView(""));
    CHECK(sv.remove_suffix(0) == StringView(""));
}

TEST_CASE("test_string_view_comparison") {
    auto sv1 = StringView("abc");
    auto sv2 = StringView("abc");
    auto sv3 = StringView("def");
    auto sv4 = StringView("abcd");
    auto sv5 = StringView("ab");
    CHECK(sv1 == sv2);
    CHECK(sv1 != sv3);
    CHECK(sv1 < sv3);
    CHECK(sv1 <= sv3);
    CHECK(sv3 > sv1);
    CHECK(sv3 >= sv1);
    CHECK(sv1 < sv4);
    CHECK(sv1 <= sv4);
    CHECK(sv4 > sv1);
    CHECK(sv4 >= sv1);
    CHECK(sv5 < sv1);
    CHECK(sv5 <= sv1);
    CHECK(sv5 <= sv5);
    CHECK(sv5 >= sv5);
}


#ifndef NDEBUG

TEST_CASE("Test StringView assert fails") {
    char base[] = "Hello World!";
    CHECK_THROWS_AS(StringView(base + 12, base), std::runtime_error);
    auto illegal_span   = StringView{};
    illegal_span.start  = base + 12;
    illegal_span.finish = base;

    CHECK_THROWS_AS(illegal_span.empty(), std::runtime_error);
    CHECK_THROWS_AS(illegal_span.size(), std::runtime_error);
    CHECK_THROWS_AS(illegal_span.begin(), std::runtime_error);
    CHECK_THROWS_AS(illegal_span.end(), std::runtime_error);

    auto str = StringView(base);
    CHECK_THROWS_AS(str[14], std::runtime_error);
    CHECK_THROWS_AS(str[-1], std::runtime_error);
}

#endif

}  // namespace cft