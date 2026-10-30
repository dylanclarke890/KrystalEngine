#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <concepts>
#include <type_traits>

namespace krys
{
  enum class HexConversionMode : bool
  {
    Lowercase,
    Uppercase
  };

  /// @brief A number written in hexadecimal by a string builder, padded with zeros to a minimum number of
  /// digits.
  class HexNumber
  {
  private:
    uint64 _number;
    uint32 _minimumDigits;
    HexConversionMode _mode;

  public:
    constexpr HexNumber(uint64 number, uint32 minimumDigits, HexConversionMode mode) noexcept
        : _number(number), _minimumDigits(minimumDigits), _mode(mode)
    {
    }

    void WriteTo(StringBuilder &builder) const noexcept
    {
      constexpr auto LowercaseDigits = u8"0123456789abcdef";
      constexpr auto UppercaseDigits = u8"0123456789ABCDEF";
      const char8 *digits = _mode == HexConversionMode::Lowercase ? LowercaseDigits : UppercaseDigits;

      Array<char8, 16> buffer {};
      uint32 length = 0;
      auto number = _number;
      do
      {
        buffer[buffer.size() - ++length] = digits[number & 0xF];
        number >>= 4;
      } while (number);

      if (_minimumDigits > length)
      {
        builder.AppendRepeated(u8'0', _minimumDigits - length);
      }
      builder.Append(utf8_stringview {buffer.data() + buffer.size() - length, length});
    }
  };

  /// @brief @p number in hexadecimal, for appending to a string builder. Negative numbers are written as
  /// their two's complement.
  template <std::integral TNumber>
  KRYS_NODISCARD constexpr HexNumber Hex(TNumber number, uint32 minimumDigits = 0,
                                         HexConversionMode mode = HexConversionMode::Uppercase) noexcept
  {
    return HexNumber {static_cast<uint64>(static_cast<std::make_unsigned_t<TNumber>>(number)), minimumDigits,
                      mode};
  }

  template <std::integral TNumber>
  KRYS_NODISCARD constexpr HexNumber Hex(TNumber number, HexConversionMode mode) noexcept
  {
    return Hex(number, 0, mode);
  }
}
