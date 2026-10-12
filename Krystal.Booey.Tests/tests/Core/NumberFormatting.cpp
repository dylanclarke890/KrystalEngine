#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include <limits>

namespace krys::boo::tests
{
  TEST_CASE("FormatCSSNumber", "[Core][NumberFormatting]")
  {
    CHECK(FormatCSSNumber(0.0) == u8"0");
    CHECK(FormatCSSNumber(-0.0) == u8"0");
    CHECK(FormatCSSNumber(1.0) == u8"1");
    CHECK(FormatCSSNumber(100.0) == u8"100");
    CHECK(FormatCSSNumber(0.5) == u8"0.5");
    CHECK(FormatCSSNumber(-2.25) == u8"-2.25");

    // Six decimal places, rounded.
    CHECK(FormatCSSNumber(1.23456789) == u8"1.234568");
    CHECK(FormatCSSNumber(0.0000004) == u8"0");
    CHECK(FormatCSSNumber(0.0000016) == u8"0.000002");
    CHECK(FormatCSSNumber(-0.0000001) == u8"0");

    // Never uses exponents.
    CHECK(FormatCSSNumber(1e21) == u8"1000000000000000000000");

    CHECK(FormatCSSNumber(std::numeric_limits<double>::infinity()) == u8"infinity");
    CHECK(FormatCSSNumber(-std::numeric_limits<double>::infinity()) == u8"-infinity");
    CHECK(FormatCSSNumber(std::numeric_limits<double>::quiet_NaN()) == u8"NaN");

    utf8_string builder = u8"x: ";
    FormatCSSNumber(builder, 0.25);
    CHECK(builder == u8"x: 0.25");
  }

  TEST_CASE("AreEssentiallyEqual", "[Core][MathExtras]")
  {
    STATIC_CHECK(AreEssentiallyEqual(1.0f, 1.0f));
    STATIC_CHECK(AreEssentiallyEqual(0.0, 0.0));
    CHECK(AreEssentiallyEqual(1.0f, 1.0f + std::numeric_limits<float>::epsilon()));
    CHECK_FALSE(AreEssentiallyEqual(1.0f, 1.001f));
    CHECK(AreEssentiallyEqual(1.0f, 1.001f, 0.01f));
    CHECK_FALSE(AreEssentiallyEqual(0.0f, 1e-30f));
    CHECK_FALSE(AreEssentiallyEqual(1.0f, std::numeric_limits<float>::quiet_NaN()));
  }

  TEST_CASE("SafeFPDivision", "[Core][MathExtras]")
  {
    STATIC_CHECK(SafeFPDivision(1.0, 2.0) == 0.5);
    CHECK(SafeFPDivision(1.0f, 0.0f) == std::numeric_limits<float>::max());
    CHECK(SafeFPDivision(std::numeric_limits<float>::min(), 4.0f) == 0.0f);
  }
}
