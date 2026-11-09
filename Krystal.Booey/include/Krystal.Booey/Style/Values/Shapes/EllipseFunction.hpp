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

#include "Krystal.Booey/CSS/Values/Shapes/EllipseFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"

namespace krys::boo::style
{
  struct Ellipse
  {
    using Extent = css::Ellipse::Extent;
    using Length = style::LengthPercentage<css::NonNegative>;
    using RadialSize = Variant<Length, Extent>;

    css::SpaceSeparatedPair<RadialSize> Radii;
    Maybe<style::Position> Position;

    KRYS_NODISCARD bool operator==(const Ellipse &) const noexcept = default;
  };
  using EllipseFunction = css::FunctionNotation<css::ValueId::Ellipse, Ellipse>;

  template <size_t I>
  const auto &get(const Ellipse &value)
  {
    if constexpr (!I)
    {
      return value.Radii;
    }
    else if constexpr (I == 1)
    {
      return value.Position;
    }
  }

  DEFINE_TYPE_MAPPING(css::Ellipse, Ellipse)

  KRYS_NODISCARD FloatPoint ResolvePosition(const Ellipse &value, FloatSize boxSize) noexcept;
  KRYS_NODISCARD FloatSize ResolveRadii(const Ellipse &, FloatSize boxSize, FloatPoint center) noexcept;
  KRYS_NODISCARD boo::Path PathForCenterCoordinate(const Ellipse &, const FloatRect &, FloatPoint) noexcept;

  template <>
  struct PathComputation<Ellipse>
  {
    KRYS_NODISCARD boo::Path operator()(const Ellipse &, const FloatRect &) noexcept;
  };

  template <>
  struct Blending<Ellipse>
  {
    KRYS_NODISCARD bool CanBlend(const Ellipse &, const Ellipse &) noexcept;
    KRYS_NODISCARD Ellipse Blend(const Ellipse &, const Ellipse &, const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Ellipse, 2)
