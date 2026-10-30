#include "Krystal.Booey/Rendering/Line/TrailingObjects.hpp"

#include <limits>

namespace krys::boo
{
  void TrailingObjects::UpdateWhitespaceCollapsingTransitionsForTrailingBoxes(
    LineWhitespaceCollapsingState &lineWhitespaceCollapsingState, const LegacyInlineIterator &lBreak,
    CollapseFirstSpace collapseFirstSpace) noexcept
  {
    if (!_whitespace)
    {
      return;
    }

    // This object is either going to be part of the last transition, or it is going to be the actual
    // endpoint. In both cases we just decrease our pos by 1 level to exclude the space, allowing it to - in
    // effect - collapse into the newline.
    if (lineWhitespaceCollapsingState.NumTransitions() % 2)
    {
      // Find the trailing space object's transition.
      auto trailingSpaceTransition = static_cast<int64>(lineWhitespaceCollapsingState.NumTransitions()) - 1;
      for (; trailingSpaceTransition > 0
             && lineWhitespaceCollapsingState.Transitions()[trailingSpaceTransition].Renderer()
                  != _whitespace.get();
           --trailingSpaceTransition)
      {
      }
      krys_debug_assert(trailingSpaceTransition >= 0);
      if (collapseFirstSpace == CollapseFirstSpace::Yes)
      {
        lineWhitespaceCollapsingState.DecrementTransitionAt(static_cast<size_t>(trailingSpaceTransition));
      }

      // Now make sure every single trailingPositionedBox following the trailingSpaceTransition properly stops
      // and starts ignoring spaces.
      auto currentTransition = static_cast<size_t>(trailingSpaceTransition) + 1;
      for (size_t i = 0; i < _boxes.size(); ++i)
      {
        if (currentTransition >= lineWhitespaceCollapsingState.NumTransitions())
        {
          // We don't have a transition for this box yet.
          lineWhitespaceCollapsingState.EnsureLineBoxInsideIgnoredSpaces(_boxes[i]);
        }
        else
        {
          krys_debug_assert(lineWhitespaceCollapsingState.Transitions()[currentTransition].Renderer()
                            == &(_boxes[i].get()));
          krys_debug_assert(lineWhitespaceCollapsingState.Transitions()[currentTransition + 1].Renderer()
                            == &(_boxes[i].get()));
        }
        currentTransition += 2;
      }
    }
    else if (!lBreak.Renderer())
    {
      krys_debug_assert(_whitespace->IsRenderText());
      krys_debug_assert(collapseFirstSpace == CollapseFirstSpace::Yes);
      // Add a new end transition that stops right at the very end.
      auto length = static_cast<uint32>(_whitespace->Text().length());
      uint32 pos = length >= 2 ? length - 2 : std::numeric_limits<uint32>::max();
      LegacyInlineIterator endMid(nullptr, _whitespace.get(), pos);
      lineWhitespaceCollapsingState.StartIgnoringSpaces(endMid);
      for (size_t i = 0; i < _boxes.size(); ++i)
      {
        lineWhitespaceCollapsingState.EnsureLineBoxInsideIgnoredSpaces(_boxes[i]);
      }
    }
  }
}
