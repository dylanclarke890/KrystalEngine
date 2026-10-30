#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/Internals/Event/EventListener.hpp"

namespace krys::boo::dom
{
  EventListenerHandle::EventListenerHandle(EventTarget &target, EventListener &listener) noexcept
      : _target(CreateWeakPtr(&target)), _listener(CreateWeakPtr(&listener))
  {
  }

  EventListenerHandle::EventListenerHandle(EventListenerHandle &&other) noexcept
      : _target(std::exchange(other._target, {})), _listener(std::exchange(other._listener, {}))
  {
  }

  EventListenerHandle &EventListenerHandle::operator=(EventListenerHandle &&other) noexcept
  {
    if (this != &other)
    {
      Remove();
      _target = std::exchange(other._target, {});
      _listener = std::exchange(other._listener, {});
    }
    return *this;
  }

  EventListenerHandle::~EventListenerHandle() noexcept
  {
    Remove();
  }

  void EventListenerHandle::Remove() noexcept
  {
    // Raw pointers: the handle can go with a behaviour that its target is destroying (a custom element's),
    // whose reference count can't be raised again.
    auto *target = _target.get();
    auto *listener = _listener.get();
    _target = nullptr;
    _listener = nullptr;
    if (target && listener && !listener->Removed())
    {
      EventTargetAlgorithms::RemoveEventListener(*target, *listener);
    }
  }

  void EventListenerHandle::Release() noexcept
  {
    _target = nullptr;
    _listener = nullptr;
  }

  bool EventListenerHandle::IsActive() const noexcept
  {
    auto *listener = _listener.get();
    return _target.get() != nullptr && listener != nullptr && !listener->Removed();
  }
}
