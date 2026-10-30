#pragma once

#include "Krystal.Booey/Animation/ViewTimeline.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  // <'view-timeline'> = [ <'view-timeline-name'> <'view-timeline-axis'>? ]#
  // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline
  struct ViewTimelines : css::CommaSeparatedFixedList<Ref<ViewTimeline>>
  {
    using css::CommaSeparatedFixedList<Ref<ViewTimeline>>::CommaSeparatedFixedList;
  };
}
