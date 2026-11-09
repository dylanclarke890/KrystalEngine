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

#include "Krystal.Booey/Style/Values/Masking/MaskBorderOutset.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  KRYS_NODISCARD static MaskBorderOutsetValue ConvertMaskBorderOutsetValue(BuilderState &state,
                                                                           const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return MaskBorderOutsetValue {0_css_number};
    }

    if (primitiveValue->IsNumber())
    {
      return ConvertCSSValueToStyle<MaskBorderOutsetValue::Number>(state, *primitiveValue);
    }
    return ConvertCSSValueToStyle<MaskBorderOutsetValue::Length>(state, *primitiveValue);
  }

  MaskBorderOutset CSSValueConversion<MaskBorderOutset>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    if (auto *quadValue = DynamicDowncast<css::QuadValue>(value))
    {
      auto &quad = quadValue->GetQuad();
      return MaskBorderOutset {
        ConvertMaskBorderOutsetValue(state, quad.Top()),
        ConvertMaskBorderOutsetValue(state, quad.Right()),
        ConvertMaskBorderOutsetValue(state, quad.Bottom()),
        ConvertMaskBorderOutsetValue(state, quad.Left()),
      };
    }

    // Values coming from CSS Typed OM may not have been converted to a Quad.
    return ConvertMaskBorderOutsetValue(state, value);
  }

  Ref<css::Value> CSSValueCreation<MaskBorderOutset>::operator()(css::ValuePool &pool,
                                                                 const RenderStyle &style,
                                                                 const MaskBorderOutset &value) noexcept
  {
    return css::QuadValue::Create({
      CreateCSSValue(pool, style, value.Values.Top()),
      CreateCSSValue(pool, style, value.Values.Right()),
      CreateCSSValue(pool, style, value.Values.Bottom()),
      CreateCSSValue(pool, style, value.Values.Left()),
    });
  }

  // MARK: - Blending

  inline bool Blending<MaskBorderOutsetValue>::CanBlend(const MaskBorderOutsetValue &a,
                                                        const MaskBorderOutsetValue &b) noexcept
  {
    return a.HasSameType(b);
  }

  inline bool Blending<MaskBorderOutsetValue>::RequiresInterpolationForAccumulativeIteration(
    const MaskBorderOutsetValue &a, const MaskBorderOutsetValue &b) noexcept
  {
    return !a.HasSameType(b);
  }

  inline MaskBorderOutsetValue Blending<MaskBorderOutsetValue>::Blend(const MaskBorderOutsetValue &a,
                                                                      const MaskBorderOutsetValue &b,
                                                                      const BlendingContext &context) noexcept
  {
    if (!a.HasSameType(b))
    {
      return MaskBorderOutsetValue {0_css_px};
    }

    if (!context.Progress && context.IsReplace())
    {
      return a;
    }
    if (context.Progress == 1 && context.IsReplace())
    {
      return b;
    }

    return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> MaskBorderOutsetValue
                                          { return style::Blend(a, b, context); },
                                          [&](const auto &, const auto &) -> MaskBorderOutsetValue
                                          { krys_unreachable(); }),
                      a._value, b._value);
  }

  bool Blending<MaskBorderOutset>::CanBlend(const MaskBorderOutset &a, const MaskBorderOutset &b) noexcept
  {
    return style::CanBlend(a.Values.Top(), b.Values.Top())
           && style::CanBlend(a.Values.Right(), b.Values.Right())
           && style::CanBlend(a.Values.Bottom(), b.Values.Bottom())
           && style::CanBlend(a.Values.Left(), b.Values.Left());
  }

  bool Blending<MaskBorderOutset>::RequiresInterpolationForAccumulativeIteration(
    const MaskBorderOutset &a, const MaskBorderOutset &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(a.Values.Top(), b.Values.Top())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Right(), b.Values.Right())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Bottom(), b.Values.Bottom())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Left(), b.Values.Left());
  }

  MaskBorderOutset Blending<MaskBorderOutset>::Blend(const MaskBorderOutset &a, const MaskBorderOutset &b,
                                                     const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return MaskBorderOutset {
      style::Blend(a.Values.Top(), b.Values.Top(), context),
      style::Blend(a.Values.Right(), b.Values.Right(), context),
      style::Blend(a.Values.Bottom(), b.Values.Bottom(), context),
      style::Blend(a.Values.Left(), b.Values.Left(), context),
    };
  }
}
