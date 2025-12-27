#pragma once

#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Events/Event.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/Types/Queue.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys
{
  /// @brief Provides basic event queuing and dispatch functionality.
  class EventManager
  {
    Queue<Unique<Event>> _pendingEvents;
    Queue<Unique<Event>> _dispatchEvents;

    /// @brief Registered handlers for events.
    Map<EventType, List<Func<bool(const Event &)>>> _listeners;

  public:
    NO_COPY(EventManager)

    /// @brief Constructs an `EventManager`.
    EventManager() noexcept = default;

    /// @brief Add an event to the queue.
    /// @param event The event to add.
    void Enqueue(Unique<Event> event) noexcept;

    /// @brief Processes all queued events.
    void DispatchAll() noexcept;

    /// @brief Register an event handler for `TEvent`. The event handler must return true or false depending
    /// on whether the event should propagate to other handlers.
    /// @attention Be careful with adding event handlers that themselves dispatch events.
    template <DerivedFrom<Event> TEvent, Predicate<const TEvent &> F>
    void On(F handler) noexcept
    {
      EventType type = TEvent::GetStaticType();
      _listeners[type].push_back([handler = std::move(handler)](const Event &event) -> bool
                                 { return handler(static_cast<const TEvent &>(event)); });
    }

  private:
    /// @brief Dispatches a single event to all registered handlers for the given event's type.
    /// @param event The event to dispatch.
    void DispatchSingle(const Event &event) const noexcept;
  };
}