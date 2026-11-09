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

#include "Krystal.Booey/Style/Values/Shapes/RectFunction.hpp"

#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  Inset ToStyle<css::RectShape>::operator()(const css::RectShape &value, const BuilderState &state) noexcept
  {
    // "An auto value makes the edge of the box coincide with the corresponding edge of the
    //  reference box: it’s equivalent to 0% as the first (top) or fourth (left) value, and
    //  equivalent to 100% as the second (right) or third (bottom) value."
    //      (https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-rect)

    // Conversion applies reflection to the trailing (right/bottom) edges to convert from rect()
    // form to inset() form. This means that all the `auto` values are converted to 0%.

    auto convertLeadingEdge =
      [&](const Variant<css::LengthPercentage<>, css::keywords::Auto> &edge) -> LengthPercentage<>
    {
      return krys::SwitchOn(
        edge, [&](const css::LengthPercentage<> &value) -> LengthPercentage<>
        { return ConvertToStyle(value, state); },
        [&](const css::keywords::Auto &) -> LengthPercentage<> { return 0_css_percentage; });
    };

    auto convertTrailingEdge =
      [&](const Variant<css::LengthPercentage<>, css::keywords::Auto> &edge) -> LengthPercentage<>
    {
      return krys::SwitchOn(
        edge, [&](const css::LengthPercentage<> &value) -> LengthPercentage<>
        { return Reflect(ConvertToStyle(value, state)); },
        [&](const css::keywords::Auto &) -> LengthPercentage<> { return 0_css_percentage; });
    };

    return {.Insets =
              {
                convertLeadingEdge(value.Edges.Top()),
                convertTrailingEdge(value.Edges.Right()),
                convertTrailingEdge(value.Edges.Bottom()),
                convertLeadingEdge(value.Edges.Left()),
              },
            .Radii = ConvertToStyle(value.Radii, state)};
  }

  InsetFunction ToStyle<css::RectFunction>::operator()(const css::RectFunction &value,
                                                       const BuilderState &state) noexcept
  {
    return {ConvertToStyle(value.Parameters, state)};
  }
}
