#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Animation/Timelines/ScrollAxis.hpp"

namespace krys::web
{
  class Element;

  /// @see https://drafts.csswg.org/scroll-animations-1/#dictdef-scrolltimelineoptions
  struct ScrollTimelineOptions
  {
    Maybe<RefPtr<web::Element>> Source;
    ScrollAxis Axis {ScrollAxis::Block};
  };
}
