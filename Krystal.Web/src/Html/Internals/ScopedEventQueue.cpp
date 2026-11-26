#include "Krystal.Web/Html/Internals/ScopedEventQueue.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Web/Dom/Algorithms/EventDispatcher.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventTarget.hpp"

namespace krys::web::html
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
    (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
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
      (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    }
  }
}
