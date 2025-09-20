#include "Krystal.Lib/Events/EventManager.hpp"
#include "Krystal.Lib/Events/Event.hpp"

namespace Krys
{
  void EventManager::Enqueue(Unique<Event> event) noexcept
  {
    _pendingEvents.emplace(std::move(event));
  }

  void EventManager::DispatchAll() noexcept
  {
    std::swap(_dispatchEvents, _pendingEvents);
    while (!_dispatchEvents.empty())
    {
      Unique<Event> event = std::move(_dispatchEvents.front());
      DispatchSingle(*event);
      _dispatchEvents.pop();
    }
  }

  void EventManager::DispatchSingle(const Event &event) const noexcept
  {
    auto it = _listeners.find(event.GetEventType());
    if (it != _listeners.end())
    {
      for (const auto &func : it->second)
      {
        const bool handled = func(event);
        if (handled)
          break;
      }
    }
  }
}
