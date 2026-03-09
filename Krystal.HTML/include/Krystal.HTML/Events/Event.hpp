#pragma once

#include "Krystal.HTML/DOMHighResTimeStamp.hpp"
#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Events/EventInit.hpp"
#include "Krystal.HTML/Events/EventPath.hpp"
#include "Krystal.HTML/Events/EventPhaseType.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class EventTarget;

  class Event : public RefCounted<Event>
  {
  protected:
    DOMStringAtom _type;
    RefPtr<EventTarget> _target {nullptr};
    RefPtr<EventTarget> _currentTarget {nullptr};
    DOMHighResTimeStamp _timeStamp {};
    RefPtr<EventPath> _path;
    EventPhaseType _eventPhase : BitCount<EventPhaseType>() {EventPhaseType::NONE};

    bool _bubbles : 1 {false};
    bool _cancellable : 1 {false};
    bool _composed : 1 {false};

    bool _stopPropagation : 1 {false};
    bool _stopImmediatePropagation : 1 {false};
    bool _cancelled : 1 {false};
    bool _inPassiveListener : 1 {false};
    bool _dispatched : 1 {false};
    bool _isTrusted : 1 {false};

    friend class EventTarget;

  public:
    virtual ~Event() noexcept = default;

#pragma region Event - https://dom.spec.whatwg.org/#event

    Event(DOMStringAtom type, const EventInit &eventInitDict = {}) noexcept;

    KRYS_NODISCARD DOMStringAtom Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD RawPtr<EventTarget> Target() const noexcept
    {
      return _target.get();
    }

    KRYS_NODISCARD RawPtr<EventTarget> SrcElement() const noexcept // legacy
    {
      return _target.get();
    }

    KRYS_NODISCARD RawPtr<EventTarget> CurrentTarget() const noexcept
    {
      return _currentTarget.get();
    }

    KRYS_NODISCARD List<Ref<EventTarget>> ComposedPath() const noexcept;

    KRYS_NODISCARD EventPhaseType EventPhase() const noexcept
    {
      return _eventPhase;
    }

    void StopPropagation() noexcept
    {
      _stopPropagation = true;
    }

    KRYS_NODISCARD bool CancelBubble() const noexcept // legacy
    {
      return _stopPropagation;
    }

    void CancelBubble(bool cancel) noexcept // legacy
    {
      if (cancel)
      {
        StopPropagation();
      }
    }

    void StopImmediatePropagation() noexcept
    {
      _stopPropagation = true;
      _stopImmediatePropagation = true;
    }

    KRYS_NODISCARD bool Bubbles() const noexcept
    {
      return _bubbles;
    }

    KRYS_NODISCARD bool Cancellable() const noexcept
    {
      return _cancellable;
    }

    KRYS_NODISCARD bool ReturnValue() const noexcept // legacy
    {
      return DefaultPrevented();
    }

    void ReturnValue(bool returnValue) noexcept // legacy
    {
      if (!returnValue)
      {
        PreventDefault();
      }
    }

    void PreventDefault() noexcept
    {
      /// @see https://dom.spec.whatwg.org/#set-the-canceled-flag
      if (_cancellable && !_inPassiveListener)
      {
        _cancelled = true;
      }
    }

    KRYS_NODISCARD bool DefaultPrevented() const noexcept
    {
      return _cancelled;
    }

    KRYS_NODISCARD bool Composed() const noexcept
    {
      return _composed;
    }

    KRYS_NODISCARD bool IsTrusted() const noexcept
    {
      return _isTrusted;
    }

    KRYS_NODISCARD DOMHighResTimeStamp TimeStamp() const noexcept
    {
      return _timeStamp;
    }

    void InitEvent(DOMStringAtom type, bool bubbles = false, bool cancelable = false) noexcept; // legacy

    virtual RawPtr<EventTarget> RelatedTarget() const noexcept
    {
      return nullptr;
    }

    virtual void RelatedTarget(RefPtr<EventTarget> &&relatedTarget) noexcept
    {
      (void)relatedTarget;
    }

    virtual List<RefPtr<EventTarget>> TouchTargetList() const noexcept
    {
      return {};
    }

#pragma endregion

    void SetEventPath(RefPtr<EventPath> &&path) noexcept
    {
      _path = std::move(path);
    }

    KRYS_NODISCARD bool IsImmediatePropagationStopped() const noexcept
    {
      return _stopImmediatePropagation;
    }
  };
}