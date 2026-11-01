#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <charconv>
#include <limits>
#include <string>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD bool IsFloatingPointExponentMarker(char8 c) noexcept
    {
      return c == u8'e' || c == u8'E';
    }

    /// @brief Steps 15 to 18 of the rules for parsing floating-point number values (conversion): rounds the
    /// value given by @p negative, the digits before and after its full stop, and its exponent, to a double.
    KRYS_NODISCARD MicroParserResult<double>
      ConvertFloatingPointValue(bool negative, dom::DOMStringView integerDigits,
                                dom::DOMStringView fractionDigits, bool negativeExponent,
                                dom::DOMStringView exponentDigits) noexcept
    {
      // The value is 0.<digits> times ten to the power of scale, with no leading zeros in digits.
      std::string digits;
      digits.append(reinterpret_cast<const char *>(integerDigits.data()), integerDigits.size());
      digits.append(reinterpret_cast<const char *>(fractionDigits.data()), fractionDigits.size());
      auto leadingZeros = digits.find_first_not_of('0');
      if (leadingZeros == std::string::npos)
      {
        return {.Value = 0};
      }
      digits.erase(0, leadingZeros);

      // Clamped well beyond the scales that round to zero or overflow.
      constexpr int64 MaxExponent = 1'000'000'000;
      int64 exponent = 0;
      for (auto c : exponentDigits)
      {
        exponent = std::min(exponent * 10 + (c - u8'0'), MaxExponent);
      }
      int64 scale = static_cast<int64>(integerDigits.size()) - static_cast<int64>(leadingZeros)
                    + (negativeExponent ? -exponent : exponent);

      // 15. Conversion: Let S be the set of finite IEEE 754 double-precision floating-point values except −0,
      //     but with two special values added: 2^1024 and −2^1024.
      // 16. Let rounded-value be the number in S that is closest to value, selecting the number with an even
      //     significand if there are two equally close values. (The two special values 2^1024 and −2^1024 are
      //     considered to have even significands for this purpose.)
      // 10^400 is past 2^1024, and 10^-400 is nearer to zero than to the smallest subnormal double.
      if (scale > 400)
      {
        // 17. If rounded-value is 2^1024 or −2^1024, return an error.
        return {.Error = MicroParserError::OutOfRange};
      }
      if (scale < -400)
      {
        return {.Value = 0};
      }

      auto text = (negative ? "-0." : "0.") + digits + "e" + std::to_string(scale);
      double roundedValue {};
      auto result = std::from_chars(text.data(), text.data() + text.size(), roundedValue);
      if (result.ec == std::errc::result_out_of_range)
      {
        // 17. If rounded-value is 2^1024 or −2^1024, return an error.
        if (scale > 0)
        {
          return {.Error = MicroParserError::OutOfRange};
        }
        return {.Value = 0};
      }

      // 18. Return rounded-value. (S has no −0.)
      return {.Value = roundedValue == 0 ? 0 : roundedValue};
    }
  }

  MicroParserResult<int64> Numbers::ParseInteger(dom::DOMStringView input) noexcept
  {
    // 1. Let input be the string being parsed.
    // 2. Let position be a pointer into input, initially pointing at the start of the string.
    position_variable position = input.begin();

    // 3. Let sign have the value "positive".
    bool positive = true;

    // 4. Skip ASCII whitespace within input given position.
    infra::StringAlgorithms::SkipWhitespace(input, position);

    // 5. If position is past the end of input, return an error.
    if (position == input.end())
    {
      return {.Error = MicroParserError::UnexpectedEndOfInput};
    }

    // 6. If the character indicated by position (the first character) is a U+002D HYPHEN-MINUS character (-):
    if (*position == u8'-')
    {
      // 6a. Let sign be "negative".
      positive = false;

      // 6b. Advance position to the next character.
      ++position;

      // 6c. If position is past the end of input, return an error.
      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }
    }
    // Otherwise, if the character indicated by position (the first character) is a U+002B PLUS SIGN character
    // (+):
    else if (*position == u8'+')
    {
      // 6a. Advance position to the next character. (The "+" is ignored, but it is not conforming.)
      ++position;

      // 6b. If position is past the end of input, return an error.
      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }
    }

    // 7. If the character indicated by position is not an ASCII digit, then return an error.
    if (!text::IsASCIIDigit(*position))
    {
      return {.Error = MicroParserError::InvalidCharacter};
    }

    // 8. Collect a sequence of code points that are ASCII digits from input given position, and interpret the
    //    resulting sequence as a base-ten integer. Let value be that integer.
    auto digits =
      infra::StringAlgorithms::CollectCodePointSequence(input, position, text::IsASCIIDigit<char8>);
    auto from = reinterpret_cast<const char *>(digits.data());
    uint64 value {};
    auto result = std::from_chars(from, from + digits.size(), value);
    // A negative value can be one further from zero than a positive one.
    constexpr auto MaxPositive = static_cast<uint64>(std::numeric_limits<int64>::max());
    if (result.ec == std::errc::result_out_of_range || value > MaxPositive + (positive ? 0 : 1))
    {
      return {.Error = MicroParserError::OutOfRange};
    }

    // 9. If sign is "positive", return value, otherwise return the result of subtracting value from zero.
    return {.Value = positive ? static_cast<int64>(value) : static_cast<int64>(0 - value)};
  }

  MicroParserResult<uint64> Numbers::ParseNonNegativeInteger(dom::DOMStringView input) noexcept
  {
    // 1. Let input be the string being parsed.
    // 2. Let value be the result of parsing input using the rules for parsing integers.
    auto value = ParseInteger(input);

    // 3. If value is an error, return an error.
    if (!value.Success())
    {
      return {.Error = value.Error};
    }

    // 4. If value is less than zero, return an error.
    if (value.Value < 0)
    {
      return {.Error = MicroParserError::InvalidSign};
    }

    // 5. Return value.
    return {.Value = static_cast<uint64>(value.Value)};
  }

  MicroParserResult<uint64> Numbers::ParseValidNonNegativeInteger(dom::DOMStringView input) noexcept
  {
    // A string is a valid non-negative integer if it consists of one or more ASCII digits.
    if (input.empty())
    {
      return {.Error = MicroParserError::UnexpectedEndOfInput};
    }
    for (auto c : input)
    {
      if (!text::IsASCIIDigit(c))
      {
        return {.Error = MicroParserError::InvalidCharacter};
      }
    }
    return ParseNonNegativeInteger(input);
  }

  bool Numbers::IsValidFloatingPointNumber(dom::DOMStringView input) noexcept
  {
    // A string is a valid floating-point number if it consists of:
    position_variable position = input.begin();
    auto collectDigits = [&]()
    {
      return infra::StringAlgorithms::CollectCodePointSequence(input, position, text::IsASCIIDigit<char8>);
    };

    // 1. Optionally, a U+002D HYPHEN-MINUS character (-).
    if (position != input.end() && *position == u8'-')
    {
      ++position;
    }

    // 2. One or both of the following, in the given order:
    // 2a. A series of one or more ASCII digits.
    bool hasIntegerDigits = !collectDigits().empty();

    // 2b. Both of the following, in the given order:
    // 2b.i. A single U+002E FULL STOP character (.).
    // 2b.ii. A series of one or more ASCII digits.
    bool hasFraction = false;
    if (position != input.end() && *position == u8'.')
    {
      ++position;
      if (collectDigits().empty())
      {
        return false;
      }
      hasFraction = true;
    }
    if (!hasIntegerDigits && !hasFraction)
    {
      return false;
    }

    // 3. Optionally:
    // 3a. Either a U+0065 LATIN SMALL LETTER E character (e) or a U+0045 LATIN CAPITAL LETTER E character
    // (E).
    if (position != input.end() && IsFloatingPointExponentMarker(*position))
    {
      ++position;

      // 3b. Optionally, a U+002D HYPHEN-MINUS character (-) or U+002B PLUS SIGN character (+).
      if (position != input.end() && (*position == u8'-' || *position == u8'+'))
      {
        ++position;
      }

      // 3c. A series of one or more ASCII digits.
      if (collectDigits().empty())
      {
        return false;
      }
    }

    return position == input.end();
  }

  MicroParserResult<double> Numbers::ParseFloatingPoint(dom::DOMStringView input) noexcept
  {
    // 1. Let input be the string being parsed.
    // 2. Let position be a pointer into input, initially pointing at the start of the string.
    position_variable position = input.begin();

    // 3. Let value have the value 1.
    // 4. Let divisor have the value 1.
    // 5. Let exponent have the value 1.
    // value, divisor and exponent are kept as the sign, the digits before and after the full stop, and the
    // exponent's sign and digits, so that conversion rounds the exact value.
    bool negative = false;
    dom::DOMStringView integerDigits;
    dom::DOMStringView fractionDigits;
    bool negativeExponent = false;
    dom::DOMStringView exponentDigits;
    auto conversion = [&]()
    {
      return ConvertFloatingPointValue(negative, integerDigits, fractionDigits, negativeExponent,
                                       exponentDigits);
    };
    auto collectDigits = [&]()
    {
      return infra::StringAlgorithms::CollectCodePointSequence(input, position, text::IsASCIIDigit<char8>);
    };

    // 6. Skip ASCII whitespace within input given position.
    infra::StringAlgorithms::SkipWhitespace(input, position);

    // 7. If position is past the end of input, return an error.
    if (position == input.end())
    {
      return {.Error = MicroParserError::UnexpectedEndOfInput};
    }

    // 8. If the character indicated by position is a U+002D HYPHEN-MINUS character (-):
    if (*position == u8'-')
    {
      // 8a. Change value and divisor to −1.
      negative = true;

      // 8b. Advance position to the next character.
      ++position;

      // 8c. If position is past the end of input, return an error.
      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }
    }
    // Otherwise, if the character indicated by position (the first character) is a U+002B PLUS SIGN character
    // (+):
    else if (*position == u8'+')
    {
      // 8a. Advance position to the next character. (The "+" is ignored, but it is not conforming.)
      ++position;

      // 8b. If position is past the end of input, return an error.
      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }
    }

    // 9. If the character indicated by position is a U+002E FULL STOP (.), and that is not the last character
    //    in input, and the character after the character indicated by position is an ASCII digit, then set
    //    value to zero and jump to the step labeled fraction.
    if (!(*position == u8'.' && position + 1 != input.end() && text::IsASCIIDigit(*(position + 1))))
    {
      // 10. If the character indicated by position is not an ASCII digit, then return an error.
      if (!text::IsASCIIDigit(*position))
      {
        return {.Error = MicroParserError::InvalidCharacter};
      }

      // 11. Collect a sequence of code points that are ASCII digits from input given position, and interpret
      //     the resulting sequence as a base-ten integer. Multiply value by that integer.
      integerDigits = collectDigits();

      // 12. If position is past the end of input, jump to the step labeled conversion.
      if (position == input.end())
      {
        return conversion();
      }
    }

    // 13. Fraction: If the character indicated by position is a U+002E FULL STOP (.), run these substeps:
    if (*position == u8'.')
    {
      // 13a. Advance position to the next character.
      ++position;

      // 13b. If position is past the end of input, or if the character indicated by position is not an ASCII
      //      digit, U+0065 LATIN SMALL LETTER E (e), or U+0045 LATIN CAPITAL LETTER E (E), then jump to the
      //      step labeled conversion.
      if (position == input.end()
          || (!text::IsASCIIDigit(*position) && !IsFloatingPointExponentMarker(*position)))
      {
        return conversion();
      }

      // 13c. If the character indicated by position is a U+0065 LATIN SMALL LETTER E character (e) or a
      // U+0045
      //      LATIN CAPITAL LETTER E character (E), skip the remainder of these substeps.
      if (!IsFloatingPointExponentMarker(*position))
      {
        // 13d. Fraction loop: Multiply divisor by ten.
        // 13e. Add the value of the character indicated by position, interpreted as a base-ten digit (0..9)
        //      and divided by divisor, to value.
        // 13f. Advance position to the next character.
        // 13h. If the character indicated by position is an ASCII digit, jump back to the step labeled
        //      fraction loop in these substeps.
        fractionDigits = collectDigits();

        // 13g. If position is past the end of input, then jump to the step labeled conversion.
        if (position == input.end())
        {
          return conversion();
        }
      }
    }

    // 14. If the character indicated by position is U+0065 (e) or a U+0045 (E):
    if (IsFloatingPointExponentMarker(*position))
    {
      // 14a. Advance position to the next character.
      ++position;

      // 14b. If position is past the end of input, then jump to the step labeled conversion.
      if (position == input.end())
      {
        return conversion();
      }

      // 14c. If the character indicated by position is a U+002D HYPHEN-MINUS character (-):
      if (*position == u8'-')
      {
        // 14c.i. Change exponent to −1.
        negativeExponent = true;

        // 14c.ii. Advance position to the next character.
        ++position;

        // 14c.iii. If position is past the end of input, then jump to the step labeled conversion.
        if (position == input.end())
        {
          return conversion();
        }
      }
      // Otherwise, if the character indicated by position is a U+002B PLUS SIGN character (+):
      else if (*position == u8'+')
      {
        // 14c.i. Advance position to the next character.
        ++position;

        // 14c.ii. If position is past the end of input, then jump to the step labeled conversion.
        if (position == input.end())
        {
          return conversion();
        }
      }

      // 14d. If the character indicated by position is not an ASCII digit, then jump to the step labeled
      //      conversion.
      if (!text::IsASCIIDigit(*position))
      {
        return conversion();
      }

      // 14e. Collect a sequence of code points that are ASCII digits from input given position, and interpret
      //      the resulting sequence as a base-ten integer. Multiply exponent by that integer.
      // 14f. Multiply value by ten raised to the exponentth power.
      exponentDigits = collectDigits();
    }

    return conversion();
  }

  MicroParserResult<double> Numbers::ParseValidFloatingPoint(dom::DOMStringView input) noexcept
  {
    if (!IsValidFloatingPointNumber(input))
    {
      return {.Error = MicroParserError::InvalidCharacter};
    }
    return ParseFloatingPoint(input);
  }

  List<double> Numbers::ParseListOfFloatingPointNumbers(dom::DOMStringView input) noexcept
  {
    constexpr auto isSpaceOrDelimiter = [](char8 c)
    {
      return text::IsASCIIWhitespace(c) || c == u8',' || c == u8';';
    };
    constexpr auto isNumberStart = [](char8 c)
    {
      return text::IsASCIIDigit(c) || c == u8'.' || c == u8'-';
    };

    // 1. Let input be the string being parsed.
    // 2. Let position be a pointer into input, initially pointing at the start of the string.
    position_variable position = input.begin();

    // 3. Let numbers be an initially empty list of floating-point numbers. This list will be the result of
    //    this algorithm.
    List<double> numbers;

    // 4. Collect a sequence of code points that are ASCII whitespace, U+002C COMMA, or U+003B SEMICOLON
    //    characters from input given position. This skips past any leading delimiters.
    infra::StringAlgorithms::AdvancePositionWhile(input, position, isSpaceOrDelimiter);

    // 5. While position is not past the end of input:
    while (position != input.end())
    {
      // 5a. Collect a sequence of code points that are not ASCII whitespace, U+002C COMMA, U+003B SEMICOLON,
      //     ASCII digits, U+002E FULL STOP, or U+002D HYPHEN-MINUS characters from input given position. This
      //     skips past leading garbage.
      infra::StringAlgorithms::AdvancePositionWhile(input, position, [&](char8 c)
                                                    { return !isSpaceOrDelimiter(c) && !isNumberStart(c); });

      // 5b. Collect a sequence of code points that are not ASCII whitespace, U+002C COMMA, or U+003B
      // SEMICOLON
      //     characters from input given position, and let unparsed number be the result.
      auto unparsedNumber = infra::StringAlgorithms::CollectCodePointSequence(
        input, position, [&](char8 c) { return !isSpaceOrDelimiter(c); });

      // 5c. Let number be the result of parsing unparsed number using the rules for parsing floating-point
      //     number values.
      auto number = ParseFloatingPoint(unparsedNumber);

      // 5d. If number is an error, set number to zero.
      // 5e. Append number to numbers.
      numbers.push_back(number.Success() ? number.Value : 0);

      // 5f. Collect a sequence of code points that are ASCII whitespace, U+002C COMMA, or U+003B SEMICOLON
      //     characters from input given position. This skips past the delimiter.
      infra::StringAlgorithms::AdvancePositionWhile(input, position, isSpaceOrDelimiter);
    }

    // 6. Return numbers.
    return numbers;
  }
}
