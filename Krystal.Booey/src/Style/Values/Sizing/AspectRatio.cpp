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

#include "Krystal.Booey/Style/Values/Sizing/AspectRatio.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  AspectRatio CSSValueConversion<AspectRatio>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (css::IsValueId(value, css::ValueId::Auto))
    {
      return css::keywords::Auto {};
    }

    if (auto *ratioValue = DynamicDowncast<css::RatioValue>(value))
    {
      return ConvertToStyle(ratioValue->GetRatio(), state);
    }

    auto list = RequiredListDowncast<css::ValueList, css::Value, 2>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    const auto &value0 = list->item(0);
    if (!css::IsValueId(value0, css::ValueId::Auto))
    {
      return css::keywords::Auto {};
    }

    const auto &value1 = list->item(1);
    auto *ratioValue = RequiredDowncast<css::RatioValue>(state, value1);
    if (!ratioValue)
    {
      return css::keywords::Auto {};
    }

    return {css::keywords::Auto {}, ConvertToStyle(ratioValue->GetRatio(), state)};
  }

  // MARK: - Blending

  bool Blending<AspectRatio>::CanBlend(const AspectRatio &a, const AspectRatio &b) noexcept
  {
    return (a.IsRatio() && b.IsRatio()) || (a.IsAutoAndRatio() && b.IsAutoAndRatio());
  }

  AspectRatio Blending<AspectRatio>::Blend(const AspectRatio &a, const AspectRatio &b,
                                           const RenderStyle &aStyle, const RenderStyle &bStyle,
                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(a._type == b._type);
    auto blendedRatio =
      boo::Blend(std::log(aStyle.LogicalAspectRatio()), std::log(bStyle.LogicalAspectRatio()), context);
    return AspectRatio {a._type, Ratio {.Numerator = {std::exp(blendedRatio)}, .Denominator = {1}}};
  }
}
