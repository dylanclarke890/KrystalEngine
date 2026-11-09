/*
 * Copyright (c) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Page/Scrolling/ScrollSnapOffsetsInfo.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class ScrollableArea;

  /// @brief A scrollable area's smooth scroll, and its snap positions.
  // WebKit's ScrollAnimator, ScrollingEffectsController, ScrollAnimationSmooth and ScrollSnapAnimatorState,
  // for the main thread and without gestures, rubber-banding or momentum.
  class ScrollAnimator
  {
    KRYS_NON_COPY_MOVABLE(ScrollAnimator);

  private:
    ScrollableArea &_scrollableArea;
    bool _isAnimating {false};
    ScrollType _scrollType {ScrollType::Programmatic};
    FloatPoint _startOffset;
    FloatPoint _currentOffset;
    FloatPoint _destinationOffset;
    /// @brief When the animation's first frame was: its time starts then, so it moves from the frame after.
    Maybe<MonotonicTime::duration> _startTime;
    double _durationInSeconds {0};
    RefPtr<TimingFunction> _timingFunction;

    Maybe<ScrollSnapOffsetsInfo> _snapOffsetsInfo;
    /// @brief What the area is snapped to on each axis, followed when layout moves the snap positions.
    List<WeakPtr<dom::Element>> _horizontalSnapTargets;
    List<WeakPtr<dom::Element>> _verticalSnapTargets;

  public:
    explicit ScrollAnimator(ScrollableArea &scrollableArea) noexcept;

    /// @brief Scrolls to @p destinationOffset, from where a running animation has got to.
    /// @return Whether it's animating: false if it's already there.
    bool ScrollToOffsetWithAnimation(const FloatPoint &destinationOffset, ScrollType scrollType) noexcept;
    void StopAnimation() noexcept
    {
      _isAnimating = false;
    }
    KRYS_NODISCARD bool IsAnimating() const noexcept
    {
      return _isAnimating;
    }
    KRYS_NODISCARD const FloatPoint &DestinationOffset() const noexcept
    {
      return _destinationOffset;
    }

    /// @brief Moves the area to where the animation is at @p now.
    /// @return Whether the animation goes on.
    bool ServiceScrollAnimation(MonotonicTime::duration now) noexcept;

    KRYS_NODISCARD bool UsesScrollSnap() const noexcept
    {
      return _snapOffsetsInfo.has_value();
    }
    KRYS_NODISCARD const ScrollSnapOffsetsInfo *SnapOffsetsInfo() const noexcept
    {
      return _snapOffsetsInfo ? &*_snapOffsetsInfo : nullptr;
    }
    void SetSnapOffsetsInfo(ScrollSnapOffsetsInfo info) noexcept
    {
      _snapOffsetsInfo = krys::move(info);
    }
    void ClearSnapOffsetsInfo() noexcept;

    /// @brief @p offset, moved to where a scroll to it snaps.
    KRYS_NODISCARD FloatPoint ScrollOffsetAdjustedForSnapping(
      const FloatPoint &offset, ScrollSnapPointSelectionMethod method) const noexcept;
    /// @brief The offset on @p axis of the snap position for a scroll to @p destination.
    /// @param velocity The scroll's direction on @p axis: 0 for the closest position.
    /// @param originalOffset Where a directional scroll started on @p axis.
    KRYS_NODISCARD float AdjustedScrollDestination(ScrollEventAxis axis, const FloatPoint &destination,
                                                   float velocity,
                                                   Maybe<float> originalOffset = null) const noexcept;

    /// @brief Notes what the area's scroll position is now snapped to.
    void UpdateSnapTargets() noexcept;
    /// @brief Moves the area to where what it was snapped to has gone, or the closest snap position.
    /// @see https://drafts.csswg.org/css-scroll-snap-1/#re-snap
    void ResnapAfterLayout() noexcept;
  };
}
