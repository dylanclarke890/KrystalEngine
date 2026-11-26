#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::web
{
  class IntersectionObserver;
  class IntersectionObserverEntry;

  /// @see https://w3c.github.io/IntersectionObserver/#callbackdef-intersectionobservercallback
  using IntersectionObserverCallback =
    Func<void(const List<Ref<IntersectionObserverEntry>> &entries, IntersectionObserver &observer)>;
}
