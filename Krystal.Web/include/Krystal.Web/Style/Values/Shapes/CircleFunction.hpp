/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Web/Css/Values/Shapes/CircleFunction.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Web/Style/Values/Primitives/Position.hpp"
#include "Krystal.Web/Style/Values/Shapes/PathComputation.hpp"

namespace krys::web::style
{
  struct Circle
  {
    using Extent = css::Circle::Extent;
    using Length = style::LengthPercentage<css::NonNegative>;
    using RadialSize = Variant<Length, Extent>;

    RadialSize Radius;
    Maybe<style::Position> Position;

    KRYS_NODISCARD bool operator==(const Circle &) const noexcept = default;
  };

  using CircleFunction = css::FunctionNotation<css::ValueId::Circle, Circle>;

  template <size_t I>
  const auto &get(const Circle &value)
  {
    if constexpr (!I)
    {
      return value.Radius;
    }
    else if constexpr (I == 1)
    {
      return value.Position;
    }
  }

  KRYS_DEFINE_TYPE_MAPPING(css::Circle, Circle)

  KRYS_NODISCARD PointF ResolvePosition(const Circle &value, SizeF boxSize) noexcept;

  KRYS_NODISCARD float ResolveRadius(const Circle &value, SizeF boxSize, PointF center) noexcept;

  KRYS_NODISCARD web::Path PathForCenterCoordinate(const Circle &, const RectF &, PointF) noexcept;

  template <>
  struct PathComputation<Circle>
  {
    KRYS_NODISCARD web::Path operator()(const Circle &, const RectF &) noexcept;
  };

  template <>
  struct Blending<Circle>
  {
    KRYS_NODISCARD static bool CanBlend(const Circle &, const Circle &) noexcept;

    KRYS_NODISCARD static Circle Blend(const Circle &, const Circle &, const BlendingContext &) noexcept;
  };
}

KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::web::style::Circle, 2)
