/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

/// @see https://drafts.csswg.org/cssom/#common-parsing-idioms
namespace krys::boo::css
{
  /// @brief Removes @p character from the front of @p string if it's there.
  /// @return Whether it was there.
  KRYS_NODISCARD inline bool SkipExactly(CSSOMStringView &string, char8 character) noexcept
  {
    if (!string.empty() && string.front() == character)
    {
      string.remove_prefix(1);
      return true;
    }
    return false;
  }

  /// @brief Space characters as defined by the CSS specification.
  /// @see http://www.w3.org/TR/css3-syntax/#whitespace
  template <typename CharacterType>
  KRYS_NODISCARD constexpr bool IsCSSSpace(CharacterType c) noexcept
  {
    return c == ' ' || c == '\t' || c == '\n';
  }

  /// @see http://dev.w3.org/csswg/css-syntax/#name-start-code-point
  template <typename CharacterType>
  KRYS_NODISCARD constexpr bool IsNameStartCodePoint(CharacterType c) noexcept
  {
    return krys::text::IsASCIIAlpha(c) || c == '_' || !krys::text::IsASCII(c);
  }

  /// @see http://dev.w3.org/csswg/css-syntax/#name-code-point
  template <typename CharacterType>
  KRYS_NODISCARD constexpr bool IsNameCodePoint(CharacterType c) noexcept
  {
    return IsNameStartCodePoint(c) || krys::text::IsASCIIDigit(c) || c == '-';
  }

  KRYS_NODISCARD constexpr bool IsValidCustomIdentifier(ValueId valueId) noexcept
  {
    // "default" is obsolete as a CSS-wide keyword but is still not allowed as a custom identifier.
    return !IsGlobalKeyword(valueId) && valueId != ValueId::Default;
  }

  /// @see https://drafts.csswg.org/css-conditional-5/#propdef-container-name
  KRYS_NODISCARD constexpr bool IsValidContainerNameIdentifier(ValueId valueId) noexcept
  {
    switch (valueId)
    {
      case ValueId::None:
      case ValueId::And:
      case ValueId::Or:
      case ValueId::Not:  return false;
      default:            return IsValidCustomIdentifier(valueId);
    }
  }
}