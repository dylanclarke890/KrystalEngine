/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Layout/Floats/PlacedFloats.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief What line building produced for one line: the runs, the floats placed or suspended, and the
  /// line's and its content's geometry.
  struct LineLayoutResult
  {
    using PlacedFloatList = PlacedFloats::List;
    using SuspendedFloatList = List<const Box *>;

    InlineItemRange ItemRange;
    Line::RunList Runs;

    struct FloatContent
    {
      PlacedFloatList PlacedFloats;
      SuspendedFloatList SuspendedFloats;
      EnumSet<UsedFloat> HasIntrusiveFloat {};
    };
    FloatContent Floats {};

    struct ContentGeometry
    {
      InlineLayoutUnit LogicalLeft {0.f};
      InlineLayoutUnit LogicalWidth {0.f};
      // Note that with negative horizontal margin value, contentLogicalLeft + contentLogicalWidth is not
      // necessarily contentLogicalRight.
      InlineLayoutUnit LogicalRightIncludingNegativeMargin {0.f};
      Maybe<InlineLayoutUnit> TrailingOverflowingContentWidth {};
    };
    ContentGeometry Content {};

    struct LineGeometry
    {
      InlineLayoutPoint LogicalTopLeft;
      InlineLayoutUnit LogicalWidth {0.f};
      InlineLayoutUnit InitialLogicalLeft {0.f};
      // Inherited floats from parent formatting context offseting line box.
      InlineLayoutUnit IntrusiveFloatsOffset {0.f};
      Maybe<InlineLayoutUnit> InitialLetterClearGap {};
    };
    LineGeometry Geometry {};

    struct HangingContent
    {
      bool ShouldContributeToScrollableOverflow {false};
      InlineLayoutUnit LogicalWidth {0.f};
      InlineLayoutUnit HangablePunctuationStartWidth {0.f};
    };
    HangingContent Hanging {};

    struct Directionality
    {
      List<int32> VisualOrderList;
      TextDirection InlineBaseDirection {TextDirection::LTR};
    };
    Directionality BidiDirectionality {};

    struct IsFirstLast
    {
      IsFirstFormattedLine FirstFormattedLine {IsFirstFormattedLine::Yes};
      bool IsLastLineWithInlineContent {true};
    };
    IsFirstLast FirstLast {};

    struct Ruby
    {
      HashMap<const Box *, InlineLayoutUnit> BaseAlignmentOffsetList {};
      InlineLayoutUnit AnnotationAlignmentOffset {0.f};
    };
    Ruby RubyAlignment {};

    // Misc
    enum InlineContentEnding : uint8
    {
      Generic,
      Hyphen,
      LineBreak
    };
    // No value means line does not have any inline content (either float, out-of-flow or block inside inline)
    Maybe<InlineContentEnding> ContentEnding {};

    enum class InflowContentType : uint8
    {
      Inline,
      Block
    };
    KRYS_NODISCARD Maybe<InflowContentType> GetInflowContentType() const noexcept
    {
      if (ContentEnding.has_value())
      {
        return InflowContentType::Inline;
      }
      if (!Runs.empty() && Runs.back().IsBlock())
      {
        return InflowContentType::Block;
      }
      return {};
    }
    KRYS_NODISCARD bool HasContentfulInFlowContent() const noexcept
    {
      return GetInflowContentType().has_value();
    }
    KRYS_NODISCARD bool HasContentfulInlineContent() const noexcept
    {
      return HasContentfulInFlowContent() && *GetInflowContentType() == InflowContentType::Inline;
    }
    KRYS_NODISCARD bool IsBlockContent() const noexcept
    {
      return HasContentfulInFlowContent() && *GetInflowContentType() == InflowContentType::Block;
    }

    KRYS_NODISCARD bool EndsWithHyphen() const noexcept
    {
      return ContentEnding && *ContentEnding == InlineContentEnding::Hyphen;
    }
    KRYS_NODISCARD bool EndsWithLineBreak() const noexcept
    {
      return ContentEnding && *ContentEnding == InlineContentEnding::LineBreak;
    }

    size_t NonSpanningInlineLevelBoxCount {0};
    // only used for line-break: after-white-space currently
    InlineLayoutUnit TrimmedTrailingWhitespaceWidth {0.f};
    // This is how much text-box-trim: start adjusts the first line box. We only need it to adjust the initial
    // letter float position (which will not be needed once we drop the float behavior)
    InlineLayoutUnit FirstLineStartTrim {0.f};
    // This is only used for cases when intrusive floats prevent any content placement at current vertical
    // position.
    Maybe<InlineLayoutUnit> HintForNextLineTopToAvoidIntrusiveFloat {};
  };
}
