/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class PlatformWheelEvent;

  /// @brief Keeps a touchpad's pan on one axis: while its last few steps all went mostly one way, the other
  /// way's delta is dropped, so a pan that wanders a little doesn't scroll sideways.
  class WheelEventDeltaFilter
  {
    KRYS_NON_COPY_MOVABLE(WheelEventDeltaFilter);

  private:
    List<FloatSize> _recentWheelEventDeltas;
    FloatSize _currentFilteredDelta;

  public:
    WheelEventDeltaFilter() noexcept = default;

    /// @brief Takes in a gesture's step, or starts over at its end.
    void UpdateFromEvent(const PlatformWheelEvent &event) noexcept;

    /// @brief The last step's delta, on its gesture's axis.
    KRYS_NODISCARD FloatSize FilteredDelta() const noexcept
    {
      return _currentFilteredDelta;
    }

    /// @brief Whether @p event scrolls by the filtered delta: the gesture's own steps (not its momentum).
    KRYS_NODISCARD static bool ShouldApplyFilteringForEvent(const PlatformWheelEvent &event) noexcept;

  private:
    void Reset() noexcept;
    void UpdateWithDelta(FloatSize delta) noexcept;
  };
}
