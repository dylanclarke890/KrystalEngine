#include "Krystal.Booey/Animation/AnimationEventBase.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"

namespace krys::boo
{
  AnimationEventBase::AnimationEventBase(dom::DOMStringAtom type, WebAnimation *animation,
                                         Maybe<Seconds> scheduledTime) noexcept
      : Event(type, dom::EventInit {.Bubbles = true, .Cancelable = false}),
        _animation(ShareRefPtr(animation)), _scheduledTime(scheduledTime)
  {
    _isTrusted = true;
    _initialized = true;
  }

  AnimationEventBase::AnimationEventBase(dom::DOMStringAtom type,
                                         const dom::EventInit &eventInitDict) noexcept
      : Event(type, eventInitDict)
  {
  }

  AnimationEventBase::~AnimationEventBase() noexcept = default;
}
