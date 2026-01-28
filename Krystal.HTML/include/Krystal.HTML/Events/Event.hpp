#pragma once

#include "Krystal.HTML/DOMHighResTimeStamp.hpp"
#include "Krystal.HTML/Events/EventInit.hpp"
#include "Krystal.HTML/Events/EventPhaseType.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/String/StringAtom.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class EventTarget;

  class Event
  {
  private:
    StringAtom _type;
    RawPtr<EventTarget> _target {nullptr};
    RawPtr<EventTarget> _currentTarget {nullptr};
    DOMHighResTimeStamp _timeStamp {};
    EventPhaseType _eventPhase : BitCount<EventPhaseType>() {EventPhaseType::NONE};

    bool _bubbles : 1 {false};
    bool _cancellable : 1 {false};
    bool _composed : 1 {false};

    bool _stopPropagation : 1 {false};
    bool _stopImmediatePropagation : 1 {false};
    bool _cancelled : 1 {false};
    bool _inPassiveListener : 1 {false};
    bool _defaultHandled : 1 {false};
    bool _isTrusted : 1 {false};

  public:
    Event(StringAtom type, const EventInit &eventInitDict = {}) noexcept;

    KRYS_NODISCARD StringAtom GetType() const noexcept
    {
      return _type;
    }

    void SetType(StringAtom type) noexcept
    {
      _type = type;
    }

    KRYS_NODISCARD RawPtr<EventTarget> GetTarget() const noexcept
    {
      return _target;
    }

    KRYS_NODISCARD RawPtr<EventTarget> GetCurrentTarget() const noexcept
    {
      return _currentTarget;
    }

    KRYS_NODISCARD DOMHighResTimeStamp GetTimeStamp() const noexcept
    {
      return _timeStamp;
    }

    KRYS_NODISCARD bool Bubbles() const noexcept
    {
      return _bubbles;
    }

    KRYS_NODISCARD bool Cancellable() const noexcept
    {
      return _cancellable;
    }

    KRYS_NODISCARD bool Composed() const noexcept
    {
      return _composed;
    }

    KRYS_NODISCARD bool IsTrusted() const noexcept
    {
      return _isTrusted;
    }

    void StopPropagation() noexcept
    {
      _stopPropagation = true;
    }

    KRYS_NODISCARD bool IsPropagationStopped() const noexcept
    {
      return _stopPropagation;
    }

    void StopImmediatePropagation() noexcept
    {
      _stopPropagation = true;
      _stopImmediatePropagation = true;
    }

    KRYS_NODISCARD bool IsImmediatePropagationStopped() const noexcept
    {
      return _stopImmediatePropagation;
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

    KRYS_NODISCARD List<EventTarget> ComposedPath() const noexcept;
  };
}