#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Enums/EventPhaseType.hpp"
#include "Krystal.Booey/DOM/Internals/Event/EventPathItem.hpp"
#include "Krystal.Booey/DOM/Types/DOMHighResTimeStamp.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class EventTarget;

  /// @see https://dom.spec.whatwg.org/#event
  class Event : public RefCounted<Event>
  {
    friend class Document;
    friend class EventDispatcher;
    friend class EventFactory;
    friend class ShadowRoot;
    friend class EventTarget;
    friend class MutationObserverAlgorithms;

    using dictionary_type = EventInit;

  protected:
    Event() noexcept = default;

  protected:
    DOMStringAtom _type;
    RefPtr<EventTarget> _target {nullptr};
    RefPtr<EventTarget> _currentTarget {nullptr};
    DOMHighResTimeStamp _timeStamp {};
    List<EventPathItem> _path;
    EventPhaseType _eventPhase : BitCount<EventPhaseType>() {EventPhaseType::NONE};
    bool _isTrusted : 1 {false};
    bool _bubbles : 1 {false};
    bool _cancellable : 1 {false};
    bool _stopPropagation : 1 {false};
    bool _stopImmediatePropagation : 1 {false};
    bool _canceled : 1 {false};
    bool _inPassiveListener : 1 {false};
    bool _composed : 1 {false};
    bool _initialized : 1 {false};
    bool _dispatched : 1 {false};
    bool _defaultHandled : 1 {false};
    bool _isDefaultEventHandlerIgnored : 1 {false};
    RefPtr<Event> _underlyingEvent;

  public:
    virtual ~Event() noexcept = default;

#pragma region Event - https://dom.spec.whatwg.org/#event

    /// @see https://dom.spec.whatwg.org/#dom-event-event
    Event(DOMStringAtom type, const EventInit &eventInitDict = {}) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-event-type
    KRYS_NODISCARD DOMStringAtom Type() const noexcept
    {
      return _type;
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-target
    KRYS_NODISCARD RefPtr<EventTarget> Target() const noexcept
    {
      return _target;
    }

    /// @note Legacy.
    /// @see https://dom.spec.whatwg.org/#dom-event-srcelement
    KRYS_NODISCARD RefPtr<EventTarget> SrcElement() const noexcept
    {
      return _target;
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-currenttarget
    KRYS_NODISCARD RefPtr<EventTarget> CurrentTarget() const noexcept
    {
      return _currentTarget;
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-composedpath
    KRYS_NODISCARD List<Ref<EventTarget>> ComposedPath() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-event-eventphase
    KRYS_NODISCARD EventPhaseType EventPhase() const noexcept
    {
      return _eventPhase;
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-stoppropagation
    void StopPropagation() noexcept
    {
      _stopPropagation = true;
    }

    /// @note Legacy.
    /// @see https://dom.spec.whatwg.org/#dom-event-cancelbubble
    KRYS_NODISCARD bool CancelBubble() const noexcept
    {
      return _stopPropagation;
    }

    /// @note Legacy.
    /// @see https://dom.spec.whatwg.org/#dom-event-cancelbubble
    void CancelBubble(bool cancel) noexcept
    {
      if (cancel)
      {
        StopPropagation();
      }
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-stopimmediatepropagation
    void StopImmediatePropagation() noexcept
    {
      _stopPropagation = true;
      _stopImmediatePropagation = true;
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-bubbles
    KRYS_NODISCARD bool Bubbles() const noexcept
    {
      return _bubbles;
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-cancelable
    KRYS_NODISCARD bool Cancelable() const noexcept
    {
      return _cancellable;
    }

    /// @note Legacy.
    /// @see https://dom.spec.whatwg.org/#dom-event-returnvalue
    KRYS_NODISCARD bool ReturnValue() const noexcept
    {
      return DefaultPrevented();
    }

    /// @note Legacy.
    /// @see https://dom.spec.whatwg.org/#dom-event-returnvalue
    void ReturnValue(bool returnValue) noexcept
    {
      if (!returnValue)
      {
        SetCanceledFlag(*this);
      }
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-preventdefault
    void PreventDefault() noexcept
    {
      SetCanceledFlag(*this);
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-defaultprevented
    KRYS_NODISCARD bool DefaultPrevented() const noexcept
    {
      return _canceled;
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-composed
    KRYS_NODISCARD bool Composed() const noexcept
    {
      return _composed;
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-istrusted
    KRYS_NODISCARD bool IsTrusted() const noexcept
    {
      return _isTrusted;
    }

    /// @see https://dom.spec.whatwg.org/#dom-event-timestamp
    KRYS_NODISCARD DOMHighResTimeStamp TimeStamp() const noexcept
    {
      return _timeStamp;
    }

    /// @note Legacy.
    /// @see https://dom.spec.whatwg.org/#dom-event-initevent
    void InitEvent(DOMStringAtom type, bool bubbles = false, bool cancelable = false) noexcept;

#pragma endregion

    /// @brief A trusted event of type @p type, for the user agent to dispatch.
    KRYS_NODISCARD static Ref<Event> Create(DOMStringAtom type, bool canBubble, bool isCancelable,
                                            bool isComposed = false) noexcept;

    /// @brief Whether the event is being dispatched (its dispatch flag).
    KRYS_NODISCARD bool IsBeingDispatched() const noexcept
    {
      return _dispatched;
    }

    /// @brief Whether a node's default event handler has handled the event, so no more of them run.
    KRYS_NODISCARD bool DefaultHandled() const noexcept
    {
      return _defaultHandled;
    }

    void SetDefaultHandled() noexcept
    {
      _defaultHandled = true;
    }

    /// @brief Whether no default event handler runs for the event (a key an input method handled).
    KRYS_NODISCARD bool IsDefaultEventHandlerIgnored() const noexcept
    {
      return _isDefaultEventHandlerIgnored;
    }

    void SetIsDefaultEventHandlerIgnored() noexcept
    {
      _isDefaultEventHandlerIgnored = true;
    }

    /// @brief The event this one was made for, such as the keypress a simulated click stands in for.
    KRYS_NODISCARD Event *UnderlyingEvent() const noexcept
    {
      return _underlyingEvent.get();
    }

    void SetUnderlyingEvent(Event *underlyingEvent) noexcept;

#pragma region Event - https://dom.spec.whatwg.org/#event

  protected:
    /// @see https://dom.spec.whatwg.org/#event-relatedtarget
    virtual RefPtr<EventTarget> RelatedTarget() const noexcept
    {
      return nullptr;
    }

    /// @see https://dom.spec.whatwg.org/#event-relatedtarget
    virtual void RelatedTarget(EventTarget *relatedTarget) noexcept
    {
      (void)relatedTarget;
    }

    /// @see https://dom.spec.whatwg.org/#event-touch-target-list
    virtual List<Ref<EventTarget>> TouchTargetList() const noexcept
    {
      return {};
    }

    /// @see https://dom.spec.whatwg.org/#event-touch-target-list
    virtual void TouchTargetList(const List<Ref<EventTarget>> &touchTargets) const noexcept
    {
      (void)touchTargets;
    }

    /// @brief Called when the event's target changes to another target (not when it's cleared).
    virtual void ReceivedTarget() noexcept
    {
    }

#pragma endregion

  public:
    /// @brief Sets the event's target, telling the event if it's a new one.
    void SetTarget(RefPtr<EventTarget> target) noexcept;

#pragma region Type Checks

    KRYS_NODISCARD virtual bool IsMouseEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsKeyboardEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsFocusEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsWheelEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsPointerEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsTouchEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsClipboardEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsInputEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsTextEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsCompositionEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsSubmitEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsFormDataEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsHashChangeEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsMessageEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsErrorEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsStorageEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsPageTransitionEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsPopStateEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsNavigateEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsNavigationCurrentEntryChangeEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsBeforeUnloadEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsBeforeTextInsertedEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsToggleEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsCommandEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsContentVisibilityAutoStateChangeEvent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsGamepadEvent() const noexcept
    {
      return false;
    }

#pragma endregion

  protected:
    /// @see https://dom.spec.whatwg.org/#concept-event-initialize
    static void Initialize(Event &event, DOMStringAtom type, bool bubbles, bool cancelable) noexcept;

    /// @see https://dom.spec.whatwg.org/#set-the-canceled-flag
    static void SetCanceledFlag(Event &event) noexcept
    {
      if (event._cancellable && !event._inPassiveListener)
      {
        event._canceled = true;
      }
    }
  };
}