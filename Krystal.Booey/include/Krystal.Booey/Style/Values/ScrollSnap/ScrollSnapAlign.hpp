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
  // <'scroll-snap-align'> = [ none | start | end | center ]{1,2}
  // https://drafts.csswg.org/css-scroll-snap-1/#propdef-scroll-snap-align
  struct ScrollSnapAlign
  {
    constexpr ScrollSnapAlign(css::keywords::None) noexcept
    {
    }
    constexpr ScrollSnapAlign(ScrollSnapAxisAlignType bothAxes) noexcept
        : BlockAlign {bothAxes}, InlineAlign {bothAxes}
    {
    }
    constexpr ScrollSnapAlign(ScrollSnapAxisAlignType blockAlign,
                              ScrollSnapAxisAlignType inlineAlign) noexcept
        : BlockAlign {blockAlign}, InlineAlign {inlineAlign}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return BlockAlign == ScrollSnapAxisAlignType::None && InlineAlign == ScrollSnapAxisAlignType::None;
    }

    ScrollSnapAxisAlignType BlockAlign {ScrollSnapAxisAlignType::None};
    ScrollSnapAxisAlignType InlineAlign {ScrollSnapAxisAlignType::None};

    KRYS_NODISCARD constexpr bool operator==(const ScrollSnapAlign &) const noexcept = default;
  };

  template <size_t I>
  constexpr const auto &get(const ScrollSnapAlign &value)
  {
    if constexpr (!I)
    {
      return value.BlockAlign;
    }
    else if constexpr (I == 1)
    {
      return value.InlineAlign;
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ScrollSnapAlign>
  {
    KRYS_NODISCARD ScrollSnapAlign operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_COALESCING_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ScrollSnapAlign, 2)
