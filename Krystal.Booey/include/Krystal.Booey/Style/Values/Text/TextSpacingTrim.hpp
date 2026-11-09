/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Booey/Text/TextSpacing.hpp"

namespace krys::boo::style
{
  // <'text-spacing-trim'> = space-all | normal | space-first | trim-start | trim-both | trim-all | auto
  // FIXME: Only `space-all`, `trim-all` and `auto` are currently supported.
  // https://drafts.csswg.org/css-text-4/#propdef-text-spacing-trim
  struct TextSpacingTrim
  {
    constexpr TextSpacingTrim(css::keywords::SpaceAll) noexcept
        : Value {boo::TextSpacingTrim::TrimType::SpaceAll}
    {
    }
    constexpr TextSpacingTrim(css::keywords::TrimAll) noexcept
        : Value {boo::TextSpacingTrim::TrimType::TrimAll}
    {
    }
    constexpr TextSpacingTrim(css::keywords::Auto) noexcept : Value {boo::TextSpacingTrim::TrimType::Auto}
    {
    }

    constexpr TextSpacingTrim(boo::TextSpacingTrim platform) noexcept : Value {platform.Type()}
    {
    }
    constexpr TextSpacingTrim(boo::TextSpacingTrim::TrimType platformType) noexcept : Value {platformType}
    {
    }

    KRYS_NODISCARD constexpr boo::TextSpacingTrim::TrimType Platform() const noexcept
    {
      return Value;
    }

    KRYS_NODISCARD constexpr bool operator==(const TextSpacingTrim &) const noexcept = default;

    boo::TextSpacingTrim::TrimType Value;
  };
  DEFINE_TYPE_WRAPPER_GET(TextSpacingTrim, Value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::TextSpacingTrim);
