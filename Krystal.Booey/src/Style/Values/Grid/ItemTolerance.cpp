/*
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Grid/ItemTolerance.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  ItemTolerance CSSValueConversion<ItemTolerance>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal:   return css::keywords::Normal {};
        case css::ValueId::Infinite: return css::keywords::Infinite {};
        default:                     break;
      }

      // If not a keyword, it must be a <length-percentage [0,∞]>
      return ConvertCSSValueToStyle<LengthPercentage<css::NonNegativeUnzoomed>>(state, *primitiveValue);
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Normal {};
  }

  // MARK: - Blending

  bool Blending<ItemTolerance>::CanBlend(const ItemTolerance &a, const ItemTolerance &b) noexcept
  {
    // Can only blend if both are length-percentage values
    return !a.IsNormal() && !a.IsInfinite() && !b.IsNormal() && !b.IsInfinite();
  }

  ItemTolerance Blending<ItemTolerance>::Blend(const ItemTolerance &a, const ItemTolerance &b,
                                               const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(!a.IsNormal());
    krys_debug_assert(!a.IsInfinite());
    krys_debug_assert(!b.IsNormal());
    krys_debug_assert(!b.IsInfinite());

    // Both must be length-percentage values
    return a.SwitchOn(
      [&](const LengthPercentage<css::NonNegativeUnzoomed> &aValue)
      {
        return b.SwitchOn([&](const LengthPercentage<css::NonNegativeUnzoomed> &bValue) -> ItemTolerance
                          { return krys::boo::style::Blend(aValue, bValue, context); },
                          [&](auto) -> ItemTolerance
                          {
                            krys_debug_assert_msg(false, "Unreachable");
                            return a;
                          });
      },
      [&](auto) -> ItemTolerance
      {
        krys_debug_assert_msg(false, "Unreachable");
        return a;
      });
  }
}
