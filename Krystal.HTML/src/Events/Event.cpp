#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.HTML/EventTarget.hpp"
#include "Krystal.Lib/Time/MonotonicTime.hpp"

namespace Krys::HTML
{
  Event::Event(StringAtom type, const EventInit &eventInitDict) noexcept
      : _type(type), _timeStamp(MonotonicTime::Now())
  {
    _bubbles = eventInitDict.Bubbles;
    _cancellable = eventInitDict.Cancellable;
    _composed = eventInitDict.Composed;
  }

  List<EventTarget> Event::ComposedPath() const noexcept
  {
    // TODO(IMPL)
    return {};
  }
}