/*
 * Copyright (C) 2024-2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Animation/Timelines/TimelineRangeValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRange.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  RefPtr<css::Value> ConvertToCSSValue(TimelineRangeValue &&value, RefPtr<dom::Element> element,
                                       style::SingleAnimationRangeType type) noexcept
  {
    if (!element)
    {
      return {};
    }

    Ref document = ShareRef(element->NodeDocument());
    return krys::SwitchOn(
      value,
      [&](dom::DOMString &rangeString) -> RefPtr<css::Value>
      {
        return css::PropertyParserHelpers::ParseSingleAnimationRange(rangeString,
                                                                     document->CssParserContext(), type);
      },
      [&](TimelineRangeOffset &rangeOffset) -> RefPtr<css::Value>
      {
        if (auto consumedRangeName = css::PropertyParserHelpers::ParseSingleAnimationRange(
              rangeOffset.RangeName, document->CssParserContext(), type))
        {
          if (RefPtr offset = rangeOffset.Offset)
          {
            if (auto offsetValue = offset->ToCSSValue())
            {
              return css::ValuePair::CreateNonCoalescing(krys::move(consumedRangeName),
                                                         krys::move(offsetValue));
            }
          }
          return consumedRangeName;
        }
        if (RefPtr offset = rangeOffset.Offset)
        {
          return offset->ToCSSValue();
        }
        return nullptr;
      },
      [&](RefPtr<css::CSSKeywordValue> &rangeKeyword) -> RefPtr<css::Value>
      { return rangeKeyword ? rangeKeyword->ToCSSValue() : nullptr; },
      [&](RefPtr<css::CSSNumericValue> &rangeValue) -> RefPtr<css::Value>
      { return rangeValue ? rangeValue->ToCSSValue() : nullptr; });
  }
}
