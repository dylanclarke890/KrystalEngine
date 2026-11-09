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

#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageWidth.hpp"

#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  KRYS_NODISCARD static BorderImageWidthValue ConvertBorderImageWidthValue(BuilderState &state,
                                                                           const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return BorderImageWidthValue {1_css_number};
    }

    if (primitiveValue->ValueId() == css::ValueId::Auto)
    {
      return css::keywords::Auto {};
    }

    if (primitiveValue->IsNumber())
    {
      return ConvertCSSValueToStyle<BorderImageWidthValue::Number>(state, *primitiveValue);
    }
    return ConvertCSSValueToStyle<BorderImageWidthValue::LengthPercentage>(state, *primitiveValue);
  }

  BorderImageWidth CSSValueConversion<BorderImageWidth>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    if (auto *widthValue = DynamicDowncast<css::BorderImageWidthValue>(value))
    {
      auto &widths = widthValue->Widths();
      return BorderImageWidth {
        ConvertBorderImageWidthValue(state, widths.Top()),
        ConvertBorderImageWidthValue(state, widths.Right()),
        ConvertBorderImageWidthValue(state, widths.Bottom()),
        ConvertBorderImageWidthValue(state, widths.Left()),
        widthValue->OverridesBorderWidths(),
      };
    }

    // Values coming from CSS Typed OM may not have been converted to a css::BorderImageWidthValue.
    return ConvertBorderImageWidthValue(state, value);
  }

  Ref<css::Value> CSSValueCreation<BorderImageWidth>::operator()(css::ValuePool &pool,
                                                                 const RenderStyle &style,
                                                                 const BorderImageWidth &value) noexcept
  {
    return css::BorderImageWidthValue::Create(
      {
        CreateCSSValue(pool, style, value.Values.Top()),
        CreateCSSValue(pool, style, value.Values.Right()),
        CreateCSSValue(pool, style, value.Values.Bottom()),
        CreateCSSValue(pool, style, value.Values.Left()),
      },
      value.LegacyWebkitBorderImage);
  }

  // MARK: - Blending

  inline bool Blending<BorderImageWidthValue>::CanBlend(const BorderImageWidthValue &a,
                                                        const BorderImageWidthValue &b) noexcept
  {
    if (a.HasSameType(b))
    {
      return true;
    }

    return (a.IsLengthPercentage() || a.IsNumber()) && (b.IsLengthPercentage() || b.IsNumber())
           && a.IsNumber() == b.IsNumber();
  }

  inline bool Blending<BorderImageWidthValue>::RequiresInterpolationForAccumulativeIteration(
    const BorderImageWidthValue &a, const BorderImageWidthValue &b) noexcept
  {
    return a.IsCalculated() || b.IsCalculated() || !a.HasSameType(b);
  }

  inline BorderImageWidthValue Blending<BorderImageWidthValue>::Blend(const BorderImageWidthValue &a,
                                                                      const BorderImageWidthValue &b,
                                                                      const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> BorderImageWidthValue
                                          { return style::Blend(a, b, context); },
                                          [&](const auto &, const auto &) -> BorderImageWidthValue
                                          { krys_unreachable(); }),
                      a._value, b._value);
  }

  bool Blending<BorderImageWidth>::CanBlend(const BorderImageWidth &a, const BorderImageWidth &b) noexcept
  {
    if (a.LegacyWebkitBorderImage != b.LegacyWebkitBorderImage)
    {
      return false;
    }

    return style::CanBlend(a.Values.Top(), b.Values.Top())
           && style::CanBlend(a.Values.Right(), b.Values.Right())
           && style::CanBlend(a.Values.Bottom(), b.Values.Bottom())
           && style::CanBlend(a.Values.Left(), b.Values.Left());
  }

  bool Blending<BorderImageWidth>::RequiresInterpolationForAccumulativeIteration(
    const BorderImageWidth &a, const BorderImageWidth &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(a.Values.Top(), b.Values.Top())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Right(), b.Values.Right())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Bottom(), b.Values.Bottom())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Left(), b.Values.Left());
  }

  BorderImageWidth Blending<BorderImageWidth>::Blend(const BorderImageWidth &a, const BorderImageWidth &b,
                                                     const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return BorderImageWidth {
      style::Blend(a.Values.Top(), b.Values.Top(), context),
      style::Blend(a.Values.Right(), b.Values.Right(), context),
      style::Blend(a.Values.Bottom(), b.Values.Bottom(), context),
      style::Blend(a.Values.Left(), b.Values.Left(), context),
      (!context.Progress || !context.IsDiscrete ? a : b).LegacyWebkitBorderImage,
    };
  }
}
