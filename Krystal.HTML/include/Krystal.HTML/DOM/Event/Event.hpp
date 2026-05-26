#pragma once

#include "Krystal.HTML/DOM/Dicts/EventInit.hpp"
#include "Krystal.HTML/DOM/Enums/EventPhaseType.hpp"
#include "Krystal.HTML/DOM/Event/EventPathItem.hpp"
#include "Krystal.HTML/Types/DOMHighResTimeStamp.hpp"
#include "Krystal.HTML/Types/DOMStringAtom.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class EventTarget;

  /// @see https://dom.spec.whatwg.org/#event
  class Event : public RefCounted<Event>
  {
    friend class EventDispatcher;
    friend class EventFactory;
    friend class ShadowRoot;
    friend class EventTarget;

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

  protected:
    /// @see https://dom.spec.whatwg.org/#event-relatedtarget
    virtual RefPtr<EventTarget> RelatedTarget() const noexcept
    {
      return nullptr;
    }

    /// @see https://dom.spec.whatwg.org/#event-relatedtarget
    virtual void RelatedTarget(RawPtr<EventTarget> relatedTarget) noexcept
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

#pragma endregion

#pragma region Type Checks

    KRYS_NODISCARD bool IsMouseEvent() const noexcept
    {
      return false;
    }

#pragma endregion

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