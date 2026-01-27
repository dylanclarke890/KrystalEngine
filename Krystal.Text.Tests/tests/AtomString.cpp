#include "Krystal.Text/AtomString.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("AtomString - different strings do not compare equal", "[AtomString]")
  {
    AtomString str1 = u8"hello";
    AtomString str2 = u8"world";

    REQUIRE(str1 != str2);
  }

  TEST_CASE("AtomString - the same string compares equal", "[AtomString]")
  {
    AtomString str1 = u8"hello";
    AtomString str2 = str1;

    REQUIRE(str1 == str2);
  }

  TEST_CASE("AtomString - different strings with the same text compare equal", "[AtomString]")
  {
    AtomString str1 = u8"hello";
    AtomString str2 = u8"hello";

    str1 == str2;

    REQUIRE(str1 == str2);
  }

  TEST_CASE("AtomString - empty strings compare equal", "[AtomString]")
  {
    AtomString str1;
    AtomString str2 = u8"";
    REQUIRE(str1 == str2);
  }

  TEST_CASE("AtomString - View returns correct string", "[AtomString]")
  {
    utf8_stringview text = u8"example";
    AtomString atomStr = text;
    REQUIRE(atomStr.View() == text);
  }

  TEST_CASE("AtomString - incrementally created string compares equal", "[AtomString]")
  {
    utf8_string original = u8"incremental";
    utf8_string temp = original.substr(0, 5); // "incre"
    temp += original.substr(5);               // "mental"
    AtomString atomStr1 = original;
    AtomString atomStr2 = std::move(temp);
    REQUIRE(atomStr1 == atomStr2);
  }
}