#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Events/EventListener.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class AbortSignal;

  /// @see https://dom.spec.whatwg.org/#concept-event-listener
  class RegisteredEventListener : public RefCounted<RegisteredEventListener>,
                                  public CanMakeWeakPtr<RegisteredEventListener>
  {
  private:
    const DOMStringAtom _type;
    const RefPtr<EventListener> _callback;
    RefPtr<AbortSignal> _signal {nullptr};
    bool _removed {false};
    bool _capture {false};
    Maybe<bool> _passive;
    bool _once {false};

  public:
    RegisteredEventListener(const DOMStringAtom type, AddEventListenerOptions &&options,
                            RefPtr<EventListener> &&callback) noexcept
        : _type(type), _callback(Krys::Move(callback)), _signal(Krys::Move(options.Signal)), _removed(false),
          _capture(options.Capture), _passive(options.Passive), _once(options.Once)
    {
    }

    KRYS_NODISCARD DOMStringAtom Type() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD RawPtr<EventListener> Callback() const noexcept
    {
      return _callback.get();
    }

    KRYS_NODISCARD RawPtr<AbortSignal> Signal() const noexcept
    {
      return _signal.get();
    }

    KRYS_NODISCARD bool Capture() const noexcept
    {
      return _capture;
    }

    KRYS_NODISCARD Maybe<bool> Passive() const noexcept
    {
      return _passive;
    }

    void Passive(bool passive) noexcept
    {
      _passive = passive;
    }

    KRYS_NODISCARD bool Once() const noexcept
    {
      return _once;
    }

    void Remove() noexcept
    {
      _removed = true;
    }

    KRYS_NODISCARD bool Removed() const noexcept
    {
      return _removed;
    }
  };
}