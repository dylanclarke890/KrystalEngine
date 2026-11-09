/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo
{
  class LayoutSize;

  namespace style
  {
    using BorderRadiusValue = css::MinimallySerialisingSpaceSeparatedSize<LengthPercentage<css::NonNegative>>;

    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
    // https://drafts.csswg.org/css-backgrounds-3/#propdef-border-radius
    struct BorderRadius : RectCorners<BorderRadiusValue>
    {
      BorderRadius(BorderRadiusValue value) noexcept
          : RectCorners<BorderRadiusValue> {value, value, value, value}
      {
      }

      BorderRadius(BorderRadiusValue topLeft, BorderRadiusValue topRight, BorderRadiusValue bottomLeft,
                   BorderRadiusValue bottomRight) noexcept
          : RectCorners<BorderRadiusValue> {krys::move(topLeft), krys::move(topRight), krys::move(bottomLeft),
                                            krys::move(bottomRight)}
      {
      }

      BorderRadius(RectCorners<BorderRadiusValue> &&corners) noexcept
          : RectCorners<BorderRadiusValue> {krys::move(corners)}
      {
      }

      KRYS_NODISCARD bool operator==(const BorderRadius &) const noexcept = default;
    };

    template <size_t I>
    const auto &get(const BorderRadius &value)
    {
      if constexpr (!I)
      {
        return value.TopLeft();
      }
      else if constexpr (I == 1)
      {
        return value.TopRight();
      }
      else if constexpr (I == 2)
      {
        return value.BottomLeft();
      }
      else if constexpr (I == 3)
      {
        return value.BottomRight();
      }
    }

    // MARK: - Conversion

    template <>
    struct ToCSS<BorderRadius>
    {
      KRYS_NODISCARD css::BorderRadius operator()(const BorderRadius &, const RenderStyle &) noexcept;
    };
    template <>
    struct ToStyle<css::BorderRadius>
    {
      KRYS_NODISCARD BorderRadius operator()(const css::BorderRadius &, const BuilderState &) noexcept;
    };

    template <>
    struct CSSValueConversion<BorderRadiusValue>
    {
      KRYS_NODISCARD BorderRadiusValue operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Evaluation

    template <>
    struct Evaluation<BorderRadius, FloatRoundedRect::Radii>
    {
      KRYS_NODISCARD FloatRoundedRect::Radii operator()(const BorderRadius &, FloatSize, ZoomNeeded) noexcept;
    };
    template <>
    struct Evaluation<BorderRadius, LayoutRoundedRect::Radii>
    {
      KRYS_NODISCARD LayoutRoundedRect::Radii operator()(const BorderRadius &, LayoutSize,
                                                         ZoomNeeded) noexcept;
    };

  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::BorderRadius, 4)
