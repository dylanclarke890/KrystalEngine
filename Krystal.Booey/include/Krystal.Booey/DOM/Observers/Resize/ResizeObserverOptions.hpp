#pragma once

#include "Krystal.Booey/DOM/Observers/Resize/ResizeObserverBoxOptions.hpp"

namespace krys::boo
{
  /// @see https://drafts.csswg.org/resize-observer/#dictdef-resizeobserveroptions
  struct ResizeObserverOptions
  {
    ResizeObserverBoxOptions Box {ResizeObserverBoxOptions::ContentBox};
  };
}
