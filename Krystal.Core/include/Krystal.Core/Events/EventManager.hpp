#pragma once

#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Events/Event.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Queue.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys
{
  /// @brief Provides basic event queuing and dispatch functionality.
  class EventManager
  {
    KRYS_NON_COPYABLE(EventManager);

  private:
    Queue<UniquePtr<Event>> _pendingEvents;
    Queue<UniquePtr<Event>> _dispatchEvents;

    /// @brief Registered handlers for events.
    HashMap<EventType, List<Func<bool(const Event &)>>> _listeners;

  public:
    /// @brief Constructs an `EventManager`.
    EventManager() noexcept = default;

    /// @brief Add an event to the queue.
    /// @param event The event to add.
    void Enqueue(UniquePtr<Event> event) noexcept;

    /// @brief Processes all queued events.
    void DispatchAll() noexcept;

    /// @brief Register an event handler for `TEvent`. The event handler must return true or false depending
    /// on whether the event should propagate to other handlers.
    /// @attention Be careful with adding event handlers that themselves dispatch events.
    template <DerivedFrom<Event> TEvent, Predicate<const TEvent &> F>
    void On(F handler) noexcept
    {
      EventType type = TEvent::GetStaticType();
      _listeners[type].push_back([handler = krys::move(handler)](const Event &event) -> bool
                                 { return handler(static_cast<const TEvent &>(event)); });
    }

  private:
    /// @brief Dispatches a single event to all registered handlers for the given event's type.
    /// @param event The event to dispatch.
    void DispatchSingle(const Event &event) const noexcept;
  };
}