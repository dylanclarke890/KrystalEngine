#include "Krystal.Booey/Rendering/LineClampUpdater.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo
{
  LineClampUpdater::LineClampUpdater(const RenderBlockFlow &blockContainer) noexcept
      : _blockContainer(blockContainer)
  {
    auto *layoutState = _blockContainer.View().FrameView().LayoutContext().LayoutState();
    if (!layoutState)
    {
      return;
    }

    _previousLineClamp = layoutState->GetLineClamp();
    if (blockContainer.IsFieldset())
    {
      layoutState->SetLineClamp({});
      return;
    }

    if (auto maximumLinesForBlockContainer = _blockContainer.Style().MaxLines().TryValue())
    {
      // New, top level line clamp.
      layoutState->SetLineClamp(RenderLayoutState::LineClamp {
        static_cast<size_t>(maximumLinesForBlockContainer->Value),
        _blockContainer.Style().GetOverflowContinue() == OverflowContinue::Discard});
      return;
    }

    if (_previousLineClamp)
    {
      // Propagated line clamp.
      if (blockContainer.EstablishesIndependentFormattingContext())
      {
        // Contents of descendants that establish independent formatting contexts are skipped over while
        // counting line boxes.
        layoutState->SetLineClamp({});
        _skipsLineCounting = true;
        return;
      }
      auto effectiveShouldDiscard =
        _previousLineClamp->ShouldDiscardOverflow
        || _blockContainer.Style().GetOverflowContinue() == OverflowContinue::Discard;
      layoutState->SetLineClamp(
        RenderLayoutState::LineClamp {_previousLineClamp->MaximumLines, effectiveShouldDiscard});
      return;
    }
  }

  LineClampUpdater::~LineClampUpdater() noexcept
  {
    auto *layoutState = _blockContainer.View().FrameView().LayoutContext().LayoutState();
    if (!layoutState)
    {
      return;
    }

    if (!_previousLineClamp)
    {
      layoutState->SetLineClamp({});
      return;
    }

    // The lines of a block container that's skipped over don't count, and the lines past the clamp leave no
    // lines for the content after it.
    auto lineCount =
      !_skipsLineCounting && _blockContainer.ChildrenInline() ? _blockContainer.LineCount() : 0;
    layoutState->SetLineClamp(RenderLayoutState::LineClamp {
      _previousLineClamp->MaximumLines - std::min(_previousLineClamp->MaximumLines, lineCount),
      _previousLineClamp->ShouldDiscardOverflow});
  }
}
