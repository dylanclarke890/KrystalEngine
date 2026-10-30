#include "Krystal.Booey/Animation/AnimationPlaybackEvent.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD Maybe<Seconds> ToSeconds(const Maybe<WebAnimationTime> &time) noexcept
    {
      if (time)
      {
        return time->Time();
      }
      return null;
    }
  }

  AnimationPlaybackEvent::AnimationPlaybackEvent(dom::DOMStringAtom type,
                                                 const AnimationPlaybackEventInit &eventInitDict) noexcept
      : AnimationEventBase(type, eventInitDict), _timelineTime(eventInitDict.TimelineTime),
        _currentTime(eventInitDict.CurrentTime)
  {
  }

  AnimationPlaybackEvent::AnimationPlaybackEvent(dom::DOMStringAtom type, WebAnimation *animation,
                                                 Maybe<WebAnimationTime> scheduledTime,
                                                 Maybe<WebAnimationTime> timelineTime,
                                                 Maybe<WebAnimationTime> currentTime) noexcept
      : AnimationEventBase(type, animation, ToSeconds(scheduledTime)), _timelineTime(timelineTime),
        _currentTime(currentTime)
  {
  }

  AnimationPlaybackEvent::~AnimationPlaybackEvent() noexcept = default;
}
