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

#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewTimelineInsets.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"

namespace krys::boo::style
{
  const ViewTimelineInsetItem &ViewTimelineInsetDefaulter::operator()() const noexcept
  {
    static NeverDestroyed<ViewTimelineInsetItem> staticValue {css::keywords::Auto {}};
    return staticValue.get();
  }

  // MARK: - Conversion

  ViewTimelineInsetItem
    CSSValueConversion<ViewTimelineInsetItem>::operator()(BuilderState &state,
                                                          const css::PrimitiveValue &value) noexcept
  {
    return ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, value);
  }

  ViewTimelineInsetItem
    CSSValueConversion<ViewTimelineInsetItem>::operator()(BuilderState &state,
                                                          const css::ValuePair &value) noexcept
  {
    return {
      ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, value.First()),
      ConvertCSSValueToStyle<ViewTimelineInsetItem::Length>(state, value.Second()),
    };
  }

  ViewTimelineInsetItem
    CSSValueConversion<ViewTimelineInsetItem>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (const auto *pair = DynamicDowncast<css::ValuePair>(value))
    {
      return this->operator()(state, *pair);
    }

    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    return this->operator()(state, *primitiveValue);
  }

  ViewTimelineInsets CSSValueConversion<ViewTimelineInsets>::operator()(BuilderState &state,
                                                                        const css::Value &value) noexcept
  {
    if (const auto *pair = DynamicDowncast<css::ValuePair>(value))
    {
      return {ConvertCSSValueToStyle<ViewTimelineInsetItem>(state, *pair)};
    }

    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      return {ConvertCSSValueToStyle<ViewTimelineInsetItem>(state, *primitiveValue)};
    }

    auto list = RequiredListDowncast<css::ValueList, css::Value>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    return ViewTimelineInsetList::Map(
      *list, [&](const auto &element) -> ViewTimelineInsetItem
      { return ConvertCSSValueToStyle<ViewTimelineInsetItem>(state, element); });
  }
}
