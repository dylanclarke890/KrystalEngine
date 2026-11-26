#pragma once

#include "Krystal.Web/Dom/Types/DOMHighResTimeStamp.hpp"

namespace krys::web
{
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-documenttimelineoptions
  struct DocumentTimelineOptions
  {
    /// @brief The timeline's zero time, relative to the document's time origin, in milliseconds.
    dom::DOMHighResTimeStamp OriginTime {0};
  };
}
