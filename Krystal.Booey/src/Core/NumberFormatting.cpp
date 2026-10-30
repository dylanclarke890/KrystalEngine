#include "Krystal.Booey/Core/NumberFormatting.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <charconv>
#include <cmath>
#include <string>
#include <string_view>

namespace krys::boo
{
  void FormatCSSNumber(StringBuilder &builder, double number) noexcept
  {
    if (std::isnan(number))
    {
      builder.Append(u8"NaN");
      return;
    }

    if (std::isinf(number))
    {
      builder.Append(number < 0 ? u8"-infinity" : u8"infinity");
      return;
    }

    Array<char, 512uz> buffer;
    auto result =
      std::to_chars(buffer.data(), buffer.data() + buffer.size(), number, std::chars_format::fixed, 6);
    krys_debug_assert(result.ec == std::errc {});

    std::string_view formatted(buffer.data(), result.ptr);
    if (formatted.find('.') != std::string_view::npos)
    {
      while (formatted.ends_with('0'))
      {
        formatted.remove_suffix(1);
      }

      if (formatted.ends_with('.'))
      {
        formatted.remove_suffix(1);
      }
    }

    // If we've truncated the trailing zeros and a trailing decimal, we may have a -0. Remove the negative
    // sign in this case.
    if (formatted == "-0")
    {
      formatted.remove_prefix(1);
    }

    builder.Append(formatted);
  }

  utf8_string FormatCSSNumber(double number) noexcept
  {
    StringBuilder builder;
    FormatCSSNumber(builder, number);
    return krys::move(builder).ToString();
  }

  void FormatNumberFixedPrecision(StringBuilder &builder, double number, uint32 significantFigures,
                                  bool truncateTrailingZeros) noexcept
  {
    krys_debug_assert(significantFigures >= 1 && significantFigures <= 100);

    if (std::isnan(number))
    {
      builder.Append(u8"NaN");
      return;
    }

    if (std::isinf(number))
    {
      builder.Append(number < 0 ? u8"-Infinity" : u8"Infinity");
      return;
    }

    // Round to the requested number of significant digits, then lay the digits out by hand.
    Array<char, 128uz> buffer;
    auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), std::abs(number),
                                std::chars_format::scientific, static_cast<int>(significantFigures) - 1);
    krys_debug_assert(result.ec == std::errc {});

    stringview scientific(buffer.data(), result.ptr);
    auto exponentPosition = scientific.find('e');
    string digits;
    for (char c : scientific.substr(0, exponentPosition))
    {
      if (c != '.')
      {
        digits.push_back(c);
      }
    }

    int exponent = 0;
    auto exponentText = scientific.substr(exponentPosition + 1);
    std::from_chars(exponentText.data() + (exponentText.front() == '+' ? 1 : 0),
                    exponentText.data() + exponentText.size(), exponent);

    string formatted;
    // -0 is written as 0.
    if (number < 0)
    {
      formatted.push_back('-');
    }

    auto mantissaStart = formatted.size();
    bool exponential = exponent < -6 || exponent >= static_cast<int>(significantFigures);
    if (exponential)
    {
      formatted.push_back(digits[0]);
      if (digits.size() > 1)
      {
        formatted.push_back('.');
        formatted.append(digits, 1);
      }
    }
    else if (exponent >= 0)
    {
      formatted.append(digits, 0, static_cast<size_t>(exponent) + 1);
      if (digits.size() > static_cast<size_t>(exponent) + 1)
      {
        formatted.push_back('.');
        formatted.append(digits, static_cast<size_t>(exponent) + 1);
      }
    }
    else
    {
      formatted.append("0.");
      formatted.append(static_cast<size_t>(-exponent - 1), '0');
      formatted.append(digits);
    }

    if (truncateTrailingZeros && formatted.find('.', mantissaStart) != string::npos)
    {
      while (formatted.ends_with('0'))
      {
        formatted.pop_back();
      }
      if (formatted.ends_with('.'))
      {
        formatted.pop_back();
      }
    }

    builder.Append(formatted);
    if (exponential)
    {
      builder.Append(u8'e', exponent < 0 ? u8'-' : u8'+', exponent < 0 ? -exponent : exponent);
    }
  }
}
