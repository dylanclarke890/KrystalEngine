/*
 * Copyright (C) 2015-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Page/WheelEventDeltaFilter.hpp"
#include "Krystal.Booey/Platform/Events/PlatformWheelEvent.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    constexpr size_t BasicWheelEventDeltaFilterWindowSize = 3;

    KRYS_NODISCARD bool DeltaIsPredominantlyVertical(const FloatSize &delta) noexcept
    {
      return std::abs(delta.Height()) > std::abs(delta.Width());
    }
  }

  bool WheelEventDeltaFilter::ShouldApplyFilteringForEvent(const PlatformWheelEvent &event) noexcept
  {
    // Maybe it's a per-platform decision about which event phases get filtered. Ideally we'd filter momentum
    // events too (but that breaks some diagonal scrolling cases).
    auto phase = event.Phase();
    return phase == PlatformWheelEventPhase::Began || phase == PlatformWheelEventPhase::Changed;
  }

  void WheelEventDeltaFilter::UpdateFromEvent(const PlatformWheelEvent &event) noexcept
  {
    switch (event.Phase())
    {
      case PlatformWheelEventPhase::Cancelled:
      case PlatformWheelEventPhase::Ended:     Reset(); break;

      case PlatformWheelEventPhase::None:      break;

      case PlatformWheelEventPhase::Began:
      case PlatformWheelEventPhase::Changed:
        // A step that moves nothing (as a gesture's start doesn't) goes no way, so leaves the axis be.
        if (event.Delta().IsZero())
        {
          _currentFilteredDelta = {};
          break;
        }
        UpdateWithDelta(event.Delta());
        break;
    }
  }

  void WheelEventDeltaFilter::UpdateWithDelta(FloatSize delta) noexcept
  {
    _currentFilteredDelta = delta;

    _recentWheelEventDeltas.push_back(delta);
    if (_recentWheelEventDeltas.size() > BasicWheelEventDeltaFilterWindowSize)
    {
      _recentWheelEventDeltas.erase(_recentWheelEventDeltas.begin());
    }

    // The dominant axis: the one every recent delta went mostly along.
    bool allVertical = !_recentWheelEventDeltas.empty();
    bool allHorizontal = !_recentWheelEventDeltas.empty();
    for (const auto &recentDelta : _recentWheelEventDeltas)
    {
      bool isVertical = DeltaIsPredominantlyVertical(recentDelta);
      allVertical &= isVertical;
      allHorizontal &= !isVertical;
    }

    if (allVertical)
    {
      _currentFilteredDelta.SetWidth(0);
    }
    else if (allHorizontal)
    {
      _currentFilteredDelta.SetHeight(0);
    }
  }

  void WheelEventDeltaFilter::Reset() noexcept
  {
    _recentWheelEventDeltas.clear();
    _currentFilteredDelta = {};
  }
}
