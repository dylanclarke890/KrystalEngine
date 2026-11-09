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

#include "Krystal.Booey/CSS/Values/Shapes/PolygonFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Shapes/FillRule.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/WindRuleComputation.hpp"

namespace krys::boo::style
{
  struct Polygon
  {
    using Vertex = css::SpaceSeparatedPoint<LengthPercentage<>>;
    using VerticesType = css::CommaSeparatedList<Vertex>;

    // FIXME: Add support the "round" clause.

    Maybe<style::FillRule> FillRule;
    VerticesType Vertices;

    KRYS_NODISCARD bool operator==(const Polygon &) const noexcept = default;
  };
  using PolygonFunction = css::FunctionNotation<css::ValueId::Polygon, Polygon>;

  template <size_t I>
  const auto &get(const Polygon &value)
  {
    if constexpr (!I)
    {
      return value.FillRule;
    }
    else if constexpr (I == 1)
    {
      return value.Vertices;
    }
  }

  DEFINE_TYPE_MAPPING(css::Polygon, Polygon)

  template <>
  struct PathComputation<Polygon>
  {
    KRYS_NODISCARD boo::Path operator()(const Polygon &, const FloatRect &) noexcept;
  };
  template <>
  struct WindRuleComputation<Polygon>
  {
    KRYS_NODISCARD boo::WindRule operator()(const Polygon &) noexcept;
  };

  template <>
  struct Blending<Polygon>
  {
    KRYS_NODISCARD bool CanBlend(const Polygon &, const Polygon &) noexcept;
    KRYS_NODISCARD Polygon Blend(const Polygon &, const Polygon &, const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Polygon, 2)
