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

#include "Krystal.Booey/Style/Values/Fonts/FontStyle.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Values/FontStyleWithAngleValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontStyle CSSValueConversion<FontStyle>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (const auto *fontStyleValue = DynamicDowncast<css::FontStyleWithAngleValue>(value))
    {
      return ConvertToStyle(fontStyleValue->GetObliqueAngle(), state);
    }

    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Normal {};
    }

    switch (auto valueID = primitiveValue->ValueId(); valueID)
    {
      case css::ValueId::Normal:  return css::keywords::Normal {};
      case css::ValueId::Italic:  return css::keywords::Italic {};
      case css::ValueId::Oblique: return css::keywords::Oblique {};
      default:
        if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
        {
          return css::keywords::Normal {};
        }

        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::Normal {};
    }
  }

  Ref<css::Value> CSSValueCreation<FontStyle>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                          const FontStyle &value) noexcept
  {
    if (!value.PlatformSlope() || !*value.PlatformSlope())
    {
      return CreateCSSValue(pool, style, css::keywords::Normal {});
    }

    if (*value.PlatformSlope() == ItalicValue())
    {
      if (value.PlatformAxis() == FontStyleAxis::ital)
      {
        return CreateCSSValue(pool, style, css::keywords::Italic {});
      }
      return CreateCSSValue(pool, style, css::keywords::Oblique {});
    }

    return css::FontStyleWithAngleValue::Create(
      ConvertToCSS(FontStyle::Angle {static_cast<float>(*value.PlatformSlope())}, style));
  }

  // MARK: - Blending

  bool Blending<FontStyle>::CanBlend(const FontStyle &a, const FontStyle &b) noexcept
  {
    return a.PlatformAxis() == FontStyleAxis::slnt && b.PlatformAxis() == FontStyleAxis::slnt;
  }

  FontStyle Blending<FontStyle>::Blend(const FontStyle &a, const FontStyle &b,
                                       const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      return context.Progress < 0.5 ? a : b;
    }

    if (!a.PlatformSlope() && !b.PlatformSlope())
    {
      return css::keywords::Normal {};
    }

    return style::Blend(a.GetAngle().value_or(FontStyle::Angle {0}),
                        b.GetAngle().value_or(FontStyle::Angle {0}), context);
  }
}
