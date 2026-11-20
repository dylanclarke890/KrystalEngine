#pragma once

#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class IntersectionObserver;
  class IntersectionObserverEntry;

  /// @see https://w3c.github.io/IntersectionObserver/#callbackdef-intersectionobservercallback
  using IntersectionObserverCallback =
    Func<void(const List<Ref<IntersectionObserverEntry>> &entries, IntersectionObserver &observer)>;
}
