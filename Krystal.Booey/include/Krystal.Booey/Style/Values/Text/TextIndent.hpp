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

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  struct TextIndentLength : LengthWrapperBase<LengthPercentage<css::AllUnzoomed>>
  {
    using Base::Base;
  };

  // <'text-indent'> = <length-percentage> && hanging? && each-line?
  // https://drafts.csswg.org/css-text-3/#propdef-text-indent
  struct TextIndent
  {
    TextIndent(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : Length {literal}
    {
    }

    TextIndent(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept : Length {literal}
    {
    }

    TextIndent(TextIndentLength &&length) noexcept : Length {krys::move(length)}
    {
    }

    TextIndent(TextIndentLength &&length, css::keywords::Hanging hanging) noexcept
        : Length {krys::move(length)}, Hanging {hanging}
    {
    }

    TextIndent(TextIndentLength &&length, css::keywords::EachLine eachLine) noexcept
        : Length {krys::move(length)}, EachLine {eachLine}
    {
    }

    TextIndent(TextIndentLength &&length, Maybe<css::keywords::Hanging> hanging,
               Maybe<css::keywords::EachLine> eachLine) noexcept
        : Length {krys::move(length)}, Hanging {hanging}, EachLine {eachLine}
    {
    }

    TextIndentLength Length;
    Maybe<css::keywords::Hanging> Hanging;
    Maybe<css::keywords::EachLine> EachLine;

    KRYS_NODISCARD bool operator==(const TextIndent &) const noexcept = default;
  };

  template <size_t I>
  const auto &get(const TextIndent &value)
  {
    if constexpr (!I)
    {
      return value.Length;
    }
    else if constexpr (I == 1)
    {
      return value.Hanging;
    }
    else if constexpr (I == 2)
    {
      return value.EachLine;
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TextIndent>
  {
    KRYS_NODISCARD TextIndent operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<TextIndent>
  {
    KRYS_NODISCARD bool CanBlend(const TextIndent &, const TextIndent &) noexcept;
    KRYS_NODISCARD TextIndent Blend(const TextIndent &, const TextIndent &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextIndentLength);
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::TextIndent, 3);
