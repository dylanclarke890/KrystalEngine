#pragma once

#include "Krystal.Web/Css/ResizeObserver/ResizeObserverBoxOptions.hpp"

namespace krys::web
{
  /// @see https://drafts.csswg.org/resize-observer/#dictdef-resizeobserveroptions
  struct ResizeObserverOptions
  {
    ResizeObserverBoxOptions Box {ResizeObserverBoxOptions::ContentBox};
  };
}
