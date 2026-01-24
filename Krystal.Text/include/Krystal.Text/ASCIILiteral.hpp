#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Hash.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCII.hpp"
#include <compare>
#include <ranges>
#include <xhash>

namespace Krys::Text
{
  class ASCIILiteral
  {
  private:
    /// @brief The characters in the literal. Includes the null terminator.
    Span<const char> _characters;

  public:
    static constexpr ASCIILiteral From(const char *string) noexcept
    {
      return ASCIILiteral {Span(string, std::char_traits<char>::length(string) + 1)};
    }

    constexpr ASCIILiteral() noexcept = default;

    constexpr ASCIILiteral(std::nullptr_t) noexcept : ASCIILiteral()
    {
    }

    template <size_t Length>
    consteval ASCIILiteral(const char (&literal)[Length]) : _characters(literal, Length)
    {
    }

    /// @brief Gets the characters in the literal, including the null terminator.
    KRYS_NODISCARD constexpr const char *Characters() const noexcept
    {
      return _characters.data();
    }

    /// @brief Gets the length of the literal, excluding the null terminator.
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

    KRYS_NODISCARD constexpr stringview ToStringView() const noexcept
    {
      return stringview(Characters(), Length());
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

    KRYS_NODISCARD constexpr bool operator<(const ASCIILiteral &other) const noexcept
    {
      return std::ranges::lexicographical_compare(ToSpan(), other.ToSpan(), std::less<char> {});
    }

    KRYS_NODISCARD constexpr bool operator>(const ASCIILiteral &other) const noexcept
    {
      return std::ranges::lexicographical_compare(ToSpan(), other.ToSpan(), std::less<char> {});
    }

    KRYS_NODISCARD constexpr bool operator==(const ASCIILiteral &other) const noexcept
    {
      return std::ranges::lexicographical_compare(ToSpan(), other.ToSpan(), std::equal_to<char> {});
    }

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
    return ASCIILiteral::From(str);
  }
}

namespace std
{
  template <>
  struct hash<::Krys::Text::ASCIILiteral>
  {
    size_t operator()(const ::Krys::Text::ASCIILiteral &literal) const noexcept
    {
      return ::Krys::Hash::fnv1a_32(literal.Characters(), literal.Length());
    }
  };
}