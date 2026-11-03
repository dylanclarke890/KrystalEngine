#pragma once

#include "Krystal.Booey/Animation/AnimationEventBase.hpp"

namespace krys::boo
{
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-animationplaybackeventinit
  struct AnimationPlaybackEventInit : public dom::EventInit
  {
    Maybe<WebAnimationTime> CurrentTime;
    Maybe<WebAnimationTime> TimelineTime;
  };

  /// @brief Fired at an animation when its play state changes: `finish`, `cancel` and `remove`.
  /// @see https://drafts.csswg.org/web-animations-1/#animationplaybackevent
  class AnimationPlaybackEvent final : public AnimationEventBase
  {
  private:
    Maybe<WebAnimationTime> _timelineTime;
    Maybe<WebAnimationTime> _currentTime;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = AnimationPlaybackEventInit;

    AnimationPlaybackEvent(dom::DOMStringAtom type, WebAnimation *animation,
                           Maybe<WebAnimationTime> scheduledTime, Maybe<WebAnimationTime> timelineTime,
                           Maybe<WebAnimationTime> currentTime) noexcept;

    /// @see https://drafts.csswg.org/web-animations-1/#dom-animationplaybackevent-animationplaybackevent
    explicit AnimationPlaybackEvent(dom::DOMStringAtom type,
                                    const AnimationPlaybackEventInit &eventInitDict = {}) noexcept;

    ~AnimationPlaybackEvent() noexcept override;

    KRYS_NODISCARD bool IsAnimationPlaybackEvent() const noexcept final
    {
      return true;
    }

    /// @see https://drafts.csswg.org/web-animations-1/#dom-animationplaybackevent-timelinetime
    KRYS_NODISCARD Maybe<WebAnimationTime> TimelineTime() const noexcept
    {
      return _timelineTime;
    }

    /// @see https://drafts.csswg.org/web-animations-1/#dom-animationplaybackevent-currenttime
    KRYS_NODISCARD Maybe<WebAnimationTime> CurrentTime() const noexcept
    {
      return _currentTime;
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_EVENT_BASE(krys::boo::AnimationPlaybackEvent,
                                                 IsAnimationPlaybackEvent())
