#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Events/EventPhaseType.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class Event;
  class EventTarget;
  class EventPathItem;
  class RegisteredEventListener;

  using EventConstructor = Func<Ref<Event>()>;
  using EventInitializer = Func<void(Event &)>;

  class EventDispatcher
  {
  public:
    EventDispatcher() = delete;
    ~EventDispatcher() = delete;

    /// @see https://dom.spec.whatwg.org/#concept-event-dispatch
    KRYS_NODISCARD static bool DispatchToTarget(Event &event, EventTarget *target,
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
    static bool FireEvent(DOMStringAtom e, EventTarget &target, EventConstructor eventConstructor = nullptr,
                          EventInitializer idlAttributeInitializer = nullptr,
                          bool legacyTargetOverrideFlag = false) noexcept;
  };
}