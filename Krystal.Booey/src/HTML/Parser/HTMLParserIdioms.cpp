#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <charconv>
#include <cmath>

namespace krys::boo::html
{
  dom::DOMString SerializeForNumberType(const Decimal &number) noexcept
  {
    if (number.IsZero())
    {
      // Decimal::ToString appends exponent, e.g. "0e-18"
      return number.IsNegative() ? u8"-0" : u8"0";
    }
    return number.ToString();
  }

  dom::DOMString SerializeForNumberType(double number) noexcept
  {
    // According to HTML5, "the best representation of the number n as a floating point number" is a string
    // produced by applying ToString() to n.
    // https://tc39.es/ecma262/#sec-numeric-types-number-tostring
    if (std::isnan(number))
    {
      return u8"NaN";
    }
    if (number == 0)
    {
      return u8"0";
    }
    if (std::isinf(number))
    {
      return number < 0 ? u8"-Infinity" : u8"Infinity";
    }

    dom::DOMString result;
    if (number < 0)
    {
      result += u8'-';
      number = -number;
    }

    // The shortest digits that round-trip, and the exponent n of the value s × 10^(n − k).
    char buffer[40];
    auto end = std::to_chars(buffer, buffer + sizeof(buffer), number, std::chars_format::scientific).ptr;
    std::string_view scientific(buffer, end - buffer);
    auto exponentPosition = scientific.find('e');
    std::string digits;
    for (char character : scientific.substr(0, exponentPosition))
    {
      if (character != '.')
      {
        digits += character;
      }
    }
    int exponent = 0;
    std::from_chars(scientific.data() + exponentPosition + 1 + (scientific[exponentPosition + 1] == '+'),
                    scientific.data() + scientific.size(), exponent);
    int k = static_cast<int>(digits.size());
    int n = exponent + 1;

    auto append = [&](std::string_view text)
    {
      result.append(reinterpret_cast<const char8 *>(text.data()), text.size());
    };

    if (k <= n && n <= 21)
    {
      append(digits);
      result.append(static_cast<size_t>(n - k), u8'0');
    }
    else if (0 < n && n <= 21)
    {
      append(std::string_view(digits).substr(0, n));
      result += u8'.';
      append(std::string_view(digits).substr(n));
    }
    else if (-6 < n && n <= 0)
    {
      append("0.");
      result.append(static_cast<size_t>(-n), u8'0');
      append(digits);
    }
    else
    {
      result += static_cast<char8>(digits[0]);
      if (k > 1)
      {
        result += u8'.';
        append(std::string_view(digits).substr(1));
      }
      result += u8'e';
      result += n - 1 < 0 ? u8'-' : u8'+';
      append(std::to_string(std::abs(n - 1)));
    }
    return result;
  }

  Decimal ParseToDecimalForNumberType(dom::DOMStringView string, const Decimal &fallbackValue) noexcept
  {
    // https://html.spec.whatwg.org/#floating-point-numbers and ParseToDoubleForNumberType
    if (string.empty())
    {
      return fallbackValue;
    }

    // Leading + and whitespace characters are not valid here.
    char8 firstCharacter = string[0];
    if (firstCharacter != u8'-' && firstCharacter != u8'.' && !text::IsASCIIDigit(firstCharacter))
    {
      return fallbackValue;
    }

    const Decimal value = Decimal::FromString(string);
    if (!value.IsFinite())
    {
      return fallbackValue;
    }

    // Numbers are considered finite IEEE 754 Double-precision floating point values.
    const Decimal doubleMax = Decimal::DoubleMax();
    if (value < -doubleMax || value > doubleMax)
    {
      return fallbackValue;
    }

    // We return +0 for -0 case.
    return value.IsZero() ? Decimal(0) : value;
  }

  Decimal ParseToDecimalForNumberType(dom::DOMStringView string) noexcept
  {
    return ParseToDecimalForNumberType(string, Decimal::NaN());
  }

  double ParseToDoubleForNumberType(dom::DOMStringView string, double fallbackValue) noexcept
  {
    // https://html.spec.whatwg.org/#floating-point-numbers
    if (string.empty())
    {
      return fallbackValue;
    }

    // Leading + and whitespace characters are not valid here.
    char8 firstCharacter = string[0];
    if (firstCharacter != u8'-' && firstCharacter != u8'.' && !text::IsASCIIDigit(firstCharacter))
    {
      return fallbackValue;
    }

    if (string.back() == u8'.')
    {
      return fallbackValue;
    }

    double value = 0;
    auto first = reinterpret_cast<const char *>(string.data());
    auto last = first + string.size();
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc {} || result.ptr != last)
    {
      return fallbackValue;
    }

    // NaN and infinity are not valid here.
    if (!std::isfinite(value))
    {
      return fallbackValue;
    }

    // Numbers are considered finite IEEE 754 Double-precision floating point values.
    if (-std::numeric_limits<double>::max() > value || value > std::numeric_limits<double>::max())
    {
      return fallbackValue;
    }

    // The following expression converts -0 to +0.
    return value ? value : 0;
  }

  double ParseToDoubleForNumberType(dom::DOMStringView string) noexcept
  {
    return ParseToDoubleForNumberType(string, std::numeric_limits<double>::quiet_NaN());
  }
}
