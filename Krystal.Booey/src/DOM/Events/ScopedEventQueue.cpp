#include "Krystal.Booey/DOM/Events/ScopedEventQueue.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventTarget.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::dom
{
  namespace
  {
    struct ScopedEventQueueState
    {
      List<Pair<Ref<Event>, Ref<EventTarget>>> Queued;
      uint32 ScopingLevel {0};
    };

    KRYS_NODISCARD ScopedEventQueueState &ScopedEventQueueStateInstance() noexcept
    {
      static ScopedEventQueueState state;
      return state;
    }
  }

  void ScopedEventQueue::DispatchOrQueue(Ref<Event> event, Ref<EventTarget> target) noexcept
  {
    auto &state = ScopedEventQueueStateInstance();
    if (state.ScopingLevel)
    {
      state.Queued.emplace_back(krys::move(event), krys::move(target));
      return;
    }
    (void)EventDispatcher::DispatchToTarget(*event, target.get());
  }

  EventQueueScope::EventQueueScope() noexcept
  {
    ++ScopedEventQueueStateInstance().ScopingLevel;
  }

  EventQueueScope::~EventQueueScope() noexcept
  {
    auto &state = ScopedEventQueueStateInstance();
    krys_debug_assert(state.ScopingLevel);
    if (--state.ScopingLevel)
    {
      return;
    }

    // Events dispatched by the listeners of these go out straight away, as no scope is alive.
    auto queued = krys::move(state.Queued);
    state.Queued.clear();
    for (auto &[event, target] : queued)
    {
      (void)EventDispatcher::DispatchToTarget(*event, target.get());
    }
  }
}
