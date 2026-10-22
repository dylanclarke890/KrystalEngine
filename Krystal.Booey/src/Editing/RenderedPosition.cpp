#include "Krystal.Booey/Editing/RenderedPosition.hpp"
#include "Krystal.Booey/Editing/InlineRunAndOffset.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
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
