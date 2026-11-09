/*
 * Copyright (C) Canon Inc. 2016
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class AnimationTimelinesController;
  class WebAnimation;

  namespace style
  {
    struct SingleAnimationRange;
  }

  using AnimationCollection = ListHashSet<Ref<WebAnimation>>;

  /// @brief A source of time for animations.
  /// @see https://drafts.csswg.org/web-animations-1/#timelines
  // FIXME: Accelerated timelines, once there are accelerated animations.
  class AnimationTimeline : public RefCounted<AnimationTimeline>, public CanMakeWeakPtr<AnimationTimeline>
  {
  protected:
    AnimationCollection _animations;

  private:
    Maybe<WebAnimationTime> _currentTime;
    Maybe<WebAnimationTime> _duration;

  protected:
    explicit AnimationTimeline(Maybe<WebAnimationTime> duration = null) noexcept;

  public:
    virtual ~AnimationTimeline() noexcept;

    KRYS_NODISCARD virtual bool IsDocumentTimeline() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsScrollTimeline() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsViewTimeline() const noexcept
    {
      return false;
    }

    /// @see https://drafts.csswg.org/web-animations-1/#monotonically-increasing-timeline
    KRYS_NODISCARD bool IsMonotonic() const noexcept
    {
      return !_duration;
    }
    KRYS_NODISCARD bool IsProgressBased() const noexcept
    {
      return !IsMonotonic();
    }

    KRYS_NODISCARD const AnimationCollection &RelevantAnimations() const noexcept
    {
      return _animations;
    }

    virtual void AnimationTimingDidChange(WebAnimation &animation) noexcept;
    virtual void RemoveAnimation(WebAnimation &animation) noexcept;

    KRYS_NODISCARD virtual Maybe<WebAnimationTime>
      CurrentTime(UseCachedCurrentTime = UseCachedCurrentTime::Yes) noexcept
    {
      return _currentTime;
    }
    KRYS_NODISCARD virtual Maybe<WebAnimationTime> Duration() const noexcept
    {
      return _duration;
    }

    virtual void DetachFromDocument() noexcept;

    enum class ShouldUpdateAnimationsAndSendEvents : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD virtual ShouldUpdateAnimationsAndSendEvents
      DocumentWillUpdateAnimationsAndSendEvents() noexcept
    {
      return ShouldUpdateAnimationsAndSendEvents::No;
    }

    virtual void SuspendAnimations() noexcept;
    virtual void ResumeAnimations() noexcept;
    KRYS_NODISCARD bool AnimationsAreSuspended() const noexcept;

    KRYS_NODISCARD virtual AnimationTimelinesController *Controller() const noexcept
    {
      return nullptr;
    }

    /// @brief The attachment range of animations whose range is `normal`.
    KRYS_NODISCARD virtual style::SingleAnimationRange DefaultRange() const noexcept;

    /// @brief Gives @p animation its position in the global animation list, if it has none yet.
    static void UpdateGlobalPosition(WebAnimation &animation) noexcept;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_TIMELINE(ToValueTypeName, predicate)                           \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::AnimationTimeline &value) noexcept                    \
    {                                                                                                        \
      return value.predicate;                                                                                \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
