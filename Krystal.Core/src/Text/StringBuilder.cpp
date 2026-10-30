#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <charconv>
#include <cmath>
#include <cstdlib>

namespace krys
{
  namespace
  {
    /// @brief Writes @p number's shortest round-trip digits as ECMAScript's `Number.prototype.toString` does:
    /// fixed notation when the decimal exponent is in [-6, 21), else `d.ddde±x`.
    template <std::floating_point T>
    void AppendShortest(utf8_string &buffer, T number) noexcept
    {
      if (std::isnan(number))
      {
        buffer.append(u8"NaN");
        return;
      }
      if (std::isinf(number))
      {
        buffer.append(number < 0 ? u8"-Infinity" : u8"Infinity");
        return;
      }
      if (number == 0)
      {
        // Negative zero is written as "0".
        buffer.push_back(u8'0');
        return;
      }

      // The shortest round-trip digits, in scientific notation: "-d.ddde±xx".
      Array<char, 64> scientific {};
      auto result = std::to_chars(scientific.data(), scientific.data() + scientific.size(), number,
                                  std::chars_format::scientific);
      krys_assert(result.ec == std::errc {});
      std::string_view text {scientific.data(), result.ptr};

      bool negative = text.front() == '-';
      if (negative)
      {
        text.remove_prefix(1);
      }

      auto e = text.find('e');
      std::string_view mantissa = text.substr(0, e);
      int32 exponent = std::atoi(text.substr(e + 1).data());

      // The significant digits, without the decimal point.
      Array<char, 32> digits {};
      size_t digitCount = 0;
      for (char c : mantissa)
      {
        if (c != '.')
        {
          digits[digitCount++] = c;
        }
      }
      std::string_view significand {digits.data(), digitCount};

      auto append = [&](std::string_view s)
      { buffer.append(reinterpret_cast<const char8 *>(s.data()), s.length()); };

      if (negative)
      {
        buffer.push_back(u8'-');
      }

      // The number of digits before the decimal point.
      int32 integralDigits = exponent + 1;
      if (-6 <= exponent && exponent < 21)
      {
        if (integralDigits <= 0)
        {
          // 0.000ddd
          append("0.");
          buffer.append(static_cast<size_t>(-integralDigits), u8'0');
          append(significand);
        }
        else if (static_cast<size_t>(integralDigits) >= digitCount)
        {
          // ddd000
          append(significand);
          buffer.append(static_cast<size_t>(integralDigits) - digitCount, u8'0');
        }
        else
        {
          // dd.ddd
          append(significand.substr(0, static_cast<size_t>(integralDigits)));
          buffer.push_back(u8'.');
          append(significand.substr(static_cast<size_t>(integralDigits)));
        }
        return;
      }

      // d.ddde±x
      append(significand.substr(0, 1));
      if (digitCount > 1)
      {
        buffer.push_back(u8'.');
        append(significand.substr(1));
      }
      buffer.push_back(u8'e');
      buffer.push_back(exponent < 0 ? u8'-' : u8'+');
      Array<char, 8> exponentDigits {};
      auto exponentResult = std::to_chars(exponentDigits.data(), exponentDigits.data() + exponentDigits.size(),
                                          exponent < 0 ? -exponent : exponent);
      append(std::string_view {exponentDigits.data(), exponentResult.ptr});
    }

    template <std::integral T>
    void AppendDecimal(utf8_string &buffer, T number) noexcept
    {
      Array<char, 24> digits {};
      auto result = std::to_chars(digits.data(), digits.data() + digits.size(), number);
      buffer.append(reinterpret_cast<const char8 *>(digits.data()), static_cast<size_t>(result.ptr - digits.data()));
    }
  }

  bool StringBuilder::ContainsOnlyASCII() const noexcept
  {
    for (char8 c : _buffer)
    {
      if (c >= 0x80)
      {
        return false;
      }
    }
    return true;
  }

  void StringBuilder::AppendASCII(std::string_view string) noexcept
  {
    for (KRYS_MAYBE_UNUSED char c : string)
    {
      krys_debug_assert(static_cast<unsigned char>(c) < 0x80);
    }
    _buffer.append(reinterpret_cast<const char8 *>(string.data()), string.length());
  }

  void StringBuilder::AppendCodePoint(char32 codePoint) noexcept
  {
    krys_debug_assert(codePoint <= 0x10FFFF && (codePoint < 0xD800 || codePoint > 0xDFFF));
    if (codePoint < 0x80)
    {
      _buffer.push_back(static_cast<char8>(codePoint));
    }
    else if (codePoint < 0x800)
    {
      _buffer.push_back(static_cast<char8>(0xC0 | (codePoint >> 6)));
      _buffer.push_back(static_cast<char8>(0x80 | (codePoint & 0x3F)));
    }
    else if (codePoint < 0x10000)
    {
      _buffer.push_back(static_cast<char8>(0xE0 | (codePoint >> 12)));
      _buffer.push_back(static_cast<char8>(0x80 | ((codePoint >> 6) & 0x3F)));
      _buffer.push_back(static_cast<char8>(0x80 | (codePoint & 0x3F)));
    }
    else
    {
      _buffer.push_back(static_cast<char8>(0xF0 | (codePoint >> 18)));
      _buffer.push_back(static_cast<char8>(0x80 | ((codePoint >> 12) & 0x3F)));
      _buffer.push_back(static_cast<char8>(0x80 | ((codePoint >> 6) & 0x3F)));
      _buffer.push_back(static_cast<char8>(0x80 | (codePoint & 0x3F)));
    }
  }

  void StringBuilder::AppendInteger(int64 number) noexcept
  {
    AppendDecimal(_buffer, number);
  }

  void StringBuilder::AppendInteger(uint64 number) noexcept
  {
    AppendDecimal(_buffer, number);
  }

  void StringBuilder::AppendNumber(float number) noexcept
  {
    AppendShortest(_buffer, number);
  }

  void StringBuilder::AppendNumber(double number) noexcept
  {
    AppendShortest(_buffer, number);
  }
}
