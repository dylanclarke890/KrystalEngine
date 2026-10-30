#pragma once

#include "Krystal.Booey/Animation/ScrollTimeline.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  // <'scroll-timeline'> = [ <'scroll-timeline-name'> <'scroll-timeline-axis'>? ]#
  // https://drafts.csswg.org/scroll-animations-1/#propdef-scroll-timeline
  struct ScrollTimelines : css::CommaSeparatedFixedList<Ref<ScrollTimeline>>
  {
    using css::CommaSeparatedFixedList<Ref<ScrollTimeline>>::CommaSeparatedFixedList;
  };
}
