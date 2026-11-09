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

#include "Krystal.Booey/Style/Values/Fonts/FontWidth.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  FontWidth CSSValueConversion<FontWidth>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Normal {};
    }

    switch (auto valueID = primitiveValue->ValueId(); valueID)
    {
      case css::ValueId::Invalid:
        return ConvertCSSValueToStyle<FontWidth::Percentage>(state, *primitiveValue);
      case css::ValueId::UltraCondensed: return css::keywords::UltraCondensed {};
      case css::ValueId::ExtraCondensed: return css::keywords::ExtraCondensed {};
      case css::ValueId::Condensed:      return css::keywords::Condensed {};
      case css::ValueId::SemiCondensed:  return css::keywords::SemiCondensed {};
      case css::ValueId::Normal:         return css::keywords::Normal {};
      case css::ValueId::SemiExpanded:   return css::keywords::SemiExpanded {};
      case css::ValueId::Expanded:       return css::keywords::Expanded {};
      case css::ValueId::ExtraExpanded:  return css::keywords::ExtraExpanded {};
      case css::ValueId::UltraExpanded:  return css::keywords::UltraExpanded {};
      default:
        if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
        {
          return css::keywords::Normal {};
        }

        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::Normal {};
    }
  }

  // MARK: Blending

  FontWidth Blending<FontWidth>::Blend(const FontWidth &a, const FontWidth &b,
                                       const BlendingContext &context) noexcept
  {
    return style::Blend(a.GetPercentage(), b.GetPercentage(), context);
  }
}
