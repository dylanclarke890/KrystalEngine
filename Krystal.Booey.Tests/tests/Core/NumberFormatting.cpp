#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include <limits>
#include <numbers>

namespace krys::boo::tests
{
  namespace
  {
    utf8_string FixedPrecision(double number, uint32 significantFigures = 6,
                               bool truncateTrailingZeros = true)
    {
      utf8_string builder;
      FormatNumberFixedPrecision(builder, number, significantFigures, truncateTrailingZeros);
      return builder;
    }
  }

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

  TEST_CASE("FormatNumberFixedPrecision", "[Core][NumberFormatting]")
  {
    using Limits = std::numeric_limits<double>;

    CHECK(FixedPrecision(Limits::infinity()) == u8"Infinity");
    CHECK(FixedPrecision(-Limits::infinity()) == u8"-Infinity");

    CHECK(FixedPrecision(-Limits::quiet_NaN()) == u8"NaN");

    CHECK(FixedPrecision(0) == u8"0");
    CHECK(FixedPrecision(-0.0) == u8"0");

    CHECK(FixedPrecision(Limits::min()) == u8"2.22507e-308");
    CHECK(FixedPrecision(Limits::lowest()) == u8"-1.79769e+308");
    CHECK(FixedPrecision(Limits::max()) == u8"1.79769e+308");

    CHECK(FixedPrecision(std::numbers::pi) == u8"3.14159");
    CHECK(FixedPrecision(std::numbers::pi_v<float>) == u8"3.14159");
    CHECK(FixedPrecision(std::numbers::pi / 2) == u8"1.5708");
    CHECK(FixedPrecision(std::numbers::pi_v<float> / 2) == u8"1.5708");
    CHECK(FixedPrecision(std::numbers::pi / 4) == u8"0.785398");
    CHECK(FixedPrecision(std::numbers::pi_v<float> / 4) == u8"0.785398");

    CHECK(FixedPrecision(2.71828182845904523536028747135266249775724709369995) == u8"2.71828");

    CHECK(FixedPrecision(299'792'458) == u8"2.99792e+8");

    CHECK(FixedPrecision(1.6180339887498948482) == u8"1.61803");

    CHECK(FixedPrecision(1e3) == u8"1000");
    CHECK(FixedPrecision(1e10) == u8"1e+10");
    CHECK(FixedPrecision(1e20) == u8"1e+20");
    CHECK(FixedPrecision(1e21) == u8"1e+21");
    CHECK(FixedPrecision(1e30) == u8"1e+30");

    CHECK(FixedPrecision(1.1e3) == u8"1100");
    CHECK(FixedPrecision(1.1e10) == u8"1.1e+10");
    CHECK(FixedPrecision(1.1e20) == u8"1.1e+20");
    CHECK(FixedPrecision(1.1e21) == u8"1.1e+21");
    CHECK(FixedPrecision(1.1e30) == u8"1.1e+30");
  }

  TEST_CASE("FormatNumberFixedPrecision: significant figures and trailing zeros", "[Core][NumberFormatting]")
  {
    CHECK(FixedPrecision(17890.0) == u8"17890");
    CHECK(FixedPrecision(17890.0, 6, false) == u8"17890.0");
    CHECK(FixedPrecision(17890.0, 3, false) == u8"1.79e+4");
    CHECK(FixedPrecision(17890.0, 8, false) == u8"17890.000");
    CHECK(FixedPrecision(17890.0, 8) == u8"17890");

    CHECK(FixedPrecision(17890.5) == u8"17890.5");
    CHECK(FixedPrecision(17890.5, 3, false) == u8"1.79e+4");
    CHECK(FixedPrecision(17890.5, 8, false) == u8"17890.500");
    CHECK(FixedPrecision(17890.5, 8) == u8"17890.5");

    CHECK(FixedPrecision(-17890.0) == u8"-17890");
    CHECK(FixedPrecision(-17890.0, 6, false) == u8"-17890.0");
    CHECK(FixedPrecision(-17890.0, 3, false) == u8"-1.79e+4");
    CHECK(FixedPrecision(-17890.0, 8, false) == u8"-17890.000");
    CHECK(FixedPrecision(-17890.0, 8) == u8"-17890");

    CHECK(FixedPrecision(-17890.5) == u8"-17890.5");
    CHECK(FixedPrecision(-17890.5, 3, false) == u8"-1.79e+4");
    CHECK(FixedPrecision(-17890.5, 8, false) == u8"-17890.500");
    CHECK(FixedPrecision(-17890.5, 8) == u8"-17890.5");

    CHECK(FixedPrecision(0.0) == u8"0");
    CHECK(FixedPrecision(0.0, 6, false) == u8"0.00000");
    CHECK(FixedPrecision(0.0, 3, false) == u8"0.00");
    CHECK(FixedPrecision(0.0, 8, false) == u8"0.0000000");
    CHECK(FixedPrecision(0.0, 8) == u8"0");

    // Small numbers switch to exponential notation below 1e-6.
    CHECK(FixedPrecision(0.000001) == u8"0.000001");
    CHECK(FixedPrecision(0.000001, 6, false) == u8"0.00000100000");
    CHECK(FixedPrecision(0.0000001) == u8"1e-7");
    CHECK(FixedPrecision(0.0000001234567) == u8"1.23457e-7");
    // Rounding can carry into a new digit.
    CHECK(FixedPrecision(9.9999995) == u8"10");
    CHECK(FixedPrecision(999999.5) == u8"1e+6");
  }
}
