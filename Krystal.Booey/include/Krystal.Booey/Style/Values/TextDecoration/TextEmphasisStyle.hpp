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

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'text-emphasis-style'> = none | [ [ filled | open ] || [ dot | circle | double-circle | triangle |
  // sesame ] ] | <string> https://drafts.csswg.org/css-text-decor/#propdef-text-emphasis-style
  struct TextEmphasisStyle
  {
    struct Shape
    {
      TextEmphasisFill Fill {TextEmphasisFill::Filled};
      TextEmphasisMark Mark;

      template <typename... F>
      decltype(auto) SwitchOn(F &&...f) const noexcept
      {
        auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

        if (Fill == TextEmphasisFill::Filled)
        {
          return visitor(Mark);
        }
        return visitor(css::SpaceSeparatedTuple {Fill, Mark});
      }

      KRYS_NODISCARD bool operator==(const Shape &) const noexcept = default;
    };

    Variant<css::keywords::None, Shape, css::CSSOMStringAtom> Value;

    TextEmphasisStyle(css::keywords::None keyword) noexcept : Value {keyword}
    {
    }

    TextEmphasisStyle(Shape shape) noexcept : Value {shape}
    {
    }

    TextEmphasisStyle(css::CSSOMStringAtom &&customMark) noexcept : Value {krys::move(customMark)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return HoldsAlternative<css::keywords::None>(Value);
    }

    // css::CSSOMString representation of the mark.
    KRYS_NODISCARD const css::CSSOMStringAtom &MarkString() const noexcept;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const TextEmphasisStyle &) const noexcept = default;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TextEmphasisStyle>
  {
    KRYS_NODISCARD TextEmphasisStyle operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextEmphasisStyle::Shape)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextEmphasisStyle)
