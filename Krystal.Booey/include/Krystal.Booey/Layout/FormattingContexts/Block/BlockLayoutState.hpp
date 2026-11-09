/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Layout/Floats/PlacedFloats.hpp"
#include "Krystal.Booey/Style/Values/Inline/TextBoxEdge.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief Block level information shared across child inline formatting contexts.
  class BlockLayoutState
  {
  public:
    struct LineClamp
    {
      size_t MaximumLines {0};
      bool ShouldDiscardOverflow {false};
    };

    enum class TextBoxTrimSide : bool
    {
      Start,
      End
    };
    using TextBoxTrim = EnumSet<TextBoxTrimSide>;

    struct MarginState
    {
      void ResetBeforeSideOfBlock() noexcept
      {
        krys_debug_assert(AtBeforeSideOfBlock);
        AtBeforeSideOfBlock = {};
      }

      void ResetMarginValues() noexcept
      {
        PositiveMargin = 0_lu;
        NegativeMargin = 0_lu;
        ContentOffsetAfterSelfCollapsingBlock = 0_lu;
      }

      KRYS_NODISCARD LayoutUnit Margin() const noexcept
      {
        return PositiveMargin - NegativeMargin;
      }

      // FIXME: This tracks RenderBlockFlow's MarginInfo for now.
      bool CanCollapseWithChildren : 1 {false};
      bool CanCollapseMarginBeforeWithChildren : 1 {false};
      bool CanCollapseMarginAfterWithChildren : 1 {false};
      bool QuirkContainer : 1 {false};
      bool AtBeforeSideOfBlock : 1 {true};
      bool AtAfterSideOfBlock : 1 {false};
      bool HasMarginBeforeQuirk : 1 {false};
      bool HasMarginAfterQuirk : 1 {false};
      bool DeterminedMarginBeforeQuirk : 1 {false};
      LayoutUnit PositiveMargin;
      LayoutUnit NegativeMargin;
      // This tracks the offset initiated by a self collapsing block when "next line top position" diverges
      // from "current line's bottom". See the last paragraph at
      // https://www.w3.org/TR/CSS22/box.html#collapsing-margins
      LayoutUnit ContentOffsetAfterSelfCollapsingBlock;
    };

  private:
    PlacedFloats &_placedFloats;
    Maybe<LineClamp> _lineClamp;
    TextBoxTrim _textBoxTrim;
    style::TextBoxEdge _textBoxEdge;
    Maybe<LayoutUnit> _intrusiveInitialLetterLogicalBottom;
    MarginState _marginState;

  public:
    BlockLayoutState(PlacedFloats &placedFloats, MarginState marginState, Maybe<LineClamp> lineClamp = null,
                     TextBoxTrim textBoxTrim = {}, style::TextBoxEdge textBoxEdge = css::keywords::Auto {},
                     Maybe<LayoutUnit> intrusiveInitialLetterLogicalBottom = null) noexcept
        : _placedFloats(placedFloats), _lineClamp(lineClamp), _textBoxTrim(textBoxTrim),
          _textBoxEdge(textBoxEdge),
          _intrusiveInitialLetterLogicalBottom(intrusiveInitialLetterLogicalBottom), _marginState(marginState)
    {
    }

    KRYS_NODISCARD PlacedFloats &GetPlacedFloats() noexcept
    {
      return _placedFloats;
    }
    KRYS_NODISCARD const PlacedFloats &GetPlacedFloats() const noexcept
    {
      return _placedFloats;
    }
    KRYS_NODISCARD Maybe<LineClamp> GetLineClamp() const noexcept
    {
      return _lineClamp;
    }
    KRYS_NODISCARD TextBoxTrim GetTextBoxTrim() const noexcept
    {
      return _textBoxTrim;
    }
    KRYS_NODISCARD style::TextBoxEdge GetTextBoxEdge() const noexcept
    {
      return _textBoxEdge;
    }
    KRYS_NODISCARD Maybe<LayoutUnit> IntrusiveInitialLetterLogicalBottom() const noexcept
    {
      return _intrusiveInitialLetterLogicalBottom;
    }

    KRYS_NODISCARD MarginState &GetMarginState() noexcept
    {
      return _marginState;
    }
    KRYS_NODISCARD const MarginState &GetMarginState() const noexcept
    {
      return _marginState;
    }
  };
}
