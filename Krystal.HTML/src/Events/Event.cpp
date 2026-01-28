#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.HTML/EventTarget.hpp"

namespace Krys::HTML
{
  Event::Event(StringAtom type, const EventInit &eventInitDict) noexcept : _type(type)
  {
    _bubbles = eventInitDict.Bubbles;
    _cancellable = eventInitDict.Cancellable;
    _composed = eventInitDict.Composed;
    _isInitialized = true;
  }

  List<EventTarget> Event::ComposedPath() const noexcept
  {
    // TODO(IMPL)
    return {};
  }
}