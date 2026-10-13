#include "Krystal.Booey/Graphics/Color/NamedColors.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("FindColor", "[Graphics][Color][NamedColors]")
  {
    SECTION("Finds every kind of entry")
    {
      const auto *aliceBlue = FindColor(u8"aliceblue");
      REQUIRE(aliceBlue);
      CHECK(aliceBlue->Name == u8"aliceblue");
      CHECK(aliceBlue->ARGBValue == 0xff'f0'f8'ffu);

      REQUIRE(FindColor(u8"yellowgreen"));
      CHECK(FindColor(u8"yellowgreen")->ARGBValue == 0xff'9a'cd'32u);

      REQUIRE(FindColor(u8"red"));
      CHECK(FindColor(u8"red")->ARGBValue == 0xff'ff'00'00u);

      // Both spellings of grey.
      REQUIRE(FindColor(u8"gray"));
      REQUIRE(FindColor(u8"grey"));
      CHECK(FindColor(u8"gray")->ARGBValue == FindColor(u8"grey")->ARGBValue);

      REQUIRE(FindColor(u8"transparent"));
      CHECK(FindColor(u8"transparent")->ARGBValue == 0x00'00'00'00u);
      REQUIRE(FindColor(u8"alpha"));
    }

    SECTION("Is exact and case-sensitive")
    {
      CHECK_FALSE(FindColor(u8""));
      CHECK_FALSE(FindColor(u8"Red"));
      CHECK_FALSE(FindColor(u8"re"));
      CHECK_FALSE(FindColor(u8"redd"));
      CHECK_FALSE(FindColor(u8"notacolor"));
      CHECK_FALSE(FindColor(u8"zzz"));
    }
  }
}
