/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Animation/Timing/AnimationEffectPhase.hpp"
#include "Krystal.Booey/Animation/Timing/FillMode.hpp"
#include "Krystal.Booey/Animation/Timing/PlaybackDirection.hpp"
#include "Krystal.Booey/Animation/Timing/WebAnimationTime.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"

namespace krys::boo
{
  /// @brief An animation effect's local time, active time, end time, active duration and phase.
  struct BasicEffectTiming
  {
    Maybe<WebAnimationTime> LocalTime;
    Maybe<WebAnimationTime> ActiveTime;
    WebAnimationTime EndTime;
    WebAnimationTime ActiveDuration;
    AnimationEffectPhase Phase;
  };

  /// @brief An animation effect's progress at a time.
  struct ResolvedEffectTiming
  {
    Maybe<double> CurrentIteration;
    AnimationEffectPhase Phase {AnimationEffectPhase::Idle};
    Maybe<double> TransformedProgress;
    Maybe<double> SimpleIterationProgress;
    TimingFunction::Before Before;
  };

  /// @brief An animation effect's timing properties, specified and computed, and the timing model's
  /// calculations.
  /// @see https://drafts.csswg.org/web-animations-2/#timing-model
  struct AnimationEffectTiming
  {
    RefPtr<boo::TimingFunction> TimingFunction {LinearTimingFunction::Create()};
    FillMode Fill {FillMode::Auto};
    PlaybackDirection Direction {PlaybackDirection::Normal};
    double IterationStart {0};
    double Iterations {1};
    WebAnimationTime StartDelay {Seconds {0}};
    WebAnimationTime EndDelay {Seconds {0}};
    WebAnimationTime IterationDuration {Seconds {0}};
    WebAnimationTime IntrinsicIterationDuration {Seconds {0}};
    WebAnimationTime ActiveDuration {Seconds {0}};
    WebAnimationTime EndTime {Seconds {0}};

    Seconds SpecifiedStartDelay {0};
    Seconds SpecifiedEndDelay {0};
    Maybe<Seconds> SpecifiedIterationDuration;

    struct ResolutionData
    {
      Maybe<WebAnimationTime> TimelineTime;
      Maybe<WebAnimationTime> TimelineDuration;
      Maybe<WebAnimationTime> StartTime;
      Maybe<WebAnimationTime> LocalTime;
      boo::EndpointInclusiveActiveInterval EndpointInclusiveActiveInterval {
        boo::EndpointInclusiveActiveInterval::No};
      double PlaybackRate {0};
    };

    /// @brief Computes the delays, iteration duration, active duration and end time from the specified ones.
    void UpdateComputedProperties(Maybe<WebAnimationTime> timelineDuration, double playbackRate) noexcept;

    KRYS_NODISCARD BasicEffectTiming GetBasicTiming(const ResolutionData &data) const noexcept;

    KRYS_NODISCARD ResolvedEffectTiming Resolve(const ResolutionData &data) const noexcept;
  };
}
