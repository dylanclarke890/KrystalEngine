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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"
#include <cmath>

namespace krys::boo
{
  using FloatBoxExtent = RectEdges<float>;
  using LayoutBoxExtent = RectEdges<LayoutUnit>;

  namespace style
  {
    // <line-width> = <length [0,∞]> | thin | medium | thick
    // https://drafts.csswg.org/css-backgrounds/#typedef-line-width
    struct LineWidth
    {
      using Length = style::Length<css::NonNegative>;

      Length Value;

      constexpr LineWidth(Length length) noexcept : Value {length}
      {
      }
      constexpr LineWidth(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : Value {literal}
      {
      }

      KRYS_NODISCARD constexpr bool IsZero() const noexcept
      {
        return Value.IsZero();
      }
      KRYS_NODISCARD constexpr bool IsPositive() const noexcept
      {
        return Value.IsPositive();
      }

      KRYS_NODISCARD explicit constexpr operator bool() const noexcept
      {
        return !IsZero();
      }

      KRYS_NODISCARD constexpr bool operator==(const LineWidth &) const noexcept = default;
      KRYS_NODISCARD constexpr auto operator<=>(const LineWidth &) const noexcept = default;
    };
    DEFINE_TYPE_WRAPPER_GET(LineWidth, Value);

    using LineWidthBox = css::MinimallySerialisingSpaceSeparatedRectEdges<style::LineWidth>;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<LineWidth>
    {
      KRYS_NODISCARD LineWidth operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Evaluate

    template <typename Result>
    struct Evaluation<LineWidth, Result>
    {
      KRYS_NODISCARD constexpr Result operator()(const LineWidth &value, ZoomNeeded zoom) noexcept
      {
        return Result(value.Value.ResolveZoom(zoom));
      }
    };

    template <>
    struct Evaluation<LineWidthBox, FloatBoxExtent>
    {
      KRYS_NODISCARD FloatBoxExtent operator()(const LineWidthBox &, ZoomNeeded) noexcept;
    };
    template <>
    struct Evaluation<LineWidthBox, LayoutBoxExtent>
    {
      KRYS_NODISCARD LayoutBoxExtent operator()(const LineWidthBox &, ZoomNeeded) noexcept;
    };

  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::LineWidth)
