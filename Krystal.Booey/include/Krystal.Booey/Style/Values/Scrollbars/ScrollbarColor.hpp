/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::style
{
  // <scrollbar-color-parts> = <color>{2}
  struct ScrollbarColorParts
  {
    Color Thumb;
    Color Track;

    KRYS_NODISCARD bool operator==(const ScrollbarColorParts &) const noexcept = default;
  };
  template <size_t I>
  KRYS_NODISCARD const auto &get(const ScrollbarColorParts &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Thumb;
    }
    else if constexpr (I == 1)
    {
      return value.Track;
    }
  }

  struct ScrollbarColorMarkableTraits
  {
    KRYS_NODISCARD static bool IsEmptyValue(const ScrollbarColorParts &value) noexcept
    {
      return krys::MarkableTraits<Color>::IsEmptyValue(value.Thumb);
    }
    KRYS_NODISCARD static ScrollbarColorParts EmptyValue() noexcept
    {
      return {krys::MarkableTraits<Color>::EmptyValue(), krys::MarkableTraits<Color>::EmptyValue()};
    }
  };

  // <'scrollbar-color'> = auto | <color>{2}
  // https://www.w3.org/TR/css-scrollbars/#propdef-scrollbar-color
  struct ScrollbarColor
      : css::ValueOrKeyword<ScrollbarColorParts, css::keywords::Auto, ScrollbarColorMarkableTraits>
  {
    using Base::Base;
    using Parts = typename Base::Value;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsParts() const noexcept
    {
      return IsValue();
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ScrollbarColor>
  {
    KRYS_NODISCARD ScrollbarColor operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ScrollbarColor>
  {
    bool Equals(const ScrollbarColor &, const ScrollbarColor &, const RenderStyle &,
                const RenderStyle &) noexcept;
    KRYS_NODISCARD bool CanBlend(const ScrollbarColor &, const ScrollbarColor &) noexcept;
    ScrollbarColor Blend(const ScrollbarColor &, const ScrollbarColor &, const RenderStyle &,
                         const RenderStyle &, const BlendingContext &) noexcept;
  };
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ScrollbarColorParts, 2)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ScrollbarColor)
