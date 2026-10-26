#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <algorithm>
#include <concepts>
#include <limits>

namespace krys::text
{
  // `ParseInteger` may allow leading and trailing spaces as defined by `IsUnicodeCompatibleASCIIWhitespace`,
  // and, after the leading spaces, allows a single leading "+".
  // `ParseIntegerAllowingTrailingJunk` is like `ParseInteger`, but allows any characters after the integer.

  enum class ParseIntegerWhitespacePolicy : bool
  {
    Disallow,
    Allow
  };

  enum class TrailingJunkPolicy : bool
  {
    Disallow,
    Allow
  };

  template <std::integral TIntegral>
  KRYS_NODISCARD constexpr Maybe<TIntegral>
    ParseInteger(utf8_stringview data, uint8 base, TrailingJunkPolicy policy,
                 ParseIntegerWhitespacePolicy whitespacePolicy = ParseIntegerWhitespacePolicy::Allow) noexcept
  {
    auto skipWhitespace = [&]
    {
      while (!data.empty() && IsUnicodeCompatibleASCIIWhitespace(data.front()))
      {
        data.remove_prefix(1);
      }
    };

    if (whitespacePolicy == ParseIntegerWhitespacePolicy::Allow)
    {
      skipWhitespace();
    }

    bool isNegative = false;
    if (std::is_signed_v<TIntegral> && !data.empty() && data.front() == u8'-')
    {
      isNegative = true;
      data.remove_prefix(1);
    }
    else if (!data.empty() && data.front() == u8'+')
    {
      data.remove_prefix(1);
    }

    auto isCharacterAllowedInBase = [](char8 character, uint8 base)
    {
      if (IsASCIIDigit(character))
      {
        return character - u8'0' < base;
      }
      return ToASCIILowerUnchecked(character) >= u8'a'
             && ToASCIILowerUnchecked(character) < u8'a' + std::min(base - 10, 26);
    };

    if (!(!data.empty() && isCharacterAllowedInBase(data.front(), base)))
    {
      return {};
    }

    using Limits = std::numeric_limits<TIntegral>;
    TIntegral value = 0;
    bool overflowed = false;
    do
    {
      auto c = data.front();
      data.remove_prefix(1);
      auto digitValue =
        static_cast<TIntegral>(IsASCIIDigit(c) ? c - u8'0' : ToASCIILowerUnchecked(c) - u8'a' + 10);
      if (overflowed)
      {
        continue;
      }
      if (isNegative)
      {
        if (value < (Limits::min() + digitValue) / static_cast<TIntegral>(base))
        {
          overflowed = true;
          continue;
        }
        value = static_cast<TIntegral>(value * base - digitValue);
      }
      else
      {
        if (value > (Limits::max() - digitValue) / static_cast<TIntegral>(base))
        {
          overflowed = true;
          continue;
        }
        value = static_cast<TIntegral>(value * base + digitValue);
      }
    } while (!data.empty() && isCharacterAllowedInBase(data.front(), base));

    if (overflowed) KRYS_UNLIKELY
    {
      return {};
    }

    if (policy == TrailingJunkPolicy::Disallow)
    {
      if (whitespacePolicy == ParseIntegerWhitespacePolicy::Allow)
      {
        skipWhitespace();
      }
      if (!data.empty())
      {
        return {};
      }
    }

    return value;
  }

  template <std::integral TIntegral>
  KRYS_NODISCARD constexpr Maybe<TIntegral>
    ParseInteger(utf8_stringview data, uint8 base = 10,
                 ParseIntegerWhitespacePolicy whitespacePolicy = ParseIntegerWhitespacePolicy::Allow) noexcept
  {
    return ParseInteger<TIntegral>(data, base, TrailingJunkPolicy::Disallow, whitespacePolicy);
  }

  template <std::integral TIntegral>
  KRYS_NODISCARD constexpr Maybe<TIntegral> ParseIntegerAllowingTrailingJunk(utf8_stringview data,
                                                                             uint8 base = 10) noexcept
  {
    return ParseInteger<TIntegral>(data, base, TrailingJunkPolicy::Allow);
  }
}
