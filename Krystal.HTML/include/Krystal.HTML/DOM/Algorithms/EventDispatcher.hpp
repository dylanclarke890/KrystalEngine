#pragma once

#include "Krystal.HTML/DOM/Enums/EventPhaseType.hpp"
#include "Krystal.HTML/DOM/Event/Event.hpp"
#include "Krystal.HTML/Factories/EventFactory.hpp"
#include "Krystal.HTML/Types/DOMStringAtom.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class EventTarget;
  class EventPathItem;
  class EventListener;

  template <DerivedFrom<Event> TEvent>
  using EventIDLInitializer = Func<void(TEvent &)>;

  /// @see https://dom.spec.whatwg.org/#dispatching-events
  class EventDispatcher
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-event-dispatch
    KRYS_NODISCARD static bool
      DispatchToTarget(Event &event, RawPtr<EventTarget> target, bool legacyTargetOverrideFlag = false,
                       RawPtr<bool> legacyOutputDidListenersThrowFlag = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-event-path-append
    static void AppendToEventPath(Event &event, EventTarget &invocationTarget,
                                  RawPtr<EventTarget> shadowAdjustedTarget, RawPtr<EventTarget> relatedTarget,
                                  List<Ref<EventTarget>> &touchTargets, bool slotInClosedTree) noexcept;

    /// @note: The legacyOutputDidListenersThrowFlag is only used by Indexed Database API.
    /// @see https://dom.spec.whatwg.org/#concept-event-listener-invoke
    static void Invoke(EventPathItem &pathStruct, Event &event, EventPhaseType phase,
                       RawPtr<bool> legacyOutputDidListenersThrowFlag) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-event-listener-inner-invoke
    static bool InnerInvoke(Event &event, SmallList<Ref<EventListener>> &listeners, EventPhaseType phase,
                            bool invocationTargetInShadowTree,
                            RawPtr<bool> legacyOutputDidListenersThrowFlag) noexcept;

    /// @brief Fires an event named `e` at `target`. Returns false if the event is
    /// cancelable and at least one of the event handlers which received the event called `PreventDefault()`,
    /// otherwise returns true.
    /// @tparam TEvent The type of event to fire, must derive from `Event`. Defaults to `Event`.
    /// @param e The type of the event to fire.
    /// @param target The target to fire the event on.
    /// @param idlAttributeInitializer An optional callback which initializes the event's IDL attributes. The
    /// event is passed as an argument to the callback, and the callback is expected to modify the event's
    /// attributes directly.
    /// @param legacyTargetOverrideFlag If true, the event will be dispatched with the legacy target override
    /// flag set.
    /// @note `legacyTargetOverrideFlag` is only used by HTML when firing events on Window objects, and causes
    /// the event to be dispatched to the Document associated with the Window instead of the Window itself.
    /// @see https://dom.spec.whatwg.org/#concept-event-fire
    template <DerivedFrom<Event> TEvent = Event>
    static bool FireEvent(DOMStringAtom e, EventTarget &target,
                          EventIDLInitializer<TEvent> idlAttributeInitializer = nullptr,
                          bool legacyTargetOverrideFlag = false) noexcept
    {
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