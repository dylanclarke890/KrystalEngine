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

#include "Krystal.Booey/Style/Values/Sizing/ContainIntrinsicSize.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  ContainIntrinsicSize CSSValueConversion<ContainIntrinsicSize>::operator()(BuilderState &state,
                                                                            const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }

      if (primitiveValue->IsLength())
      {
        return ContainIntrinsicSize::Length {primitiveValue->ResolveAsLength<float>(
          state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f))};
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::None {};
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return css::keywords::None {};
    }

    if (pair->first->ValueId() != css::ValueId::Auto)
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::None {};
    }

    if (pair->second->ValueId() == css::ValueId::None)
    {
      return {css::keywords::Auto {}, css::keywords::None {}};
    }

    if (const auto &second = pair->second; second->IsLength())
    {
      return {css::keywords::Auto {}, ContainIntrinsicSize::Length {second->ResolveAsLength<float>(
                                        state.CssToLengthConversionData().CopyWithAdjustedZoom(1.0f))}};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::None {};
  }

  // MARK: - Blending

  bool Blending<ContainIntrinsicSize>::CanBlend(const ContainIntrinsicSize &a,
                                                const ContainIntrinsicSize &b) noexcept
  {
    return a._type == b._type && a.HasLength();
  }

  ContainIntrinsicSize Blending<ContainIntrinsicSize>::Blend(const ContainIntrinsicSize &a,
                                                             const ContainIntrinsicSize &b,
                                                             const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(a._type == b._type);
    krys_debug_assert(a.HasLength());

    return ContainIntrinsicSize {a._type, style::Blend(a._length, b._length, context)};
  }
}
