/*
 * Copyright (C) 2018-2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <algorithm>
#include <limits>

namespace krys::text
{
  // `ParseInteger` may allow leading and trailing spaces as defined by `IsUnicodeCompatibleASCIIWhitespace`,
  // and, after the leading spaces, allows a single leading "+".
  // `ParseIntegerAllowingTrailingJunk` is like `ParseInteger`, but allows any characters after the integer.

  enum class ParseIntegerWhitespacePolicy : bool
  {
    Disallow,
    Allow,
  };

  enum class TrailingJunkPolicy : bool
  {
    Disallow,
    Allow,
  };

  template <Integral TIntegral>
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
    if (Signed<TIntegral> && !data.empty() && data.front() == u8'-')
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

  template <Integral TIntegral>
  KRYS_NODISCARD constexpr Maybe<TIntegral>
    ParseInteger(utf8_stringview data, uint8 base = 10,
                 ParseIntegerWhitespacePolicy whitespacePolicy = ParseIntegerWhitespacePolicy::Allow) noexcept
  {
    return ParseInteger<TIntegral>(data, base, TrailingJunkPolicy::Disallow, whitespacePolicy);
  }

  template <Integral TIntegral>
  KRYS_NODISCARD constexpr Maybe<TIntegral> ParseIntegerAllowingTrailingJunk(utf8_stringview data,
                                                                             uint8 base = 10) noexcept
  {
    return ParseInteger<TIntegral>(data, base, TrailingJunkPolicy::Allow);
  }
}
