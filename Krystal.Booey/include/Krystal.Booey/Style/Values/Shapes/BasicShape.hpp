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

#include "Krystal.Booey/CSS/Values/Shapes/BasicShape.hpp"
#include "Krystal.Booey/Style/Values/Shapes/CircleFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/EllipseFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/InsetFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PolygonFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/RectFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/ShapeFunction.hpp"
#include "Krystal.Booey/Style/Values/Shapes/WindRuleComputation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/XywhFunction.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // NOTE: This differs from css::BasicShape due to lack of RectFunction and XywhFunction, both of
  // which convert to InsetFunction during style conversion.
  using BasicShape =
    Variant<CircleFunction, EllipseFunction, InsetFunction, PathFunction, PolygonFunction, ShapeFunction>;

  template <typename T>
  concept ShapeWithCenterCoordinate = std::same_as<T, CircleFunction> || std::same_as<T, EllipseFunction>;

  // MARK: - Conversion

  template <>
  struct ToCSS<BasicShape>
  {
    KRYS_NODISCARD css::BasicShape operator()(const BasicShape &, const RenderStyle &,
                                              PathConversion = PathConversion::None) noexcept;
  };
  template <>
  struct ToStyle<css::BasicShape>
  {
    KRYS_NODISCARD BasicShape operator()(const css::BasicShape &, const BuilderState &,
                                         Maybe<float> zoom = 1.0f) noexcept;
  };

  template <>
  struct CSSValueCreation<BasicShape>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, const BasicShape &,
                                              PathConversion = PathConversion::None) noexcept;
  };
  template <>
  struct CSSValueConversion<BasicShape>
  {
    KRYS_NODISCARD BasicShape operator()(BuilderState &, const css::Value &,
                                         Maybe<float> zoom = 1.0f) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<BasicShape>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const BasicShape &, PathConversion = PathConversion::None) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<BasicShape>
  {
    KRYS_NODISCARD bool CanBlend(const BasicShape &, const BasicShape &) noexcept;
    KRYS_NODISCARD BasicShape Blend(const BasicShape &, const BasicShape &, const BlendingContext &) noexcept;
  };

  // MARK: - Path

  template <>
  struct PathComputation<BasicShape>
  {
    KRYS_NODISCARD boo::Path operator()(const BasicShape &, const FloatRect &) noexcept;
  };

  // MARK: - Winding

  template <>
  struct WindRuleComputation<BasicShape>
  {
    KRYS_NODISCARD boo::WindRule operator()(const BasicShape &) noexcept;
  };
}
