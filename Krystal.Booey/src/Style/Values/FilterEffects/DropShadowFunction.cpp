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

#include "Krystal.Booey/Style/Values/FilterEffects/DropShadowFunction.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/FilterEffects/DropShadowFunction.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Rendering/Style/DropShadowFilterOperationWithStyleColor.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD css::DropShadow ToCSSDropShadow(const DropShadowFilterOperationWithStyleColor &operation,
                                                 const RenderStyle &style) noexcept
  {
    return {
      .Color = ConvertToCSS(operation.StyleColor(), style),
      .Location = {ConvertToCSS(Length<css::AllUnzoomed> {static_cast<float>(operation.Location().X())},
                                style),
                   ConvertToCSS(Length<css::AllUnzoomed> {static_cast<float>(operation.Location().Y())},
                                style)},
      .StdDeviation =
        ConvertToCSS(Length<css::NonNegativeUnzoomed> {static_cast<float>(operation.StdDeviation())}, style),
    };
  }

  KRYS_NODISCARD Ref<FilterOperation> CreateFilterOperation(const css::DropShadow &filter,
                                                            const BuilderState &state) noexcept
  {
    const auto &zoomFactor = state.Style().UsedZoomForLength();
    int x = css::RoundForImpreciseConversion<int>(
      ConvertToStyle(get<0>(filter.Location), state).ResolveZoom(zoomFactor));
    int y = css::RoundForImpreciseConversion<int>(
      ConvertToStyle(get<1>(filter.Location), state).ResolveZoom(zoomFactor));
    int stdDeviation = filter.StdDeviation
                         ? css::RoundForImpreciseConversion<int>(
                             ConvertToStyle(*filter.StdDeviation, state).ResolveZoom(zoomFactor))
                         : 0;
    auto color = filter.Color ? ToStyleColor(*filter.Color, state) : style::Color {CurrentColor {}};

    return DropShadowFilterOperationWithStyleColor::Create(IntPoint {x, y}, stdDeviation, color);
  }
}
