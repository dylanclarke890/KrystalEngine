#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "catch2/catch_test_macros.hpp"
#include <limits>

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD string Format(double value, CSSOMStringView suffix = u8"") noexcept
    {
      auto formatted = FormatCSSNumberValue(SerialisableNumber {.Value = value, .Suffix = suffix});
      return string(reinterpret_cast<const char *>(formatted.data()), formatted.size());
    }
  }

  TEST_CASE("FormatCSSNumberValue", "[CSS][NumericTypesSerialisation]")
  {
    CHECK(Format(0.0) == "0");
    CHECK(Format(1.0) == "1");
    CHECK(Format(100.0) == "100");
    CHECK(Format(-3.0) == "-3");
    CHECK(Format(1.5) == "1.5");
    CHECK(Format(-0.25) == "-0.25");
    CHECK(Format(1e21) == "1000000000000000000000");
  }

  TEST_CASE("FormatCSSNumberValue(rounds to 6 decimal places)", "[CSS][NumericTypesSerialisation]")
  {
    CHECK(Format(0.1234564) == "0.123456");
    CHECK(Format(0.1234566) == "0.123457");
    CHECK(Format(1.0 / 3.0) == "0.333333");
    CHECK(Format(44.9999999) == "45");
    CHECK(Format(0.0000004) == "0");
  }

  TEST_CASE("FormatCSSNumberValue(negative zero)", "[CSS][NumericTypesSerialisation]")
  {
    CHECK(Format(-0.0) == "0");
    // Rounds to -0 at 6 decimal places.
    CHECK(Format(-0.0000001) == "0");
  }

  TEST_CASE("FormatCSSNumberValue(suffix)", "[CSS][NumericTypesSerialisation]")
  {
    CHECK(Format(10.0, u8"px") == "10px");
    CHECK(Format(1.5, u8"em") == "1.5em");
    CHECK(Format(50.0, u8"%") == "50%");
  }

  TEST_CASE("FormatCSSNumberValue(non-finite)", "[CSS][NumericTypesSerialisation]")
  {
    constexpr double infinity = std::numeric_limits<double>::infinity();

    CHECK(Format(infinity) == "infinity");
    CHECK(Format(-infinity) == "-infinity");
    CHECK(Format(std::numeric_limits<double>::quiet_NaN()) == "NaN");

    // Non-finite dimensions are written as a product so they stay valid CSS.
    CHECK(Format(infinity, u8"px") == "infinity * 1px");
    CHECK(Format(-infinity, u8"deg") == "-infinity * 1deg");
    CHECK(Format(std::numeric_limits<double>::quiet_NaN(), u8"%") == "NaN * 1%");
  }

  TEST_CASE("FormatCSSNumberValue(appends)", "[CSS][NumericTypesSerialisation]")
  {
    CSSOMString builder = u8"width: ";
    FormatCSSNumberValue(builder, SerialisableNumber {.Value = 2.5, .Suffix = u8"px"});
    CHECK(string(reinterpret_cast<const char *>(builder.data()), builder.size()) == "width: 2.5px");
  }
}
