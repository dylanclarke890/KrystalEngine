#include "Krystal.HTML/HTML/MicroParsers/Numbers.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML::MicroParsers;

  namespace
  {
    template <typename T>
    void TestParse(const char8 *input, MicroParserResult<T> expected)
    {
      auto result = [&]() -> MicroParserResult<T>
      {
        if constexpr (SameType<T, int64>)
        {
          return Numbers::ParseInteger(input);
        }
        else if constexpr (SameType<T, uint64>)
        {
          return Numbers::ParseNonNegativeInteger(input);
        }
        else if constexpr (SameType<T, double>)
        {
          return Numbers::ParseFloatingPoint(input);
        }
        else
        {
          static_assert(DependentFalse<T>, "Unsupported type for TestParse");
        }
      }();
      REQUIRE(result.Success() == expected.Success());
      REQUIRE(result.Value == expected.Value);
      REQUIRE(result.Value == expected.Value);
      REQUIRE(result.Error == expected.Error);
    }

    template <typename T>
    void TestParse(const char8 *input, T expectedValue)
    {
      TestParse<T>(input, MicroParserResult<T> {.Value = expectedValue});
    }

    template <typename T>
    void TestParse(const char8 *input, MicroParserError expectedError)
    {
      TestParse<T>(input, MicroParserResult<T> {.Error = expectedError});
    }
  }

  TEST_CASE("Numbers::ParseInteger", "[MicroParsers]")
  {
    SECTION("Valid")
    {
      TestParse<int64>(u8"  -12345abc", -12'345);
      TestParse<int64>(u8"  +67890", 67'890);
      TestParse<int64>(u8"  0", 0);
      TestParse<int64>(u8"  -0", 0);
      TestParse<int64>(u8"  42", 42);
    }

    SECTION("Invalid")
    {
      TestParse<int64>(u8"  -abc", MicroParserError::InvalidCharacter);
      TestParse<int64>(u8"  abc", MicroParserError::InvalidCharacter);
      TestParse<int64>(u8"  ", MicroParserError::UnexpectedEndOfInput);
      TestParse<int64>(u8"", MicroParserError::UnexpectedEndOfInput);
    }
  }

  TEST_CASE("Numbers::ParseNonNegativeInteger", "[MicroParsers]")
  {
    SECTION("Valid")
    {
      TestParse<uint64>(u8"  +67890", 67'890);
      TestParse<uint64>(u8"  0", 0);
      TestParse<uint64>(u8"  42", 42);
    }

    SECTION("Invalid")
    {
      TestParse<uint64>(u8"  -12345abc", MicroParserError::InvalidSign);
      TestParse<uint64>(u8"  -0", MicroParserError::InvalidSign);
      TestParse<uint64>(u8"  abc", MicroParserError::InvalidCharacter);
      TestParse<uint64>(u8"  ", MicroParserError::UnexpectedEndOfInput);
      TestParse<uint64>(u8"", MicroParserError::UnexpectedEndOfInput);
    }
  }

  TEST_CASE("Numbers::ParseFloatingPoint", "[MicroParsers]")
  {
    SECTION("Valid")
    {
      TestParse<double>(u8"  +67890", 67'890.0);
      TestParse<double>(u8"  -12345abc", -12345.0);
      TestParse<double>(u8"  0", 0.0);
      TestParse<double>(u8"  -0", 0.0);
      TestParse<double>(u8"  42", 42.0);
      TestParse<double>(u8"  .12", 0.12);
      TestParse<double>(u8"  1.12", 1.12);
      TestParse<double>(u8"  -1.12", -1.12);
      TestParse<double>(u8"  -.12", -.12);
      TestParse<double>(u8"  1e2", 100.0);
      TestParse<double>(u8"  1.5e2", 150.0);
      TestParse<double>(u8"  1.5e-2", 0.015);
      TestParse<double>(u8"  -1.5e-2", -0.015);
      TestParse<double>(u8"  .5e2", 50.0);
    }

    SECTION("Invalid")
    {
      TestParse<double>(u8"  abc", MicroParserError::InvalidCharacter);
      TestParse<double>(u8"  ", MicroParserError::UnexpectedEndOfInput);
      TestParse<double>(u8"", MicroParserError::UnexpectedEndOfInput);
      TestParse<double>(u8"  -abc", MicroParserError::InvalidCharacter);
      TestParse<double>(u8"  -", MicroParserError::UnexpectedEndOfInput);
      TestParse<double>(u8"  +", MicroParserError::UnexpectedEndOfInput);
      TestParse<double>(u8"  .", MicroParserError::InvalidCharacter);
    }
  }
}