#pragma once

#include "Krystal.HTML/HTML/MicroParsers/MicroParserResult.hpp"
#include "Krystal.HTML/Infra/StringAlgorithms.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Text/ASCII.hpp"
#include <charconv>

namespace Krys::HTML::MicroParsers
{
  /// @see https://html.spec.whatwg.org/#numbers
  class Numbers
  {
  public:
    using position_variable = StringAlgorithms::position_variable;

    /// @see https://html.spec.whatwg.org/#rules-for-parsing-integers
    KRYS_NODISCARD static MicroParserResult<int64> ParseInteger(DOMStringView input) noexcept
    {
      position_variable position = input.begin();
      StringAlgorithms::SkipWhitespace(input, position);

      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }

      bool positive = true;
      if (*position == '-')
      {
        positive = false;
        ++position;
      }
      else if (*position == '+')
      {
        ++position;
      }

      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }

      if (!Krys::Text::IsASCIIDigit(*position))
      {
        return {.Error = MicroParserError::InvalidCharacter};
      }

      auto result = ParseNumber<int64>(position, input);
      if (!result.Success())
      {
        return result;
      }

      return {.Value = positive ? result.Value : -result.Value};
    }

    /// @see https://html.spec.whatwg.org/#rules-for-parsing-non-negative-integers
    KRYS_NODISCARD static MicroParserResult<uint64> ParseNonNegativeInteger(DOMStringView input) noexcept
    {
      position_variable position = input.begin();
      StringAlgorithms::SkipWhitespace(input, position);

      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }

      if (*position == '-')
      {
        return {.Error = MicroParserError::InvalidSign};
      }
      else if (*position == '+')
      {
        ++position;
      }

      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }

      if (!Krys::Text::IsASCIIDigit(*position))
      {
        return {.Error = MicroParserError::InvalidCharacter};
      }

      return ParseNumber<uint64>(position, input);
    }

    /// @see https://html.spec.whatwg.org/#rules-for-parsing-floating-point-number-values
    KRYS_NODISCARD static MicroParserResult<double> ParseFloatingPoint(DOMStringView input) noexcept
    {
      constexpr char DecimalPoint = '.';
      constexpr char Exponent = 'e';

      position_variable position = input.begin();
      StringAlgorithms::SkipWhitespace(input, position);

      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }

      bool positive = true;
      if (*position == '-')
      {
        positive = false;
        ++position;
      }
      else if (*position == '+')
      {
        ++position;
      }

      if (position == input.end())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }

      auto result = ParseNumber<double>(position, input);
      if (!result.Success())
      {
        return result;
      }

      return {.Value = positive ? result.Value : -result.Value};
    }

    template <Number T>
    KRYS_NODISCARD static MicroParserResult<T> ParseNumber(Numbers::position_variable &position,
                                                            DOMStringView &input)
    {
      const char *from = reinterpret_cast<const char *>(&*position);

      if constexpr (SameType<T, double>)
      {
        constexpr char DecimalPoint = '.';
        constexpr char Exponent = 'e';
        constexpr char Plus = '+';
        constexpr char Minus = '-';
        StringAlgorithms::AdvancePositionWhile(input, position,
                                               [&](char8 c)
                                               {
                                                 return c == DecimalPoint || c == Plus || c == Minus
                                                        || Krys::Text::ToASCIILowerUnchecked(c) == Exponent
                                                        || Krys::Text::IsASCIIDigit<char8>(c);
                                               });
      }
      else
      {
        StringAlgorithms::AdvancePositionWhile(input, position, Krys::Text::IsASCIIDigit<char8>);
      }

      const char *to = reinterpret_cast<const char *>(input.data() + std::distance(input.begin(), position));

      T value {};
      auto result = std::from_chars(from, to, value);

      if (result.ec == std::errc {}) KRYS_LIKELY
      {
        return {.Value = value};
      }
      else if (result.ec == std::errc::result_out_of_range)
      {
        return {.Error = MicroParserError::OutOfRange};
      }

      return {.Error = MicroParserError::InvalidCharacter};
    }
  };
}