#include "Krystal.Lib/String/StringAtom.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("StringAtom - different strings do not compare equal", "[StringAtom]")
  {
    StringAtom str1 = u8"hello";
    StringAtom str2 = u8"world";

    REQUIRE(str1 != str2);
  }

  TEST_CASE("StringAtom - the same string compares equal", "[StringAtom]")
  {
    StringAtom str1 = u8"hello";
    StringAtom str2 = str1;

    REQUIRE(str1 == str2);
  }

  TEST_CASE("StringAtom - different strings with the same text compare equal", "[StringAtom]")
  {
    StringAtom str1 = u8"hello";
    StringAtom str2 = u8"hello";

    REQUIRE(str1 == str2);
  }

  TEST_CASE("StringAtom - empty strings compare equal", "[StringAtom]")
  {
    StringAtom str1;
    StringAtom str2 = u8"";
    REQUIRE(str1 == str2);
  }

  TEST_CASE("StringAtom - View returns correct string", "[StringAtom]")
  {
    utf8_stringview text = u8"example";
    StringAtom atomStr = text;
    REQUIRE(atomStr.View() == text);
  }

  TEST_CASE("StringAtom - incrementally created string compares equal", "[StringAtom]")
  {
    utf8_string original = u8"incremental";
    utf8_string temp = original.substr(0, 5); // "incre"
    temp += original.substr(5);               // "mental"
    StringAtom atomStr1 = original;
    StringAtom atomStr2 = std::move(temp);
    REQUIRE(atomStr1 == atomStr2);
  }
}