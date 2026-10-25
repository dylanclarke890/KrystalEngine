#pragma once

#include "Krystal.Booey/Animation/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectOptions.hpp"
#include "Krystal.Booey/Animation/TimelineRangeValue.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief The options of `Element.animate()`: the effect's, and the animation's identifier, timeline and
  /// attachment range.
  /// @see https://drafts.csswg.org/web-animations-2/#dictdef-keyframeanimationoptions
  struct KeyframeAnimationOptions : KeyframeEffectOptions
  {
    dom::DOMString Id;
    /// @brief The timeline, if given: null for none; when it isn't given, the document's default timeline.
    Maybe<RefPtr<AnimationTimeline>> Timeline;
    TimelineRangeValue RangeStart {dom::DOMString(u8"normal")};
    TimelineRangeValue RangeEnd {dom::DOMString(u8"normal")};
  };
}
