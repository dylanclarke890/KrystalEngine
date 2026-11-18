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
    0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u, 0x08u, 0x09u, 0x0Au, 0x0Bu, 0x0Cu, 0x0Du, 0x0Eu,
    0x0Fu, 0x10u, 0x11u, 0x12u, 0x13u, 0x14u, 0x15u, 0x16u, 0x17u, 0x18u, 0x19u, 0x1Au, 0x1Bu, 0x1Cu, 0x1Du,
    0x1Eu, 0x1Fu, 0x20u, 0x21u, 0x22u, 0x23u, 0x24u, 0x25u, 0x26u, 0x27u, 0x28u, 0x29u, 0x2Au, 0x2Bu, 0x2Cu,
    0x2Du, 0x2Eu, 0x2Fu, 0x30u, 0x31u, 0x32u, 0x33u, 0x34u, 0x35u, 0x36u, 0x37u, 0x38u, 0x39u, 0x3Au, 0x3Bu,
    0x3Cu, 0x3Du, 0x3Eu, 0x3Fu, 0x40u, 0x61u, 0x62u, 0x63u, 0x64u, 0x65u, 0x66u, 0x67u, 0x68u, 0x69u, 0x6Au,
    0x6Bu, 0x6Cu, 0x6Du, 0x6Eu, 0x6Fu, 0x70u, 0x71u, 0x72u, 0x73u, 0x74u, 0x75u, 0x76u, 0x77u, 0x78u, 0x79u,
    0x7Au, 0x5Bu, 0x5Cu, 0x5Du, 0x5Eu, 0x5Fu, 0x60u, 0x61u, 0x62u, 0x63u, 0x64u, 0x65u, 0x66u, 0x67u, 0x68u,
    0x69u, 0x6Au, 0x6Bu, 0x6Cu, 0x6Du, 0x6Eu, 0x6Fu, 0x70u, 0x71u, 0x72u, 0x73u, 0x74u, 0x75u, 0x76u, 0x77u,
    0x78u, 0x79u, 0x7Au, 0x7Bu, 0x7Cu, 0x7Du, 0x7Eu, 0x7Fu, 0x80u, 0x81u, 0x82u, 0x83u, 0x84u, 0x85u, 0x86u,
    0x87u, 0x88u, 0x89u, 0x8Au, 0x8Bu, 0x8Cu, 0x8Du, 0x8Eu, 0x8Fu, 0x90u, 0x91u, 0x92u, 0x93u, 0x94u, 0x95u,
    0x96u, 0x97u, 0x98u, 0x99u, 0x9Au, 0x9Bu, 0x9Cu, 0x9Du, 0x9Eu, 0x9Fu, 0xA0u, 0xA1u, 0xA2u, 0xA3u, 0xA4u,
    0xA5u, 0xA6u, 0xA7u, 0xA8u, 0xA9u, 0xAAu, 0xABu, 0xACu, 0xADu, 0xAEu, 0xAFu, 0xB0u, 0xB1u, 0xB2u, 0xB3u,
    0xB4u, 0xB5u, 0xB6u, 0xB7u, 0xB8u, 0xB9u, 0xBAu, 0xBBu, 0xBCu, 0xBDu, 0xBEu, 0xBFu, 0xC0u, 0xC1u, 0xC2u,
    0xC3u, 0xC4u, 0xC5u, 0xC6u, 0xC7u, 0xC8u, 0xC9u, 0xCAu, 0xCBu, 0xCCu, 0xCDu, 0xCEu, 0xCFu, 0xD0u, 0xD1u,
    0xD2u, 0xD3u, 0xD4u, 0xD5u, 0xD6u, 0xD7u, 0xD8u, 0xD9u, 0xDAu, 0xDBu, 0xDCu, 0xDDu, 0xDEu, 0xDFu, 0xE0u,
    0xE1u, 0xE2u, 0xE3u, 0xE4u, 0xE5u, 0xE6u, 0xE7u, 0xE8u, 0xE9u, 0xEAu, 0xEBu, 0xECu, 0xEDu, 0xEEu, 0xEFu,
    0xF0u, 0xF1u, 0xF2u, 0xF3u, 0xF4u, 0xF5u, 0xF6u, 0xF7u, 0xF8u, 0xF9u, 0xFAu, 0xFBu, 0xFCu, 0xFDu, 0xFEu,
    0xFFu};

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
  KRYS_NODISCARD constexpr bool IsASCIIUpper(TChar c) noexcept
  {
    auto ch = static_cast<comparable_char_t<TChar>>(c);
    return ch >= 'A' && ch <= 'Z';
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
  KRYS_NODISCARD std::basic_string<typename T::value_type> ToASCIILower(const T &input) noexcept
  {
    std::basic_string<typename T::value_type> result {};
    for (auto codePoint : input)
    {
      result.push_back(ToASCIILower<typename T::value_type>(codePoint));
    }

    return result;
  }

  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr TChar ToASCIIUpper(TChar c) noexcept
  {
    auto ch = static_cast<sized_unsigned_t<TChar>>(c);
    return static_cast<TChar>(ch & ~(IsASCIILower(ch) << 5));
  }

  template <typename T>
  KRYS_NODISCARD std::basic_string<typename T::value_type> ToASCIIUpper(const T &input) noexcept
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

    return static_cast<uint8>(ch < 'A' ? ch - '0' : (ch - 'A' + 10u) & 0xFu);
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

  /// @brief Whether the character `c` equals `expected`, ignoring ASCII case. Unlike `IsASCIICaselessEqual`,
  /// compares single characters.
  template <typename TChar, typename UChar>
  KRYS_NODISCARD constexpr bool IsASCIIAlphaCaselessEqual(TChar c, UChar expected) noexcept
  {
    return ToASCIILower(static_cast<char32>(c)) == ToASCIILower(static_cast<char32>(expected));
  }

  namespace detail
  {
    /// @brief @p string itself if it has a length and can be indexed, otherwise a view of it (a string
    /// literal, for example).
    template <typename TString>
    KRYS_NODISCARD constexpr decltype(auto) AsIndexableString(const TString &string) noexcept
    {
      if constexpr (requires {
                      string.length();
                      string[0uz];
                    })
      {
        return (string);
      }
      else
      {
        return std::basic_string_view {string};
      }
    }
  }

  /// @brief Whether @p a and @p b are equal, ignoring ASCII case. Either can be a string, a view or a string
  /// literal.
  template <typename A, typename B>
  KRYS_NODISCARD constexpr bool IsASCIICaselessEqual(const A &a, const B &b) noexcept
  {
    const auto &first = detail::AsIndexableString(a);
    const auto &second = detail::AsIndexableString(b);
    if (first.length() != second.length())
    {
      return false;
    }

    // Each code unit is folded on its own and compared unsigned, so a `char` past 0x7F equals the same
    // `char8_t`.
    auto folded = []<typename TChar>(TChar c) noexcept
    {
      return static_cast<char32>(static_cast<sized_unsigned_t<TChar>>(ToASCIILower(c)));
    };
    for (size_t i = 0uz; i < first.length(); i++)
    {
      if (folded(first[i]) != folded(second[i]))
      {
        return false;
      }
    }

    return true;
  }

  /// @brief Checks whether an ASCII character matches a pre-normalized ASCII literal.
  /// ignoring case on the input character.
  template <IsCharOrByte TChar>
  KRYS_NODISCARD constexpr bool IsASCIICaselessEqual(TChar character, char expected) noexcept
  {
    krys_debug_assert(ToASCIILowerUnchecked<TChar>(expected) == static_cast<TChar>(expected));

    auto ch = static_cast<comparable_char_t<TChar>>(character);
    return ToASCIILowerUnchecked(ch) == static_cast<TChar>(expected);
  }

#pragma endregion
}
