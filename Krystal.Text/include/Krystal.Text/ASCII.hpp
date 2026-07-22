#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <cassert>

namespace Krys::Text
{
  constexpr inline Array<uint8, 256> AsciiCaseFoldTable {
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

  template <typename T>
  concept ASCIIChar = ConvertibleTo<remove_cv_t<T>, char> || SameType<T, byte>;

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCII(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return !(ch & ~0x7F);
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIILower(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch >= 'a' && ch <= 'z';
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIUpper(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch >= 'A' && ch <= 'Z';
  }

  /// @brief Can be used for comparing any ASCII input character to a lowercase English character. The
  /// `IsASCIIAlphaCaselessEqual` function should be used for regular comparison of ASCII alpha characters,
  /// but switch statements in the CSS tokenizer, for example, instead make direct use of
  /// `ToASCIILowerUnchecked`.
  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr TChar ToASCIILowerUnchecked(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return static_cast<TChar>(ch | 0x20);
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr TChar ToASCIILower(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return static_cast<TChar>(ch | (IsASCIIUpper(ch) << 5));
  }

  template <>
  inline char ToASCIILower(char character)
  {
    return static_cast<char>(AsciiCaseFoldTable[static_cast<uint8>(character)]);
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr TChar ToASCIIUpper(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return static_cast<TChar>(ch & ~(IsASCIILower(ch) << 5));
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIAlpha(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return IsASCIILower(ToASCIILowerUnchecked(ch));
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIDigit(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch >= '0' && ch <= '9';
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIAlphanumeric(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return IsASCIIDigit(ch) || IsASCIIAlpha(ch);
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIHexDigit(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return IsASCIIDigit(ch) || (ToASCIILowerUnchecked(ch) >= 'a' && ToASCIILowerUnchecked(ch) <= 'f');
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr uint8 ToASCIIHexValue(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    assert(IsASCIIHexDigit(ch));
    return ch < 'A' ? ch - '0' : (ch - 'A' + 10) & 0xF;
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr uint8 ToASCIIHexValue(TChar first, TChar second) noexcept
  {
    using T = conditional_t<SameType<TChar, byte>, char, TChar>;

    auto a = static_cast<T>(first);
    auto b = static_cast<T>(second);

    return ToASCIIHexValue(a) << 4 | ToASCIIHexValue(b);
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIBinaryDigit(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch == '0' || ch == '1';
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIOctalDigit(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch >= '0' && ch <= '7';
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIPrintable(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch >= ' ' && ch <= '~';
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIGraphic(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch >= '!' && ch <= '~';
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsTabOrSpace(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch == ' ' || ch == '\t';
  }

  /// @brief Infra's "ASCII whitespace" <https://infra.spec.whatwg.org/#ascii-whitespace>
  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIWhitespace(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == '\f';
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsNotASCIIWhitespace(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return !IsASCIIWhitespace(ch);
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool ToASCIIDigitUnchecked(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return static_cast<TChar>(ch - '0');
  }

  /// @brief Different from IsASCIIWhitespace; JSON/HTTP/XML do not accept '\f' as a whitespace.
  template <ASCIIChar TChar>
  constexpr bool IsASCIIWhitespaceWithoutFF(TChar character) noexcept
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
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
  }

  /// @brief Skips whitespace characters in the given input stream, updating 'position' to the first
  /// non-whitespace character, or the end of the stream.
  template <ASCIIChar TChar>
  constexpr static void SkipASCIIWhitespace(Span<const TChar> input, size_t &position) noexcept
  {
    while (position < input.size() && IsASCIIWhitespace(input[position]))
    {
      position++;
    }
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsUnicodeCompatibleASCIIWhitespace(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return IsASCIIWhitespace(ch) || ch == '\v';
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIIDigitOrPunctuation(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return (ch >= '!' && ch <= '@') || (ch >= '[' && ch <= '`') || (ch >= '{' && ch <= '~');
  }

  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool IsASCIINonWhitespaceControl(TChar character) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    return ch == 0x00 || ch == 0x01 || ch == 0x07 || ch == 0x08;
  }

  /// @brief Checks whether an ASCII character matches a pre-normalized ASCII literal.
  /// ignoring case on the input character.
  template <ASCIIChar TChar>
  KRYS_NODISCARD constexpr bool MatchesASCIINormalizedLiteral(TChar character, char expected) noexcept
  {
    auto ch = static_cast<conditional_t<SameType<TChar, byte>, char, TChar>>(character);
    assert(ToASCIILowerUnchecked(expected) == expected);
    return ToASCIILowerUnchecked(ch) == static_cast<TChar>(expected);
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

  KRYS_NODISCARD inline string ToASCIILowercase(stringview input) noexcept
  {
    string result;
    for (char codePoint : input)
    {
      result.push_back(ToASCIILower(codePoint));
    }

    return result;
  }

  KRYS_NODISCARD inline utf8_string ToASCIILowercase(utf8_stringview input) noexcept
  {
    utf8_string result;
    for (char8 codePoint : input)
    {
      result.push_back(ToASCIILower(codePoint));
    }

    return result;
  }

  KRYS_NODISCARD inline utf8_string ToASCIIUppercase(utf8_stringview input) noexcept
  {
    utf8_string result;
    for (char8 codePoint : input)
    {
      result.push_back(ToASCIIUpper(codePoint));
    }

    return result;
  }

  KRYS_NODISCARD constexpr bool ASCIICaseInsensitiveMatch(utf8_stringview a, utf8_stringview b) noexcept
  {
    if (a.length() != b.length())
    {
      return false;
    }

    for (size_t i = 0; i < a.length(); i++)
    {
      if (ToASCIILowerUnchecked(a[i]) != ToASCIILowerUnchecked(b[i]))
      {
        return false;
      }
    }

    return true;
  }

  KRYS_NODISCARD constexpr bool ASCIICaseInsensitiveMatch(utf32_stringview a, utf32_stringview b) noexcept
  {
    if (a.length() != b.length())
    {
      return false;
    }

    for (size_t i = 0; i < a.length(); i++)
    {
      if (ToASCIILowerUnchecked(a[i]) != ToASCIILowerUnchecked(b[i]))
      {
        return false;
      }
    }

    return true;
  }
}