#include "Krystal.Text/Regex.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::text::tests
{
  TEST_CASE("Regex", "[Text][Regex]")
  {
    SECTION("Invalid patterns don't compile")
    {
      CHECK_FALSE(Regex::Compile(u8"[a-"));
      CHECK_FALSE(Regex::Compile(u8"(a"));
      // Unknown escapes are errors.
      CHECK_FALSE(Regex::Compile(u8"\\q"));
    }

    SECTION("Matches the whole text")
    {
      auto regex = Regex::Compile(u8"[0-9]{3}");
      REQUIRE(regex);
      CHECK(regex->MatchesWhole(u8"123"));
      CHECK_FALSE(regex->MatchesWhole(u8"1234"));
      CHECK_FALSE(regex->MatchesWhole(u8"a123"));
      CHECK_FALSE(regex->MatchesWhole(u8""));
    }

    SECTION("Matches non-ASCII text")
    {
      auto regex = Regex::Compile(u8"\\p{L}+");
      REQUIRE(regex);
      CHECK(regex->MatchesWhole(u8"été"));
      CHECK(regex->MatchesWhole(u8"日本"));
      CHECK_FALSE(regex->MatchesWhole(u8"abc1"));
    }

    SECTION("Moves keep the compiled expression")
    {
      auto regex = Regex::Compile(u8"a+");
      REQUIRE(regex);
      Regex moved = krys::move(*regex);
      CHECK(moved.MatchesWhole(u8"aaa"));
    }
  }
}
