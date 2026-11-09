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

namespace krys::boo::style
{
  // <'line-height'> = normal | <number [0,∞]> | <length-percentage [0,∞]>
  // NOTE: <number [0,∞]> gets converted to <length-percentage [0,∞]>.
  // https://drafts.csswg.org/css-inline/#propdef-line-height
  struct LineHeight : LengthWrapperBase<LengthPercentage<css::NonNegativeUnzoomed>, css::keywords::Normal>
  {
    using Base::Base;

    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return HoldsAlternative<css::keywords::Normal>();
    }

    KRYS_NODISCARD size_t ValueForHash() const noexcept
    {
      return SwitchOn([&](const css::keywords::Normal &) -> size_t { return Hash::Combine(0); },
                      [&](const Fixed &fixed) -> size_t { return Hash::Combine(1, fixed.UnresolvedValue()); },
                      [&](const Percentage &percentage) -> size_t
                      { return Hash::Combine(2, percentage.Value); },
                      [&](const Calc &) -> size_t { return Hash::Combine(3); });
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<LineHeight>
  {
    KRYS_NODISCARD LineHeight operator()(BuilderState &, const css::Value &,
                                         float multiplier = 1.0f) noexcept;
    KRYS_NODISCARD LineHeight operator()(BuilderState &, const css::PrimitiveValue &,
                                         float multiplier = 1.0f) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<LineHeight>
  {
    KRYS_NODISCARD bool CanBlend(const LineHeight &, const LineHeight &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const LineHeight &,
                                                                      const LineHeight &) noexcept;
    KRYS_NODISCARD LineHeight Blend(const LineHeight &, const LineHeight &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::LineHeight)
