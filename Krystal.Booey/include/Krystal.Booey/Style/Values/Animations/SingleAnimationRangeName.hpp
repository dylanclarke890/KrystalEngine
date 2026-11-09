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

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  namespace css
  {
    enum class ValueId : uint16;
  }

  namespace style
  {
    // <timeline-range-name> = cover | contain | entry | exit | entry-crossing | exit-crossing
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-cover
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-contain
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-entry
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-exit
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-entry-crossing
    // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-exit-crossing

    enum class SingleAnimationRangeName : uint8
    {
      Normal,
      Omitted,
      Cover,
      Contain,
      Entry,
      Exit,
      EntryCrossing,
      ExitCrossing
    };

    KRYS_NODISCARD css::ValueId ConvertSingleAnimationRangeNameToValueId(SingleAnimationRangeName) noexcept;
    KRYS_NODISCARD SingleAnimationRangeName ConvertValueIdToSingleAnimationRangeName(css::ValueId) noexcept;

    KRYS_NODISCARD css::CSSOMString
      ConvertSingleAnimationRangeNameToRangeString(SingleAnimationRangeName) noexcept;
    KRYS_NODISCARD SingleAnimationRangeName
      ConvertRangeStringToSingleTimelineRangeName(const css::CSSOMString &) noexcept;
  }
}
