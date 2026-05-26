#pragma once

#include "Krystal.HTML/DOM/Dicts/AddEventListenerOptions.hpp"
#include "Krystal.HTML/DOM/Event/EventListenerCallback.hpp"
#include "Krystal.HTML/Types/DOMStringAtom.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class AbortSignal;

  /// @see https://dom.spec.whatwg.org/#concept-event-listener
  class EventListener : public RefCounted<EventListener>, public CanMakeWeakPtr<EventListener>
  {
    friend class EventTargetAlgorithms;

  private:
    const DOMStringAtom _type;
    const RefPtr<EventListenerCallback> _callback;
    RefPtr<AbortSignal> _signal {nullptr};
    bool _removed {false};
    bool _capture {false};
    Maybe<bool> _passive;
    bool _once {false};

  public:
    EventListener(const DOMStringAtom type, RefPtr<EventListenerCallback> &&callback,
                  AddEventListenerOptions &&options) noexcept
        : _type(type), _callback(Krys::Move(callback)), _signal(Krys::Move(options.Signal)), _removed(false),
          _capture(options.Capture), _passive(options.Passive), _once(options.Once)
    {
    }

    /// @see https://dom.spec.whatwg.org/#event-listener-type
    KRYS_NODISCARD DOMStringAtom Type() const noexcept
    {
      return _type;
    }

    /// @see https://dom.spec.whatwg.org/#event-listener-callback
    KRYS_NODISCARD RawPtr<EventListenerCallback> Callback() const noexcept
    {
      return _callback.get();
    }

    /// @see https://dom.spec.whatwg.org/#event-listener-capture
    KRYS_NODISCARD bool Capture() const noexcept
    {
      return _capture;
    }

    /// @see https://dom.spec.whatwg.org/#event-listener-passive
    KRYS_NODISCARD Maybe<bool> Passive() const noexcept
    {
      return _passive;
    }

    /// @see https://dom.spec.whatwg.org/#event-listener-once
    KRYS_NODISCARD bool Once() const noexcept
    {
      return _once;
    }

    /// @see https://dom.spec.whatwg.org/#event-listener-signal
    KRYS_NODISCARD RawPtr<AbortSignal> Signal() const noexcept
    {
      return _signal.get();
    }

    /// @see https://dom.spec.whatwg.org/#event-listener-removed
    KRYS_NODISCARD bool Removed() const noexcept
    {
      return _removed;
    }
  };
}