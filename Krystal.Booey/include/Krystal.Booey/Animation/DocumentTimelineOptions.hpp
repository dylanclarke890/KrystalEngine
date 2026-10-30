#pragma once

#include "Krystal.Booey/DOM/Types/DOMHighResTimeStamp.hpp"

namespace krys::boo
{
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-documenttimelineoptions
  struct DocumentTimelineOptions
  {
    /// @brief The timeline's zero time, relative to the document's time origin, in milliseconds.
    dom::DOMHighResTimeStamp OriginTime {0};
  };
}
