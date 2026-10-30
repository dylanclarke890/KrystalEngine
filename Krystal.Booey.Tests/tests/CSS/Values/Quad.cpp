#include "Krystal.Booey/CSS/Values/Quad.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("Quad(Serialise)", "[CSS][Values][Quad]")
  {
    SECTION("All four sides equal serialise as one value")
    {
      CHECK(Quad::Serialise(u8"1px", u8"1px", u8"1px", u8"1px") == u8"1px");
    }

    SECTION("Top/bottom and right/left pairs serialise as two values")
    {
      CHECK(Quad::Serialise(u8"1px", u8"2px", u8"1px", u8"2px") == u8"1px 2px");
    }

    SECTION("Right equal to left serialises as three values")
    {
      CHECK(Quad::Serialise(u8"1px", u8"2px", u8"3px", u8"2px") == u8"1px 2px 3px");
    }

    SECTION("Left different from right serialises all four values")
    {
      CHECK(Quad::Serialise(u8"1px", u8"2px", u8"3px", u8"4px") == u8"1px 2px 3px 4px");
      CHECK(Quad::Serialise(u8"1px", u8"1px", u8"1px", u8"2px") == u8"1px 1px 1px 2px");
    }

    SECTION("Bottom differing from top keeps three values even if right equals top")
    {
      CHECK(Quad::Serialise(u8"1px", u8"1px", u8"2px", u8"1px") == u8"1px 1px 2px");
    }
  }
}
