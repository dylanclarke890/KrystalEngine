/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  struct TextShadow
  {
    style::Color Color;
    css::SpaceSeparatedPoint<Length<css::AllUnzoomed>> Location;
    Length<css::NonNegativeUnzoomed> Blur;

    KRYS_NODISCARD bool operator==(const TextShadow &) const noexcept = default;
  };

  template <size_t I>
  const auto &get(const TextShadow &value)
  {
    if constexpr (!I)
    {
      return value.Color;
    }
    else if constexpr (I == 1)
    {
      return value.Location;
    }
    else if constexpr (I == 2)
    {
      return value.Blur;
    }
  }

  // <text-shadow-list> = <single-text-shadow>#
  using TextShadowList = ShadowList<TextShadow>;

  // <'text-shadow'> = none | <text-list>
  // https://www.w3.org/TR/css-text-decor-3/#propdef-text-shadow
  using TextShadows = Shadows<TextShadow>;

  // MARK: - Conversions

  template <>
  struct ToCSS<TextShadow>
  {
    KRYS_NODISCARD css::TextShadow operator()(const TextShadow &, const RenderStyle &) noexcept;
  };
  template <>
  struct ToStyle<css::TextShadow>
  {
    KRYS_NODISCARD TextShadow operator()(const css::TextShadow &, const BuilderState &) noexcept;
  };

  // `TextShadowList` is special-cased to return a `css::TextShadowPropertyValue`.
  template <>
  struct CSSValueCreation<TextShadowList>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const TextShadowList &) noexcept;
  };
  template <>
  struct CSSValueConversion<TextShadows>
  {
    KRYS_NODISCARD TextShadows operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<TextShadowList>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const TextShadowList &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<TextShadow>
  {
    KRYS_NODISCARD TextShadow Blend(const TextShadow &, const TextShadow &, const RenderStyle &,
                                    const RenderStyle &, const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<TextShadows>
  {
    KRYS_NODISCARD bool CanBlend(const TextShadows &, const TextShadows &, CompositeOperation) noexcept;
    KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(const TextShadows &,
                                                                                const TextShadows &) noexcept
    {
      return true;
    }
    KRYS_NODISCARD TextShadows Blend(const TextShadows &, const TextShadows &, const RenderStyle &,
                                     const RenderStyle &, const BlendingContext &) noexcept;
  };

  // MARK: - Shadow-specific Interfaces

  KRYS_NODISCARD constexpr ShadowStyle GetShadowStyle(const TextShadow &) noexcept
  {
    return ShadowStyle::Normal;
  }

  KRYS_NODISCARD constexpr bool IsInset(const TextShadow &) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr LayoutUnit PaintingSpread(const TextShadow &, const style::ZoomFactor &) noexcept
  {
    return LayoutUnit();
  }
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::TextShadow, 3)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextShadows)
