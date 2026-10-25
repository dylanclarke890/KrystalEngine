#pragma once

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  namespace dom
  {
    class Element;
  }

  /// @see https://drafts.csswg.org/scroll-animations-1/#dictdef-scrolltimelineoptions
  struct ScrollTimelineOptions
  {
    Maybe<RefPtr<dom::Element>> Source;
    ScrollAxis Axis {ScrollAxis::Block};
  };
}
