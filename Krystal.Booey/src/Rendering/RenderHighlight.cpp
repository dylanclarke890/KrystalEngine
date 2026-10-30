#include "Krystal.Booey/Rendering/RenderHighlight.hpp"
#include "Krystal.Booey/DOM/Highlight.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include <limits>

namespace krys::boo
{
  RenderObject *RendererAfterOffset(const RenderObject &renderer, uint32 offset) noexcept
  {
    auto *child = renderer.ChildAt(offset);
    return child ? child : renderer.NextInPreOrderAfterChildren();
  }

  RenderObject *RenderRangeIterator::Next() noexcept
  {
    _current = _current->NextInPreOrder();
    return _current;
  }

  bool RenderHighlight::SetRenderRange(const dom::HighlightRange &highlightRange) noexcept
  {
    if (highlightRange.StartPosition().IsNull() || highlightRange.EndPosition().IsNull())
    {
      return false;
    }

    auto &startPosition = highlightRange.StartPosition();
    auto &endPosition = highlightRange.EndPosition();

    if (!startPosition.GetContainerNode() || !endPosition.GetContainerNode())
    {
      return false;
    }

    auto *startRenderer = startPosition.GetContainerNode()->Renderer();
    auto *endRenderer = endPosition.GetContainerNode()->Renderer();

    if (!startRenderer || !endRenderer)
    {
      return false;
    }

    auto startOffset = static_cast<uint32>(startPosition.ComputeOffsetInContainerNode());
    auto endOffset = static_cast<uint32>(endPosition.ComputeOffsetInContainerNode());

    SetRenderRange({startRenderer, endRenderer, startOffset, endOffset});
    return true;
  }

  void RenderHighlight::SetRenderRange(const RenderRange &renderRange) noexcept
  {
    krys_debug_assert(renderRange.Start() && renderRange.End());
    _renderRange = renderRange;
  }

  RenderObject::HighlightState
    RenderHighlight::HighlightStateForRenderer(const RenderObject &renderer) const noexcept
  {
    if (_isSelection)
    {
      return renderer.SelectionState();
    }

    if (&renderer == _renderRange.Start())
    {
      if (_renderRange.Start() && _renderRange.End() && _renderRange.Start() == _renderRange.End())
      {
        return RenderObject::HighlightState::Both;
      }
      if (_renderRange.Start())
      {
        return RenderObject::HighlightState::Start;
      }
    }
    if (&renderer == _renderRange.End())
    {
      return RenderObject::HighlightState::End;
    }

    auto *rangeEnd = _renderRange.End();
    auto *highlightEnd = rangeEnd ? RendererAfterOffset(*rangeEnd, _renderRange.EndOffset()) : nullptr;

    RenderRangeIterator highlightIterator(_renderRange.Start());
    for (auto *currentRenderer = _renderRange.Start(); currentRenderer && currentRenderer != highlightEnd;
         currentRenderer = highlightIterator.Next())
    {
      if (currentRenderer == _renderRange.Start())
      {
        continue;
      }
      if (!currentRenderer->CanBeSelectionLeaf())
      {
        continue;
      }
      if (&renderer == currentRenderer)
      {
        return RenderObject::HighlightState::Inside;
      }
    }
    return RenderObject::HighlightState::None;
  }

  RenderObject::HighlightState
    RenderHighlight::HighlightStateForTextBox(const RenderText &renderer,
                                              const TextBoxSelectableRange &textBoxRange) const noexcept
  {
    auto state = HighlightStateForRenderer(renderer);

    if (state == RenderObject::HighlightState::None || state == RenderObject::HighlightState::Inside)
    {
      return state;
    }

    auto startOffset = StartOffset();
    auto endOffset = EndOffset();

    // The position after a hard line break is considered to be past its end.
    krys_debug_assert(textBoxRange.Start + textBoxRange.Length >= (textBoxRange.IsLineBreak ? 1u : 0u));
    uint32 lastSelectable = textBoxRange.Start + textBoxRange.Length - (textBoxRange.IsLineBreak ? 1 : 0);

    bool containsStart = state != RenderObject::HighlightState::End && startOffset >= textBoxRange.Start
                         && startOffset < textBoxRange.Start + textBoxRange.Length;
    bool containsEnd = state != RenderObject::HighlightState::Start && endOffset > textBoxRange.Start
                       && endOffset <= lastSelectable;
    if (containsStart && containsEnd)
    {
      return RenderObject::HighlightState::Both;
    }
    if (containsStart)
    {
      return RenderObject::HighlightState::Start;
    }
    if (containsEnd)
    {
      return RenderObject::HighlightState::End;
    }
    if ((state == RenderObject::HighlightState::End || startOffset < textBoxRange.Start)
        && (state == RenderObject::HighlightState::Start || endOffset > lastSelectable))
    {
      return RenderObject::HighlightState::Inside;
    }

    return RenderObject::HighlightState::None;
  }

  std::pair<uint32, uint32>
    RenderHighlight::RangeForTextBox(const RenderText &renderer,
                                     const TextBoxSelectableRange &textBoxRange) const noexcept
  {
    auto state = HighlightStateForTextBox(renderer, textBoxRange);

    switch (state)
    {
      case RenderObject::HighlightState::Inside:
        return textBoxRange.Clamp(0, std::numeric_limits<uint32>::max());
      case RenderObject::HighlightState::Start:
        return textBoxRange.Clamp(StartOffset(), std::numeric_limits<uint32>::max());
      case RenderObject::HighlightState::End:  return textBoxRange.Clamp(0, EndOffset());
      case RenderObject::HighlightState::Both: return textBoxRange.Clamp(StartOffset(), EndOffset());
      case RenderObject::HighlightState::None: return {0, 0};
    }

    krys_debug_assert(false);
    return {0, 0};
  }
}
