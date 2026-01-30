#pragma once

#include "Krystal.HTML/Events/EventListener.hpp"
#include "Krystal.HTML/Events/EventListenerOptions.hpp"
#include "Krystal.HTML/Events/AddEventListenerOptions.hpp"
#include "Krystal.Lib/Pointers/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/String/StringAtom.hpp"

namespace Krys::HTML
{
  class EventTarget : public WeakRefCounted<EventTarget>
  {
  public:
    EventTarget() noexcept;

    virtual bool AddEventListener(StringAtom type, IntrusiveRefPtr<EventListener> &&callback,
                                  const AddEventListenerOptions &options) noexcept;

    virtual bool RemoveEventListener(StringAtom type, IntrusiveRefPtr<EventListener> &&callback,
                                     const EventListenerOptions &options) noexcept;

    virtual bool DispatchEvent(Event &event) noexcept;
  };
}