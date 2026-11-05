#pragma once

#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Event;
  class EventTarget;

  /// @brief Holds back the events dispatched while an `EventQueueScope` is alive, and dispatches them in
  /// order when the outermost scope ends: an editing command's `input` events fire once its changes are made,
  /// not from the middle of them.
  struct ScopedEventQueue
  {
    /// @brief Dispatches @p event to @p target now, or once the outermost `EventQueueScope` ends if one is
    /// alive.
    static void DispatchOrQueue(Ref<Event> event, Ref<EventTarget> target) noexcept;
  };

  /// @brief Queues the events given to `ScopedEventQueue::DispatchOrQueue` while it's alive; the outermost
  /// scope dispatches them when it ends.
  class EventQueueScope
  {
    KRYS_NON_COPY_MOVABLE(EventQueueScope);

  public:
    EventQueueScope() noexcept;
    ~EventQueueScope() noexcept;
  };
}
