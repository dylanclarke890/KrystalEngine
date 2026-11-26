#include "Krystal.Booey/Dom/Events/EventListener.hpp"
#include "Krystal.Booey/Dom/Abort/AbortSignal.hpp"

namespace krys::boo
{
  EventListener::EventListener(DOMStringAtom type, RefPtr<EventListenerCallback> &&callback,
                               AddEventListenerOptions &&options) noexcept
      : _type(krys::move(type)), _callback(krys::move(callback)), _signal(ShareRefPtr(options.Signal)),
        _capture(options.Capture), _passive(options.Passive), _once(options.Once)
  {
  }

  EventListener::~EventListener() noexcept = default;
}
