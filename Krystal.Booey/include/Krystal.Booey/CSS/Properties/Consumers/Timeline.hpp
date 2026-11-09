/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  enum class SingleAnimationRangeType : bool;
}

namespace krys::boo::css
{
  class TokenRange;
  struct PropertyParserState;
  struct ParserContext;
  class Value;

  enum class ValueId : uint16;

  namespace PropertyParserHelpers
  {
    KRYS_NODISCARD bool IsAnimationRangeKeyword(ValueId id) noexcept;

    // MARK: - Consumer functions

    // <scroll()> = scroll( [ <scroller> || <axis> ]? )
    // https://drafts.csswg.org/scroll-animations-1/#scroll-notation
    KRYS_NODISCARD RefPtr<Value> ConsumeAnimationTimelineScroll(TokenRange &tokens,
                                                                PropertyParserState &state) noexcept;

    // <view()> = view( [ <axis> || <'view-timeline-inset'> ]? )
    // https://drafts.csswg.org/scroll-animations-1/#view-notation
    KRYS_NODISCARD RefPtr<Value> ConsumeAnimationTimelineView(TokenRange &tokens,
                                                              PropertyParserState &state) noexcept;

    // <single-view-timeline-inset-item> = <single-view-timeline-inset>{1,2}
    // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline-inset
    KRYS_NODISCARD RefPtr<Value> ConsumeSingleViewTimelineInsetItem(TokenRange &tokens,
                                                                    PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ParseSingleViewTimelineInsetItem(const CSSOMString &input,
                                                                  const ParserContext &context) noexcept;

    // <single-animation-range> = normal | <length-percentage> | <timeline-range-name> <length-percentage>?
    // https://drafts.csswg.org/scroll-animations-1/#propdef-animation-range-start
    KRYS_NODISCARD RefPtr<Value> ConsumeSingleAnimationRange(TokenRange &tokens, PropertyParserState &state,
                                                             style::SingleAnimationRangeType type) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeSingleAnimationRangeStart(TokenRange &tokens,
                                                                  PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ConsumeSingleAnimationRangeEnd(TokenRange &tokens,
                                                                PropertyParserState &state) noexcept;

    KRYS_NODISCARD RefPtr<Value> ParseSingleAnimationRange(const CSSOMString &input,
                                                           const ParserContext &context,
                                                           style::SingleAnimationRangeType type) noexcept;
  }
}