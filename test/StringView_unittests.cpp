#include <catch2/catch.hpp>
#include <cstring>

#include "utils/StringView.hpp"

namespace cft {

TEST_CASE("test_ctor") {

    auto sv1 = StringView();
    REQUIRE(sv1.empty());
    REQUIRE(sv1.size() == 0);
    REQUIRE(sv1.data() == nullptr);
    REQUIRE(sv1.begin() == sv1.end());

    char const* cstr = "Hello";
    auto        sv2  = StringView(cstr, cstr + 5);
    REQUIRE(sv2.size() == 5);
    REQUIRE(sv2.data() == cstr);
    REQUIRE(sv2.begin() == cstr);
    REQUIRE(sv2.end() == cstr + 5);
    REQUIRE(sv2[0] == 'H');
    REQUIRE(sv2[1] == 'e');
    REQUIRE(sv2[2] == 'l');
    REQUIRE(sv2[3] == 'l');
    REQUIRE(sv2[4] == 'o');

    auto sv3 = StringView(cstr, 5);
    REQUIRE(sv3.size() == 5);
    REQUIRE(sv3.data() == cstr);
    REQUIRE(sv3.begin() == cstr);
    REQUIRE(sv3.end() == cstr + 5);
    REQUIRE(sv3[0] == 'H');
    REQUIRE(sv3[1] == 'e');
    REQUIRE(sv3[2] == 'l');
    REQUIRE(sv3[3] == 'l');
    REQUIRE(sv3[4] == 'o');

    std::string cxxstr = "Hello, World!";
    auto        sv4    = StringView(cxxstr);
    REQUIRE(sv4.size() == 13);
    REQUIRE(sv4.data() == cxxstr.data());
    REQUIRE(sv4.begin() == cxxstr.data());
    REQUIRE(sv4.end() == cxxstr.data() + 13);
    REQUIRE(sv4.get_substr(0, 5) == sv3);

    auto sv5 = StringView(cstr);
    REQUIRE(sv5 == sv4.get_substr(0, 5));

    char bstr[] = "World!";
    auto sv6    = StringView(bstr, bstr + 6);
    REQUIRE(sv6.size() == 6);
    REQUIRE(sv6.data() == bstr);
    REQUIRE(sv6.begin() == bstr);
    REQUIRE(sv6.end() == bstr + 6);
    REQUIRE(sv6 == sv4.get_substr(7, 13));
}

TEST_CASE("test_string_view_prefix_suffix") {
    auto sv = StringView("Hello, World!");
    REQUIRE(sv.size() == 13);
    REQUIRE(sv.find_first_if([](char c) { return std::isspace(c); }) == 6);
    REQUIRE(sv.find_last_if([](char c) { return c == ','; }) == 5);
    REQUIRE(sv.find_last_if([](char c) { return c == 'H'; }) == 0);
    REQUIRE(sv.find_first_if([](char c) { return c == '#'; }) == 13);
    REQUIRE(sv.find_last_if([](char c) { return c == '#'; }) == 13);
    REQUIRE(sv.remove_prefix(7) == StringView("World!"));
    REQUIRE(sv.remove_suffix(6) == StringView("Hello,"));
}

TEST_CASE("test_string_view_comparison") {
    auto sv1 = StringView("abc");
    auto sv2 = StringView("abc");
    auto sv3 = StringView("def");
    auto sv4 = StringView("abcd");
    auto sv5 = StringView("ab");
    REQUIRE(sv1 == sv2);
    REQUIRE(sv1 != sv3);
    REQUIRE(sv1 < sv3);
    REQUIRE(sv1 <= sv3);
    REQUIRE(sv3 > sv1);
    REQUIRE(sv3 >= sv1);
    REQUIRE(sv1 < sv4);
    REQUIRE(sv1 <= sv4);
    REQUIRE(sv4 > sv1);
    REQUIRE(sv4 >= sv1);
    REQUIRE(sv5 < sv1);
    REQUIRE(sv5 <= sv1);
    REQUIRE(sv5 <= sv5);
    REQUIRE(sv5 >= sv5);
}

}  // namespace cft