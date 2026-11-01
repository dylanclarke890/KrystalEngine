#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "catch2/catch_test_macros.hpp"
#include <cmath>
#include <limits>

namespace krys::boo::html::tests
{
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
      INFO(reinterpret_cast<const char *>(input));
      REQUIRE(result.Success() == expected.Success());
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

  TEST_CASE("Numbers::ParseInteger: parses an optionally signed integer after whitespace",
            "[HTML][MicroParsers][Numbers][html:rules-for-parsing-integers]")
  {
    SECTION("Valid")
    {
      TestParse<int64>(u8"  -12345abc", -12'345);
      TestParse<int64>(u8"  +67890", 67'890);
      TestParse<int64>(u8"  0", 0);
      TestParse<int64>(u8"  -0", 0);
      TestParse<int64>(u8"  42", 42);
      TestParse<int64>(u8"\t\n\f\r 7 8", 7);
      TestParse<int64>(u8"0012", 12);
      TestParse<int64>(u8"9223372036854775807", std::numeric_limits<int64>::max());
      TestParse<int64>(u8"-9223372036854775808", std::numeric_limits<int64>::min());
    }

    SECTION("Invalid")
    {
      TestParse<int64>(u8"  -abc", MicroParserError::InvalidCharacter);
      TestParse<int64>(u8"  abc", MicroParserError::InvalidCharacter);
      TestParse<int64>(u8"+-1", MicroParserError::InvalidCharacter);
      TestParse<int64>(u8"--1", MicroParserError::InvalidCharacter);
      TestParse<int64>(u8".5", MicroParserError::InvalidCharacter);
      TestParse<int64>(u8"\v1", MicroParserError::InvalidCharacter);
      TestParse<int64>(u8"  ", MicroParserError::UnexpectedEndOfInput);
      TestParse<int64>(u8"", MicroParserError::UnexpectedEndOfInput);
      TestParse<int64>(u8"-", MicroParserError::UnexpectedEndOfInput);
      TestParse<int64>(u8"+", MicroParserError::UnexpectedEndOfInput);
    }

    SECTION("Outside int64's range")
    {
      TestParse<int64>(u8"9223372036854775808", MicroParserError::OutOfRange);
      TestParse<int64>(u8"-9223372036854775809", MicroParserError::OutOfRange);
      TestParse<int64>(u8"99999999999999999999999", MicroParserError::OutOfRange);
    }
  }

  TEST_CASE("Numbers::ParseNonNegativeInteger: parses an integer and rejects negative ones",
            "[HTML][MicroParsers][Numbers][html:rules-for-parsing-non-negative-integers]")
  {
    SECTION("Valid")
    {
      TestParse<uint64>(u8"  +67890", 67'890);
      TestParse<uint64>(u8"  0", 0);
      TestParse<uint64>(u8"  42", 42);
      // Negative zero isn't less than zero.
      TestParse<uint64>(u8"  -0", 0);
      TestParse<uint64>(u8"-00x", 0);
    }

    SECTION("Invalid")
    {
      TestParse<uint64>(u8"  -12345abc", MicroParserError::InvalidSign);
      TestParse<uint64>(u8"  -1", MicroParserError::InvalidSign);
      TestParse<uint64>(u8"  abc", MicroParserError::InvalidCharacter);
      TestParse<uint64>(u8"  ", MicroParserError::UnexpectedEndOfInput);
      TestParse<uint64>(u8"", MicroParserError::UnexpectedEndOfInput);
      TestParse<uint64>(u8"9223372036854775808", MicroParserError::OutOfRange);
    }
  }

  TEST_CASE("Numbers::ParseValidNonNegativeInteger: parses only ASCII digits",
            "[HTML][MicroParsers][Numbers][html:valid-non-negative-integer]")
  {
    CHECK(Numbers::ParseValidNonNegativeInteger(u8"0").Value == 0u);
    CHECK(Numbers::ParseValidNonNegativeInteger(u8"480").Value == 480u);
    CHECK(Numbers::ParseValidNonNegativeInteger(u8"007").Value == 7u);
    CHECK_FALSE(Numbers::ParseValidNonNegativeInteger(u8"").Success());
    CHECK_FALSE(Numbers::ParseValidNonNegativeInteger(u8" 1").Success());
    CHECK_FALSE(Numbers::ParseValidNonNegativeInteger(u8"1 ").Success());
    CHECK_FALSE(Numbers::ParseValidNonNegativeInteger(u8"+1").Success());
    CHECK_FALSE(Numbers::ParseValidNonNegativeInteger(u8"-0").Success());
    CHECK_FALSE(Numbers::ParseValidNonNegativeInteger(u8"1.5").Success());
  }

  TEST_CASE("Numbers::IsValidFloatingPointNumber: a sign, digits, a fraction and an exponent",
            "[HTML][MicroParsers][Numbers][html:valid-floating-point-number]")
  {
    for (auto valid : {u8"1", u8"-1", u8"0001", u8".5", u8"-.5", u8"1.5", u8"1e5", u8"1E+5", u8"1.5e-5",
                       u8".5E05", u8"-0"})
    {
      INFO(reinterpret_cast<const char *>(valid));
      CHECK(Numbers::IsValidFloatingPointNumber(valid));
    }
    for (auto invalid : {u8"", u8"+1", u8"1.", u8".", u8"-", u8"-.", u8"1e", u8"1e+", u8"1.e5", u8".e5",
                         u8" 1", u8"1 ", u8"inf", u8"NaN", u8"0x10", u8"--1", u8"1e5.5", u8"1,5"})
    {
      INFO(reinterpret_cast<const char *>(invalid));
      CHECK_FALSE(Numbers::IsValidFloatingPointNumber(invalid));
    }
  }

  TEST_CASE("Numbers::ParseValidFloatingPoint: parses only valid floating-point numbers",
            "[HTML][MicroParsers][Numbers][html:valid-floating-point-number]")
  {
    CHECK(Numbers::ParseValidFloatingPoint(u8"1.5").Value == 1.5);
    CHECK(Numbers::ParseValidFloatingPoint(u8"-2").Value == -2);
    CHECK(Numbers::ParseValidFloatingPoint(u8".5").Value == 0.5);
    CHECK(Numbers::ParseValidFloatingPoint(u8"1e2").Value == 100);
    // A valid number too small for a double is zero; one too large isn't a number.
    auto tiny = Numbers::ParseValidFloatingPoint(u8"1e-400");
    CHECK(tiny.Success());
    CHECK(tiny.Value == 0);
    CHECK_FALSE(Numbers::ParseValidFloatingPoint(u8"1e400").Success());
    CHECK_FALSE(Numbers::ParseValidFloatingPoint(u8"").Success());
    CHECK_FALSE(Numbers::ParseValidFloatingPoint(u8"+1").Success());
    CHECK_FALSE(Numbers::ParseValidFloatingPoint(u8"1.").Success());
    CHECK_FALSE(Numbers::ParseValidFloatingPoint(u8"1.e5").Success());
    CHECK_FALSE(Numbers::ParseValidFloatingPoint(u8"1x").Success());
    CHECK_FALSE(Numbers::ParseValidFloatingPoint(u8" 1").Success());
    CHECK_FALSE(Numbers::ParseValidFloatingPoint(u8"inf").Success());
  }

  TEST_CASE("Numbers::ParseFloatingPoint: parses a number after whitespace, ignoring what follows it",
            "[HTML][MicroParsers][Numbers][html:rules-for-parsing-floating-point-number-values]")
  {
    SECTION("Valid")
    {
      TestParse<double>(u8"  +67890", 67'890.0);
      TestParse<double>(u8"  -12345abc", -12345.0);
      TestParse<double>(u8"  0", 0.0);
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

    SECTION("A full stop or exponent without digits ends the number")
    {
      TestParse<double>(u8"1.", 1.0);
      TestParse<double>(u8"1.x", 1.0);
      TestParse<double>(u8"1.e2", 100.0);
      TestParse<double>(u8"1e", 1.0);
      TestParse<double>(u8"1e+", 1.0);
      TestParse<double>(u8"1e-", 1.0);
      TestParse<double>(u8"1ex", 1.0);
      TestParse<double>(u8"1e2.5", 100.0);
      TestParse<double>(u8"1.5.5", 1.5);
      TestParse<double>(u8"1-2", 1.0);
    }

    SECTION("Rounds the exact value to the nearest double")
    {
      TestParse<double>(u8"0.1", 0.1);
      TestParse<double>(u8"0.30000000000000004", 0.30000000000000004);
      TestParse<double>(u8"123456789012345678901234567890", 123456789012345678901234567890.0);
      TestParse<double>(u8"1.7976931348623158e308", std::numeric_limits<double>::max());
      TestParse<double>(u8"5e-324", std::numeric_limits<double>::denorm_min());
      TestParse<double>(u8"000000000000000000001e-20", 1e-20);
      TestParse<double>(u8"0000.00001e+4", 0.1);
    }

    SECTION("Values too small for a double are zero, never negative zero")
    {
      for (auto input : {u8"-0", u8"-0.0", u8"1e-400", u8"-1e-400", u8"2e-324", u8"0e999999999999"})
      {
        INFO(reinterpret_cast<const char *>(input));
        auto result = Numbers::ParseFloatingPoint(input);
        REQUIRE(result.Success());
        CHECK(result.Value == 0);
        CHECK_FALSE(std::signbit(result.Value));
      }
    }

    SECTION("Values that round past the largest double are errors")
    {
      TestParse<double>(u8"1e400", MicroParserError::OutOfRange);
      TestParse<double>(u8"-1e400", MicroParserError::OutOfRange);
      TestParse<double>(u8"1.797693134862315808e308", MicroParserError::OutOfRange);
      TestParse<double>(u8"1e99999999999999999999", MicroParserError::OutOfRange);
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
      TestParse<double>(u8".e1", MicroParserError::InvalidCharacter);
      TestParse<double>(u8"+-1", MicroParserError::InvalidCharacter);
      TestParse<double>(u8"--1", MicroParserError::InvalidCharacter);
      TestParse<double>(u8"inf", MicroParserError::InvalidCharacter);
    }
  }

  TEST_CASE("Numbers::ParseListOfFloatingPointNumbers: numbers between whitespace, commas and semicolons",
            "[HTML][MicroParsers][Numbers][html:rules-for-parsing-a-list-of-floating-point-numbers]")
  {
    CHECK(Numbers::ParseListOfFloatingPointNumbers(u8"1, 2;3  4") == List<double> {1, 2, 3, 4});
    CHECK(Numbers::ParseListOfFloatingPointNumbers(u8" ,;1.5,-.5,1e1, ") == List<double> {1.5, -0.5, 10});
    // Anything after a number is ignored, and a part that isn't a number is zero.
    CHECK(Numbers::ParseListOfFloatingPointNumbers(u8"10px,x,20") == List<double> {10, 0, 20});
    CHECK(Numbers::ParseListOfFloatingPointNumbers(u8"a1,-") == List<double> {1, 0});
    // Leading garbage before a number is skipped, a "+" with it.
    CHECK(Numbers::ParseListOfFloatingPointNumbers(u8"+5,1e400,1e-400") == List<double> {5, 0, 0});
    CHECK(Numbers::ParseListOfFloatingPointNumbers(u8"").empty());
    CHECK(Numbers::ParseListOfFloatingPointNumbers(u8" ;, ").empty());
    CHECK_FALSE(std::signbit(Numbers::ParseListOfFloatingPointNumbers(u8"-0")[0]));
  }
}
