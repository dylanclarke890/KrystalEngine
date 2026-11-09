/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Ranges/BoundaryPoint.hpp"
#include "Krystal.Booey/DOM/Ranges/Position.hpp"
#include "Krystal.Booey/Editing/Selection/TextAffinity.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Rendering/Selection/CaretRectComputation.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderObject;
  class VisiblePosition;

  /// @brief Where a position renders: the leaf box on its line and the caret offset in it.
  class RenderedPosition
  {
  private:
    WeakPtr<RenderObject> _renderer;
    inline_iterator::LeafBoxIterator _box;
    uint32 _offset {0};

    mutable Maybe<inline_iterator::LeafBoxIterator> _previousLeafOnLine;
    mutable Maybe<inline_iterator::LeafBoxIterator> _nextLeafOnLine;

  public:
    RenderedPosition() noexcept;
    explicit RenderedPosition(const VisiblePosition &position) noexcept;
    explicit RenderedPosition(const dom::Position &position, Affinity affinity) noexcept;
    KRYS_NODISCARD bool IsEquivalent(const RenderedPosition &other) const noexcept;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return !_renderer;
    }
    KRYS_NODISCARD inline_iterator::LineBoxIterator LineBox() const noexcept
    {
      return _box ? _box->GetLineBox() : inline_iterator::LineBoxIterator();
    }
    KRYS_NODISCARD inline_iterator::LeafBoxIterator Box() const noexcept
    {
      return _box;
    }
    KRYS_NODISCARD uint32 Offset() const noexcept
    {
      return _offset;
    }

    KRYS_NODISCARD uint8 BidiLevelOnLeft() const noexcept;
    KRYS_NODISCARD uint8 BidiLevelOnRight() const noexcept;
    KRYS_NODISCARD RenderedPosition LeftBoundaryOfBidiRun(uint8 bidiLevelOfRun) const noexcept;
    KRYS_NODISCARD RenderedPosition RightBoundaryOfBidiRun(uint8 bidiLevelOfRun) const noexcept;

    enum class ShouldMatchBidiLevel : bool
    {
      MatchBidiLevel,
      IgnoreBidiLevel
    };
    KRYS_NODISCARD bool AtLeftBoundaryOfBidiRun() const noexcept
    {
      return AtLeftBoundaryOfBidiRun(ShouldMatchBidiLevel::IgnoreBidiLevel, 0);
    }
    KRYS_NODISCARD bool AtRightBoundaryOfBidiRun() const noexcept
    {
      return AtRightBoundaryOfBidiRun(ShouldMatchBidiLevel::IgnoreBidiLevel, 0);
    }
    // The following two functions return true only if the current position is at the end of the bidi run
    // of the specified bidi embedding level.
    KRYS_NODISCARD bool AtLeftBoundaryOfBidiRun(uint8 bidiLevelOfRun) const noexcept
    {
      return AtLeftBoundaryOfBidiRun(ShouldMatchBidiLevel::MatchBidiLevel, bidiLevelOfRun);
    }
    KRYS_NODISCARD bool AtRightBoundaryOfBidiRun(uint8 bidiLevelOfRun) const noexcept
    {
      return AtRightBoundaryOfBidiRun(ShouldMatchBidiLevel::MatchBidiLevel, bidiLevelOfRun);
    }

    KRYS_NODISCARD dom::Position PositionAtLeftBoundaryOfBiDiRun() const noexcept;
    KRYS_NODISCARD dom::Position PositionAtRightBoundaryOfBiDiRun() const noexcept;

    KRYS_NODISCARD bool AtLeftmostOffsetInBox() const noexcept
    {
      return _box && _offset == _box->LeftmostCaretOffset();
    }
    KRYS_NODISCARD bool AtRightmostOffsetInBox() const noexcept
    {
      return _box && _offset == _box->RightmostCaretOffset();
    }

    KRYS_NODISCARD IntRect AbsoluteRect(CaretRectMode caretRectMode = CaretRectMode::Normal) const noexcept;

    KRYS_NODISCARD Maybe<dom::BoundaryPoint> BoundaryPoint() const noexcept;

  private:
    explicit RenderedPosition(const RenderObject *renderer, inline_iterator::LeafBoxIterator box,
                              uint32 offset) noexcept;

    KRYS_NODISCARD inline_iterator::LeafBoxIterator PreviousLeafOnLine() const noexcept;
    KRYS_NODISCARD inline_iterator::LeafBoxIterator NextLeafOnLine() const noexcept;
    KRYS_NODISCARD bool AtLeftBoundaryOfBidiRun(ShouldMatchBidiLevel shouldMatchBidiLevel,
                                                uint8 bidiLevelOfRun) const noexcept;
    KRYS_NODISCARD bool AtRightBoundaryOfBidiRun(ShouldMatchBidiLevel shouldMatchBidiLevel,
                                                 uint8 bidiLevelOfRun) const noexcept;
  };

  KRYS_NODISCARD bool RenderObjectContainsPosition(const RenderObject *target,
                                                   const dom::Position &position) noexcept;
}
