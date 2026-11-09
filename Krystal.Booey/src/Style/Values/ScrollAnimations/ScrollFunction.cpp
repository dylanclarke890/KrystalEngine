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

#include "Krystal.Booey/Style/Values/ScrollAnimations/ScrollFunction.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/ScrollValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ScrollFunction CSSValueConversion<ScrollFunction>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    const auto *scrollValue = RequiredDowncast<css::ScrollValue>(state, value);
    if (!scrollValue)
    {
      return ScrollFunction {ScrollFunctionParameters {Scroller::Nearest, ScrollAxis::Block}};
    }
    return this->operator()(state, *scrollValue);
  }

  ScrollFunction CSSValueConversion<ScrollFunction>::operator()(BuilderState &,
                                                                const css::ScrollValue &value) noexcept
  {
    return ScrollFunction {ScrollFunctionParameters {
      value.Scroller() ? FromValueId<Scroller>(value.Scroller()->ValueId()) : Scroller::Nearest,
      value.Axis() ? FromValueId<ScrollAxis>(value.Axis()->ValueId()) : ScrollAxis::Block}};
  }

  Ref<css::Value> CSSValueCreation<ScrollFunction>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                               const ScrollFunction &value) noexcept
  {
    return css::ScrollValue::Create(CreateCSSValue(pool, style, value->Scroller),
                                    CreateCSSValue(pool, style, value->Axis));
  }

  // MARK: - Serialization

  void Serialiser<ScrollFunctionParameters>::operator()(StringBuilder &builder,
                                                        const css::SerialisationContext &context,
                                                        const RenderStyle &style,
                                                        const ScrollFunctionParameters &value) noexcept
  {
    auto hasScroller = value.Scroller != Scroller::Nearest;
    auto hasAxis = value.Axis != ScrollAxis::Block;

    bool needsSpace = false;
    if (hasScroller)
    {
      Serialise(builder, context, style, value.Scroller);
      needsSpace = true;
    }

    if (hasAxis)
    {
      if (needsSpace)
      {
        builder.Append(u8' ');
      }
      Serialise(builder, context, style, value.Axis);
    }
  }
}
