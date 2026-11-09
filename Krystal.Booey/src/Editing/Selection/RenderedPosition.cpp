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

#include "Krystal.Booey/Editing/Selection/RenderedPosition.hpp"
#include "Krystal.Booey/Editing/Selection/InlineRunAndOffset.hpp"
#include "Krystal.Booey/Editing/Selection/VisiblePosition.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatQuad.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD inline const RenderObject *RendererFromPosition(const dom::Position &position) noexcept
    {
      krys_debug_assert(position.IsNotNull());
      dom::Node *rendererNode = nullptr;
      switch (position.GetAnchorType())
      {
        case dom::Position::AnchorType::PositionIsOffsetInAnchor:
          rendererNode = position.ComputeNodeAfterPosition();
          if (!rendererNode || !rendererNode->Renderer())
          {
            rendererNode = position.AnchorNode()->LastChild();
          }
          break;

        case dom::Position::AnchorType::PositionIsBeforeAnchor:
        case dom::Position::AnchorType::PositionIsAfterAnchor:  break;

        case dom::Position::AnchorType::PositionIsBeforeChildren:
          rendererNode = position.AnchorNode()->FirstChild();
          break;
        case dom::Position::AnchorType::PositionIsAfterChildren:
          rendererNode = position.AnchorNode()->LastChild();
          break;
      }
      if (!rendererNode || !rendererNode->Renderer())
      {
        rendererNode = position.AnchorNode();
      }
      return rendererNode->Renderer();
    }
  }

  RenderedPosition::RenderedPosition() noexcept = default;

  RenderedPosition::RenderedPosition(const RenderObject *renderer, inline_iterator::LeafBoxIterator box,
                                     uint32 offset) noexcept
      : _renderer(CreateWeakPtr(renderer)), _box(box), _offset(offset)
  {
  }

  RenderedPosition::RenderedPosition(const VisiblePosition &position) noexcept
      : RenderedPosition(position.DeepEquivalent(), position.GetAffinity())
  {
  }

  RenderedPosition::RenderedPosition(const dom::Position &position, Affinity affinity) noexcept
  {
    if (position.IsNull())
    {
      return;
    }

    auto boxAndOffset = position.GetInlineBoxAndOffset(affinity);
    _box = boxAndOffset.Box;
    _offset = boxAndOffset.Offset;
    if (_box)
    {
      _renderer = CreateWeakPtr(&_box->Renderer());
    }
    else
    {
      _renderer = CreateWeakPtr(RendererFromPosition(position));
    }
  }

  inline_iterator::LeafBoxIterator RenderedPosition::PreviousLeafOnLine() const noexcept
  {
    if (!_previousLeafOnLine)
    {
      _previousLeafOnLine = _box->NextLineLeftwardOnLineIgnoringLineBreak();
    }
    return *_previousLeafOnLine;
  }

  inline_iterator::LeafBoxIterator RenderedPosition::NextLeafOnLine() const noexcept
  {
    if (!_nextLeafOnLine)
    {
      _nextLeafOnLine = _box->NextLineRightwardOnLineIgnoringLineBreak();
    }
    return *_nextLeafOnLine;
  }

  bool RenderedPosition::IsEquivalent(const RenderedPosition &other) const noexcept
  {
    return (_renderer.get() == other._renderer.get() && _box == other._box && _offset == other._offset)
           || (AtLeftmostOffsetInBox() && other.AtRightmostOffsetInBox()
               && PreviousLeafOnLine() == other._box)
           || (AtRightmostOffsetInBox() && other.AtLeftmostOffsetInBox() && NextLeafOnLine() == other._box);
  }

  uint8 RenderedPosition::BidiLevelOnLeft() const noexcept
  {
    auto box = AtLeftmostOffsetInBox() ? PreviousLeafOnLine() : _box;
    return box ? box->BidiLevel() : 0;
  }

  uint8 RenderedPosition::BidiLevelOnRight() const noexcept
  {
    auto box = AtRightmostOffsetInBox() ? NextLeafOnLine() : _box;
    return box ? box->BidiLevel() : 0;
  }

  RenderedPosition RenderedPosition::LeftBoundaryOfBidiRun(uint8 bidiLevelOfRun) const noexcept
  {
    if (!_box || bidiLevelOfRun > _box->BidiLevel())
    {
      return RenderedPosition();
    }

    auto box = _box;
    do
    {
      auto prev = box->NextLineLeftwardOnLineIgnoringLineBreak();
      if (!prev || prev->BidiLevel() < bidiLevelOfRun)
      {
        return RenderedPosition(&box->Renderer(), box, box->LeftmostCaretOffset());
      }
      box = prev;
    } while (box);

    krys_unreachable();
  }

  RenderedPosition RenderedPosition::RightBoundaryOfBidiRun(uint8 bidiLevelOfRun) const noexcept
  {
    if (!_box || bidiLevelOfRun > _box->BidiLevel())
    {
      return RenderedPosition();
    }

    auto box = _box;
    do
    {
      auto next = box->NextLineRightwardOnLineIgnoringLineBreak();
      if (!next || next->BidiLevel() < bidiLevelOfRun)
      {
        return RenderedPosition(&box->Renderer(), box, box->RightmostCaretOffset());
      }
      box = next;
    } while (box);

    krys_unreachable();
  }

  bool RenderedPosition::AtLeftBoundaryOfBidiRun(ShouldMatchBidiLevel shouldMatchBidiLevel,
                                                 uint8 bidiLevelOfRun) const noexcept
  {
    if (!_box)
    {
      return false;
    }

    if (AtLeftmostOffsetInBox())
    {
      if (shouldMatchBidiLevel == ShouldMatchBidiLevel::IgnoreBidiLevel)
      {
        return !PreviousLeafOnLine() || PreviousLeafOnLine()->BidiLevel() < _box->BidiLevel();
      }
      return _box->BidiLevel() >= bidiLevelOfRun
             && (!PreviousLeafOnLine() || PreviousLeafOnLine()->BidiLevel() < bidiLevelOfRun);
    }

    if (AtRightmostOffsetInBox())
    {
      if (shouldMatchBidiLevel == ShouldMatchBidiLevel::IgnoreBidiLevel)
      {
        return NextLeafOnLine() && _box->BidiLevel() < NextLeafOnLine()->BidiLevel();
      }
      return NextLeafOnLine() && _box->BidiLevel() < bidiLevelOfRun
             && NextLeafOnLine()->BidiLevel() >= bidiLevelOfRun;
    }

    return false;
  }

  bool RenderedPosition::AtRightBoundaryOfBidiRun(ShouldMatchBidiLevel shouldMatchBidiLevel,
                                                  uint8 bidiLevelOfRun) const noexcept
  {
    if (!_box)
    {
      return false;
    }

    if (AtRightmostOffsetInBox())
    {
      if (shouldMatchBidiLevel == ShouldMatchBidiLevel::IgnoreBidiLevel)
      {
        return !NextLeafOnLine() || NextLeafOnLine()->BidiLevel() < _box->BidiLevel();
      }
      return _box->BidiLevel() >= bidiLevelOfRun
             && (!NextLeafOnLine() || NextLeafOnLine()->BidiLevel() < bidiLevelOfRun);
    }

    if (AtLeftmostOffsetInBox())
    {
      if (shouldMatchBidiLevel == ShouldMatchBidiLevel::IgnoreBidiLevel)
      {
        return PreviousLeafOnLine() && _box->BidiLevel() < PreviousLeafOnLine()->BidiLevel();
      }
      return PreviousLeafOnLine() && _box->BidiLevel() < bidiLevelOfRun
             && PreviousLeafOnLine()->BidiLevel() >= bidiLevelOfRun;
    }

    return false;
  }

  dom::Position RenderedPosition::PositionAtLeftBoundaryOfBiDiRun() const noexcept
  {
    krys_debug_assert(AtLeftBoundaryOfBidiRun());

    if (AtLeftmostOffsetInBox())
    {
      return dom::MakeDeprecatedLegacyPosition(ShareRefPtr(_renderer->GetNode()), _offset);
    }

    return dom::MakeDeprecatedLegacyPosition(ShareRefPtr(NextLeafOnLine()->Renderer().GetNode()),
                                             NextLeafOnLine()->LeftmostCaretOffset());
  }

  dom::Position RenderedPosition::PositionAtRightBoundaryOfBiDiRun() const noexcept
  {
    krys_debug_assert(AtRightBoundaryOfBidiRun());

    if (AtRightmostOffsetInBox())
    {
      return dom::MakeDeprecatedLegacyPosition(ShareRefPtr(_renderer->GetNode()), _offset);
    }

    return dom::MakeDeprecatedLegacyPosition(ShareRefPtr(PreviousLeafOnLine()->Renderer().GetNode()),
                                             PreviousLeafOnLine()->RightmostCaretOffset());
  }

  IntRect RenderedPosition::AbsoluteRect(CaretRectMode caretRectMode) const noexcept
  {
    if (IsNull())
    {
      return IntRect();
    }

    IntRect localRect = SnappedIntRect(ComputeLocalCaretRect(*_renderer, {_box, _offset}, caretRectMode));
    return localRect == IntRect()
             ? IntRect()
             : _renderer->LocalToAbsoluteQuad(FloatQuad(FloatRect(localRect))).EnclosingBoundingBox();
  }

  Maybe<dom::BoundaryPoint> RenderedPosition::BoundaryPoint() const noexcept
  {
    if (!_box)
    {
      return null;
    }

    auto *node = _box->Renderer().GetNode();
    if (!node)
    {
      return null;
    }

    return dom::BoundaryPoint {ShareRef(*node), Offset()};
  }

  bool RenderObjectContainsPosition(const RenderObject *target, const dom::Position &position) noexcept
  {
    for (auto *renderer = RendererFromPosition(position); renderer && renderer->GetNode();
         renderer = renderer->Parent())
    {
      if (renderer == target)
      {
        return true;
      }
    }
    return false;
  }
}
