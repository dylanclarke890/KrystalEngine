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

#include "Krystal.Booey/Style/Values/Inline/VerticalAlign.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  VerticalAlign CSSValueConversion<VerticalAlign>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Baseline {};
    }

    if (primitiveValue->IsValueId())
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Baseline:               return css::keywords::Baseline {};
        case css::ValueId::Sub:                    return css::keywords::Sub {};
        case css::ValueId::Super:                  return css::keywords::Super {};
        case css::ValueId::Top:                    return css::keywords::Top {};
        case css::ValueId::TextTop:                return css::keywords::TextTop {};
        case css::ValueId::Middle:                 return css::keywords::Middle {};
        case css::ValueId::Bottom:                 return css::keywords::Bottom {};
        case css::ValueId::TextBottom:             return css::keywords::TextBottom {};
        case css::ValueId::InternalBaselineMiddle: return css::keywords::InternalBaselineMiddle {};
        default:                                   break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Baseline {};
    }

    return ConvertCSSValueToStyle<VerticalAlignLength>(state, *primitiveValue);
  }

  // MARK: - Blending

  bool Blending<VerticalAlign>::CanBlend(const VerticalAlign &a, const VerticalAlign &b) noexcept
  {
    return a._value.index() == b._value.index();
  }

  bool Blending<VerticalAlign>::RequiresInterpolationForAccumulativeIteration(const VerticalAlign &a,
                                                                              const VerticalAlign &b) noexcept
  {
    if (a._value.index() != b._value.index())
    {
      return true;
    }
    if (!a.IsLength())
    {
      return false;
    }
    return style::RequiresInterpolationForAccumulativeIteration(*a.TryLength(), *b.TryLength());
  }

  VerticalAlign Blending<VerticalAlign>::Blend(const VerticalAlign &a, const VerticalAlign &b,
                                               const BlendingContext &context) noexcept
  {
    if (!a.IsLength() || !b.IsLength())
    {
      return context.Progress < 0.5 ? a : b;
    }

    krys_debug_assert(CanBlend(a, b));
    return style::Blend(*a.TryLength(), *b.TryLength(), context);
  }
}
