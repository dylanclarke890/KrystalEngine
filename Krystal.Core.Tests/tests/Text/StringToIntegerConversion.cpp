#include "Krystal.Core/Text/StringToIntegerConversion.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::text::tests
{
  TEST_CASE("ParseInteger: digits, signs and whitespace", "[Text][StringToIntegerConversion]")
  {
    CHECK(ParseInteger<int32>(u8"42") == 42);
    CHECK(ParseInteger<int32>(u8"-42") == -42);
    CHECK(ParseInteger<int32>(u8"+42") == 42);
    CHECK(ParseInteger<int32>(u8" \t\v42\n ") == 42);
    CHECK_FALSE(ParseInteger<int32>(u8" 42", 10, ParseIntegerWhitespacePolicy::Disallow));
    CHECK_FALSE(ParseInteger<int32>(u8"42x"));
    CHECK_FALSE(ParseInteger<int32>(u8""));
    CHECK_FALSE(ParseInteger<int32>(u8"-"));
    CHECK_FALSE(ParseInteger<int32>(u8"+-1"));
    CHECK_FALSE(ParseInteger<uint32>(u8"-1"));
    CHECK(ParseIntegerAllowingTrailingJunk<int32>(u8"42px") == 42);
    CHECK(ParseInteger<int32>(u8"ff", 16) == 255);
    CHECK(ParseInteger<int32>(u8"FF", 16) == 255);
    CHECK_FALSE(ParseInteger<int32>(u8"fg", 16));
  }

  TEST_CASE("ParseInteger: overflow", "[Text][StringToIntegerConversion]")
  {
    CHECK(ParseInteger<int32>(u8"2147483647") == 2'147'483'647);
    CHECK(ParseInteger<int32>(u8"-2147483648") == std::numeric_limits<int32>::min());
    CHECK_FALSE(ParseInteger<int32>(u8"2147483648"));
    CHECK_FALSE(ParseInteger<int32>(u8"-2147483649"));
    CHECK(ParseInteger<uint8>(u8"255") == 255);
    CHECK_FALSE(ParseInteger<uint8>(u8"256"));
    CHECK_FALSE(ParseIntegerAllowingTrailingJunk<int32>(u8"99999999999px"));
  }
}
