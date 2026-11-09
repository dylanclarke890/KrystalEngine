/*
 * Copyright (C) 2008-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Animation/Timing/WebAnimationTime.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/SVG/Animation/SMILTime.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  class SVGElement;
  class SVGSMILElement;
  class SVGSvgElement;

  /// @brief The timeline of an `<svg>` element's animations: it schedules them by target and attribute,
  /// and samples them in the rendering update of each frame while any is changing, sleeping on a timer until
  /// a frame before the next change otherwise.
  class SMILTimeContainer final : public RefCounted<SMILTimeContainer>,
                                  public CanMakeWeakPtr<SMILTimeContainer>
  {
    using ElementAttributePair = Pair<SVGElement *, dom::QualifiedName>;
    using AnimationsVector = List<WeakPtr<SVGSMILElement>>;
    using GroupedAnimationsMap = List<Pair<ElementAttributePair, AnimationsVector>>;

  private:
    MonotonicTime::duration _beginTime {};
    MonotonicTime::duration _pauseTime {};
    MonotonicTime::duration _accumulatedActiveTime {};
    MonotonicTime::duration _resumeTime {};
    MonotonicTime::duration _presetStartTime {};

    bool _documentOrderIndexesDirty {false};
    bool _animationFrameScheduled {false};
    Timer _timer;
    GroupedAnimationsMap _scheduledAnimations;
    WeakPtr<SVGSvgElement> _ownerSVGElement;

  public:
    KRYS_NODISCARD static Ref<SMILTimeContainer> Create(SVGSvgElement &owner) noexcept
    {
      return AdoptRef(*new SMILTimeContainer(owner));
    }

    void Schedule(SVGSMILElement *, SVGElement *, const dom::QualifiedName &) noexcept;
    void Unschedule(SVGSMILElement *, SVGElement *, const dom::QualifiedName &) noexcept;
    void NotifyIntervalsChanged() noexcept;

    /// @brief The time between frames: the document timeline's animation interval.
    KRYS_NODISCARD Seconds AnimationFrameDelay() const noexcept;
    /// @brief Samples the animations, if this rendering update was asked for.
    void ServiceAnimations() noexcept;

    KRYS_NODISCARD SMILTime Elapsed() const noexcept;

    KRYS_NODISCARD bool IsActive() const noexcept;
    KRYS_NODISCARD bool IsPaused() const noexcept;

    void Begin() noexcept;
    void Pause() noexcept;
    void Resume() noexcept;
    void SetElapsed(SMILTime) noexcept;

    void SetDocumentOrderIndexesDirty() noexcept
    {
      _documentOrderIndexesDirty = true;
    }

  private:
    explicit SMILTimeContainer(SVGSvgElement &owner) noexcept;

    KRYS_NODISCARD bool IsStarted() const noexcept;
    void TimerFired() noexcept;
    /// @brief Samples the animations at @p fireTime: in the next rendering update if that's within a frame,
    /// or else a frame before it.
    void ScheduleUpdate(SMILTime elapsed, SMILTime fireTime) noexcept;
    void ScheduleAnimationFrame() noexcept;
    void UpdateAnimations(SMILTime elapsed, bool seekToTime = false) noexcept;

    template <typename TFunction>
    void ProcessScheduledAnimations(const TFunction &callback) noexcept;
    void UpdateDocumentOrderIndexes() noexcept;
    void SortByPriority(AnimationsVector &smilElements, SMILTime elapsed) noexcept;

    KRYS_NODISCARD MonotonicTime::duration LastResumeTime() const noexcept
    {
      return _resumeTime != MonotonicTime::duration {} ? _resumeTime : _beginTime;
    }
  };
}
