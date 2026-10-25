#pragma once

namespace krys::boo
{
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-documenttimelineoptions
  struct DocumentTimelineOptions
  {
    /// @brief The timeline's zero time, relative to the document's time origin, in milliseconds.
    // FIXME: A `DOMHighResTimeStamp`, once that's a double rather than an integer.
    double OriginTime {0};
  };
}
