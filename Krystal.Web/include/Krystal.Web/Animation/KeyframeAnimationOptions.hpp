#pragma once

#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Animation/KeyframeEffectOptions.hpp"
#include "Krystal.Web/Animation/Timelines/AnimationTimeline.hpp"
#include "Krystal.Web/Animation/Timelines/TimelineRangeValue.hpp"

namespace krys::web
{
  /// @brief The options of `Element.animate()`: the effect's, and the animation's identifier, timeline and
  /// attachment range.
  /// @see https://drafts.csswg.org/web-animations-2/#dictdef-keyframeanimationoptions
  struct KeyframeAnimationOptions : KeyframeEffectOptions
  {
    DOMString Id;
    /// @brief The timeline, if given: null for none; when it isn't given, the document's default timeline.
    Maybe<RefPtr<AnimationTimeline>> Timeline;
    TimelineRangeValue RangeStart {DOMString(u8"normal")};
    TimelineRangeValue RangeEnd {DOMString(u8"normal")};
  };
}
