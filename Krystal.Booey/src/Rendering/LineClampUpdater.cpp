/**
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
