#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::text
{
  template <IsChar TCodeUnit>
  constexpr TCodeUnit ASCIIReplacementCodeUnit = static_cast<TCodeUnit>('?');

  constexpr char32 MaxASCIIChar = 127u;
  constexpr char32 MaxExtendedASCIIChar = 255u;

  constexpr Array<uint8, 256uz> AsciiCaseFoldTable {
    0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u, 0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu,
    0x0fu, 0x10u, 0x11u, 0x12u, 0x13u, 0x14u, 0x15u, 0x16u, 0x17u, 0x18u, 0x19u, 0x1au, 0x1bu, 0x1cu, 0x1du,
    0x1eu, 0x1fu, 0x20u, 0x21u, 0x22u, 0x23u, 0x24u, 0x25u, 0x26u, 0x27u, 0x28u, 0x29u, 0x2au, 0x2bu, 0x2cu,
    0x2du, 0x2eu, 0x2fu, 0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u, 0x38u, 0x39u, 0x3au, 0x3bu,
    0x3cu, 0x3du, 0x3eu, 0x3fu, 0x40u, 0x61u, 0x62u, 0x63u, 0x64u, 0x65u, 0x66u, 0x67u, 0x68u, 0x69u, 0x6au,
    0x6bu, 0x6cu, 0x6du, 0x6eu, 0x6fu, 0x70u, 0x71u, 0x72u, 0x73u, 0x74u, 0x75u, 0x76u, 0x77u, 0x78u, 0x79u,
    0x7au, 0x5bu, 0x5cu, 0x5du, 0x5eu, 0x5fu, 0x60u, 0x61u, 0x62u, 0x63u, 0x64u, 0x65u, 0x66u, 0x67u, 0x68u,
    0x69u, 0x6au, 0x6bu, 0x6cu, 0x6du, 0x6eu, 0x6fu, 0x70u, 0x71u, 0x72u, 0x73u, 0x74u, 0x75u, 0x76u, 0x77u,
    0x78u, 0x79u, 0x7au, 0x7bu, 0x7cu, 0x7du, 0x7eu, 0x7fu, 0x80u, 0x81u, 0x82u, 0x83u, 0x84u, 0x85u, 0x86u,
    0x87u, 0x88u, 0x89u, 0x8au, 0x8bu, 0x8cu, 0x8du, 0x8eu, 0x8fu, 0x90u, 0x91u, 0x92u, 0x93u, 0x94u, 0x95u,
    0x96u, 0x97u, 0x98u, 0x99u, 0x9au, 0x9bu, 0x9cu, 0x9du, 0x9eu, 0x9fu, 0xa0u, 0xa1u, 0xa2u, 0xa3u, 0xa4u,
    0xa5u, 0xa6u, 0xa7u, 0xa8u, 0xa9u, 0xaau, 0xabu, 0xacu, 0xadu, 0xaeu, 0xafu, 0xb0u, 0xb1u, 0xb2u, 0xb3u,
    0xb4u, 0xb5u, 0xb6u, 0xb7u, 0xb8u, 0xb9u, 0xbau, 0xbbu, 0xbcu, 0xbdu, 0xbeu, 0xbfu, 0xc0u, 0xc1u, 0xc2u,
    0xc3u, 0xc4u, 0xc5u, 0xc6u, 0xc7u, 0xc8u, 0xc9u, 0xcau, 0xcbu, 0xccu, 0xcdu, 0xceu, 0xcfu, 0xd0u, 0xd1u,
    0xd2u, 0xd3u, 0xd4u, 0xd5u, 0xd6u, 0xd7u, 0xd8u, 0xd9u, 0xdau, 0xdbu, 0xdcu, 0xddu, 0xdeu, 0xdfu, 0xe0u,
    0xe1u, 0xe2u, 0xe3u, 0xe4u, 0xe5u, 0xe6u, 0xe7u, 0xe8u, 0xe9u, 0xeau, 0xebu, 0xecu, 0xedu, 0xeeu, 0xefu,
    0xf0u, 0xf1u, 0xf2u, 0xf3u, 0xf4u, 0xf5u, 0xf6u, 0xf7u, 0xf8u, 0xf9u, 0xfau, 0xfbu, 0xfcu, 0xfdu, 0xfeu,
    0xffu};

  template <IsCharOrByte T>
  using comparable_char_t = conditional_t<SameType<T, byte>, uchar, T>;

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCII(TChar c) noexcept
  {
    return !(static_cast<sized_unsigned_t<TChar>>(c) & ~MaxASCIIChar);
  }

  template <IsStringView TStringView>
  KRYS_NODISCARD constexpr bool IsASCII(TStringView str) noexcept
  {
    for (auto c : str)
    {
      if (!IsASCII(c))
      {
        return false;
      }
    }

    return true;
  }

#pragma region Alpha

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIILower(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch >= 'a' && ch <= 'z';
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr TChar ToASCIILower(TChar c) noexcept
  {
    using char_t = sized_unsigned_t<TChar>;
    if constexpr (SameType<char_t, uint8>)
    {
      return static_cast<TChar>(AsciiCaseFoldTable[static_cast<uint8>(c)]);
    }
    else
    {
      auto ch = static_cast<char_t>(c);
      return static_cast<TChar>(ch | (static_cast<char_t>(IsASCIIUpper(c)) << 5u));
    }
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr TChar ToASCIILowerUnchecked(TChar c) noexcept
  {
    auto ch = static_cast<sized_unsigned_t<TChar>>(c);
    return static_cast<TChar>(ch | 0x20u);
  }

  template <typename T>
  KRYS_NODISCARD std::basic_string<typename T::value_type> ToASCIILower(T input) noexcept
  {
    std::basic_string<typename T::value_type> result {};
    for (auto codePoint : input)
    {
      result.push_back(ToASCIILower<typename T::value_type>(codePoint));
    }

    return result;
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIUpper(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch >= 'A' && ch <= 'Z';
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr TChar ToASCIIUpper(TChar c) noexcept
  {
    auto ch = static_cast<sized_unsigned_t<TChar>>(c);
    return static_cast<TChar>(ch & ~(IsASCIILower(ch) << 5));
  }

  template <typename T>
  KRYS_NODISCARD std::basic_string<typename T::value_type> ToASCIIUpper(T &&input) noexcept
  {
    std::basic_string<typename T::value_type> result {};
    for (auto codePoint : input)
    {
      result.push_back(ToASCIIUpper(codePoint));
    }

    return result;
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIAlpha(TChar c) noexcept
  {
    return IsASCIILower(ToASCIILowerUnchecked(c));
  }

#pragma endregion

#pragma region Digit

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIDigit(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch >= '0' && ch <= '9';
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIBinaryDigit(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch == '0' || ch == '1';
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIOctalDigit(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch >= '0' && ch <= '7';
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool ToASCIIDigitUnchecked(TChar c) noexcept
  {
    auto ch = static_cast<sized_unsigned_t<TChar>>(c);
    return static_cast<TChar>(ch - '0');
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIAlphanumeric(TChar c) noexcept
  {
    return IsASCIIDigit(c) || IsASCIIAlpha(c);
  }

#pragma endregion

#pragma region Control

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIPrintable(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch >= ' ' && ch <= '~';
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIDigitOrPunctuation(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return (ch >= '!' && ch <= '@') || (ch >= '[' && ch <= '`') || (ch >= '{' && ch <= '~');
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIINonWhitespaceControl(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch == 0x00 || ch == 0x01 || ch == 0x07 || ch == 0x08;
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIGraphic(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch >= '!' && ch <= '~';
  }

#pragma endregion

#pragma region Hexadecimal

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIHexDigit(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return IsASCIIDigit(ch) || (ToASCIILowerUnchecked(ch) >= 'a' && ToASCIILowerUnchecked(ch) <= 'f');
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr uint8 ToASCIIHexValue(TChar c) noexcept
  {
    auto ch = static_cast<sized_unsigned_t<TChar>>(c);
    krys_debug_assert(IsASCIIHexDigit<TChar>(ch));

    return ch < 'A' ? ch - '0' : (ch - 'A' + 10u) & 0xFu;
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr uint8 ToASCIIHexValue(TChar first, TChar second) noexcept
  {
    using T = conditional_t<SameType<TChar, byte>, char, TChar>;

    auto a = static_cast<T>(first);
    auto b = static_cast<T>(second);

    return ToASCIIHexValue(a) << 4u | ToASCIIHexValue(b);
  }

  KRYS_NODISCARD constexpr char LowerNibbleToASCIIHexDigit(uint8 value) noexcept
  {
    uint8 nibble = value & 0xF;
    return nibble + (nibble < 10 ? '0' : 'A' - 10);
  }

  KRYS_NODISCARD constexpr char UpperNibbleToASCIIHexDigit(uint8 value) noexcept
  {
    uint8 nibble = value >> 4;
    return nibble + (nibble < 10 ? '0' : 'A' - 10);
  }

  KRYS_NODISCARD constexpr char LowerNibbleToLowercaseASCIIHexDigit(uint8 value) noexcept
  {
    uint8 nibble = value & 0xF;
    return nibble + (nibble < 10 ? '0' : 'a' - 10);
  }

  KRYS_NODISCARD constexpr char UpperNibbleToLowercaseASCIIHexDigit(uint8 value) noexcept
  {
    uint8 nibble = value >> 4;
    return nibble + (nibble < 10 ? '0' : 'a' - 10);
  }

#pragma endregion

#pragma region Whitespace

  /// @note This also matches Infra's concept of "ASCII whitespace".
  /// @see https://infra.spec.whatwg.org/#ascii-whitespace
  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIWhitespace(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == '\f';
  }

  /// @brief Different from IsASCIIWhitespace; JSON/HTTP/XML do not accept '\f' as a whitespace.
  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIIWhitespaceWithoutFF(TChar c) noexcept
  {
    // ECMA-404 specifies the following:
    // > Whitespace is any sequence of one or more of the following code points:
    // > character tabulation (U+0009), line feed (U+000A), carriage return (U+000D), and space (U+0020).
    //
    // This matches HTTP whitespace:
    // https://fetch.spec.whatwg.org/#http-whitespace-byte
    //
    // And XML whitespace:
    // https://www.w3.org/TR/2008/REC-xml-20081126/#NT-S
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsUnicodeCompatibleASCIIWhitespace(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return IsASCIIWhitespace(ch) || ch == '\v';
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsTabOrSpace(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch == ' ' || ch == '\t';
  }

  /// @brief Skips whitespace characters in the given input stream, updating 'position' to the first
  /// non-whitespace character, or the end of the stream.
  template <IsCharOrByte TChar>
  constexpr static void SkipASCIIWhitespace(Span<const TChar> input, size_t &position) noexcept
  {
    while (position < input.size() && IsASCIIWhitespace(input[position]))
    {
      position++;
    }
  }

#pragma endregion

#pragma region Comparison

  template <typename A, typename B>
  KRYS_NODISCARD constexpr bool IsASCIICaselessEqual(A &&a, B &&b) noexcept
  {
    if (a.length() != b.length())
    {
      return false;
    }

    for (size_t i = 0uz; i < a.length(); i++)
    {
      if (ToASCIILowerUnchecked(a[i]) != ToASCIILowerUnchecked(b[i]))
      {
        return false;
      }
    }

    return true;
  }

  /// @brief Checks whether an ASCII character matches a pre-normalized ASCII literal.
  /// ignoring case on the input character.
  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool MatchesASCIINormalizedLiteral(TChar character, char expected) noexcept
  {
    krys_debug_assert(ToASCIILowerUnchecked(expected) == expected);

    auto ch = static_cast<comparable_char_t<TChar>>(character);
    return ToASCIILowerUnchecked(ch) == static_cast<TChar>(expected);
  }

#pragma endregion
}