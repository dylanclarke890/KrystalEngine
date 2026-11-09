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

#include "Krystal.Booey/Style/Values/Shapes/BasicShape.hpp"

#include "Krystal.Booey/CSS/Values/BasicShapeValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  css::BasicShape ToCSS<BasicShape>::operator()(const BasicShape &value, const RenderStyle &style,
                                                PathConversion conversion) noexcept
  {
    return krys::SwitchOn(
      value, [&](const auto &shape) { return css::BasicShape {ConvertToCSS(shape, style)}; },
      [&](const PathFunction &path) { return css::BasicShape {ConvertToCSS(path, style, conversion)}; });
  }

  BasicShape ToStyle<css::BasicShape>::operator()(const css::BasicShape &value,
                                                  const BuilderState &builderState,
                                                  Maybe<float> zoom) noexcept
  {
    return krys::SwitchOn(
      value, [&](const auto &shape) { return BasicShape {ConvertToStyle(shape, builderState)}; },
      [&](const css::PathFunction &path) { return BasicShape {ConvertToStyle(path, builderState, zoom)}; });
  }

  Ref<css::Value> CSSValueCreation<BasicShape>::operator()(css::ValuePool &, const RenderStyle &style,
                                                           const BasicShape &value,
                                                           PathConversion conversion) noexcept
  {
    return css::BasicShapeValue::Create(ConvertToCSS(value, style, conversion));
  }

  BasicShape CSSValueConversion<BasicShape>::operator()(BuilderState &builderState, const css::Value &value,
                                                        Maybe<float> zoom) noexcept
  {
    return ConvertToStyle(Downcast<css::BasicShapeValue>(value).GetShape(), builderState, zoom);
  }

  // MARK: - Serialization

  void Serialiser<BasicShape>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                          const RenderStyle &style, const BasicShape &value,
                                          PathConversion conversion) noexcept
  {
    css::Serialise(builder, context, ConvertToCSS(value, style, conversion));
  }

  // MARK: - Blending

  bool Blending<BasicShape>::CanBlend(const BasicShape &a, const BasicShape &b) noexcept
  {
    return std::visit(
      krys::CreateVisitor(
        []<typename T>(const T &a, const T &b) { return style::CanBlend(a, b); },
        [](const ShapeFunction &a, const PathFunction &b) { return CanBlendShapeWithPath(*a, *b); },
        [](const PathFunction &a, const ShapeFunction &b) { return CanBlendShapeWithPath(*b, *a); },
        [](const auto &, const auto &) { return false; }),
      a, b);
  }

  BasicShape Blending<BasicShape>::Blend(const BasicShape &a, const BasicShape &b,
                                         const BlendingContext &context) noexcept
  {
    return std::visit(
      krys::CreateVisitor(
        [&]<typename T>(const T &a, const T &b) -> BasicShape
        { return {krys::boo::style::Blend(a, b, context)}; },
        [&](const ShapeFunction &a, const PathFunction &b) -> BasicShape
        { return {krys::boo::style::Blend(a, ShapeFunction {*MakeShapeFromPath(*b)}, context)}; },
        [&](const PathFunction &a, const ShapeFunction &b) -> BasicShape
        { return {krys::boo::style::Blend(ShapeFunction {*MakeShapeFromPath(*a)}, b, context)}; },
        [&](const auto &, const auto &) -> BasicShape { krys_unreachable(); }),
      a, b);
  }

  // MARK: - Path

  boo::Path PathComputation<BasicShape>::operator()(const BasicShape &shape, const FloatRect &rect) noexcept
  {
    return krys::SwitchOn(shape,
                          [&](const auto &shape) { return krys::boo::style::ComputePath(shape, rect); });
  }

  // MARK: - Winding

  WindRule WindRuleComputation<BasicShape>::operator()(const BasicShape &shape) noexcept
  {
    return krys::SwitchOn(
      shape, [&](const style::PathFunction &path) { return krys::boo::style::ComputeWindRule(path); },
      [&](const style::PolygonFunction &polygon) { return krys::boo::style::ComputeWindRule(polygon); },
      [&](const style::ShapeFunction &shape) { return krys::boo::style::ComputeWindRule(shape); },
      [&](const auto &) { return WindRule::NonZero; });
  }
}
