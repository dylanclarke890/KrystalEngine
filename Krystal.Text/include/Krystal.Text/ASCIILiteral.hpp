#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCII.hpp"
#include <compare>

namespace Krys::Text
{
  class ASCIILiteral
  {
  private:
    /// @brief The characters in the literal. Includes the null terminator.
    Span<const char> _characters;

  public:
    constexpr ASCIILiteral() noexcept = default;
    constexpr ASCIILiteral(std::nullptr_t) noexcept : ASCIILiteral()
    {
    }

    static constexpr ASCIILiteral FromLiteral(const char *string)
    {
      return ASCIILiteral {Span(string, std::char_traits<char>::length(string) + 1)};
    }

    template <size_t Length>
    consteval ASCIILiteral(const char (&literal)[Length]) : _characters(literal, Length)
    {
      static_assert(_characters[Length - 1] == '\0');
    }

    /// @brief Gets the characters in the literal, including the null terminator.
    KRYS_NODISCARD constexpr const char *Characters() const noexcept
    {
      return _characters.data();
    }

    /// @brief Gets the length of the literal, including the null terminator.
    KRYS_NODISCARD constexpr size_t Length() const noexcept
    {
      return !_characters.empty() ? _characters.size() - 1 : 0;
    }

    KRYS_NODISCARD constexpr bool IsNull() const noexcept
    {
      return _characters.empty();
    }

    KRYS_NODISCARD constexpr Span<const char> ToSpan() const noexcept
    {
      return _characters.first(Length());
    }

    KRYS_NODISCARD constexpr Span<const char> SpanWithNullTerminator() const noexcept
    {
      return _characters;
    }

    KRYS_NODISCARD constexpr size_t IsEmpty() const
    {
      return _characters.size() <= 1;
    }

    KRYS_NODISCARD constexpr char operator[](size_t index) const
    {
      return _characters[index];
    }

    KRYS_NODISCARD constexpr char CharacterAt(size_t index) const
    {
      return _characters[index];
    }

    KRYS_NODISCARD constexpr auto operator<=>(const ASCIILiteral &other) const noexcept = default;

  private:
    constexpr explicit ASCIILiteral(Span<const char> spanWithNullTerminator) noexcept
        : _characters(spanWithNullTerminator)
    {
#if KRYS_ENV(DEV)
      for (size_t i = 0uz, size = Length(); i < size; ++i)
      {
        assert(IsASCII(_characters[i]));
      }
#endif
    }
  };

  KRYS_NODISCARD consteval ASCIILiteral operator""_s(const char *str, size_t) noexcept
  {
    return ASCIILiteral::FromLiteral(str);
  }
}