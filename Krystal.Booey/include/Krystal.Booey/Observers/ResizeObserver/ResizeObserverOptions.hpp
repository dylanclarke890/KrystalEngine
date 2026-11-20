#pragma once

#include "Krystal.Booey/Observers/ResizeObserver/ResizeObserverBoxOptions.hpp"

namespace krys::boo
{
  /// @see https://drafts.csswg.org/resize-observer/#dictdef-resizeobserveroptions
  struct ResizeObserverOptions
  {
    ResizeObserverBoxOptions Box {ResizeObserverBoxOptions::ContentBox};
  };
}
