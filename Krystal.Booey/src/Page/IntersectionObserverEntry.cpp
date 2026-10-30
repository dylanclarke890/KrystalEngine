#include "Krystal.Booey/Page/IntersectionObserverEntry.hpp"

namespace krys::boo
{
  IntersectionObserverEntry::IntersectionObserverEntry(const Init &init) noexcept
      : _time(init.Time), _boundingClientRect(dom::DOMRectReadOnly::FromRect(init.BoundingClientRect)),
        _intersectionRect(dom::DOMRectReadOnly::FromRect(init.IntersectionRect)),
        _intersectionRatio(init.IntersectionRatio), _target(init.Target), _isIntersecting(init.IsIntersecting)
  {
    if (init.RootBounds)
    {
      _rootBounds = dom::DOMRectReadOnly::FromRect(*init.RootBounds);
    }
  }
}
