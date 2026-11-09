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

#include "Krystal.Booey/Style/Values/Animations/SingleAnimationTimeline.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/ScrollValue.hpp"
#include "Krystal.Booey/CSS/Values/ViewValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  SingleAnimationTimeline
    CSSValueConversion<SingleAnimationTimeline>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (const auto *scrollValue = DynamicDowncast<css::ScrollValue>(value))
    {
      return ConvertCSSValueToStyle<ScrollFunction>(state, *scrollValue);
    }

    if (const auto *viewValue = DynamicDowncast<css::ViewValue>(value))
    {
      return ConvertCSSValueToStyle<ViewFunction>(state, *viewValue);
    }

    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    switch (primitiveValue->ValueId())
    {
      case css::ValueId::Auto: return css::keywords::Auto {};
      case css::ValueId::None: return css::keywords::None {};
      default:                 break;
    }

    return ConvertCSSValueToStyle<css::CustomIdentifier>(state, *primitiveValue);
  }
}
