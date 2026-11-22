#pragma once

#include "Krystal.Booey/Animation/Timelines/ScrollAxis.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class Element;

  /// @see https://drafts.csswg.org/scroll-animations-1/#dictdef-scrolltimelineoptions
  struct ScrollTimelineOptions
  {
    Maybe<RefPtr<boo::Element>> Source;
    ScrollAxis Axis {ScrollAxis::Block};
  };
}
