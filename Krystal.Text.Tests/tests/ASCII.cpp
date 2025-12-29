#include "Krystal.Text/ASCII.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("ASCII(IsASCII)", "[ASCII]")
  {
    REQUIRE(ASCII::IsASCII(Rune('\0')) == true);
    REQUIRE(ASCII::IsASCII('~') == true);
    REQUIRE(ASCII::IsASCII(Rune(0x80u)) == false);
    REQUIRE(ASCII::IsASCII(Rune(0xFFu)) == false);
  }

  TEST_CASE("ASCII(IsWhiteSpace)", "[ASCII]")
  {
    REQUIRE(ASCII::IsWhiteSpace(' ') == true);
    REQUIRE(ASCII::IsWhiteSpace('\t') == true);
    REQUIRE(ASCII::IsWhiteSpace('\n') == true);
    REQUIRE(ASCII::IsWhiteSpace('\r') == true);
    REQUIRE(ASCII::IsWhiteSpace('\f') == true);
    REQUIRE(ASCII::IsWhiteSpace('\v') == true);
    REQUIRE(ASCII::IsWhiteSpace('A') == false);
    REQUIRE(ASCII::IsWhiteSpace('1') == false);
  }

  TEST_CASE("ASCII(IsAlpha)", "[ASCII]")
  {
    for (char8 ch = 0; ch < 128; ch++)
    {
      bool expected = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
      REQUIRE(ASCII::IsAlpha(ch) == expected);
    }
  }

  TEST_CASE("ASCII(IsQuote)", "[ASCII]")
  {
    REQUIRE(ASCII::IsQuote('\'') == true);
    REQUIRE(ASCII::IsQuote('"') == true);
    REQUIRE(ASCII::IsQuote('A') == false);
    REQUIRE(ASCII::IsQuote(' ') == false);
  }

  TEST_CASE("ASCII(ToLower)", "[ASCII]")
  {
    REQUIRE(ASCII::ToLower('A') == 'a');
    REQUIRE(ASCII::ToLower('Z') == 'z');
    REQUIRE(ASCII::ToLower('a') == 'a');
    REQUIRE(ASCII::ToLower('z') == 'z');
    REQUIRE(ASCII::ToLower('1') == '1');

    utf8_string str = u8"Hello, World!";
    REQUIRE(ASCII::ToLower(str) == u8"hello, world!");
  }

  TEST_CASE("ASCII(ToUpper)", "[ASCII]")
  {
    REQUIRE(ASCII::ToUpper('a') == 'A');
    REQUIRE(ASCII::ToUpper('z') == 'Z');
    REQUIRE(ASCII::ToUpper('A') == 'A');
    REQUIRE(ASCII::ToUpper('Z') == 'Z');
    REQUIRE(ASCII::ToUpper('1') == '1');

    utf8_string str = u8"Hello, World!";
    REQUIRE(ASCII::ToUpper(str) == u8"HELLO, WORLD!");
  }

  TEST_CASE("ASCII(SkipWhiteSpace", "[ASCII]")
  {
    // 3 spaces + 1 tab + 1 newline + 2 spaces
    utf8_string str = u8"   \t\n  Hello";
    size_t index = 0;

    ASCII::SkipWhiteSpace(str, index);
    REQUIRE(index == str.find('H'));
  }
}