#pragma once

#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectOptions.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief The options of `Element.animate()`: the effect's, and the animation's identifier and timeline.
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-keyframeanimationoptions
  // FIXME: `rangeStart` and `rangeEnd`, once there are scroll timelines.
  struct KeyframeAnimationOptions : KeyframeEffectOptions
  {
    dom::DOMString Id;
    /// @brief The timeline, if given: null for none; when it isn't given, the document's default timeline.
    Maybe<RefPtr<AnimationTimeline>> Timeline;
  };
}
