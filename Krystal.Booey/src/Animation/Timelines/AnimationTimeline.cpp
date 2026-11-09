/*
 * Copyright (C) Canon Inc. 2016
 * Copyright (C) 2017-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Animation/Timelines/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/Timelines/AnimationTimelinesController.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRange.hpp"

namespace krys::boo
{
  AnimationTimeline::AnimationTimeline(Maybe<WebAnimationTime> duration) noexcept : _duration(duration)
  {
  }

  AnimationTimeline::~AnimationTimeline() noexcept = default;

  void AnimationTimeline::AnimationTimingDidChange(WebAnimation &animation) noexcept
  {
    UpdateGlobalPosition(animation);

    if (animation.Pending())
    {
      if (auto *controller = Controller())
      {
        controller->AddPendingAnimation(animation);
      }
    }

    if (_animations.add(ShareRef(animation)).second)
    {
      RefPtr timeline = ShareRefPtr(animation.Timeline());
      if (timeline && timeline.get() != this)
      {
        timeline->RemoveAnimation(animation);
      }
      else if (timeline.get() == this)
      {
        if (RefPtr keyframeEffect = ShareRefPtr(animation.GetKeyframeEffect()))
        {
          if (auto styleable = keyframeEffect->TargetStyleable())
          {
            styleable->AnimationWasAdded(animation);
          }
        }
      }
    }
  }

  void AnimationTimeline::UpdateGlobalPosition(WebAnimation &animation) noexcept
  {
    static uint64 globalPosition = 0;
    if (!animation.GlobalPosition() && animation.CanHaveGlobalPosition())
    {
      animation.SetGlobalPosition(++globalPosition);
    }
  }

  void AnimationTimeline::RemoveAnimation(WebAnimation &animation) noexcept
  {
    krys_debug_assert(!animation.Timeline() || animation.Timeline() == this);
    Ref protectedAnimation = ShareRef(animation);
    _animations.remove(protectedAnimation);
    if (RefPtr keyframeEffect = ShareRefPtr(animation.GetKeyframeEffect()))
    {
      if (auto styleable = keyframeEffect->TargetStyleable())
      {
        styleable->AnimationWasRemoved(animation);
        if (auto *effectStack = styleable->GetKeyframeEffectStack())
        {
          effectStack->RemoveEffect(*keyframeEffect);
        }
      }
    }
  }

  void AnimationTimeline::DetachFromDocument() noexcept
  {
    if (auto *controller = Controller())
    {
      controller->RemoveTimeline(*this);
    }

    while (!_animations.isEmpty())
    {
      Ref animation = _animations.first();
      animation->Remove();
    }
  }

  void AnimationTimeline::SuspendAnimations() noexcept
  {
    for (const auto &animation : _animations)
    {
      animation->SetSuspended(true);
    }
  }

  void AnimationTimeline::ResumeAnimations() noexcept
  {
    for (const auto &animation : _animations)
    {
      animation->SetSuspended(false);
    }
  }

  bool AnimationTimeline::AnimationsAreSuspended() const noexcept
  {
    auto *controller = Controller();
    return controller && controller->AnimationsAreSuspended();
  }

  style::SingleAnimationRange AnimationTimeline::DefaultRange() const noexcept
  {
    return {
      .Start = {css::keywords::Normal {}},
      .End = {css::keywords::Normal {}},
    };
  }
}
