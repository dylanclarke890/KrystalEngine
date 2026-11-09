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

#include "Krystal.Booey/Style/Values/Motion/OffsetAnchor.hpp"

#include "Krystal.Booey/CSS/Values/PositionValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  OffsetAnchor CSSValueConversion<OffsetAnchor>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::Auto)
    {
      return OffsetAnchor {css::keywords::Auto {}};
    }

    auto *positionValue = RequiredDowncast<css::PositionValue>(state, value);
    if (!positionValue)
    {
      return OffsetAnchor {css::keywords::Auto {}};
    }
    return OffsetAnchor {ConvertToStyle(positionValue->GetPosition(), state)};
  }

  // MARK: - Blending

  bool Blending<OffsetAnchor>::CanBlend(const OffsetAnchor &a, const OffsetAnchor &b) noexcept
  {
    return a.IsPosition() && b.IsPosition();
  }

  bool Blending<OffsetAnchor>::RequiresInterpolationForAccumulativeIteration(const OffsetAnchor &a,
                                                                             const OffsetAnchor &b) noexcept
  {
    krys_debug_assert(CanBlend(a, b));
    return style::RequiresInterpolationForAccumulativeIteration(*a.TryPosition(), *b.TryPosition());
  }

  OffsetAnchor Blending<OffsetAnchor>::Blend(const OffsetAnchor &a, const OffsetAnchor &b,
                                             const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return OffsetAnchor {style::Blend(*a.TryPosition(), *b.TryPosition(), context)};
  }

  // MARK: - Evaluation
}
