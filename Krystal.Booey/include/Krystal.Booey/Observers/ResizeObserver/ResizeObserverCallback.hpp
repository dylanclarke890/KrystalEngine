#pragma once

#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class ResizeObserver;
  class ResizeObserverEntry;

  /// @see https://drafts.csswg.org/resize-observer/#callbackdef-resizeobservercallback
  using ResizeObserverCallback =
    Func<void(const List<Ref<ResizeObserverEntry>> &entries, ResizeObserver &observer)>;
}
