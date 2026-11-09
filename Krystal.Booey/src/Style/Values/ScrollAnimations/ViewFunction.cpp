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

#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewFunction.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/ViewValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ViewFunction CSSValueConversion<ViewFunction>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    const auto *viewValue = RequiredDowncast<css::ViewValue>(state, value);
    if (!viewValue)
    {
      return ViewFunction {ViewFunctionParameters {
        .Axis = ScrollAxis::Block,
        .Insets = {css::keywords::Auto {}},
      }};
    }

    return this->operator()(state, *viewValue);
  }

  ViewFunction CSSValueConversion<ViewFunction>::operator()(BuilderState &state,
                                                            const css::ViewValue &value) noexcept
  {
    auto axis = [&]
    {
      if (const auto &axisValue = value.Axis())
      {
        return FromValueId<ScrollAxis>(axisValue->ValueId());
      }
      return ScrollAxis::Block;
    }();

    auto startInset = [&]
    {
      if (const auto &startInsetValue = value.StartInset())
      {
        return ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, *startInsetValue);
      }
      return ViewTimelineInsetItem::Length {css::keywords::Auto {}};
    }();

    auto endInset = [&]
    {
      if (const auto &endInsetValue = value.EndInset())
      {
        return ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, *endInsetValue);
      }
      return startInset;
    }();

    return ViewFunction {ViewFunctionParameters {
      .Axis = axis,
      .Insets = {krys::move(startInset), krys::move(endInset)},
    }};
  }

  Ref<css::Value> CSSValueCreation<ViewFunction>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                             const ViewFunction &value) noexcept
  {
    return css::ViewValue::Create(CreateCSSValue(pool, style, value->Axis),
                                  CreateCSSValue(pool, style, value->Insets.Start()),
                                  CreateCSSValue(pool, style, value->Insets.End()));
  }

  // MARK: - Serialization

  void Serialiser<ViewFunctionParameters>::operator()(StringBuilder &builder,
                                                      const css::SerialisationContext &context,
                                                      const RenderStyle &style,
                                                      const ViewFunctionParameters &value) noexcept
  {
    bool needsSpace = false;
    if (value.Axis != ScrollAxis::Block)
    {
      Serialise(builder, context, style, value.Axis);
      needsSpace = true;
    }

    if (value.Insets.Start() == value.Insets.End())
    {
      if (value.Insets.Start().IsAuto())
      {
        return;
      }

      if (needsSpace)
      {
        builder.Append(u8' ');
      }
      Serialise(builder, context, style, value.Insets.Start());
    }
    else
    {
      if (needsSpace)
      {
        builder.Append(u8' ');
      }

      Serialise(builder, context, style, value.Insets.Start());
      builder.Append(u8' ');
      Serialise(builder, context, style, value.Insets.End());
    }
  }
}
