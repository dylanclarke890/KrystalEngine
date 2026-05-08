#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.HTML/Events/EventFactory.hpp"
#include "Krystal.HTML/Events/EventPhaseType.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class EventTarget;
  class EventPathItem;
  class RegisteredEventListener;

  template <DerivedFrom<Event> TEvent>
  using EventIDLInitializer = Func<void(TEvent &)>;

  class EventDispatcher
  {
  public:
    EventDispatcher() = delete;
    ~EventDispatcher() = delete;

    /// @see https://dom.spec.whatwg.org/#concept-event-dispatch
    KRYS_NODISCARD static bool DispatchToTarget(Event &event, RawPtr<EventTarget> target,
                                                bool legacyTargetOverrideFlag = false,
                                                bool legacyOutputDidListenersThrowFlag = false) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-event-path-append
    static void AppendToEventPath(Event &event, EventTarget &invocationTarget,
                                  RawPtr<EventTarget> shadowAdjustedTarget, EventTarget &relatedTarget,
                                  List<Ref<EventTarget>> &touchTargets, bool slotInClosedTree) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-event-listener-invoke
    static void Invoke(EventPathItem &pathStruct, Event &event, EventPhaseType phase,
                       bool legacyOutputDidListenersThrowFlag) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-event-listener-inner-invoke
    static bool InnerInvoke(Event &event, SmallList<Ref<RegisteredEventListener>> &listeners,
                            EventPhaseType phase, bool invocationTargetInShadowTree,
                            bool &legacyOutputDidListenersThrowFlag) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-event-fire
    template <DerivedFrom<Event> TEvent = Event>
    static bool FireEvent(DOMStringAtom e, EventTarget &target,
                          EventIDLInitializer<TEvent> idlAttributeInitializer = nullptr,
                          bool legacyTargetOverrideFlag = false) noexcept
    {
      // SPEC-VIOLATION(HTML): realms and global objects currently not supported.
      // Let event be the result of creating an event given eventConstructor, in the relevant realm of target.
      Ref<TEvent> event = EventFactory::Create<TEvent>();
      event->_type = e;

      if (idlAttributeInitializer)
      {
        idlAttributeInitializer(*event);
      }

      return DispatchToTarget(*event, &target, legacyTargetOverrideFlag);
    }
  };
}