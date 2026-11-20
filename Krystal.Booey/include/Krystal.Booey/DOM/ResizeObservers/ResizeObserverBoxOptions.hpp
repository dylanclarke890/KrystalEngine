#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  /// @brief The box a resize observer observes.
  /// @see https://drafts.csswg.org/resize-observer/#enumdef-resizeobserverboxoptions
  enum class ResizeObserverBoxOptions : uint8
  {
    BorderBox,
    ContentBox,
    DevicePixelContentBox,
  };
}
