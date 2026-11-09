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

#include "Krystal.Booey/Style/Values/Borders/CornerShapeValue.hpp"

#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  CornerShapeValue CSSValueConversion<CornerShapeValue>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Round:    return css::keywords::Round {};
        case css::ValueId::Scoop:    return css::keywords::Scoop {};
        case css::ValueId::Bevel:    return css::keywords::Bevel {};
        case css::ValueId::Notch:    return css::keywords::Notch {};
        case css::ValueId::Straight: return css::keywords::Straight {};
        case css::ValueId::Squircle: return css::keywords::Squircle {};
        default:                     break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Round {};
    }

    auto superellipseFunction =
      RequiredFunctionDowncast<css::ValueId::Superellipse, css::PrimitiveValue>(state, value);
    if (!superellipseFunction)
    {
      return css::keywords::Round {};
    }

    const auto &superellipseDescriptor = superellipseFunction->item(0);
    if (superellipseDescriptor.ValueId() == css::ValueId::Infinity)
    {
      return {SuperellipseFunction {Number<css::NonNegative>(std::numeric_limits<double>::infinity())}};
    }

    if (superellipseDescriptor.IsNumber())
    {
      return {SuperellipseFunction {Number<css::NonNegative>(
        std::max(0.0, superellipseDescriptor.ResolveAsNumber<double>(state.CssToLengthConversionData())))}};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Round {};
  }

  // MARK: - Blending

  // https://drafts.csswg.org/css-borders-4/#corner-shape-interpolation

  KRYS_NODISCARD static Number<css::NonNegative>
    ConvertExponentToInterpolationValue(const CornerShapeValue &cornerShape) noexcept
  {
    auto exponent = cornerShape.Superellipse->Value;

    // 1. If exponent is 0, return 0.
    if (exponent == 0.0)
    {
      return 0.0;
    }

    // 2. If exponent is ∞, return 1.
    if (exponent == std::numeric_limits<double>::infinity())
    {
      return 1.0;
    }

    // 3. Return 1/(2^(1/exponent)).
    return 1.0 / std::pow(2.0, 1.0 / exponent);
  }

  KRYS_NODISCARD static CornerShapeValue
    ConvertInterpolationValueToExponent(Number<css::NonNegative> interpolationValue) noexcept
  {
    // 1. If interpolationValue is 0, return 0.
    if (interpolationValue.Value == 0.0)
    {
      return {SuperellipseFunction {0.0}};
    }

    // 2. If interpolationValue is 1, return ∞.
    if (interpolationValue.Value == 1.0)
    {
      return {SuperellipseFunction {std::numeric_limits<double>::infinity()}};
    }

    // 3. Return ln(0.5)/ln(interpolationValue).
    return {SuperellipseFunction {std::log(0.5) / std::log(interpolationValue.Value)}};
  }

  CornerShapeValue Blending<CornerShapeValue>::Blend(const CornerShapeValue &a, const CornerShapeValue &b,
                                                     const BlendingContext &context) noexcept
  {
    auto aInterpolationValue = ConvertExponentToInterpolationValue(a);
    auto bInterpolationValue = ConvertExponentToInterpolationValue(b);

    auto interpolatedValue = style::Blend(aInterpolationValue, bInterpolationValue, context);

    return ConvertInterpolationValueToExponent(interpolatedValue);
  }
}
