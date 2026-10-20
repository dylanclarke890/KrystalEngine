#pragma once

#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class WebAnimation;

  /// @brief An event about an animation: a playback event, or a CSS animation or transition event.
  class AnimationEventBase : public dom::Event
  {
  private:
    RefPtr<WebAnimation> _animation;
    Maybe<Seconds> _scheduledTime;

  protected:
    /// @brief A trusted event, fired by the user agent, that bubbles and isn't cancelable.
    AnimationEventBase(dom::DOMStringAtom type, WebAnimation *animation,
                       Maybe<Seconds> scheduledTime) noexcept;
    AnimationEventBase(dom::DOMStringAtom type, const dom::EventInit &eventInitDict) noexcept;

  public:
    ~AnimationEventBase() noexcept override;

    KRYS_NODISCARD virtual bool IsAnimationPlaybackEvent() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsCSSAnimationEvent() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsCSSTransitionEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD WebAnimation *Animation() const noexcept
    {
      return _animation.get();
    }
    KRYS_NODISCARD Maybe<Seconds> ScheduledTime() const noexcept
    {
      return _scheduledTime;
    }

    /// @brief Sets the target to dispatch the event at, once it's dispatched from the pending animation event
    /// queue.
    void SetTarget(RefPtr<dom::EventTarget> &&target) noexcept
    {
      _target = krys::move(target);
    }
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_EVENT_BASE(ToValueTypeName, predicate)                         \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::AnimationEventBase &value) noexcept                   \
    {                                                                                                        \
      return value.predicate;                                                                                \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
