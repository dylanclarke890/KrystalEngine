/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBoxLegacyPath.hpp"
#include "Krystal.Booey/Rendering/Line/LegacyRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::inline_iterator
{
  /// @brief A line of the legacy line layout (SVG text's).
  class LineBoxIteratorLegacyPath
  {
  private:
    WeakPtr<const LegacyRootInlineBox> _rootInlineBox;

  public:
    explicit LineBoxIteratorLegacyPath(const LegacyRootInlineBox *rootInlineBox) noexcept
        : _rootInlineBox(CreateWeakPtr(rootInlineBox))
    {
    }

    KRYS_NODISCARD float ContentLogicalTop() const noexcept
    {
      return _rootInlineBox->LineTop().ToFloat();
    }
    KRYS_NODISCARD float ContentLogicalBottom() const noexcept
    {
      return _rootInlineBox->LineBottom().ToFloat();
    }
    KRYS_NODISCARD float ContentLogicalTopAdjustedForPrecedingLineBox() const noexcept
    {
      return _rootInlineBox->SelectionTop().ToFloat();
    }
    KRYS_NODISCARD float ContentLogicalBottomAdjustedForFollowingLineBox() const noexcept
    {
      return _rootInlineBox->SelectionBottom().ToFloat();
    }
    KRYS_NODISCARD float LogicalTop() const noexcept
    {
      return _rootInlineBox->LineBoxTop().ToFloat();
    }
    KRYS_NODISCARD float LogicalBottom() const noexcept
    {
      return _rootInlineBox->LineBoxBottom().ToFloat();
    }
    KRYS_NODISCARD float LogicalWidth() const noexcept
    {
      return _rootInlineBox->LineBoxWidth().ToFloat();
    }
    KRYS_NODISCARD float InkOverflowLogicalTop() const noexcept
    {
      return _rootInlineBox->LogicalTopVisualOverflow().ToFloat();
    }
    KRYS_NODISCARD float InkOverflowLogicalBottom() const noexcept
    {
      return _rootInlineBox->LogicalBottomVisualOverflow().ToFloat();
    }
    KRYS_NODISCARD float ScrollableOverflowTop() const noexcept
    {
      return LogicalTop();
    }
    KRYS_NODISCARD float ScrollableOverflowBottom() const noexcept
    {
      return LogicalBottom();
    }

    KRYS_NODISCARD bool HasEllipsis() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD FloatRect EllipsisVisualRectIgnoringBlockDirection() const noexcept
    {
      krys_debug_assert(false);
      return {};
    }

    KRYS_NODISCARD TextRun EllipsisText() const noexcept
    {
      krys_debug_assert(false);
      return TextRun {utf8_stringview()};
    }

    KRYS_NODISCARD float ContentLogicalLeft() const noexcept
    {
      return _rootInlineBox->LogicalLeft();
    }
    KRYS_NODISCARD float ContentLogicalRight() const noexcept
    {
      return _rootInlineBox->LogicalRight();
    }
    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return _rootInlineBox->IsHorizontal();
    }
    KRYS_NODISCARD FontBaseline BaselineType() const noexcept
    {
      return _rootInlineBox->BaselineType();
    }

    KRYS_NODISCARD const RenderBlockFlow &FormattingContextRoot() const noexcept
    {
      return _rootInlineBox->BlockFlow();
    }

    KRYS_NODISCARD bool IsFirstAfterPageBreak() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool HasBlockLevelBox() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD size_t LineIndex() const noexcept
    {
      return FormattingContextRoot().LegacyRootBox() ? 1 : 0;
    }

    void TraverseNext() noexcept
    {
      _rootInlineBox = CreateWeakPtr<const LegacyRootInlineBox>(_rootInlineBox->NextRootBox());
    }

    void TraversePrevious() noexcept
    {
      _rootInlineBox = CreateWeakPtr<const LegacyRootInlineBox>(_rootInlineBox->PrevRootBox());
    }

    KRYS_NODISCARD friend bool operator==(const LineBoxIteratorLegacyPath &a,
                                          const LineBoxIteratorLegacyPath &b) noexcept
    {
      return a._rootInlineBox.get() == b._rootInlineBox.get();
    }

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_rootInlineBox;
    }

    KRYS_NODISCARD BoxLegacyPath FirstLeafBox() const noexcept
    {
      return BoxLegacyPath {_rootInlineBox->FirstLeafDescendant()};
    }

    KRYS_NODISCARD BoxLegacyPath LastLeafBox() const noexcept
    {
      return BoxLegacyPath {_rootInlineBox->LastLeafDescendant()};
    }
  };
}
