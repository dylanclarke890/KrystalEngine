#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/AnimationTimelinesController.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"

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
}
