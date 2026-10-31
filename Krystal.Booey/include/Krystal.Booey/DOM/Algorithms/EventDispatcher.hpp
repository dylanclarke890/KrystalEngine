#pragma once

#include "Krystal.Booey/DOM/Enums/EventPhaseType.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Internals/EventFactory.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo
{
  class LocalDOMWindow;
}

namespace krys::boo::dom
{
  class EventTarget;
  class EventPathItem;
  class EventListener;

  /// @see https://dom.spec.whatwg.org/#dispatching-events
  class EventDispatcher
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-event-dispatch
    KRYS_NODISCARD static bool DispatchToTarget(Event &event, EventTarget *target,
                                                bool legacyTargetOverrideFlag = false,
                                                bool *legacyOutputDidListenersThrowFlag = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-event-path-append
    static void AppendToEventPath(Event &event, EventTarget &invocationTarget,
                                  EventTarget *shadowAdjustedTarget, EventTarget *relatedTarget,
                                  List<Ref<EventTarget>> &touchTargets, bool slotInClosedTree) noexcept;

    /// @note: The legacyOutputDidListenersThrowFlag is only used by Indexed Database API.
    /// @see https://dom.spec.whatwg.org/#concept-event-listener-invoke
    static void Invoke(EventPathItem &pathItem, Event &event, EventPhaseType phase,
                       bool *legacyOutputDidListenersThrowFlag) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-event-listener-inner-invoke
    static bool InnerInvoke(Event &event, SmallList<Ref<EventListener>> &listeners, EventPhaseType phase,
                            bool invocationTargetInShadowTree,
                            bool *legacyOutputDidListenersThrowFlag) noexcept;

    /// @brief The window whose current event a listener on @p target sets: the target, if it's a window, or
    /// its node document's window.
    KRYS_NODISCARD static LocalDOMWindow *RelevantWindow(EventTarget &target) noexcept;

    /// @brief Fires a trusted @p TEvent named @p e at @p target.
    /// @param attributes The event's other IDL attributes, as the dictionary its constructor takes.
    /// @param legacyTargetOverrideFlag Whether the event is dispatched with the legacy target override flag,
    /// which HTML sets when it fires an event at a window to give it the window's document as its target.
    /// @return False if the event was canceled; otherwise true.
    /// @see https://dom.spec.whatwg.org/#concept-event-fire
    template <DerivedFrom<Event> TEvent = Event>
    static bool FireEvent(DOMStringAtom e, EventTarget &target,
                          const typename TEvent::dictionary_type &attributes = {},
                          bool legacyTargetOverrideFlag = false) noexcept
    {
      // 1. Let recordingTimingAsTask be the result of calling start recording task time if needed given
      // target's relevant settings object.
      // FIXME: Record task timing, once there's Long Animation Frames timing.

      // 2. If eventConstructor is not given, then let eventConstructor be Event. (TEvent defaults to Event.)

      // 3. Let event be the result of creating an event given eventConstructor, in the relevant realm of
      // target.
      // 5. Initialize any other IDL attributes of event as described in the invocation of this algorithm.
      // (The invocation describes them as @p attributes, which create an event takes in place of the default
      // dictionary.)
      Ref<TEvent> event = EventFactory::Create<TEvent>(attributes);

      // 4. Initialize event's type attribute to e.
      event->_type = e;

      // 6. Let result be the result of dispatching event at target, with legacy target override flag set if
      // set.
      bool result = DispatchToTarget(*event, &target, legacyTargetOverrideFlag);

      // 7. If recordingTimingAsTask is true:
      //   7a. Assert: target's relevant settings object is a Document.
      //   7b. Record task end time given the unsafe shared current time and target's relevant settings
      //   object.
      // (See step 1.)

      // 8. Return result.
      return result;
    }
  };
}