#include "Krystal.Booey/HTML/MicroParsers/Dimensions.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("Dimensions::ParseHTMLDimension", "[HTML][MicroParsers]")
  {
    auto pixels = Dimensions::ParseHTMLDimension(u8"50");
    REQUIRE(pixels);
    CHECK(pixels->Number == 50);
    CHECK(pixels->DimensionType == HTMLDimension::Type::Pixel);

    auto percentage = Dimensions::ParseHTMLDimension(u8"  25.5%");
    REQUIRE(percentage);
    CHECK(percentage->Number == 25.5);
    CHECK(percentage->DimensionType == HTMLDimension::Type::Percentage);

    // Anything after the number is ignored.
    auto trailing = Dimensions::ParseHTMLDimension(u8"12px");
    REQUIRE(trailing);
    CHECK(trailing->Number == 12);
    CHECK(trailing->DimensionType == HTMLDimension::Type::Pixel);

    auto fraction = Dimensions::ParseHTMLDimension(u8"3.");
    REQUIRE(fraction);
    CHECK(fraction->Number == 3);

    CHECK_FALSE(Dimensions::ParseHTMLDimension(u8""));
    CHECK_FALSE(Dimensions::ParseHTMLDimension(u8"   "));
    CHECK_FALSE(Dimensions::ParseHTMLDimension(u8".5"));
    CHECK_FALSE(Dimensions::ParseHTMLDimension(u8"-5"));
    CHECK_FALSE(Dimensions::ParseHTMLDimension(u8"abc"));

    auto relative = Dimensions::ParseHTMLDimension(u8"5*");
    REQUIRE(relative);
    CHECK(relative->Number == 5);
  }

  TEST_CASE("Dimensions::ParseHTMLMultiLength", "[HTML][MicroParsers]")
  {
    // Relative lengths aren't supported.
    CHECK_FALSE(Dimensions::ParseHTMLMultiLength(u8"5*"));

    auto pixels = Dimensions::ParseHTMLMultiLength(u8"5");
    REQUIRE(pixels);
    CHECK(pixels->Number == 5);
    auto percentage = Dimensions::ParseHTMLMultiLength(u8"5%");
    REQUIRE(percentage);
    CHECK(percentage->DimensionType == HTMLDimension::Type::Percentage);
  }
}
