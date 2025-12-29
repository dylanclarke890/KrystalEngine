#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/Concepts.hpp"
#include "Krystal.Text/Rune.hpp"

namespace Krys
{
  struct ASCII : NonCopyMovable<ASCII>
  {
    ASCII() = delete;
    ~ASCII() = delete;

    constexpr static uint8 MaxASCIIValue = 0x7Fu;
    constexpr static uint8 ExtendedASCIIStart = MaxASCIIValue + 1u;

    /// @brief Determines whether 'ch' is a valid ASCII character.
    template <IsCharacter T>
    KRYS_NODISCARD constexpr static bool IsASCII(T ch) noexcept
    {
      return static_cast<uint32>(ch) < ExtendedASCIIStart;
    }

    /// @brief Determines whether 'ch' is a whitespace character.
    template <IsCharacter T>
    KRYS_NODISCARD constexpr static bool IsWhiteSpace(T ch) noexcept
    {
      uint32 c = static_cast<uint32>(ch);
      return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
    }

    /// @brief Determines whether 'ch' is a alphabetic character.
    template <IsCharacter T>
    KRYS_NODISCARD constexpr static bool IsAlpha(T ch) noexcept
    {
      uint32 c = static_cast<uint32>(ch);
      return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

    /// @brief Determines whether 'ch' is a alphanumeric character.
    template <IsCharacter T>
    KRYS_NODISCARD constexpr static bool IsAlphaNumeric(T ch) noexcept
    {
      uint32 c = static_cast<uint32>(ch);
      return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
    }

    /// @brief Determines whether 'ch' is a single or double quote character.
    template <IsCharacter T>
    KRYS_NODISCARD constexpr static bool IsQuote(T ch) noexcept
    {
      uint32 c = static_cast<uint32>(ch);
      return c == '"' || c == '\'';
    }

    /// @brief Converts 'ch' to lowercase, if it is an uppercase ASCII character.
    template <IsCharacter T>
    KRYS_NODISCARD constexpr static T ToLower(T ch) noexcept
    {
      uint32 c = static_cast<uint32>(ch);
      if (c >= 'A' && c <= 'Z')
      {
        return static_cast<T>(c + 32u);
      }
      return ch;
    }

    /// @brief Converts all of the ASCII characters in 'str' to lowercase.
    constexpr static utf8_string &ToLower(utf8_string &str) noexcept
    {
      for (char8 &ch : str)
      {
        ch = ASCII::ToLower<char8>(ch);
      }
      return str;
    }

    /// @brief Converts all of the ASCII characters in 'str' to lowercase.
    KRYS_NODISCARD constexpr static utf8_string ToLower(const utf8_string &str) noexcept
    {
      utf8_string copy = str;
      ToLower(copy);
      return copy;
    }

    /// @brief Converts 'ch' to lowercase, if it is an uppercase ASCII character.
    template <IsCharacter T>
    KRYS_NODISCARD constexpr static T ToUpper(T ch) noexcept
    {
      uint32 c = static_cast<uint32>(ch);
      if (c >= 'a' && c <= 'z')
      {
        return static_cast<T>(c - 32u);
      }
      return ch;
    }

    /// @brief Converts all of the ASCII characters in 'str' to uppercase.
    constexpr static utf8_string &ToUpper(utf8_string &str) noexcept
    {
      for (char8 &ch : str)
      {
        ch = ToUpper(ch);
      }
      return str;
    }

    /// @brief Converts all of the ASCII characters in 'str' to uppercase.
    KRYS_NODISCARD constexpr static utf8_string ToUpper(const utf8_string &str) noexcept
    {
      utf8_string copy = str;
      ToUpper(copy);
      return copy;
    }

    /// @brief Skips whitespace characters in the given input stream, updating 'position' to the first
    /// non-whitespace character, or the end of the stream.
    template <typename T>
    constexpr static void SkipWhiteSpace(T input, size_t &position) noexcept
    {
      while (position < input.size() && IsWhiteSpace(input[position]))
      {
        position++;
      }
    }
  };
}