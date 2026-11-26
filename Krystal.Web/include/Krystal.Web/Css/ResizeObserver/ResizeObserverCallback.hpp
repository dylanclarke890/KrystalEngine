#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::web
{
  class ResizeObserver;
  class ResizeObserverEntry;

  /// @see https://drafts.csswg.org/resize-observer/#callbackdef-resizeobservercallback
  using ResizeObserverCallback =
    Func<void(const List<Ref<ResizeObserverEntry>> &entries, ResizeObserver &observer)>;
}
