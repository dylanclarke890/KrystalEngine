#include "Krystal.HTML/Events/EventDispatcher.hpp"
#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"

namespace Krys::HTML
{
  ExceptionOr<bool> EventDispatcher::DispatchTo(EventTarget &target, Event &event)
  {
    // TODO(IMPL): https://dom.spec.whatwg.org/#concept-event-dispatch
    return false;
  }
}