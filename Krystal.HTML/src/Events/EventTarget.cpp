#include "Krystal.HTML/Events/EventTarget.hpp"

namespace Krys::HTML
{
  EventTarget::EventTarget() noexcept = default;

  bool EventTarget::AddEventListener(StringAtom type, RefPtr<EventListener> &&callback,
                                     const AddEventListenerOptions &options) noexcept
  {
    return false;
  }

  bool EventTarget::RemoveEventListener(StringAtom type, RefPtr<EventListener> &&callback,
                                        const EventListenerOptions &options) noexcept
  {
    return false;
  }

  bool EventTarget::DispatchEvent(Event &event) noexcept
  {
    return false;
  }
}