#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/MicroParsers/MicroParserResult.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include <charconv>
#include <cmath>

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#numbers
  class Numbers
  {
  public:
    using position_variable = infra::StringAlgorithms::position_variable;

    /// @see https://html.spec.whatwg.org/#rules-for-parsing-integers
    KRYS_NODISCARD static MicroParserResult<int64> ParseInteger(dom::DOMStringView input) noexcept
    {
      position_variable position = input.begin();
      infra::StringAlgorithms::SkipWhitespace(input, position);

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

      if (!krys::text::IsASCIIDigit(*position))
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
    KRYS_NODISCARD static MicroParserResult<uint64> ParseNonNegativeInteger(dom::DOMStringView input) noexcept
    {
      position_variable position = input.begin();
      infra::StringAlgorithms::SkipWhitespace(input, position);

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

      if (!krys::text::IsASCIIDigit(*position))
      {
        return {.Error = MicroParserError::InvalidCharacter};
      }

      return ParseNumber<uint64>(position, input);
    }

    /// @see https://html.spec.whatwg.org/#rules-for-parsing-floating-point-number-values
    KRYS_NODISCARD static MicroParserResult<double> ParseFloatingPoint(dom::DOMStringView input) noexcept
    {
      position_variable position = input.begin();
      infra::StringAlgorithms::SkipWhitespace(input, position);

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

    /// @brief Parses @p input only if all of it is a valid non-negative integer: one or more ASCII digits.
    /// @see https://html.spec.whatwg.org/#valid-non-negative-integer
    KRYS_NODISCARD static MicroParserResult<uint64>
      ParseValidNonNegativeInteger(dom::DOMStringView input) noexcept
    {
      if (input.empty())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }
      for (auto c : input)
      {
        if (!krys::text::IsASCIIDigit(c))
        {
          return {.Error = MicroParserError::InvalidCharacter};
        }
      }
      position_variable position = input.begin();
      return ParseNumber<uint64>(position, input);
    }

    /// @brief Parses @p input only if all of it is a valid floating-point number: no leading `+`, no trailing
    /// `.`, and finite.
    /// @see https://html.spec.whatwg.org/#valid-floating-point-number
    KRYS_NODISCARD static MicroParserResult<double> ParseValidFloatingPoint(dom::DOMStringView input) noexcept
    {
      if (input.empty())
      {
        return {.Error = MicroParserError::UnexpectedEndOfInput};
      }
      if (input.front() == '+' || input.back() == '.')
      {
        return {.Error = MicroParserError::InvalidCharacter};
      }

      double value {};
      auto from = reinterpret_cast<const char *>(input.data());
      auto to = from + input.size();
      auto result = std::from_chars(from, to, value);
      if (result.ec != std::errc {} || result.ptr != to || !std::isfinite(value))
      {
        return {.Error = MicroParserError::InvalidCharacter};
      }
      return {.Value = value};
    }

    template <Number T>
    KRYS_NODISCARD static MicroParserResult<T> ParseNumber(Numbers::position_variable &position,
                                                           dom::DOMStringView &input)
    {
      const char *from = reinterpret_cast<const char *>(&*position);

      if constexpr (SameType<T, double>)
      {
        constexpr char DecimalPoint = '.';
        constexpr char Exponent = 'e';
        constexpr char Plus = '+';
        constexpr char Minus = '-';
        infra::StringAlgorithms::AdvancePositionWhile(input, position,
                                                      [&](char8 c)
                                                      {
                                                        return c == DecimalPoint || c == Plus || c == Minus
                                                               || krys::text::ToASCIILowerUnchecked(c)
                                                                    == Exponent
                                                               || krys::text::IsASCIIDigit<char8>(c);
                                                      });
      }
      else
      {
        infra::StringAlgorithms::AdvancePositionWhile(input, position, krys::text::IsASCIIDigit<char8>);
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