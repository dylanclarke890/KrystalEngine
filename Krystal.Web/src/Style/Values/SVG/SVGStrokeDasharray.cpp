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

#include "Krystal.Web/Style/Values/SVG/SVGStrokeDasharray.hpp"

#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Web/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Web/Style/Builder/BuilderState.hpp"
#include "Krystal.Web/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Web/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::web::style
{
  using namespace css::literals;

  // MARK: - Conversion

  SVGStrokeDasharrayValue
    CSSValueConversion<SVGStrokeDasharrayValue>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return 0_css_px;
    }

    if (primitiveValue->IsNumberOrInteger())
    {
      return SVGStrokeDasharrayValueLength {SVGStrokeDasharrayValueLength::Fixed {
        ConvertCSSValueToStyle<Number<css::All, float>>(state, *primitiveValue).Value}};
    }
    return ConvertCSSValueToStyle<SVGStrokeDasharrayValueLength>(state, *primitiveValue);
  }

  // MARK: - Blending

  SVGStrokeDasharray Blending<SVGStrokeDasharray>::Blend(const SVGStrokeDasharray &a,
                                                         const SVGStrokeDasharray &b,
                                                         const BlendingContext &context) noexcept
  {
    auto aLength = a.size();
    auto bLength = b.size();
    if (!aLength || !bLength)
    {
      return context.Progress < 0.5 ? a : b;
    }

    auto resultLength = aLength;
    if (aLength != bLength)
    {
      if (!(std::max(aLength, bLength) % std::min(aLength, bLength)))
      {
        resultLength = std::max(aLength, bLength);
      }
      else
      {
        resultLength = aLength * bLength;
      }
    }

    return SVGStrokeDasharrayList(
      std::from_range, std::views::iota(0uz, resultLength)
                         | std::views::transform(
                           [&](auto i) { return style::Blend(a[i % aLength], b[i % bLength], context); }));
  }
}
